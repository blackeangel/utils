#pragma once
// ---------------------------------------------------------------------------
// PAX field names that libarchive exposes via its xattr API:
//
//   TAR PAX key                       → libarchive xattr name
//   SCHILY.xattr.security.selinux     → "security.selinux"
//   SCHILY.xattr.security.capability  → "security.capability"
//
// The older Red Hat convention "RHT.security.selinux" is also supported on
// the **read** path: if libarchive does not surface it as an xattr we fall
// back to searching the raw PAX header block ourselves (see archive_rw.cpp).
// ---------------------------------------------------------------------------

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace pax {

// xattr names as used by libarchive
constexpr std::string_view XATTR_SELINUX = "security.selinux";
constexpr std::string_view XATTR_CAPS    = "security.capability";

// Raw-PAX key used by Red Hat / star (fall-back read path)
constexpr std::string_view KEY_RHT_SELINUX = "RHT.security.selinux";

// ---------------------------------------------------------------------------
// Linux capabilities (vfs_caps_t)
//
// The xattr value is a 20-byte little-endian struct:
//   uint32_t magic_etc  (index 0)  — header/version word
//   uint32_t permitted  (index 1)  — permitted capability bitmask
//   uint32_t inheritable(index 2)
//   uint32_t permitted2 (index 3)  — for v3 (128-bit)
//   uint32_t inheritable2(index 4)
//
// The original Python tar_repacker stores only the "permitted" word as a
// hex string, e.g. "0x2000000".  We follow the same convention.
// ---------------------------------------------------------------------------

constexpr size_t CAP_STRUCT_SIZE = 20; // 5 × uint32_t

inline uint32_t read_le32(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
inline void write_le32(uint8_t* p, uint32_t v) {
    p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF; p[3] = (v >> 24) & 0xFF;
}

// Raw capability bytes (20 bytes) → hex string "0x2000000", or "" if zero.
inline std::string cap_bytes_to_hex(const void* data, size_t size) {
    if (size < 8) return "";
    uint32_t val = read_le32(static_cast<const uint8_t*>(data) + 4);
    if (val == 0) return "";
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%x", val);
    return buf;
}

// Hex string "0x2000000" → 20-byte capability blob.
// Returns empty vector if hex is empty or zero.
inline std::vector<uint8_t> cap_hex_to_bytes(std::string_view hex) {
    if (hex.empty()) return {};
    char* end = nullptr;
    unsigned long val = std::strtoul(hex.data(), &end, 16);
    if (!end || end == hex.data() || val == 0) return {};

    std::vector<uint8_t> blob(CAP_STRUCT_SIZE, 0);
    write_le32(blob.data() + 4, (uint32_t)val);
    return blob;
}

// ---------------------------------------------------------------------------
// Minimal PAX-record parser for the RHT.security.selinux fall-back.
//
// A PAX data block is a sequence of records:
//   "<decimal-length> <key>=<value>\n"
// This function scans the raw bytes and returns the value for `want_key`,
// or "" if not found.
// ---------------------------------------------------------------------------
inline std::string find_pax_field(const uint8_t* data, size_t data_len,
                                  std::string_view want_key)
{
    size_t pos = 0;
    while (pos < data_len) {
        // Parse record length
        size_t len_start = pos;
        while (pos < data_len && data[pos] != ' ') ++pos;
        if (pos >= data_len) break;

        size_t record_len = 0;
        for (size_t i = len_start; i < pos; ++i) {
            if (data[i] < '0' || data[i] > '9') { pos = data_len; break; }
            record_len = record_len * 10 + (data[i] - '0');
        }
        if (record_len == 0 || len_start + record_len > data_len) break;

        ++pos; // skip space
        size_t record_end = len_start + record_len; // points to '\n'

        // Find '='
        size_t key_start = pos;
        while (pos < record_end && data[pos] != '=') ++pos;
        if (pos >= record_end) { pos = record_end; continue; }

        std::string_view key(reinterpret_cast<const char*>(data + key_start),
                             pos - key_start);
        ++pos; // skip '='

        if (key == want_key && record_end > pos) {
            // value is from pos to record_end-1 (exclude '\n')
            return { reinterpret_cast<const char*>(data + pos),
                     record_end - 1 - pos };
        }

        pos = record_end;
    }
    return {};
}

} // namespace pax
