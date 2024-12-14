#include "../include/main.hpp"

const uint32_t MD1IMG_MAGIC1 = 0x58881688;
const uint32_t MD1IMG_MAGIC2 = 0x58891689;

// Заголовки .gz и .xz
const uint16_t GZ_HEADER=0x1F8B;
const uint64_t XZ_HEADER=0xFD377A585A00;

const std::string FILE_MAP_MARKER = "md1_file_map";

#pragma pack(push, 1)
struct Header {
    uint32_t magic1;         // 4 bytes
    uint32_t data_size;      // 4 bytes
    char name[32];           // 32 bytes
    uint32_t base;           // 4 bytes
    uint32_t mode;           // 4 bytes
    uint32_t magic2;         // 4 bytes
    uint32_t data_offset;    // 4 bytes
    uint32_t hdr_version;    // 4 bytes
    uint32_t img_type;       // 4 bytes
    uint32_t img_list_end;   // 4 bytes
    uint32_t align_size;     // 4 bytes
    uint32_t dsize_extend;   // 4 bytes
    uint32_t maddr_extend;   // 4 bytes
    uint8_t reserved[432];  // 432 bytes, заполненные значением 0xFF
};
#pragma pack(pop)

// Приведение строки к нижнему регистру
std::string to_lowercase(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}


// Проверка, заканчивается ли строка на определенный суффикс
bool ends_with(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Проверка заголовка gzip файла (GZ_HEADER)
bool is_gz_format(const std::vector<char>& data) {
    return data.size() > 2 &&
           (static_cast<uint8_t>(data[0]) << 8 | static_cast<uint8_t>(data[1])) == GZ_HEADER;
}

// Проверка заголовка xz файла (XZ_HEADER)
bool is_xz_format(const std::vector<char>& data) {
    return data.size() > 6 &&
           (static_cast<uint64_t>(data[0]) << 40 |
            static_cast<uint64_t>(data[1]) << 32 |
            static_cast<uint64_t>(data[2]) << 24 |
            static_cast<uint64_t>(data[3]) << 16 |
            static_cast<uint64_t>(data[4]) << 8  |
            static_cast<uint64_t>(data[5])) == XZ_HEADER;
}


// Сжатие в GZ
std::vector<char> compress_gz(const std::vector<char>& data) {
    std::vector<char> compressed_data;
    z_stream zs{};
    deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

    zs.avail_in = data.size();
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));

    char outbuffer[32768];
    int ret;

    do {
        zs.avail_out = sizeof(outbuffer);
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        ret = deflate(&zs, Z_FINISH);

        compressed_data.insert(compressed_data.end(), outbuffer, outbuffer + sizeof(outbuffer) - zs.avail_out);
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("gzip compression failed.");
    }

    return compressed_data;
}

// Сжатие в XZ
std::vector<char> compress_xz(const std::vector<char>& data) {
    std::vector<char> compressed_data;
/*    lzma_stream strm = LZMA_STREAM_INIT;

    if (lzma_easy_encoder(&strm, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC64) != LZMA_OK) {
        throw std::runtime_error("xz compression failed to initialize.");
    }

    strm.next_in = reinterpret_cast<const uint8_t*>(data.data());
    strm.avail_in = data.size();

    char outbuffer[32768];
    lzma_ret ret;

    do {
        strm.avail_out = sizeof(outbuffer);
        strm.next_out = reinterpret_cast<uint8_t*>(outbuffer);
        ret = lzma_code(&strm, LZMA_FINISH);

        compressed_data.insert(compressed_data.end(), outbuffer, outbuffer + sizeof(outbuffer) - strm.avail_out);
    } while (ret == LZMA_OK);

    lzma_end(&strm);

    if (ret != LZMA_STREAM_END) {
        throw std::runtime_error("xz compression failed.");
    }
*/
    return compressed_data;
}


