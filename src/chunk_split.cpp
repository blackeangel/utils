#include "../include/main.hpp"

constexpr size_t COPY_BUF_SIZE = 8 * 1024 * 1024;

// Конвертация в Little Endian
constexpr uint16_t to_le16(uint16_t value) {
    return std::endian::native == std::endian::little ? value : __builtin_bswap16(value);
}

constexpr uint32_t to_le32(uint32_t value) {
    return std::endian::native == std::endian::little ? value : __builtin_bswap32(value);
}

void CHUNK_SPLIT::show_help() {
    std::cout << R"***(

    chunk_split

    Usage:

    chunk_split [OPTIONS] <raw_image_file>

    The <raw_image_file> will be split into as many sparse
    files as needed.  Each sparse file will contain a single
    DONT CARE chunk to offset to the correct block and then
    a single RAW chunk containing a portion of the data from
    the raw image file.  The sparse files will be named by
    appending a number to the name of the raw image file.

    OPTIONS (Defaults are enclosed by square brackets):
      -s SUFFIX      Format appended number with SUFFIX, default '.chunk.%02d'
      -B SIZE        Use a block size of SIZE, default 4K
      -C SIZE        Use a chunk size of SIZE
      -P PART        Use the number of parts
    SIZE is a decimal integer that may optionally be
    followed by a suffix that specifies a multiplier for
    the integer:
           c         1 byte (the default when omitted)
           w         2 bytes
           b         512 bytes
           kB        1000 bytes
           K         1024 bytes
           MB        1000*1000 bytes
           M         1024*1024 bytes
           GB        1000*1000*1000 bytes
           G         1024*1024*1024 bytes
)***";
}

// Функция для парсинга размера с учетом суффиксов
int parse_size(std::string size_str, size_t &size) {
    constexpr size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();

    if (size_str.empty()) {
        std::cerr << "Error: Input string is empty.\n";
        return 1;
    }

    size_t idx = 0;
    unsigned long long value = 0;

    try {
        value = std::stoull(size_str, &idx);
    } catch (const std::invalid_argument &) {
        std::cerr << "Error: Invalid number format in input string.\n";
        return 1;
    } catch (const std::out_of_range &) {
        std::cerr << "Error: Number out of range.\n";
        return 1;
    }

    if (idx == size_str.size()) {
        if (value > MAX_SIZE_T) {
            std::cerr << "Error: Value exceeds size_t range.\n";
            return 1;
        }
        size = static_cast<size_t>(value);
        return 0;
    }

    std::string_view suffix = size_str.substr(idx);
    size_t mult = 0;

    if (suffix == "c") {
        mult = 1;
    } else if (suffix == "w") {
        mult = 2;
    } else if (suffix == "b") {
        mult = 512;
    } else if (suffix == "kB") {
        mult = 1000;
    } else if (suffix == "K") {
        mult = 1024;
    } else if (suffix == "MB") {
        mult = 1000 * 1000;
    } else if (suffix == "M") {
        mult = 1024 * 1024;
    } else if (suffix == "GB") {
        mult = 1000 * 1000 * 1000;
    } else if (suffix == "G") {
        mult = 1024 * 1024 * 1024;
    } else {
        std::cerr << "Error: Unsupported suffix '" << suffix << "'.\n";
        return 1;
    }

    if (value > MAX_SIZE_T / mult) {
        std::cerr << "Error: Value too large for multiplication.\n";
        return 1;
    }

    size = static_cast<size_t>(value * mult);
    return 0;
}

