/*
 * img2sdat.cpp — C++ reimplementation of xpirt/img2sdat (Python)
 * Converts sparse EXT4 image (.img) → Android sparse data image
 * (.new.dat + .transfer.list)
 *
 * Authors (original Python): xpirt, luxi78, howellzhu
 * C++ port: Claude / Anthropic 2026
 * Adapted to UtilBase pattern for the utils project.
 */

#include "../include/main.hpp"

#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <sys/stat.h>
#include <brotli/encode.h>

#ifdef _WIN32
#  include <direct.h>
#  define MKDIR(p) ::_mkdir(p)
#else
#  define MKDIR(p) ::mkdir((p), 0755)
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Streaming Brotli output streambuf
// ─────────────────────────────────────────────────────────────────────────────
class BrotliBuf : public std::streambuf {
public:
    BrotliBuf(const std::string& path, int quality, int window)
        : out_(path, std::ios::binary), ok_(false)
    {
        if (!out_) return;
        enc_ = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!enc_) return;
        BrotliEncoderSetParameter(enc_, BROTLI_PARAM_QUALITY, quality);
        BrotliEncoderSetParameter(enc_, BROTLI_PARAM_LGWIN,   window);
        ok_ = true;
    }
    ~BrotliBuf() { if (enc_) BrotliEncoderDestroyInstance(enc_); }

    bool finish() { return ok_ && pump(nullptr, 0, BROTLI_OPERATION_FINISH); }
    bool   good()          const { return ok_; }
    size_t bytes_written() const { return bytes_written_; }

protected:
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        if (!ok_) return 0;
        if (!pump(reinterpret_cast<const uint8_t*>(s),
                  static_cast<size_t>(n), BROTLI_OPERATION_PROCESS))
            ok_ = false;
        return ok_ ? n : 0;
    }
    int overflow(int c) override {
        if (!ok_ || c == EOF) return c;
        uint8_t b = static_cast<uint8_t>(c);
        xsputn(reinterpret_cast<const char*>(&b), 1);
        return ok_ ? c : EOF;
    }

private:
    bool pump(const uint8_t* in_ptr, size_t in_left, BrotliEncoderOperation op) {
        uint8_t outbuf[65536];
        do {
            uint8_t* out_ptr  = outbuf;
            size_t   out_left = sizeof(outbuf);
            if (!BrotliEncoderCompressStream(enc_, op,
                                             &in_left, &in_ptr,
                                             &out_left, &out_ptr, nullptr))
                return false;
            size_t written = sizeof(outbuf) - out_left;
            if (written) {
                out_.write(reinterpret_cast<const char*>(outbuf),
                           static_cast<std::streamsize>(written));
                bytes_written_ += written;
            }
        } while (in_left > 0 || BrotliEncoderHasMoreOutput(enc_));
        return out_.good();
    }

    std::ofstream        out_;
    BrotliEncoderState*  enc_           = nullptr;
    bool                 ok_;
    size_t               bytes_written_ = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// Android sparse image format
// ─────────────────────────────────────────────────────────────────────────────
static const uint32_t SPARSE_MAGIC    = 0xed26ff3a;
static const uint16_t CHUNK_RAW       = 0xCAC1;
static const uint16_t CHUNK_FILL      = 0xCAC2;
static const uint16_t CHUNK_DONT_CARE = 0xCAC3;

#pragma pack(push, 1)
struct SparseHeader {
    uint32_t magic, blk_sz_unused[2];
    uint16_t major_version, minor_version, file_hdr_sz, chunk_hdr_sz;
    uint32_t blk_sz, total_blks, total_chunks, image_checksum;
};
struct ChunkHeader {
    uint16_t chunk_type, reserved1;
    uint32_t chunk_sz, total_sz;
};
#pragma pack(pop)

// Fix: proper SparseHeader layout
#pragma pack(push, 1)
struct SparseHdr2 {
    uint32_t magic;
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t file_hdr_sz;
    uint16_t chunk_hdr_sz;
    uint32_t blk_sz;
    uint32_t total_blks;
    uint32_t total_chunks;
    uint32_t image_checksum;
};
#pragma pack(pop)

// ─────────────────────────────────────────────────────────────────────────────
// RangeSet
// ─────────────────────────────────────────────────────────────────────────────
struct RangeSet {
    std::vector<std::pair<int64_t,int64_t>> ranges;

