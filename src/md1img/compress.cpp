#include "main.h"

// ── compress_gz — zlib deflate (gzip format) ──────────────────────────────────
std::vector<char> compress_gz(const std::vector<char>& data, int level) {
    z_stream strm{};
    // 16 + MAX_WBITS → gzip wrapper
    if (deflateInit2(&strm, level, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw std::runtime_error("compress_gz: deflateInit2 failed");

    uLong bound = deflateBound(&strm, static_cast<uLong>(data.size()));
    std::vector<char> out(static_cast<size_t>(bound));

    strm.next_in   = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    strm.avail_in  = static_cast<uInt>(data.size());
    strm.next_out  = reinterpret_cast<Bytef*>(out.data());
    strm.avail_out = static_cast<uInt>(out.size());

    if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&strm);
        throw std::runtime_error("compress_gz: deflate failed");
    }
    size_t used = out.size() - strm.avail_out;
    deflateEnd(&strm);
    out.resize(used);
    return out;
}

// ── compress_xz — liblzma XZ container ───────────────────────────────────────
std::vector<char> compress_xz(const std::vector<char>& data, int level) {
    lzma_stream strm = LZMA_STREAM_INIT;

    uint32_t preset = static_cast<uint32_t>(level < 0 ? 6 : (level > 9 ? 9 : level));
    lzma_ret ret = lzma_easy_encoder(&strm, preset, LZMA_CHECK_CRC64);
    if (ret != LZMA_OK)
        throw std::runtime_error("compress_xz: lzma_easy_encoder failed (" + std::to_string(ret) + ")");

    std::vector<char> out;
    strm.next_in  = reinterpret_cast<const uint8_t*>(data.data());
    strm.avail_in = data.size();

    char buf[65536];
    do {
        strm.next_out  = reinterpret_cast<uint8_t*>(buf);
        strm.avail_out = sizeof(buf);
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            lzma_end(&strm);
            throw std::runtime_error("compress_xz: lzma_code failed (" + std::to_string(ret) + ")");
        }
        out.insert(out.end(), buf, buf + sizeof(buf) - strm.avail_out);
    } while (ret != LZMA_STREAM_END);

    lzma_end(&strm);
    return out;
}

// ── compress_lzma_raw — liblzma LZMA alone (legacy MTK format) ───────────────
// libarchive не предоставляет API для точных параметров lc/lp/pb/dict_size,
// поэтому liblzma используется напрямую.
std::vector<char> compress_lzma_raw(const std::vector<char>& data,
                                     uint8_t lc, uint8_t lp, uint8_t pb,
                                     uint32_t dict_size) {
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_options_lzma opt{};
    if (lzma_lzma_preset(&opt, 6))
        throw std::runtime_error("compress_lzma_raw: failed to set preset");
    opt.lc = lc; opt.lp = lp; opt.pb = pb; opt.dict_size = dict_size;

    lzma_ret ret = lzma_alone_encoder(&strm, &opt);
    if (ret != LZMA_OK)
        throw std::runtime_error("compress_lzma_raw: encoder init failed (" + std::to_string(ret) + ")");

    std::vector<char> out;
    strm.next_in  = reinterpret_cast<const uint8_t*>(data.data());
    strm.avail_in = data.size();

    char buf[65536];
    do {
        strm.next_out  = reinterpret_cast<uint8_t*>(buf);
        strm.avail_out = sizeof(buf);
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            lzma_end(&strm);
            throw std::runtime_error("compress_lzma_raw: encoding error (" + std::to_string(ret) + ")");
        }
        out.insert(out.end(), buf, buf + sizeof(buf) - strm.avail_out);
    } while (ret != LZMA_STREAM_END);

    lzma_end(&strm);
    return out;
}
