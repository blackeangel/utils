/*
 * sdat2img.cpp — converts Android sparse data image back to raw image.
 * Supports both plain .new.dat and Brotli-compressed .new.dat.br input.
 *
 * Original Python: xpirt, luxi78, howellzhu
 * C++ / Brotli streaming rewrite: blackeangel
 * Adapted to UtilBase pattern for the utils project.
 */

#include "../include/main.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <brotli/decode.h>

static const uint64_t S2I_BLOCK = 4096;
static const size_t   S2I_CHUNK = 1 << 16;

// ── Transfer list ─────────────────────────────────────────────────────────────

struct S2ITransfer {
    uint64_t total_blocks = 0;
    std::vector<std::pair<uint64_t,uint64_t>> ranges;
};

static S2ITransfer s2i_parse_transfer(const std::string& path)
{
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open transfer.list: " + path);

    S2ITransfer td;
    std::string line;
    std::getline(in, line);            // version
    std::getline(in, line);            // total blocks
    td.total_blocks = std::stoull(line);
    std::getline(in, line);            // stash entries
    std::getline(in, line);            // max stash size

    while (std::getline(in, line)) {
        if (line.rfind("new ", 0) != 0) continue;
        std::istringstream ss(line);
        std::string cmd, range_str;
        ss >> cmd >> range_str;
        std::stringstream rs(range_str);
        std::string tok;
        std::vector<uint64_t> nums;
        while (std::getline(rs, tok, ','))
            if (!tok.empty()) nums.push_back(std::stoull(tok));
        if (nums.size() < 3) continue;
        for (size_t i = 1; i + 1 < nums.size(); i += 2)
            td.ranges.emplace_back(nums[i], nums[i+1]);
    }
    return td;
}

// ── carry-buffer flush ────────────────────────────────────────────────────────

static void s2i_flush(
    std::vector<uint8_t>& carry, size_t& carry_size,
    std::ofstream& out,
    const std::vector<std::pair<uint64_t,uint64_t>>& ranges,
    size_t& ri, uint64_t& cur_blk)
{
    size_t off = 0;
    while (carry_size - off >= S2I_BLOCK && ri < ranges.size()) {
        auto [start, end] = ranges[ri];
        uint64_t range_blks = end - start;
        if (cur_blk >= range_blks) { cur_blk = 0; ri++; continue; }
        uint64_t avail    = (carry_size - off) / S2I_BLOCK;
        uint64_t to_write = std::min(range_blks - cur_blk, avail);
        out.seekp(static_cast<std::streamoff>((start + cur_blk) * S2I_BLOCK));
        out.write(reinterpret_cast<const char*>(carry.data() + off),
                  static_cast<std::streamsize>(to_write * S2I_BLOCK));
        off     += to_write * S2I_BLOCK;
        cur_blk += to_write;
    }
    size_t rem = carry_size - off;
    if (rem && off) std::copy(carry.data()+off, carry.data()+carry_size, carry.data());
    carry_size = rem;
}

// ── plain .new.dat ────────────────────────────────────────────────────────────

static ProcessResult s2i_copy_plain(const S2ITransfer& td,
                                    std::ifstream& in, std::ofstream& out)
{
    std::vector<uint8_t> buf(S2I_BLOCK);
    size_t ri = 0; uint64_t cur_blk = 0, written = 0;

    while (ri < td.ranges.size()) {
        in.read(reinterpret_cast<char*>(buf.data()), S2I_BLOCK);
        if (in.gcount() != static_cast<std::streamsize>(S2I_BLOCK)) break;
        auto [start, end] = td.ranges[ri];
        out.seekp(static_cast<std::streamoff>((start + cur_blk) * S2I_BLOCK));
        out.write(reinterpret_cast<const char*>(buf.data()), S2I_BLOCK);
        if (++cur_blk >= end - start) { cur_blk = 0; ri++; }
        if (++written % 5000 == 0)
            std::cout << "\r  " << written << " blocks" << std::flush;
    }
    std::cout << "\r  " << written << " blocks written\n";
    return ProcessResult::ok;
}

// ── Brotli-compressed .new.dat.br  (streaming, constant RAM) ─────────────────