    int64_t size() const {
        int64_t s = 0;
        for (auto& r : ranges) s += r.second - r.first;
        return s;
    }

    std::string encode() const {
        std::string s = std::to_string(ranges.size() * 2);
        for (auto& r : ranges)
            s += "," + std::to_string(r.first) + "," + std::to_string(r.second);
        return s;
    }

    void normalize() {
        if (ranges.empty()) return;
        std::sort(ranges.begin(), ranges.end());
        std::vector<std::pair<int64_t,int64_t>> out;
        for (auto& r : ranges) {
            if (!out.empty() && r.first <= out.back().second)
                out.back().second = std::max(out.back().second, r.second);
            else
                out.push_back(r);
        }
        ranges = std::move(out);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// SparseImage
// ─────────────────────────────────────────────────────────────────────────────
struct SparseImage {
    std::string path;
    uint32_t    block_size = 0;
    int64_t     total_blks = 0;
    RangeSet    care_map;
    RangeSet    zero_map;
    int64_t     new_blocks_written = 0;

    std::unordered_map<int64_t,int64_t> build_raw_block_map() {
        std::unordered_map<int64_t,int64_t> m;
        std::ifstream f(path, std::ios::binary);
        if (!f) return m;
        SparseHdr2 hdr{};
        f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
        if (!f || hdr.magic != SPARSE_MAGIC) return m;
        f.seekg(hdr.file_hdr_sz, std::ios::beg);
        int64_t cur_blk = 0;
        for (uint32_t i = 0; i < hdr.total_chunks; i++) {
            ChunkHeader ch{};
            f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            if (!f) break;
            if (hdr.chunk_hdr_sz > sizeof(ch))
                f.seekg(static_cast<std::streamoff>(hdr.chunk_hdr_sz - sizeof(ch)), std::ios::cur);
            int64_t data_start = static_cast<int64_t>(f.tellg());
            int64_t data_bytes = static_cast<int64_t>(ch.total_sz) - hdr.chunk_hdr_sz;
            if (ch.chunk_type == CHUNK_RAW)
                for (uint32_t b = 0; b < ch.chunk_sz; b++)
                    m[cur_blk + b] = data_start + int64_t(b) * hdr.blk_sz;
            f.seekg(data_start + data_bytes);
            cur_blk += ch.chunk_sz;
        }
        return m;
    }

    std::vector<bool> read_ext4_bitmap_sparse(
            const std::unordered_map<int64_t,int64_t>& raw_map) {
        std::ifstream f(path, std::ios::binary);
        if (!f || raw_map.empty()) return {};
        auto read_from = [&](int64_t blk, int64_t off, void* dest, int64_t len) -> bool {
            auto it = raw_map.find(blk);
            if (it == raw_map.end()) return false;
            f.seekg(it->second + off);
            f.read(reinterpret_cast<char*>(dest), len);
            return f.good();
        };
        uint8_t sb[340] = {};
        if (!read_from(0, 1024, sb, sizeof(sb))) return {};
        auto u16 = [&](int o) -> uint16_t { return uint16_t(sb[o])|(uint16_t(sb[o+1])<<8); };
        auto u32 = [&](int o) -> uint32_t {
            return uint32_t(sb[o])|(uint32_t(sb[o+1])<<8)|(uint32_t(sb[o+2])<<16)|(uint32_t(sb[o+3])<<24); };
        if (u16(56) != 0xEF53) return {};
        uint32_t bs = 1024u << u32(24);
        uint32_t first_db = u32(20), blks_per_grp = u32(32);
        bool is_64bit = (u32(96) & 0x80) != 0;
        uint16_t desc_sz = u16(254); if (desc_sz < 32) desc_sz = 32;
        int64_t total_b = u32(4);
        if (is_64bit) total_b |= int64_t(u32(336)) << 32;
        if (bs == 0 || blks_per_grp == 0 || total_b <= 0) return {};
        int64_t num_grps = (total_b - first_db + blks_per_grp - 1) / blks_per_grp;
        int64_t gdt_start_blk = (bs == 1024) ? 2 : 1;
        std::vector<bool> alloc(total_b, false);
        std::vector<uint8_t> bmap(bs);
        for (int64_t g = 0; g < num_grps; g++) {
            int64_t gd_byte = g * desc_sz;
            int64_t gd_blk = gdt_start_blk + gd_byte / bs;
            int64_t gd_off = gd_byte % bs;
            uint8_t gd[64] = {};
            if (!read_from(gd_blk, gd_off, gd, desc_sz)) continue;
            auto gd32 = [&](int o) -> uint32_t {
                return uint32_t(gd[o])|(uint32_t(gd[o+1])<<8)|(uint32_t(gd[o+2])<<16)|(uint32_t(gd[o+3])<<24); };
            int64_t bmap_blk = gd32(0);
            if (is_64bit && desc_sz >= 64) bmap_blk |= int64_t(gd32(32)) << 32;
            if (!read_from(bmap_blk, 0, bmap.data(), bs)) continue;
            int64_t grp_start = int64_t(first_db) + g * int64_t(blks_per_grp);
            int64_t grp_count = std::min(int64_t(blks_per_grp), total_b - grp_start);
            for (int64_t b = 0; b < grp_count; b++)
                if ((bmap[b >> 3] >> (b & 7)) & 1)
                    alloc[grp_start + b] = true;
        }
        return alloc;
    }

    bool read_and_write(std::ostream& new_dat) {
        std::ifstream f(path, std::ios::binary);
        if (!f) { std::cerr << "Cannot open: " << path << "\n"; return false; }
        SparseHdr2 hdr{};
        f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
        if (!f || hdr.magic != SPARSE_MAGIC) {
            std::cout << "  (Not a sparse image — reading as raw)\n";
            return read_raw_and_write(new_dat);
        }
        block_size = hdr.blk_sz;
        total_blks = hdr.total_blks;
        std::vector<bool> alloc;
        {
            auto raw_map = build_raw_block_map();
            alloc = read_ext4_bitmap_sparse(raw_map);
            if (alloc.empty())
                std::cout << "  (ext4 bitmap unavailable — all FILL(0) → zero)\n";
            else
                std::cout << "  ext4 bitmap loaded: " << total_blks << " blocks\n";
        }
        auto is_allocated = [&](int64_t b) -> bool {
            if (alloc.empty()) return true;
            return b < (int64_t)alloc.size() && alloc[b];
        };
        f.seekg(hdr.file_hdr_sz, std::ios::beg);
        std::vector<uint8_t> buf(block_size);
        const std::vector<uint8_t> zeros(block_size, 0);
        int64_t cur_blk = 0;
        for (uint32_t i = 0; i < hdr.total_chunks; i++) {
            ChunkHeader ch{};
            f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            if (!f) break;
            if (hdr.chunk_hdr_sz > sizeof(ch))
                f.seekg(static_cast<std::streamoff>(hdr.chunk_hdr_sz - sizeof(ch)), std::ios::cur);
            int64_t data_start = static_cast<int64_t>(f.tellg());
            int64_t data_bytes = static_cast<int64_t>(ch.total_sz) - hdr.chunk_hdr_sz;
            if (ch.chunk_type == CHUNK_RAW) {
                for (uint32_t b = 0; b < ch.chunk_sz; b++) {
                    f.read(reinterpret_cast<char*>(buf.data()), block_size);
                    care_map.ranges.push_back({cur_blk + b, cur_blk + b + 1});
                    if (buf == zeros) {
                        zero_map.ranges.push_back({cur_blk + b, cur_blk + b + 1});
                    } else {
                        new_dat.write(reinterpret_cast<char*>(buf.data()), block_size);
                        if (++new_blocks_written % 1000 == 0)
                            std::cout << "\r  " << new_blocks_written << " blocks" << std::flush;
                    }
                }
            } else if (ch.chunk_type == CHUNK_FILL) {
                uint32_t fv = 0;
                f.read(reinterpret_cast<char*>(&fv), 4);
                if (fv == 0) {
                    for (uint32_t b = 0; b < ch.chunk_sz; b++) {
                        if (is_allocated(cur_blk + b)) {
                            care_map.ranges.push_back({cur_blk + b, cur_blk + b + 1});
                            zero_map.ranges.push_back({cur_blk + b, cur_blk + b + 1});
                        }
                    }
                } else {
                    for (uint32_t j = 0; j < block_size / 4; j++) {
                        buf[j*4+0] = uint8_t(fv);      buf[j*4+1] = uint8_t(fv>>8);
                        buf[j*4+2] = uint8_t(fv>>16);  buf[j*4+3] = uint8_t(fv>>24);
                    }
                    for (uint32_t b = 0; b < ch.chunk_sz; b++) {
                        new_dat.write(reinterpret_cast<char*>(buf.data()), block_size);
                        care_map.ranges.push_back({cur_blk + b, cur_blk + b + 1});
                        if (++new_blocks_written % 1000 == 0)
                            std::cout << "\r  " << new_blocks_written << " blocks" << std::flush;
                    }
                }
            }
            // CHUNK_DONT_CARE: skip
            f.seekg(data_start + data_bytes);
            cur_blk += ch.chunk_sz;
        }
        std::cout << "\r  " << new_blocks_written << " blocks written to new.dat\n";
        care_map.normalize();
        zero_map.normalize();
        return true;
    }

    std::vector<bool> read_ext4_allocated() {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f) return {};
        int64_t file_size = static_cast<int64_t>(f.tellg());
        if (file_size < 1024 + 340) return {};
        f.seekg(1024);
        uint8_t sb[340];
        f.read(reinterpret_cast<char*>(sb), sizeof(sb));
        auto u16 = [&](int o) -> uint16_t { return uint16_t(sb[o])|(uint16_t(sb[o+1])<<8); };
        auto u32 = [&](int o) -> uint32_t {
            return uint32_t(sb[o])|(uint32_t(sb[o+1])<<8)|(uint32_t(sb[o+2])<<16)|(uint32_t(sb[o+3])<<24); };
        if (u16(56) != 0xEF53) return {};
        uint32_t bs = 1024u << u32(24);
        uint32_t first_db = u32(20), blks_per_grp = u32(32);
        uint32_t inodes_per_g = u32(40), inode_sz = u16(88);
        bool is_64bit = (u32(96) & 0x80) != 0;
        uint16_t desc_sz = u16(254); if (desc_sz < 32) desc_sz = 32;
        int64_t total_b = u32(4);
        if (is_64bit) total_b |= int64_t(u32(336)) << 32;
        if (bs == 0 || blks_per_grp == 0 || total_b <= 0 || inode_sz == 0 || inodes_per_g == 0) return {};
        int64_t it_size = (int64_t(inodes_per_g) * inode_sz + bs - 1) / bs;
        int64_t num_grps = (total_b - first_db + blks_per_grp - 1) / blks_per_grp;
        int64_t gdt_off  = (bs == 1024 ? 2 : 1) * int64_t(bs);
        std::vector<bool> alloc(total_b, false);
        for (int64_t g = 0; g < num_grps; g++) {
            int64_t gd_off = gdt_off + g * desc_sz;
            if (gd_off + desc_sz > file_size) break;
            f.seekg(gd_off);
            uint8_t gd[64] = {};
            f.read(reinterpret_cast<char*>(gd), desc_sz);
            auto gd32 = [&](int o) -> uint32_t {
                return uint32_t(gd[o])|(uint32_t(gd[o+1])<<8)|(uint32_t(gd[o+2])<<16)|(uint32_t(gd[o+3])<<24); };
            int64_t it_blk = gd32(8);
            if (is_64bit && desc_sz >= 64) it_blk |= int64_t(gd32(40)) << 32;
            for (int64_t b = 0; b < it_size; b++) {
                int64_t abs_b = it_blk + b;
                if (abs_b >= 0 && abs_b < total_b) alloc[abs_b] = true;
            }
        }
        return alloc;
    }

    bool read_raw_and_write(std::ostream& new_dat) {
        block_size = 4096;
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f) { std::cerr << "Cannot open: " << path << "\n"; return false; }
        int64_t file_size = static_cast<int64_t>(f.tellg());
        f.seekg(0, std::ios::beg);
        total_blks = file_size / block_size;
        if (total_blks == 0) { std::cerr << "Error: image is empty.\n"; return false; }
        std::vector<bool> alloc = read_ext4_allocated();
        std::vector<uint8_t> buf(block_size);
        const std::vector<uint8_t> zeros(block_size, 0);
        for (int64_t b = 0; b < total_blks; b++) {
            f.read(reinterpret_cast<char*>(buf.data()), block_size);
            if (!f) break;
            if (buf == zeros) {
                bool is_alloc = (!alloc.empty() && b < (int64_t)alloc.size() && alloc[b]);
                if (is_alloc) {
                    care_map.ranges.push_back({b, b + 1});
                    zero_map.ranges.push_back({b, b + 1});
                }
            } else {
                new_dat.write(reinterpret_cast<char*>(buf.data()), block_size);
                care_map.ranges.push_back({b, b + 1});
                if (++new_blocks_written % 5000 == 0)
                    std::cout << "\r  " << new_blocks_written << " non-zero" << std::flush;
            }
        }
        std::cout << "\r  " << new_blocks_written << " non-zero blocks, "
                  << zero_map.size() << " zero blocks.\n";
        care_map.normalize();
        zero_map.normalize();
        return true;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Write transfer list
// ─────────────────────────────────────────────────────────────────────────────
static void write_transfer_list(std::ostream& out,
                                const RangeSet& care_map,
                                const RangeSet& zero_map,
                                int version)
{
    out << version << "\n" << care_map.size() << "\n";
    if (version >= 2) { out << "0\n0\n"; }

    if (care_map.ranges.size() > 1) {
        RangeSet erase_rs;
        for (size_t i = 0; i + 1 < care_map.ranges.size(); i++) {
            int64_t gap_s = care_map.ranges[i].second;
            int64_t gap_e = care_map.ranges[i + 1].first;
            if (gap_s < gap_e) erase_rs.ranges.push_back({gap_s, gap_e});
        }
        if (!erase_rs.ranges.empty())
            out << "erase " << erase_rs.encode() << "\n";
    }

    auto is_zero = [&](int64_t b) -> bool {
        const auto& zr = zero_map.ranges;
        auto it = std::upper_bound(zr.begin(), zr.end(), std::make_pair(b, b),
                                   [](const auto& v, const auto& r){ return v.first < r.first; });
        if (it != zr.begin()) { --it; if (b >= it->first && b < it->second) return true; }
        return false;
    };

    const int64_t BATCH = 1024;
    std::vector<std::pair<int64_t,int64_t>> batch;
    int64_t batch_sz = 0;
    auto flush_batch = [&]() {
        if (batch.empty()) return;
        RangeSet rs; rs.ranges = batch;
        out << "new " << rs.encode() << "\n";
        batch.clear(); batch_sz = 0;
    };
    for (auto& care_r : care_map.ranges) {
        for (int64_t b = care_r.first; b < care_r.second; b++) {
            if (is_zero(b)) continue;
            if (!batch.empty() && batch.back().second == b) batch.back().second++;
            else batch.push_back({b, b + 1});
            if (++batch_sz == BATCH) flush_batch();
        }
    }
    flush_batch();

    for (auto& care_r : care_map.ranges) {
        RangeSet z;
        for (auto& zr : zero_map.ranges)
            if (zr.first >= care_r.first && zr.second <= care_r.second)
                z.ranges.push_back(zr);
        if (!z.ranges.empty()) out << "zero " << z.encode() << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Filesystem helpers
// ─────────────────────────────────────────────────────────────────────────────
static bool dir_exists_i2s(const std::string& p) {
    struct stat st{};
    return stat(p.c_str(), &st) == 0 && (st.st_mode & S_IFDIR);
}
static bool makedirs_i2s(const std::string& p) {
    if (p.empty() || dir_exists_i2s(p)) return true;
    size_t pos = p.find_last_of("/\\");
    if (pos != std::string::npos && pos > 0)
        if (!makedirs_i2s(p.substr(0, pos))) return false;
    return MKDIR(p.c_str()) == 0 || dir_exists_i2s(p);
}

// ─────────────────────────────────────────────────────────────────────────────
// UtilBase interface
// ─────────────────────────────────────────────────────────────────────────────
void Img2Sdat::show_help() {
    std::cout <<
        "\nimg2sdat\n\n"
        "Usage:\n\n"
        "  img2sdat <system.img> [-o outdir] [-v version] [-p prefix] [-b [quality]]\n\n"
        "  <system.img>    input sparse or raw EXT4 image\n"
        "  -o outdir       output directory (default: .)\n"
        "  -v version      transfer list version 1-4 (default: 4)\n"
        "                    1=Android 5.0  2=5.1  3=6.0  4=7.x/8.x\n"
        "  -p prefix       output filename prefix (default: system)\n"
        "  -b [quality]    compress new.dat with Brotli → <prefix>.new.dat.br\n"
        "                    quality 0-11, default 6. Streaming — constant RAM usage.\n\n"
        "Outputs: <prefix>.new.dat[.br]  <prefix>.transfer.list\n\n";
}

ParseResult Img2Sdat::parse_cmd_line(int argc, char* argv[]) {
    // argv[0] = input image (main() already stripped binary name + subcommand)
    if (argc < 1) { show_help(); return ParseResult::not_enough; }

    input_path = argv[0];
    outdir     = ".";
    prefix     = "system";
    version    = 4;
    use_brotli = false;
    br_quality = 6;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if ((a == "-o" || a == "--outdir")  && i+1 < argc) { outdir  = argv[++i]; }
        else if ((a == "-v" || a == "--version") && i+1 < argc) {
            version = std::stoi(argv[++i]);
            if (version < 1 || version > 4) {
                std::cerr << "Error: version must be 1-4\n";
                return ParseResult::wrong_option;
            }
        }
        else if ((a == "-p" || a == "--prefix") && i+1 < argc) { prefix = argv[++i]; }
        else if (a == "-b" || a == "--brotli") {
            use_brotli = true;
            if (i+1 < argc) {
                const std::string& nxt = argv[i+1];
                if (!nxt.empty() && nxt[0] != '-' &&
                    nxt.find_first_not_of("0123456789") == std::string::npos) {
                    br_quality = std::stoi(nxt);
                    if (br_quality < 0 || br_quality > 11) {
                        std::cerr << "Error: Brotli quality must be 0-11\n";
                        return ParseResult::wrong_option;
                    }
                    ++i;
                }
            }
        }
        else { std::cerr << "Unknown option: " << a << "\n"; show_help(); return ParseResult::wrong_option; }
    }
    return ParseResult::ok;
}

ProcessResult Img2Sdat::process() {
    std::cout << "img2sdat C++ v1.3\n\n"
              << "Input   : " << input_path << "\n"
              << "Outdir  : " << outdir     << "\n"
              << "Version : " << version    << "\n"
              << "Prefix  : " << prefix     << "\n";
    if (use_brotli) std::cout << "Brotli  : quality=" << br_quality << "\n";
    std::cout << "\n";

    if (!makedirs_i2s(outdir)) {
        std::cerr << "Cannot create directory: " << outdir << "\n";
        return ProcessResult::open_error;
    }

    const std::string base    = outdir + "/" + prefix;
    const std::string tlist   = base + ".transfer.list";
    const std::string nd_path = use_brotli ? (base + ".new.dat.br") : (base + ".new.dat");

    SparseImage img;
    img.path = input_path;

    if (use_brotli) {
        std::cout << "Reading image and compressing → " << nd_path << " ...\n";
        BrotliBuf bbuf(nd_path, br_quality, BROTLI_DEFAULT_WINDOW);
        if (!bbuf.good()) {
            std::cerr << "Cannot create: " << nd_path << "\n";
            return ProcessResult::open_error;
        }
        std::ostream bstream(&bbuf);
        if (!img.read_and_write(bstream)) return ProcessResult::read_error;
        if (!bbuf.finish()) {
            std::cerr << "Brotli compression failed\n";
            return ProcessResult::write_error;
        }
        size_t br_sz  = bbuf.bytes_written();
        size_t raw_sz = static_cast<size_t>(img.new_blocks_written) * img.block_size;
        std::cout << "  " << raw_sz << " bytes → " << br_sz << " bytes";
        if (raw_sz > 0) {
            uint64_t pct10 = uint64_t(br_sz) * 1000 / raw_sz;
            std::cout << " (" << (pct10/10) << "." << (pct10%10) << "%)";
        }
        std::cout << "\n";
    } else {
        std::cout << "Reading image and writing " << nd_path << " ...\n";
        std::ofstream fout(nd_path, std::ios::binary);
        if (!fout) { std::cerr << "Cannot create: " << nd_path << "\n"; return ProcessResult::open_error; }
        if (!img.read_and_write(fout)) return ProcessResult::read_error;
    }

    std::cout << "  block_size  = " << img.block_size         << " bytes\n"
              << "  total_blks  = " << img.total_blks          << "\n"
              << "  new_blocks  = " << img.new_blocks_written   << "\n"
              << "  zero_blocks = " << img.zero_map.size()      << "\n"
              << "  care_blocks = " << img.care_map.size()      << "\n\n";

    std::cout << "Writing " << tlist << " ...\n";
    {
        std::ofstream fout(tlist);
        if (!fout) { std::cerr << "Cannot create: " << tlist << "\n"; return ProcessResult::write_error; }
        write_transfer_list(fout, img.care_map, img.zero_map, version);
    }

    std::cout << "\nDone!\n  " << tlist << "\n  " << nd_path << "\n";
    return ProcessResult::ok;
}