// Распаковка .gz файлов
std::vector<char> decompress_gz(const std::vector<char>& compressed_data) {
    std::vector<char> decompressed_data;
    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data.data()));
    stream.avail_in = compressed_data.size();

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Ошибка инициализации zlib");
    }

    char buffer[4096];
    do {
        stream.next_out = reinterpret_cast<Bytef*>(buffer);
        stream.avail_out = sizeof(buffer);

        int result = inflate(&stream, Z_NO_FLUSH);
        if (result == Z_STREAM_ERROR || result == Z_DATA_ERROR || result == Z_MEM_ERROR) {
            inflateEnd(&stream);
            throw std::runtime_error("Ошибка распаковки zlib");
        }

        decompressed_data.insert(decompressed_data.end(), buffer, buffer + (sizeof(buffer) - stream.avail_out));
    } while (stream.avail_out == 0);

    inflateEnd(&stream);
    return decompressed_data;
}

// Распаковка .xz файлов
std::vector<char> decompress_xz(const std::vector<char>& compressed_data) {
    std::vector<char> decompressed_data;
   /* lzma_stream stream = LZMA_STREAM_INIT;

    if (lzma_auto_decoder(&stream, UINT64_MAX, 0) != LZMA_OK) {
        throw std::runtime_error("Ошибка инициализации xz-utils");
    }

    stream.next_in = reinterpret_cast<const uint8_t*>(compressed_data.data());
    stream.avail_in = compressed_data.size();

    char buffer[4096];
    lzma_ret result;
    do {
        stream.next_out = reinterpret_cast<uint8_t*>(buffer);
        stream.avail_out = sizeof(buffer);

        result = lzma_code(&stream, LZMA_FINISH);
        if (result != LZMA_OK && result != LZMA_STREAM_END) {
            lzma_end(&stream);
            throw std::runtime_error("Ошибка распаковки xz-utils");
        }

        decompressed_data.insert(decompressed_data.end(), buffer, buffer + (sizeof(buffer) - stream.avail_out));
    } while (result != LZMA_STREAM_END);

    lzma_end(&stream);*/
    return decompressed_data;
}



// Функция для извлечения числового префикса из имени файла
int extract_number(const std::string &filename) {
    size_t pos = 0;
    int number = 0;

    // Извлекаем цифры из начала строки
    while (pos < filename.size() && std::isdigit(filename[pos])) {
        number = number * 10 + (filename[pos] - '0');
        ++pos;
    }
    return number;
}

// Функция для удаления порядкового номера из имени файла
std::string strip_number(const std::string &filename) {
    size_t pos = 0;
    while (pos < filename.size() && std::isdigit(filename[pos])) {
        ++pos;
    }
    if (pos < filename.size() && filename[pos] == '_') {
        return filename.substr(pos + 1);
    }
    return filename;
}

// Функция для чтения маппинга файлов
std::unordered_map<std::string, std::string> read_file_mapping(const std::string &path) {
    std::unordered_map<std::string, std::string> mapping;
    std::ifstream map_file(path);

    if (!map_file) {
        std::cerr << "Error opening map file: " << path << std::endl;
        return mapping;
    }

    std::string line;
    while (std::getline(map_file, line)) {
        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(delimiter_pos + 1);
            std::string value = line.substr(0, delimiter_pos);
            mapping[key] = value;
        }
    }
    return mapping;
}

// Функция для инициализации заголовка
void initialize_header(Header &header, const std::string &name, uint32_t data_size, uint32_t base) {
    header.magic1 = MD1IMG_MAGIC1;
    header.data_size = data_size;
    std::strncpy(header.name, name.c_str(), sizeof(header.name));
    header.base = base;
    header.mode = 0;
    header.magic2 = MD1IMG_MAGIC2;
    header.data_offset = sizeof(Header);
    header.hdr_version = 0;
    header.img_type = 0;
    header.img_list_end = 0;
    header.align_size = 0;
    header.dsize_extend = 0;
    header.maddr_extend = 0;
    std::memset(header.reserved, 0xFF, sizeof(header.reserved)); //забиваем FF как это в оригинальных файлах
}

// Компаратор для сортировки файлов
bool file_sort_comparator(const std::filesystem::path &file1, const std::filesystem::path &file2) {
    int base1 = extract_number(file1.filename().string());
    int base2 = extract_number(file2.filename().string());
    return base1 < base2;
}