// Формирование имени файла на основе шаблона и номера
std::string format_file_name(const std::string &template_str, const std::string &base_name, const std::string &ext, int number) {
    std::ostringstream oss;
    size_t i = 0;
    while (i < template_str.size()) {
        if (template_str[i] == '%' && i + 1 < template_str.size() && template_str[i + 1] == 'd') {
            // Найдено "%d", подставляем число
            oss << number;
            i += 2;
        } else if (template_str[i] == '%' && i + 1 < template_str.size() && template_str[i + 1] == '0' &&
                   std::isdigit(template_str[i + 2])) {
            // Найдено "%0<number>d", например, "%03d"
            size_t j = i + 2;
            while (j < template_str.size() && std::isdigit(template_str[j])) {
                ++j;
            }
            if (j < template_str.size() && template_str[j] == 'd') {
                int width = std::stoi(template_str.substr(i + 2, j - (i + 2)));
                oss << std::setfill('0') << std::setw(width) << number;
                i = j + 1; // Пропускаем 'd'
            } else {
                throw std::invalid_argument("Invalid format string: " + template_str);
            }
        } else {
            oss << template_str[i];
            ++i;
        }
    }
    return base_name + oss.str() + ext;
}

// Копирование данных между файлами
void copy_file_data(std::istream &in, std::ostream &out, size_t size) {
    std::vector<char> buffer(COPY_BUF_SIZE);
    while (size > 0) {
        size_t chunk = std::min(size, buffer.size());
        in.read(buffer.data(), chunk);
        if (!in) throw std::runtime_error("Error reading input file");
        out.write(buffer.data(), chunk);
        if (!out) throw std::runtime_error("Error writing to output file");
        size -= chunk;
    }
}

// Основная функция обработки файла
void process_file(const std::string &input_path, const std::string &output_dir, const std::string &suffix, size_t block_size, size_t chunk_size) {
    // Открываем входной файл
    std::ifstream in_file(input_path, std::ios::binary);
    if (!in_file) throw std::runtime_error("Cannot open input file");

    auto file_size = std::filesystem::file_size(input_path);
    if (file_size % block_size != 0) {
        throw std::runtime_error("Input file size is not a multiple of block size");
    }

    if (chunk_size % block_size != 0) {
        throw std::runtime_error("Chunk size must be a multiple of block size");
    }

    // Извлекаем базовое имя файла и расширение
    std::string base_name = std::filesystem::path(input_path).stem().string();
    std::string ext = std::filesystem::path(input_path).extension().string();

    size_t left_to_write = file_size;
    size_t file_count = 0;

    while (left_to_write > 0) {
        // Формируем имя выходного файла с использованием шаблона
        std::string out_file_name = format_file_name(suffix, base_name, ext, file_count++);
        std::string out_path = (std::filesystem::path(output_dir) / out_file_name).string();

        std::ofstream out_file(out_path, std::ios::binary);
        if (!out_file) throw std::runtime_error("Cannot create output file: " + out_path);

        sparse_header_t sparse_hdr{};
        sparse_hdr.magic = to_le32(SPARSE_HEADER_MAGIC);
        sparse_hdr.major_version = to_le16(1);
        sparse_hdr.minor_version = to_le16(0);
        sparse_hdr.file_hdr_sz = to_le16(sizeof(sparse_header_t));
        sparse_hdr.chunk_hdr_sz = to_le16(sizeof(chunk_header_t));
        sparse_hdr.blk_sz = to_le32(block_size);
        sparse_hdr.total_chunks = to_le32(2);
        sparse_hdr.image_checksum = to_le32(0);

        chunk_header_t dont_care_hdr{}, raw_hdr{};
        dont_care_hdr.chunk_type = to_le16(CHUNK_TYPE_DONT_CARE);
        dont_care_hdr.total_sz = to_le32(sizeof(chunk_header_t));

        size_t to_write = std::min(chunk_size, left_to_write);
        raw_hdr.chunk_type = to_le16(CHUNK_TYPE_RAW);
        raw_hdr.chunk_sz = to_le32(to_write / block_size);
        raw_hdr.total_sz = to_le32(to_write + sizeof(chunk_header_t));

        // Запись заголовков
        out_file.write(reinterpret_cast<char *>(&sparse_hdr), sizeof(sparse_hdr));
        out_file.write(reinterpret_cast<char *>(&dont_care_hdr), sizeof(dont_care_hdr));
        out_file.write(reinterpret_cast<char *>(&raw_hdr), sizeof(raw_hdr));

        // Копируем данные из входного файла
        copy_file_data(in_file, out_file, to_write);

        left_to_write -= to_write;
        std::cout << "Created file: " << out_path << "\n";
        std::flush(std::cout);
    }

}

