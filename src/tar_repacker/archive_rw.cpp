#include "archive_rw.hpp"
#include "pax_helpers.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <algorithm>
#include <cstring>
#include "format_compat.hpp"
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void la_check(int rc, archive* a, const char* ctx) {
    if (rc != ARCHIVE_OK && rc != ARCHIVE_WARN) {
        const char* msg = archive_error_string(a);
        throw std::runtime_error(std::format("{}: {}", ctx,
                                             msg ? msg : "unknown error"));
    }
}

// ---------------------------------------------------------------------------
// ArchiveReader
// ---------------------------------------------------------------------------

ArchiveReader::ArchiveReader(const std::string& path) {
    arc_ = archive_read_new();
    if (!arc_) throw std::runtime_error("archive_read_new failed");

    // Enable ALL decompression filters (gz, bz2, xz, zst, lz4, lzo, …)
    archive_read_support_filter_all(arc_);
    // Enable ALL archive formats: TAR, ZIP, 7-Zip, RAR/RAR5, ISO9660,
    // CAB, CPIO, XAR, LZH/LHA, AR, mtree, warc, …
    archive_read_support_format_all(arc_);

    int rc = archive_read_open_filename(arc_, path.c_str(), 65536);
    la_check(rc, arc_, std::format("open '{}'", path).c_str());
}

ArchiveReader::~ArchiveReader() {
    if (arc_) archive_read_free(arc_);
}

// ---------------------------------------------------------------------------
bool ArchiveReader::next(ArchiveEntry& out) {
    int rc = archive_read_next_header(arc_, &cur_);
    if (rc == ARCHIVE_EOF) return false;
    la_check(rc, arc_, "archive_read_next_header");

    out = ArchiveEntry{};

    // ---- Basic POSIX fields -----------------------------------------------
    out.name     = archive_entry_pathname(cur_) ? archive_entry_pathname(cur_) : "";
    out.linkname = archive_entry_symlink(cur_)  ? archive_entry_symlink(cur_)  : "";
    out.type     = (unsigned int)archive_entry_filetype(cur_);
    out.uid      = (uint32_t)archive_entry_uid(cur_);
    out.gid      = (uint32_t)archive_entry_gid(cur_);
    out.mode     = (uint32_t)archive_entry_perm(cur_);   // permission bits only
    out.mtime    = (int64_t) archive_entry_mtime(cur_);
    out.uname    = archive_entry_uname(cur_) ? archive_entry_uname(cur_) : "";
    out.gname    = archive_entry_gname(cur_) ? archive_entry_gname(cur_) : "";

    // Strip leading '/' so all paths are relative
    while (!out.name.empty() && out.name.front() == '/')
        out.name.erase(0, 1);
    // Strip trailing '/' from directory names
    if (!out.name.empty() && out.name.back() == '/')
        out.name.pop_back();

    // ---- xattrs (SCHILY.xattr.* PAX fields) --------------------------------
    read_xattrs(out);

    // ---- File data ----------------------------------------------------------
    if (archive_entry_size_is_set(cur_) && archive_entry_size(cur_) > 0) {
        out.data.resize((size_t)archive_entry_size(cur_));
        ssize_t got = archive_read_data(arc_, out.data.data(), out.data.size());
        if (got < 0)
            throw std::runtime_error(std::format("archive_read_data for '{}': {}",
                out.name, archive_error_string(arc_) ? archive_error_string(arc_) : ""));
        out.data.resize((size_t)got);
    }

    return true;
}

// ---------------------------------------------------------------------------
void ArchiveReader::read_xattrs(ArchiveEntry& e) const {
    archive_entry_xattr_reset(cur_);

    const char*  xname  = nullptr;
    const void*  xval   = nullptr;
    size_t       xsize  = 0;

    while (archive_entry_xattr_next(cur_, &xname, &xval, &xsize) == ARCHIVE_OK) {
        if (!xname) continue;

        // SELinux context
        if (strcmp(xname, pax::XATTR_SELINUX.data()) == 0) {
            // Value is a null-terminated string; strip the null if present
            size_t len = xsize;
            if (len > 0 && static_cast<const char*>(xval)[len - 1] == '\0')
                --len;
            e.selinux_ctx.assign(static_cast<const char*>(xval), len);
            continue;
        }

        // Linux capabilities (20-byte binary blob)
        if (strcmp(xname, pax::XATTR_CAPS.data()) == 0) {
            e.cap_hex = pax::cap_bytes_to_hex(xval, xsize);
            continue;
        }
    }

    // ---- RHT.security.selinux note ----------------------------------------
    // Some older Android system.tar files use the PAX key
    // "RHT.security.selinux" (Red Hat / star convention) instead of the
    // POSIX-compliant "SCHILY.xattr.security.selinux".  libarchive 3.x does
    // not expose unrecognised PAX fields via its public API, so we rely on
    // the archive having been produced with a modern tool that uses the
    // SCHILY.xattr.* convention.  Modern AOSP build-tools (mk2fs, e2fsdroid,
    // img2simg) all use SCHILY.xattr.security.selinux.
}

