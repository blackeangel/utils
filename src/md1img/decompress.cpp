#include "main.h"
#include <archive.h>
#include <archive_entry.h>

// ─── Вспомогательная функция: распаковка через libarchive из памяти ───────────
static std::vector<char> la_decompress_from_memory(
        const std::vector<char>& compressed_data, int filter)
{
    struct archive* a = archive_read_new();
    // Route to the correct filter function by ID
    if (filter == ARCHIVE_FILTER_GZIP)
        archive_read_support_filter_gzip(a);
    else if (filter == ARCHIVE_FILTER_XZ)
        archive_read_support_filter_xz(a);
    else
        archive_read_support_filter_all(a);
    archive_read_support_format_raw(a);

    if (archive_read_open_memory(a,
            const_cast<char*>(compressed_data.data()),
            compressed_data.size()) != ARCHIVE_OK) {
        std::string err = archive_error_string(a);
        archive_read_free(a);
        throw std::runtime_error("libarchive decompress open: " + err);
    }

    struct archive_entry* entry;
    if (archive_read_next_header(a, &entry) != ARCHIVE_OK) {
        std::string err = archive_error_string(a);
        archive_read_free(a);
        throw std::runtime_error("libarchive decompress header: " + err);
    }

    std::vector<char> out;
    char buf[65536];
    la_ssize_t n;
    while ((n = archive_read_data(a, buf, sizeof(buf))) > 0)
        out.insert(out.end(), buf, buf + n);

    if (n < 0) {
        std::string err = archive_error_string(a);
        archive_read_free(a);
        throw std::runtime_error("libarchive decompress read: " + err);
    }

    archive_read_close(a);
    archive_read_free(a);
    return out;
}

// Распаковка GZ — через libarchive (заменяет прямой zlib inflate)
std::vector<char> decompress_gz(const std::vector<char>& data) {
    return la_decompress_from_memory(data, ARCHIVE_FILTER_GZIP);
}

// Распаковка XZ/LZMA (auto-detect) — через libarchive
// libarchive read_support_filter_xz автоматически обрабатывает:
//   — XZ-контейнер (magic FD 37 7A 58 5A 00)
//   — legacy LZMA raw stream (magic 5D 00 ... / props byte)
// Поведение аналогично lzma_auto_decoder из liblzma.
std::vector<char> decompress_xz(const std::vector<char>& data) {
    return la_decompress_from_memory(data, ARCHIVE_FILTER_XZ);
}