static ProcessResult s2i_decompress_brotli(const S2ITransfer& td,
                                            std::ifstream& in, std::ofstream& out)
{
    BrotliDecoderState* state =
        BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!state) { std::cerr << "Brotli init failed\n"; return ProcessResult::read_error; }

    std::vector<uint8_t> inbuf(S2I_CHUNK);
    std::vector<uint8_t> outbuf(S2I_CHUNK);
    std::vector<uint8_t> carry(S2I_CHUNK + S2I_BLOCK);
    size_t carry_size = 0;
    size_t ri = 0; uint64_t cur_blk = 0;
    bool finished = false;

    while (!finished) {
        in.read(reinterpret_cast<char*>(inbuf.data()), S2I_CHUNK);
        size_t bytes_read    = static_cast<size_t>(in.gcount());
        const uint8_t* next_in = inbuf.data();
        size_t avail_in      = bytes_read;

        while (true) {
            uint8_t* next_out  = outbuf.data();
            size_t   avail_out = S2I_CHUNK;

            auto res = BrotliDecoderDecompressStream(
                state, &avail_in, &next_in, &avail_out, &next_out, nullptr);

            size_t produced = S2I_CHUNK - avail_out;
            if (produced) {
                std::copy(outbuf.data(), outbuf.data() + produced,
                          carry.data() + carry_size);
                carry_size += produced;
                s2i_flush(carry, carry_size, out, td.ranges, ri, cur_blk);
                std::cout << "\r  range " << (ri+1) << "/" << td.ranges.size() << std::flush;
            }

            if (res == BROTLI_DECODER_RESULT_SUCCESS)          { finished = true; break; }
            if (res == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) break;
            if (res == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) continue;
            if (res == BROTLI_DECODER_RESULT_ERROR) {
                std::cerr << "\nBrotli error: "
                          << BrotliDecoderErrorString(BrotliDecoderGetErrorCode(state)) << "\n";
                BrotliDecoderDestroyInstance(state);
                return ProcessResult::read_error;
            }
        }
        if (!finished && bytes_read == 0) break;
    }

    BrotliDecoderDestroyInstance(state);

    if (!finished) {
        std::cerr << "\nStream ended before completion\n";
        return ProcessResult::read_error;
    }
    if (ri < td.ranges.size())
        std::cerr << "\nWarning: not all ranges written ("
                  << ri << "/" << td.ranges.size() << ")\n";

    std::cout << "\r  " << td.ranges.size() << "/" << td.ranges.size()
              << " ranges written\n";
    return ProcessResult::ok;
}

// ── UtilBase interface ────────────────────────────────────────────────────────

void Sdat2Img::show_help() {
    std::cout <<
        "\nsdat2img\n\n"
        "Usage:\n\n"
        "  sdat2img <transfer.list> <input.new.dat[.br]> [output.img]\n\n"
        "  transfer.list        transfer list produced by img2sdat\n"
        "  input.new.dat        plain sparse data file\n"
        "  input.new.dat.br     Brotli-compressed sparse data file\n"
        "  output.img           output raw image (default: derived from input name)\n\n"
        "  Brotli input is detected automatically from the .br extension.\n\n";
}

ParseResult Sdat2Img::parse_cmd_line(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        show_help();
        return argc < 2 ? ParseResult::not_enough : ParseResult::wrong_option;
    }

    transfer_path = argv[0];
    input_path    = argv[1];

    if (argc == 3) {
        output_path = argv[2];
    } else {
        output_path = input_path;
        if (output_path.size() >= 3 &&
            output_path.substr(output_path.size()-3) == ".br")
            output_path.resize(output_path.size()-3);
        const std::string nd = ".new.dat";
        auto pos = output_path.rfind(nd);
        if (pos != std::string::npos)
            output_path.replace(pos, nd.size(), ".img");
        else
            output_path += ".img";
    }
    return ParseResult::ok;
}

ProcessResult Sdat2Img::process() {
    std::cout << "sdat2img\n\n"
              << "  Transfer : " << transfer_path << "\n"
              << "  Input    : " << input_path    << "\n"
              << "  Output   : " << output_path   << "\n\n";

    S2ITransfer td;
    try { td = s2i_parse_transfer(transfer_path); }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return ProcessResult::open_error;
    }

    if (td.total_blocks == 0 || td.ranges.empty()) {
        std::cerr << "Error: transfer list is empty or invalid\n";
        return ProcessResult::read_error;
    }
    std::cout << "  Blocks   : " << td.total_blocks << "\n"
              << "  Ranges   : " << td.ranges.size() << "\n\n";

    std::ifstream in(input_path, std::ios::binary);
    if (!in) { std::cerr << "Cannot open: " << input_path << "\n"; return ProcessResult::open_error; }
    std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
    if (!out) { std::cerr << "Cannot create: " << output_path << "\n"; return ProcessResult::open_error; }

    // Pre-allocate image to full size
    uint64_t img_size = td.total_blocks * S2I_BLOCK;
    out.seekp(static_cast<std::streamoff>(img_size - 1));
    out.write("", 1);
    out.flush();

    bool is_brotli = input_path.size() >= 3 &&
                     input_path.substr(input_path.size()-3) == ".br";

    std::cout << "  Mode     : " << (is_brotli ? "Brotli decompress" : "plain copy") << "\n";

    ProcessResult res = is_brotli
        ? s2i_decompress_brotli(td, in, out)
        : s2i_copy_plain(td, in, out);

    if (res == ProcessResult::ok)
        std::cout << "\nDone → " << output_path << "\n";
    return res;
}
