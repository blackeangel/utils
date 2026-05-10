#include "archive_rw.hpp"
#include "android_ids.hpp"
#include "metadata.hpp"

#include <archive_entry.h>      // AE_IFREG / AE_IFDIR / AE_IFLNK constants
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// extract_tar
// ---------------------------------------------------------------------------
void extract_tar(const std::string& tar_path, const fs::path& out_dir) {

    fs::create_directories(out_dir);

    // Start with a fresh metadata file
    fs::path meta_path = out_dir / "_metadata.txt";
    fs::remove(meta_path);

    ArchiveReader reader(tar_path);
    ArchiveEntry  entry;

    std::vector<MetadataEntry> meta_entries;

    while (reader.next(entry)) {

        if (entry.name.empty()) continue;

        fs::path dest = out_dir / entry.name;

        // ------------------------------------------------------------------
        // Metadata record
        // ------------------------------------------------------------------
        MetadataEntry me;
        me.path         = entry.name;
        me.uid          = entry.uid;
        me.gid          = entry.gid;
        me.mode         = entry.mode;
        me.selinux      = entry.selinux_ctx;
        me.capabilities = entry.cap_hex;
        me.symlink      = (entry.type == AE_IFLNK) ? entry.linkname : "";

        // ------------------------------------------------------------------
        // Extract to disk
        // ------------------------------------------------------------------
        if (entry.type == AE_IFDIR) {
            fs::create_directories(dest);
        }
        else if (entry.type == AE_IFLNK) {
            // Materialise symlinks as empty regular files (safe on Windows,
            // no special privileges needed anywhere)
            fs::create_directories(dest.parent_path());
            std::ofstream ofs(dest, std::ios::binary | std::ios::trunc);
            if (!ofs)
                fprintf(stderr, "Warning: cannot create placeholder for symlink '%s'\n",
                        dest.string().c_str());
        }
        else if (entry.type == AE_IFREG) {
            fs::create_directories(dest.parent_path());
            std::ofstream ofs(dest, std::ios::binary | std::ios::trunc);
            if (!ofs)
                throw std::runtime_error(std::format("Cannot write '{}'", dest.string()));
            if (!entry.data.empty())
                ofs.write(reinterpret_cast<const char*>(entry.data.data()),
                          (std::streamsize)entry.data.size());
        }
        else {
            // Hard-links, devices, FIFOs – record metadata, skip data
        }

        meta_entries.push_back(std::move(me));
        printf("  extracted: %s\n", entry.name.c_str());
    }

    save_metadata(meta_path, meta_entries);

    printf("\nDone. Metadata written to: %s\n", meta_path.string().c_str());
    printf("Total entries: %zu\n", meta_entries.size());
}
