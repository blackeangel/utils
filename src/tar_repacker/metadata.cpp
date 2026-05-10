#include "metadata.hpp"

#include <algorithm>
#include <cstdio>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------------------------
// Mode formatting / parsing (matching Python's oct() / int(s, 8))
// ---------------------------------------------------------------------------

std::string format_mode(uint32_t mode) {
    // Python oct(0o755) → "0o755"
    char buf[32];
    snprintf(buf, sizeof(buf), "0o%o", mode);
    return buf;
}

uint32_t parse_mode(const std::string& s) {
    // Accept "0o755", "0755", or plain "755"
    const char* p = s.c_str();
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'o' || s[1] == 'O'))
        p += 2;
    char* end = nullptr;
    unsigned long val = std::strtoul(p, &end, 8);
    return (uint32_t)val;
}

// ---------------------------------------------------------------------------
// Save
// ---------------------------------------------------------------------------

void save_metadata(const std::filesystem::path& file,
                   std::vector<MetadataEntry>& entries)
{
    // Sort by path (same as Python's list.sort())
    std::sort(entries.begin(), entries.end(),
              [](const MetadataEntry& a, const MetadataEntry& b) {
                  return a.path < b.path;
              });

    // Append mode (matching Python's open(..., 'a') with newline='\n')
    std::ofstream out(file, std::ios::app | std::ios::binary);
    if (!out)
        throw std::runtime_error(std::format("Cannot open '{}' for writing",
                                             file.string()));

    for (const auto& e : entries) {
        // Always write all 7 fields; empty fields produce consecutive spaces
        // so that Python's split() / structural pattern matching works:
        //   7 tokens → (path uid gid mode context cap symlink)
        //   6 tokens → (path uid gid mode context symlink)   [cap empty]
        //   5 tokens → (path uid gid mode context)           [both empty]
        //   4 tokens → (path uid gid mode)                   [context empty too]
        std::string line = std::format("{} {} {} {} {} {} {}",
            e.path, e.uid, e.gid,
            format_mode(e.mode),
            e.selinux, e.capabilities, e.symlink);
        out << line << '\n';
    }
}

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------

MetadataMap load_metadata(const std::filesystem::path& file) {
    std::ifstream in(file);
    if (!in)
        throw std::runtime_error(std::format("Cannot open metadata file '{}'",
                                             file.string()));

    MetadataMap result;
    std::string line;

    while (std::getline(in, line)) {
        // Strip CR if present (Windows line endings)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty()) continue;

        // Split by whitespace (mirrors Python's str.split())
        std::vector<std::string> fields;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token)
            fields.push_back(std::move(token));

        MetadataEntry e;
        switch (fields.size()) {
            case 7:
                e.path         = fields[0];
                e.uid          = (uint32_t)std::stoul(fields[1]);
                e.gid          = (uint32_t)std::stoul(fields[2]);
                e.mode         = parse_mode(fields[3]);
                e.selinux      = fields[4];
                e.capabilities = fields[5];
                e.symlink      = fields[6];
                break;
            case 6:
                // Python matches this as [path uid gid mode context symlink]
                e.path         = fields[0];
                e.uid          = (uint32_t)std::stoul(fields[1]);
                e.gid          = (uint32_t)std::stoul(fields[2]);
                e.mode         = parse_mode(fields[3]);
                e.selinux      = fields[4];
                e.symlink      = fields[5];
                break;
            case 5:
                e.path    = fields[0];
                e.uid     = (uint32_t)std::stoul(fields[1]);
                e.gid     = (uint32_t)std::stoul(fields[2]);
                e.mode    = parse_mode(fields[3]);
                e.selinux = fields[4];
                break;
            case 4:
                e.path = fields[0];
                e.uid  = (uint32_t)std::stoul(fields[1]);
                e.gid  = (uint32_t)std::stoul(fields[2]);
                e.mode = parse_mode(fields[3]);
                break;
            default:
                // Warn but don't crash on malformed lines
                if (!fields.empty())
                    fprintf(stderr, "Warning: invalid metadata line: %s\n", line.c_str());
                continue;
        }

        result[e.path] = std::move(e);
    }

    return result;
}