// Функция для инициализации заголовка из файла meta_info
void initialize_header_from_file(Header &header, const std::unordered_map<std::string, std::string> &meta_data, uint32_t data_size) {
    header.magic1 = std::stoul(meta_data.at("magic1"), nullptr, 16);
    header.data_size = data_size;
    //header.data_size = std::stoul(meta_data.at("data_size"));
    std::strncpy(header.name, meta_data.at("name").c_str(), sizeof(header.name));
    header.base = std::stoul(meta_data.at("base"), nullptr, 16);
    header.mode = std::stoul(meta_data.at("mode"), nullptr, 16);
    header.magic2 = std::stoul(meta_data.at("magic2"), nullptr, 16);
    header.data_offset = std::stoul(meta_data.at("data_offset"), nullptr, 16);
    header.hdr_version = std::stoul(meta_data.at("hdr_version"), nullptr, 16);
    header.img_type = std::stoul(meta_data.at("img_type"), nullptr, 16);
    header.img_list_end = std::stoul(meta_data.at("img_list_end"), nullptr, 16);
    header.align_size = std::stoul(meta_data.at("align_size"), nullptr, 16);
    header.dsize_extend = std::stoul(meta_data.at("dsize_extend"), nullptr, 16);
    header.maddr_extend = std::stoul(meta_data.at("maddr_extend"), nullptr, 16);
    std::memset(header.reserved, 0xFF, sizeof(header.reserved)); //забиваем FF как это в оригинальных файлах
}

// Функция для чтения данных из файла meta_info
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> read_meta_info(const std::string &meta_info_path) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> meta_info_data;
    std::ifstream meta_file(meta_info_path);

    if (!meta_file) {
        std::cerr << "Error: unable to open meta_info file: " << meta_info_path << std::endl;
        return meta_info_data;  // Возвращаем пустую структуру в случае ошибки
    }

    std::string line;
    std::unordered_map<std::string, std::string> current_meta_data;
    std::string current_file;

    // Читаем данные по каждому файлу
    while (std::getline(meta_file, line)) {
        if (line.find("name=") == 0) {
            if (!current_file.empty()) {
                meta_info_data[current_file] = current_meta_data;
            }
            current_file = line.substr(5);  // Убираем "name="
            current_meta_data.clear();
        }
        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            current_meta_data[key] = value;
        }
    }

    // Добавляем последний файл в маппинг
    if (!current_file.empty()) {
        meta_info_data[current_file] = current_meta_data;
    }

    return meta_info_data;
}

// Функция для поиска соответствий с учётом регистра и префиксов
std::string find_mapped_name(const std::unordered_map<std::string, std::string> &file_mapping, const std::string &file_name) {
    // Приводим имя файла к нижнему регистру
    std::string lower_name = to_lowercase(file_name);

    // Формируем варианты имен для поиска с разными суффиксами
    std::string gz_name = lower_name + ".gz";
    std::string xz_name = lower_name + ".xz";

    // Ищем среди ключей в file_mapping, приведя их к нижнему регистру
    for (const auto& [key, value] : file_mapping) {
        std::string lower_key = to_lowercase(key);
        if (lower_key == lower_name || lower_key == gz_name || lower_key == xz_name) {
            return value;
        }
    }
    return ""; // Если соответствие не найдено
}



std::string find_compression_type(const std::unordered_map<std::string, std::string> &file_mapping, const std::string &file_name) {
    // Удаляем числовой префикс и приводим имя файла к нижнему регистру
    std::string stripped_name = to_lowercase(strip_number(file_name));

    for (const auto &[key, value] : file_mapping) {
        std::string key_lower = to_lowercase(key);

        // Если ключ в маппинге совпадает с именем файла без префикса
        if (stripped_name == key_lower.substr(0, key_lower.find_last_of("."))) {
            return key_lower;
        }
    }

    // Если соответствие не найдено, возвращаем пустую строку
    return "";
}