// ---------------------------------------------------------------------------
// ArchiveWriter
// ---------------------------------------------------------------------------

ArchiveWriter::ArchiveWriter(const std::string& path) {
    arc_ = archive_write_new();
    if (!arc_) throw std::runtime_error("archive_write_new failed");

    // PAX interchange format – full PAX header support, unlimited path lengths,
    // arbitrary xattr round-trip via SCHILY.xattr.* encoding
    la_check(archive_write_set_format_pax(arc_),       arc_, "set format pax");

    // No compression – output is a plain .tar; the caller can pipe it through
    // gzip/xz/zstd if desired.
    la_check(archive_write_add_filter_none(arc_),      arc_, "add filter none");

    la_check(archive_write_open_filename(arc_, path.c_str()), arc_,
             std::format("open '{}' for write", path).c_str());
}

ArchiveWriter::~ArchiveWriter() {
    if (!done_) { try { finish(); } catch (...) {} }
    if (arc_)  archive_write_free(arc_);
}

void ArchiveWriter::finish() {
    if (done_) return;
    la_check(archive_write_close(arc_), arc_, "archive_write_close");
    done_ = true;
}

// ---------------------------------------------------------------------------
void ArchiveWriter::write(const ArchiveEntry& entry) {
    archive_entry* e = archive_entry_new();
    if (!e) throw std::runtime_error("archive_entry_new failed");

    // ---- Basic POSIX fields -----------------------------------------------
    std::string path = entry.name;

    // Ensure directory entries end with '/'
    if (entry.type == AE_IFDIR && !path.empty() && path.back() != '/')
        path += '/';

    archive_entry_set_pathname(e, path.c_str());
    archive_entry_set_filetype(e, entry.type);
    archive_entry_set_perm(e,    entry.mode & 07777);
    archive_entry_set_uid(e,     entry.uid);
    archive_entry_set_gid(e,     entry.gid);
    archive_entry_set_mtime(e,   entry.mtime, 0);

    if (!entry.uname.empty())    archive_entry_set_uname(e, entry.uname.c_str());
    if (!entry.gname.empty())    archive_entry_set_gname(e, entry.gname.c_str());
    if (!entry.linkname.empty()) archive_entry_set_symlink(e, entry.linkname.c_str());

    // For regular files, set the size so libarchive writes the size PAX field
    if (entry.type == AE_IFREG)
        archive_entry_set_size(e, (int64_t)entry.data.size());

    // ---- xattrs → SCHILY.xattr.* PAX fields --------------------------------

    // SELinux: written as SCHILY.xattr.security.selinux by libarchive
    if (!entry.selinux_ctx.empty()) {
        archive_entry_xattr_add_entry(e,
            pax::XATTR_SELINUX.data(),
            entry.selinux_ctx.c_str(),
            entry.selinux_ctx.size() + 1); // include null terminator
    }

    // Capabilities: written as SCHILY.xattr.security.capability by libarchive
    if (!entry.cap_hex.empty()) {
        auto blob = pax::cap_hex_to_bytes(entry.cap_hex);
        if (!blob.empty()) {
            archive_entry_xattr_add_entry(e,
                pax::XATTR_CAPS.data(),
                blob.data(),
                blob.size());
        }
    }

    // ---- Write header + data -----------------------------------------------
    int rc = archive_write_header(arc_, e);
    la_check(rc, arc_, std::format("write header '{}'", path).c_str());

    if (entry.type == AE_IFREG && !entry.data.empty()) {
        ssize_t written = archive_write_data(arc_,
                                             entry.data.data(),
                                             entry.data.size());
        if (written < 0 || (size_t)written != entry.data.size())
            throw std::runtime_error(std::format("archive_write_data failed for '{}'", path));
    }

    archive_entry_free(e);
}
