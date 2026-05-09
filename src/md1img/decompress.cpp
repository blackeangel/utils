#include "main.h"

// ── decompress_gz — zlib inflate (gzip format) ────────────────────────────────
std::vector<char> decompress_gz(const std::vector<char>& data) {
    z_stream strm{};
    strm.next_in  = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    strm.avail_in = static_cast<uInt>(data.size());

    // 16 + MAX_WBITS → accept gzip wrapper
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
        throw std::runtime_error("decompress_gz: inflateInit2 failed");

    std::vector<char> out;
    char buf[65536];
    int ret;
    do {
        strm.next_out  = reinterpret_cast<Bytef*>(buf);
        strm.avail_out = sizeof(buf);
        ret = inflate(&strm, Z_SYNC_FLUSH);
        if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR || ret == Z_STREAM_ERROR) {
            inflateEnd(&strm);
            throw std::runtime_error("decompress_gz: inflate error (" + std::to_string(ret) + ")");
        }
        out.insert(out.end(), buf, buf + sizeof(buf) - strm.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return out;
}

// ── decompress_xz — liblzma auto-decoder ─────────────────────────────────────
// lzma_auto_decoder обрабатывает оба формата:
//   — XZ-контейнер     (magic FD 37 7A 58 5A 00)
//   — LZMA alone/raw   (legacy MTK формат, magic 5D 00 ...)
std::vector<char> decompress_xz(const std::vector<char>& data) {
    lzma_stream strm = LZMA_STREAM_INIT;

    // UINT64_MAX = нет ограничения на размер вывода
    lzma_ret ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK)
        throw std::runtime_error("decompress_xz: lzma_auto_decoder failed (" + std::to_string(ret) + ")");

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
            throw std::runtime_error("decompress_xz: lzma_code error (" + std::to_string(ret) + ")");
        }
        out.insert(out.end(), buf, buf + sizeof(buf) - strm.avail_out);
    } while (ret != LZMA_STREAM_END);

    lzma_end(&strm);
    return out;
}
