#include "archive_rw.hpp"
#include "android_ids.hpp"
#include "metadata.hpp"

#include <archive_entry.h>      // AE_IFREG / AE_IFDIR / AE_IFLNK

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
static std::vector<uint8_t> read_file_bytes(const fs::path& p) {
    std::ifstream ifs(p, std::ios::binary | std::ios::ate);
    if (!ifs)
        throw std::runtime_error(std::format("Cannot read '{}'", p.string()));
    auto sz = ifs.tellg();
    ifs.seekg(0);
    std::vector<uint8_t> buf((size_t)sz);
    if (sz > 0)
        ifs.read(reinterpret_cast<char*>(buf.data()), sz);
    return buf;
}

// ---------------------------------------------------------------------------
// Collect every path under `root` recursively, normalised as generic POSIX
// strings relative to `root`.  Parent directories are guaranteed to appear
// before their children.
// ---------------------------------------------------------------------------
static std::vector<std::string> collect_paths(const fs::path& root) {
    std::set<std::string>    seen;
    std::vector<std::string> result;

    for (const auto& de : fs::recursive_directory_iterator(
             root, fs::directory_options::skip_permission_denied))
    {
        fs::path rel = fs::relative(de.path(), root);
        std::string rel_str = rel.generic_string();

        if (rel_str == "_metadata.txt") continue;

        // Ensure all parents appear first
        for (fs::path p = rel.parent_path();
             !p.empty() && p != p.parent_path();
             p = p.parent_path())
        {
            std::string ps = p.generic_string();
            if (seen.insert(ps).second)
                result.push_back(ps);
        }

        if (seen.insert(rel_str).second)
            result.push_back(rel_str);
    }

    std::stable_sort(result.begin(), result.end());
    return result;
}

// ---------------------------------------------------------------------------
// pack_tar
// ---------------------------------------------------------------------------
void pack_tar(const fs::path& src_dir, const std::string& tar_path) {

    fs::path meta_path = src_dir / "_metadata.txt";
    if (!fs::exists(meta_path))
        throw std::runtime_error(
            std::format("Metadata file not found: {}", meta_path.string()));

    MetadataMap meta  = load_metadata(meta_path);
    auto        paths = collect_paths(src_dir);

    ArchiveWriter writer(tar_path);
    size_t count = 0;

    for (const auto& rel : paths) {
        fs::path disk = src_dir / rel;

        // Lookup metadata
        const MetadataEntry* me = nullptr;
        if (auto it = meta.find(rel); it != meta.end())
            me = &it->second;

        if (!me)
            fprintf(stderr,
                    "Warning: no metadata for '%s'; using uid=0 gid=0 mode=0644\n",
                    rel.c_str());

        ArchiveEntry ae;
        ae.name        = rel;
        ae.uid         = me ? me->uid  : 0;
        ae.gid         = me ? me->gid  : 0;
        ae.mode        = me ? me->mode : 0644;
        ae.selinux_ctx = me ? me->selinux      : "";
        ae.cap_hex     = me ? me->capabilities : "";

        // Resolve symbolic names for uname / gname (Android ID table)
        ae.uname = android::id_to_name(ae.uid);
        ae.gname = android::id_to_name(ae.gid);

        // ---- Determine entry type ------------------------------------------
        bool is_symlink = me && !me->symlink.empty();

        if (is_symlink) {
            ae.type     = AE_IFLNK;
            ae.linkname = me->symlink;
            if (!ae.mode) ae.mode = 0777;
        }
        else if (fs::is_directory(disk)) {
            ae.type = AE_IFDIR;
            if (!ae.mode) ae.mode = 0755;
        }
        else if (fs::is_regular_file(disk)) {
            ae.type = AE_IFREG;
            if (!ae.mode) ae.mode = 0644;
            ae.data = read_file_bytes(disk);
        }
        else {
            fprintf(stderr, "Skipping unsupported file type: %s\n", rel.c_str());
            continue;
        }

        writer.write(ae);
        printf("  packed: %s\n", rel.c_str());
        ++count;
    }

    writer.finish();
    printf("\nDone. Written %zu entries to: %s\n", count, tar_path.c_str());
}
