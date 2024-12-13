#include "../include/main.hpp"

void SharedBlockDetector::show_help() {
    std::cout <<
              R"***(
shared_block_detector

Usage:

  shared_block_detector <file>
)***";
}

// Функция для определения, является ли файл форматом ext4
bool is_ext4_shared(const std::vector<char> &buffer) {
    if (is_ext4(buffer)) {
        // Приводим буфер к типу ext2_super_block, чтобы можно было обращаться к полям
        const auto *superblock = reinterpret_cast<const ext2_super_block *>(buffer.data() + 1024);
        // Проверяем наличие флага поддержки shared blocks
        if (superblock->s_feature_ro_compat & EXT4_FEATURE_RO_COMPAT_SHARED_BLOCKS) {
            std::cout << "true" << std::endl;
            return true;
        }
    }
    return false;
}

// Функция для определения, является ли файл форматом sparse ext4
bool is_sparse_ext4(std::vector<char> &buffer) {
    size_t offset_pos = 0;
    size_t ext4_header_pos = 0;
    bool ext4_found = false;
    auto EXT4_MAG_HEX = "53EF";
    std::vector<char> search_str;       // искомая строка
    long long start_offset = 0;         // начальное смещение
    long long search_size = LLONG_MAX;  // размер блока поиска
    long long max_count = LLONG_MAX;    // максимальное кол-во смещений
    bool forward = true;                // поиск с начала (иначе с конца)
    bool hex_offsets = true;            // 16-ричный вывод
    bool hex_uppercase = false;         // 16-ричный вывод в верхнем регистре
    bool hex_prefix = false;            // добавлять префикс для 16-ричных чисел
    char sep_char = '\n';               // разделитель смещений
    bool show_stat = true;              // вывод статистики
    parse_hexstring(EXT4_MAG_HEX, search_str);
    auto result = find_in_vector(
            [&](std::vector<char> &, long long offset, long long number) {
                if (number > 1) { std::cout << sep_char; }
                if (hex_offsets && hex_prefix) { std::cout << "0x"; }
                ext4_header_pos = offset;
                ext4_found = true;
                return ProcessResult::ok;
            }, buffer, search_str, forward, start_offset, search_size, 1);
    if (ext4_found) {
        // Вычисляем смещение от начала файла до сигнатуры ext4
        offset_pos = ext4_header_pos - 1080;
    } else {
        std::cout << "It's not a ext4 format!" << std::endl;
        return false;
    }
    std::vector<char> new_buffer;
    std::copy(buffer.begin() + offset_pos, buffer.end(), std::back_inserter(new_buffer));

    // Перемещаемся к месту, где должен быть заголовок ext4
    const auto *superblock = reinterpret_cast<const ext2_super_block *>(new_buffer.data()+1024);

    // Проверяем, соответствует ли заголовок ext4
    if (superblock->s_magic == EXT2_SUPER_MAGIC) {
        // Проверяем наличие флага shared blocks
        if (superblock->s_feature_ro_compat & EXT4_FEATURE_RO_COMPAT_SHARED_BLOCKS) {
            std::cout << "true" << std::endl;
            return true;
        }
    }

    return false;
}

ParseResult SharedBlockDetector::parse_cmd_line(int argc, char *argv[]) {
    if (argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    filename = argv[0];
    return ParseResult::ok;
}

ProcessResult SharedBlockDetector::process() {
    // Проверка на наличие файла и его доступность
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Error: Unable to open file." << std::endl;
        return ProcessResult::open_error;
    }

    // Чтение первых 4 Мб файла
    std::vector<char> buffer(4194304);
    file.read(buffer.data(), 4194304);
    file.close();

    // Проверка на формат sparse
    if (is_sparse(buffer)) {
        // Проверка на формат sparse ext4
        if (is_sparse_ext4(buffer)) {
            return ProcessResult::ok;
        }
    }

    // Проверка на формат ext4
    if (is_ext4_shared(buffer)) {
        return ProcessResult::ok;
    }

    // Если не найдены shared blocks
    std::cout << "false" << std::endl;
    file.close();
    return ProcessResult::breaked;
}
