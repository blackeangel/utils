#pragma once
// ---------------------------------------------------------------------------
// archive_rw.hpp  –  C++20 RAII wrapper around libarchive
//
// libarchive handles:
//   • Format detection + parsing  (TAR ustar / GNU / PAX / pax-interchange)
//   • Decompression               (.tar, .tar.gz, .tar.bz2, .tar.xz, .tar.zst)
//   • PAX extended headers        (path, uid, gid, uname, gname, …)
//   • xattr round-trip            (SCHILY.xattr.*)
//
// We add:
//   • RHT.security.selinux  fall-back read (raw PAX scan via pax_helpers.hpp)
//   • _metadata.txt   uid/gid/mode/selinux/capabilities/symlink accounting
// ---------------------------------------------------------------------------

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward-declare libarchive types so callers don't need archive.h
struct archive;
struct archive_entry;

// ---------------------------------------------------------------------------
// ArchiveEntry  –  all metadata + data for one archive member
// ---------------------------------------------------------------------------
struct ArchiveEntry {
    std::string name;           // relative path (no leading '/')
    std::string linkname;       // symlink target (or "")
    unsigned int type = 0;      // AE_IFREG / AE_IFDIR / AE_IFLNK / …

    uint32_t uid  = 0;
    uint32_t gid  = 0;
    uint32_t mode = 0;          // full POSIX mode (type bits + permission bits)
    int64_t  mtime = 0;

    std::string uname;
    std::string gname;

    std::string selinux_ctx;    // SELinux context string, or ""
    std::string cap_hex;        // capability hex string "0x2000000", or ""

    std::vector<uint8_t> data;  // file content (empty for dirs / symlinks)
};

// ---------------------------------------------------------------------------
// ArchiveReader  –  iterate over entries in a (possibly compressed) TAR
// ---------------------------------------------------------------------------
class ArchiveReader {
public:
    explicit ArchiveReader(const std::string& path);
    ~ArchiveReader();

    // Reads the next entry.  Returns false at end-of-archive.
    bool next(ArchiveEntry& out);

    // Call after next() to read the raw bytes for the current entry.
    // (next() already calls this internally and stores data in ArchiveEntry.)

private:
    struct archive*       arc_  = nullptr;
    struct archive_entry* cur_  = nullptr;

    // Read all xattrs from `cur_` and populate selinux_ctx / cap_hex.
    void read_xattrs(ArchiveEntry& e) const;

    // Disallow copy
    ArchiveReader(const ArchiveReader&) = delete;
    ArchiveReader& operator=(const ArchiveReader&) = delete;
};

// ---------------------------------------------------------------------------
// ArchiveWriter  –  write a TAR file with PAX extended headers
// ---------------------------------------------------------------------------
class ArchiveWriter {
public:
    explicit ArchiveWriter(const std::string& path);
    ~ArchiveWriter();

    // Write one entry.  The entry's `data` field is used for regular files.
    void write(const ArchiveEntry& entry);

    // Flush + close (called automatically by destructor if not called yet).
    void finish();

private:
    struct archive* arc_   = nullptr;
    bool            done_  = false;

    ArchiveWriter(const ArchiveWriter&) = delete;
    ArchiveWriter& operator=(const ArchiveWriter&) = delete;
};