void pack_files(const std::string &input_dir, const std::string &output_file) {
    std::vector<std::filesystem::path> files;
    std::filesystem::path map_file_path;
    bool map_file_found = false;
    std::filesystem::path meta_info_path;
    bool meta_info_found = false;

    // Ищем файл с маппингом (md1_file_map) и meta_info
    for (const auto &entry : std::filesystem::directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("md1_file_map") != std::string::npos) {
                map_file_path = entry.path();
                map_file_found = true;
            } else if (filename == "meta_info") {
                meta_info_path = entry.path();
                meta_info_found = true;
            }
        }
    }

    // Если файл с маппингом не найден, возвращаем ошибку
    if (!map_file_found) {
        std::cerr << "Error: md1_file_map not found in directory: " << input_dir << std::endl;
        return;
    }

    // Чтение маппинга из файла
    std::unordered_map<std::string, std::string> file_mapping = read_file_mapping(map_file_path.string());

    // Чтение данных из meta_info, если он найден
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> meta_info_data;
    if (meta_info_found) {
        meta_info_data = read_meta_info(meta_info_path.string());
    }

    // Собираем список файлов для упаковки
    for (const auto &entry : std::filesystem::directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().filename().string() != "meta_info") {
            files.push_back(entry.path());
        }
    }

    // Сортируем файлы по именам
    std::sort(files.begin(), files.end(), file_sort_comparator);

    // Открываем выходной файл
    std::ofstream output(output_file, std::ios::binary);
    if (!output) {
        std::cerr << "Error opening output file: " << output_file << std::endl;
        return;
    }

    uint32_t base_address = 0;

    // Обрабатываем файлы
    for (const auto &file : files) {
        std::string file_name = file.filename().string();

        // Определяем нужный тип сжатия, если он указан в file_mapping
        std::string compression_type = find_compression_type(file_mapping, file_name);

        // Читаем содержимое файла
        std::ifstream input(file, std::ios::binary);
        if (!input) {
            std::cerr << "Error opening input file: " << file << std::endl;
            continue;
        }
        std::vector<char> file_data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

        // Извлекаем имя файла без префикса для поиска в meta_info
        std::string stripped_name = strip_number(file_name);

        // Ищем соответствие в file_mapping для mapped_name
        std::string mapped_name = find_mapped_name(file_mapping, stripped_name);
        if (mapped_name.empty()) {
            mapped_name = stripped_name; // Если соответствие не найдено, используем исходное имя
        }

        // Проверяем, нужно ли сжать файл
        if (!compression_type.empty()) {
            if (compression_type.ends_with(".gz")) {
                file_data = compress_gz(file_data);
            } else if (compression_type.ends_with(".xz")) {
                file_data = compress_xz(file_data);
            }
        }

        // Инициализируем заголовок
        Header header{};
        if (meta_info_found && meta_info_data.find(mapped_name) != meta_info_data.end()) {
            initialize_header_from_file(header, meta_info_data[mapped_name], file_data.size());
        } else {
            initialize_header(header, mapped_name, file_data.size(), base_address);
        }

        // Записываем заголовок и данные в выходной файл
        output.write(reinterpret_cast<const char *>(&header), sizeof(Header));
        output.write(file_data.data(), file_data.size());

        std::cout << "Packed: " << file.filename().string() << " as " << mapped_name << " (Size: " << file_data.size() << " bytes)" << std::endl;

        // Выравнивание по 16 байт
        uint32_t padding = 16 - (file_data.size() % 16);
        if (padding != 16) {
            std::vector<char> padding_data(padding, 0);
            output.write(padding_data.data(), padding_data.size());
        }

        // Обновляем base_address
        base_address += file_data.size() + sizeof(Header) + padding;
    }

    std::cout << "Packing complete: " << output_file << std::endl;
}



