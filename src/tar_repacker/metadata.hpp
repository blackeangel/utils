#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// One row in the _metadata.txt file.
// Format (space-separated, matching the Python tar_repacker.py):
//   <path> <uid> <gid> <oct_mode> [<selinux>] [<capabilities_hex>] [<symlink>]
//
// Fields are always written in all-7-fields form with empty slots producing
// consecutive spaces, so Python's whitespace-split / structural pattern matching
// is compatible with this output.
struct MetadataEntry {
    std::string path;           // relative, no leading '/'
    uint32_t    uid  = 0;
    uint32_t    gid  = 0;
    uint32_t    mode = 0;
    std::string selinux;        // SELinux context, e.g. "u:object_r:system_file:s0"
    std::string capabilities;   // hex string, e.g. "0x2000000", or ""
    std::string symlink;        // symlink target, or ""
};

using MetadataMap = std::map<std::string, MetadataEntry>;

// Append entries to (or create) the metadata file.
// The file gets one line per entry, sorted by path.
void save_metadata(const std::filesystem::path& file,
                   std::vector<MetadataEntry>& entries);

// Read a metadata file written by save_metadata (or by the Python version).
// Returns a map keyed by the path field.
MetadataMap load_metadata(const std::filesystem::path& file);

// Format the oct_mode as Python's oct() does: "0o755"
std::string format_mode(uint32_t mode);

// Parse Python-style oct mode: "0o755" or "0755" → integer
uint32_t parse_mode(const std::string& s);