ParseResult CHUNK_SPLIT::parse_cmd_line(int argc, char *argv[]) {

    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-s" && i + 1 < argc) {
            suffix = argv[++i];
        } else if (arg == "-B" && i + 1 < argc) {
            block_size_str = argv[++i];
        } else if (arg == "-C" && i + 1 < argc) {
            chunk_size_str = argv[++i];
        } else if (arg == "-P" && i + 1 < argc) {
            parts_count = std::stoull(argv[++i]);
        } else if (arg[0] != '-' && input_path.empty()) {
            input_path = arg;
        } else if (arg[0] != '-' && output_dir.empty()) {
            output_dir = arg;
        } else {
            show_help();
            return ParseResult::not_enough;
        }
    }

    /*int opt;
    // Парсинг аргументов командной строки
    opt = getopt(argc, argv, "s:B:C:P:");
    while ( opt != -1) {
        switch (opt) {
            case 's': suffix = optarg;
                break;
            case 'B': block_size_str = optarg;
                break;
            case 'C': chunk_size_str = optarg;
                break;
            case 'P': parts_count = std::stoull(optarg);
                break;
            default:
                show_help();
                return ParseResult::not_enough;
        }
    }
    if (optind >= argc) {
        //std::cerr << "Missing input file\n";
        show_help();
        return ParseResult::not_enough;
    }
    input_path = argv[optind];
*/
    return ParseResult::ok;
}


  ProcessResult CHUNK_SPLIT::process() {
    if (parse_size(block_size_str, block_size)) {
        std::cerr << "Failed to parse block size.\n";
        return ProcessResult::breaked;
    }

    if (!chunk_size_str.empty()) {
        if (parse_size(chunk_size_str, chunk_size)) {
            std::cerr << "Failed to parse chunk size.\n";
            return ProcessResult::breaked;
        }
    }


    // Если выходная папка не указана, используем имя входного файла для создания папки
    if (output_dir.empty()) {
        output_dir = std::filesystem::path(input_path).parent_path() / std::filesystem::path(input_path).stem();
    }

    try {
        auto file_size = std::filesystem::file_size(input_path);

        if (chunk_size > 0 && parts_count > 0) {
            throw std::runtime_error("-C (chunk size) and -P (parts count) options are mutually exclusive");
        }

        // Вычисляем chunk_size на основе parts_count, если он задан
        if (parts_count > 0) {
            // Вычисление размера одной части
            chunk_size = (file_size + parts_count - 1) / parts_count; // ceil division
        }

        if (chunk_size == 0) {
            std::cerr << "Either chunk size or number of parts must be specified\n";
            return ProcessResult::breaked;
        }

        // Проверка кратности block_size
        if (chunk_size % block_size != 0) {
            size_t remainder = chunk_size % block_size;
            chunk_size += block_size - remainder; // Округляем до ближайшего большего значения кратного block_size

            if (chunk_size > file_size && parts_count > 0) {
                throw std::runtime_error("Calculated chunk size exceeds file size, unable to split into parts");
            }
        }

        // Проверка на минимальный размер последней части
        if (parts_count > 0 && (file_size % chunk_size != 0)) {
            size_t last_chunk_size = file_size - (chunk_size * (file_size / chunk_size));
            if (last_chunk_size > 0 && last_chunk_size < block_size) {
                throw std::runtime_error("Last chunk size is smaller than block size");
            }
        }

        // Создаем выходную папку (включая промежуточные директории)
        if (!exists(output_dir)) {
            std::filesystem::create_directories(output_dir);
        }
        process_file(input_path, output_dir.string(), suffix, block_size, chunk_size);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return ProcessResult::breaked;
    }

    return ProcessResult::ok;
}