// Функция для поиска "md1_file_map" с конца файла
std::streampos find_file_map_offset(const std::string &path) {
    const size_t buffer_size = 16384;  // Буфер 16КБ
    std::ifstream input_file(path, std::ios::binary | std::ios::ate);
    if (!input_file) {
        std::cerr << "Error opening file: " << path << std::endl;
        return -1;
    }

    std::streampos file_size = input_file.tellg();
    if (file_size < buffer_size) {
        std::cerr << "File too small to contain md1_file_map" << std::endl;
        return -1;
    }

    std::vector<char> buffer(buffer_size);
    std::streampos pos = file_size;

    while (pos > 0) {
        size_t read_size = std::min(buffer_size, static_cast<size_t>(pos));
        pos = pos - static_cast<std::streampos>(read_size);
        input_file.seekg(pos);
        input_file.read(buffer.data(), read_size);

        // Ищем строку "md1_file_map" в буфере
        std::string data(buffer.data(), read_size);
        size_t found = data.rfind(FILE_MAP_MARKER);
        if (found != std::string::npos) {
            return pos + static_cast<std::streampos>(found);
        }
    }

    std::cerr << "md1_file_map not found" << std::endl;
    return -1;
}

// Функция для чтения маппинга файлов после нахождения "md1_file_map"
std::unordered_map<std::string, std::string> read_file_mapping(const std::string &path, std::streampos map_offset) {
    std::unordered_map<std::string, std::string> mapping;
    std::ifstream input_file(path, std::ios::binary);
    if (!input_file) {
        std::cerr << "Error opening file: " << path << std::endl;
        return mapping;
    }

    input_file.seekg(map_offset + static_cast<std::streamoff>(504)); //504 bytes - остатки от header md1_file_ma, которые надо пропустить, иначе не всё переводится в нормальные файлы
    std::string buffer(std::istreambuf_iterator<char>(input_file), {});

    std::stringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            mapping[key] = value;
        }
    }

    return mapping;
}

void process_file(const std::string &input_path, const std::string &output_dir) {
    // Ищем файл маппинга
    std::unordered_map<std::basic_string<char>, std::basic_string<char>> file_mapping;
    std::streampos map_offset = find_file_map_offset(input_path);
    if (map_offset != -1) {
        file_mapping = read_file_mapping(input_path, map_offset);
    }
    std::filesystem::path input_file_path(input_path);
    std::error_code ec;
    auto file_size = std::filesystem::file_size(input_file_path, ec);
    if (ec) {
        std::cerr << "Error getting file size: " << ec.message() << std::endl;
        return;
    }

    std::ifstream input_file(input_path, std::ios::binary);
    if (!input_file) {
        std::cerr << "Error opening file: " << input_path << std::endl;
        return;
    }

    size_t offset = 0;
    std::filesystem::create_directories(output_dir); // Создаем выходную папку, если она не существует

    int file_counter = 1;  // Добавляем переменную для отслеживания порядка файлов

    // Открываем файл для записи meta_info
    std::ofstream meta_info_file(output_dir + "/meta_info");
    if (!meta_info_file) {
        std::cerr << "Error opening meta_info file for writing" << std::endl;
        return;
    }

    while (offset < file_size) {
        input_file.seekg(offset, std::ios::beg);
        Header header{};
        input_file.read(reinterpret_cast<char *>(&header), sizeof(Header));

        if (header.magic1 != MD1IMG_MAGIC1 || header.magic2 != MD1IMG_MAGIC2) {
            break;
        }

        std::string name(header.name, strnlen(header.name, sizeof(header.name)));
        std::cout << "Found " << name << "@0x" << std::hex << header.base << ",0x" << header.data_size << std::dec << std::endl;

        // Запись заголовка в meta_info
        meta_info_file << "name=" << name << "\n";
        meta_info_file << "magic1=0x" << std::hex << header.magic1 << std::dec << "\n";
        meta_info_file << "data_size=" << header.data_size << "\n";
        meta_info_file << "base=0x" << std::hex << header.base << std::dec << "\n";
        meta_info_file << "mode=0x" << std::hex << header.mode << std::dec << "\n";
        meta_info_file << "magic2=0x" << std::hex << header.magic2 << std::dec << "\n";
        meta_info_file << "data_offset=0x" << std::hex << header.data_offset << std::dec << "\n";
        meta_info_file << "hdr_version=0x" << std::hex << header.hdr_version << std::dec << "\n";
        meta_info_file << "img_type=0x" << std::hex << header.img_type << std::dec << "\n";
        meta_info_file << "img_list_end=0x" << std::hex << header.img_list_end << std::dec << "\n";
        meta_info_file << "align_size=0x" << std::hex << header.align_size << std::dec << "\n";
        meta_info_file << "dsize_extend=0x" << std::hex << header.dsize_extend << std::dec << "\n";
        meta_info_file << "maddr_extend=0x" << std::hex << header.maddr_extend << std::dec << "\n\n";

        std::string output_name;
        if (file_mapping.find(name) != file_mapping.end()) {
            output_name = output_dir + "/" + std::to_string(file_counter) + "_" + file_mapping.at(name);
        } else {
            output_name = output_dir + "/" + std::to_string(file_counter) + "_" + name;
        }

        offset += header.data_offset;
        input_file.seekg(offset, std::ios::beg);

        std::vector<char> data(header.data_size);
        input_file.read(data.data(), header.data_size);

        // Проверка формата файла для распаковки
        std::string output_name1 = to_lowercase(output_name);
        if (ends_with(output_name1, ".gz") && is_gz_format(data)) {
            output_name = output_name.substr(0, output_name.size() - 3);  // Убираем ".gz" из имени
            auto decompressed_data = decompress_gz(data);
            std::ofstream(output_name, std::ios::binary).write(decompressed_data.data(), decompressed_data.size());
            std::cout << "Decompressed " << name << " to " << output_name << std::endl;
        } else if (ends_with(output_name1, ".xz") && is_xz_format(data)) {
            output_name = output_name.substr(0, output_name.size() - 3);  // Убираем ".xz" из имени
            auto decompressed_data = decompress_xz(data);
            std::ofstream(output_name, std::ios::binary).write(decompressed_data.data(), decompressed_data.size());
            std::cout << "Decompressed " << name << " to " << output_name << std::endl;
        } else {
            // Если распаковка не требуется, сохраняем файл как есть
            std::ofstream output_file(output_name, std::ios::binary);
            if (!output_file) {
                std::cerr << "Error opening output file for writing: " << output_name << std::endl;
                return;
            }
            output_file.write(data.data(), header.data_size);
            std::cout << name << " written to " << output_name << std::endl;
        }

        offset += header.data_size;
        if (offset % 16 != 0) {
            offset += 16 - (offset % 16);
        }
        file_counter += 1;
    }

    // Закрываем файл meta_info
    meta_info_file.close();
    std::cout << "meta_info file written to " << output_dir << "/meta_info" << std::endl;
}

