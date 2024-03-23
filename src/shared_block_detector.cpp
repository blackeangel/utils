#include "../include/main.hpp"
void SharedBlockDetector::show_help()
{
    std::cout <<
              R"***(
shared_block_detector

Usage:
  utils shared_block_detector <file>
)***";
}

// Функция для определения, является ли файл форматом ext4
bool is_ext4(const std::vector<char>& buffer) {
    // Приводим буфер к типу ext2_super_block, чтобы можно было обращаться к полям
    const ext2_super_block* superblock = reinterpret_cast<const ext2_super_block*>(buffer.data() + 1024);

    // Проверяем сигнатуру ext4
    if (superblock->s_magic == EXT2_SUPER_MAGIC) {
        // Проверяем наличие флага поддержки shared blocks
        if (superblock->s_feature_ro_compat & EXT4_FEATURE_RO_COMPAT_SHARED_BLOCKS) {
            std::cout << "true" << std::endl;
            return true;
        }
    }

    return false;
}

// Функция для определения, является ли файл форматом Android sparse
bool is_sparse(const std::vector<char>& buffer) {
    uint32_t sparse_offset = 0;

    if (buffer.size() < sizeof(uint32_t)) {
        std::cerr << "Error: Buffer size is too small." << std::endl;
        return false;
    }

    for (size_t i = 0; i < buffer.size() - sizeof(uint32_t); ++i) {
        const uint32_t* magic = reinterpret_cast<const uint32_t*>(&buffer[i]);
        if (*magic == SPARSE_HEADER_MAGIC) {
            //std::cout << "Android sparse format detected." << std::endl;
            return true;
        }
    }

    return false;
}

// Функция для определения, является ли файл форматом sparse ext4
bool is_sparse_ext4(const std::vector<char>& buffer) {
    size_t offset = 0;
    size_t ext4_header_pos = 0;
    bool ext4_found = false;

    // Поиск сигнатуры файловой системы ext4
    for (size_t i = 0; i < buffer.size() - sizeof(uint32_t); ++i) {
        const ext2_super_block* superblock = reinterpret_cast<const ext2_super_block*>(&buffer[i]);
        if (superblock->s_magic == EXT2_SUPER_MAGIC) {
            //std::cout << "EXT4 format detected." << std::endl;
            ext4_header_pos = i;
            ext4_found = true;
            break;
        }
    }

        if (ext4_found) {
            // Вычисляем смещение от начала файла до сигнатуры ext4
            offset = ext4_header_pos - 1080;
        }else{
            std::cout << "It's not a ext4 format!" << std::endl;
            return false;
        }
        // Перемещаемся к месту, где должен быть заголовок ext4
        const ext2_super_block* superblock = reinterpret_cast<const ext2_super_block*>(buffer.data() + offset);

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
ParseResult SharedBlockDetector::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    filename = argv[0];
    return ParseResult::ok;
}

ProcessResult SharedBlockDetector::process()
{
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
    if (is_ext4(buffer)) {
        return ProcessResult::ok;
    }

    // Если не найдены shared blocks
    std::cout << "false" << std::endl;
    file.close();
    return ProcessResult::breaked;
}

