#include "main.h"
#include <archive.h>
#include <archive_entry.h>

// ─── Вспомогательная функция: сжатие через libarchive в память ────────────────
static std::vector<char> la_compress_to_memory(
        const std::vector<char>& data, int filter, int level)
{
    size_t buf_size = data.size() + data.size() / 4 + 65536;
    std::vector<char> out(buf_size);
    size_t used = 0;

    struct archive* a = archive_write_new();
    archive_write_add_filter(a, filter);
    archive_write_set_format_raw(a);
    archive_write_set_filter_option(a, nullptr, "compression-level",
                                    std::to_string(level).c_str());

    if (archive_write_open_memory(a, out.data(), out.size(), &used) != ARCHIVE_OK)
        throw std::runtime_error(std::string("libarchive compress open: ") +
                                 archive_error_string(a));

    struct archive_entry* entry = archive_entry_new();
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_size(entry, static_cast<la_int64_t>(data.size()));
    archive_entry_set_pathname(entry, "data");
    archive_write_header(a, entry);
    archive_entry_free(entry);

    if (archive_write_data(a, data.data(), data.size()) < 0)
        throw std::runtime_error(std::string("libarchive compress write: ") +
                                 archive_error_string(a));

    archive_write_close(a);
    archive_write_free(a);
    out.resize(used);
    return out;
}

// Сжатие в GZ — через libarchive (заменяет прямой zlib deflate)
std::vector<char> compress_gz(const std::vector<char>& data, int level) {
    return la_compress_to_memory(data, ARCHIVE_FILTER_GZIP, level);
}

// Сжатие в XZ-контейнер — через libarchive
std::vector<char> compress_xz(const std::vector<char>& data, int level) {
    return la_compress_to_memory(data, ARCHIVE_FILTER_XZ, level);
}

// Сжатие в LZMA raw stream (старый формат без XZ-контейнера).
// libarchive не предоставляет API для точных параметров lc/lp/pb/dict_size,
// поэтому эта функция по-прежнему использует liblzma напрямую —
// это единственное оставшееся прямое использование liblzma в md1img.
std::vector<char> compress_lzma_raw(const std::vector<char>& data,
                                     uint8_t lc, uint8_t lp, uint8_t pb,
                                     uint32_t dict_size) {
    std::vector<char> compressed;
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_options_lzma opt{};
    if (lzma_lzma_preset(&opt, 6))
        throw std::runtime_error("lzma raw: failed to set preset");
    opt.lc = lc; opt.lp = lp; opt.pb = pb; opt.dict_size = dict_size;

    if (lzma_alone_encoder(&strm, &opt) != LZMA_OK)
        throw std::runtime_error("lzma raw: encoder init failed");

    strm.next_in  = reinterpret_cast<const uint8_t*>(data.data());
    strm.avail_in = data.size();
    char buf[32768]; lzma_ret ret;
    do {
        strm.avail_out = sizeof(buf);
        strm.next_out  = reinterpret_cast<uint8_t*>(buf);
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            lzma_end(&strm);
            throw std::runtime_error("lzma raw: encoding error (" + std::to_string(ret) + ")");
        }
        compressed.insert(compressed.end(), buf, buf + sizeof(buf) - strm.avail_out);
    } while (ret != LZMA_STREAM_END);
    lzma_end(&strm);
    return compressed;
}