void MD1IMG::show_help() {
    std::cout <<
              R"***(
md1img

Usage:

  md1img <pack|unpack> <input> [output_dir]
)***";
}

ParseResult MD1IMG::parse_cmd_line(int argc, char *argv[]) {
    if (argc < 2) {
        show_help();
        return ParseResult::not_enough;
    }

    mode = argv[0];
    input_path = argv[1];

    if (mode == "unpack") {
        if (argc > 2) {
            output_dir = argv[2];
        } else {
            std::filesystem::path input_file_path(input_path);
            output_dir = input_file_path.parent_path() / input_file_path.stem();
        }
    }

    return ParseResult::ok;
}

ProcessResult MD1IMG::process() {

    if (mode == "unpack") {
        process_file(input_path, output_dir.string());
    }
    else if (mode == "pack") {
        std::filesystem::path input_dir_path(input_path);

        if (!std::filesystem::exists(input_dir_path) || !std::filesystem::is_directory(input_dir_path)) {
            std::cerr << "Error: input directory does not exist or is not a directory." << std::endl;
            return ProcessResult::open_error;
        }

        // Формируем имя выходного файла: <имя_папки>-new.img
        std::filesystem::path output_file_path = input_dir_path.parent_path() / (input_dir_path.stem().string() + "-new.img");

        // Пакуем файлы в новый образ
        pack_files(input_path, output_file_path.string());
    } else {
        std::cerr << "Invalid mode: " << mode << ". Use 'pack' or 'unpack'." << std::endl;
        return ProcessResult::breaked;
    }

    return ProcessResult::ok;
}