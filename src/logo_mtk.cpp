#include "../include/main.hpp"

struct MTK_logo {
    struct {
        uint8_t start[4];               // offset 0x0 start file                            |
        uint32_t total_blocks[1];       // offset 0x04 total_blocks too?                    |
        char magic[4];                  //offset 0x08 'logo'                                |=>header 512 bytes
        uint8_t hvost[500];             //ofsset 0x13..0x199 any data in header            _|
    } header;
    uint32_t count_pictures[1];         //offset 0x200 number of pictures (little-endian value)
    uint32_t total_blocks[1];           //offset 0x205 total data size excluding header (512 bytes) (little-endian value)
};

constexpr int SIZE_MTK_HEADER = 512; // Размер MTK заголовка
constexpr int SIZE_INT = 4; // Размер целочисленного значения
constexpr int PAD_TO = 32; // Значение для выравнивания

// Структура для хранения информации о разрешении экрана
struct Resolution {
    int height;
    int width;
};

// Распространенные разрешения экрана
static std::vector<Resolution> common_resolutions = {
   // Hight,    Wight
        {1,      102},
        {1,      135},
        {1,      163},
        {1,	     169},
        {1,	     304},
        {2,       79},
        {2,      152},
        {20,	 138},
        {24,     135},
        {24,     152},
        {27,	  15},
        {27,	  30},
        {28,	 169},
        {29,	 163},
        {32,      23},
        {32,      34},
        {36,	  27},
        {36,      32},
        {40,      28},
        {40,      32},
        {51,	  36},
        {51,	 218},
        {52,	 304},
        {54,      38},
        {54,      48},
        {57,	  64},
        {64,	  45},
        {64,	  57},
        {105,	  32},
        {105,	  63},
        {121,	  84},
        {121,	 108},
        {128,	  72},
        {139,	  45},
        {160,    120},
        {214,    286},
        {240,    160},
        {250,    680},
        {263,      2},
        {263,     47},
        {320,    240},
        {360,    240},
        {360,	 640},
        {376,    240},
        {384,    240},
        {400,    240},
        {428,    240},
        {432,    240},
        {445,    197},
        {480,    270},
        {480,    272},
        {480,    320},
        {480,    360},
        {570,	 320},
        {640,    360},
        {640,    384},
        {640,    480},
        {720,    480},
        {768,    480},
        {800,    480},
        {800,	 600},
        {800,   2560},
        {848,    480},
        {852,    480},
        {853,    480},
        {854,    480},
        {856,	 480},
        {960,    540},
        {960,    640},
        {1024,   576},
        {1024,	 600},
        {1024,   768},
        {1152,   768},
        {1280,   720},
        {1280,   768},
        {1280,   800},
        {1360,   768},
        {1366,   768},
        {1152,   864},
        {1152,   870},
        {1152,   900},
        {1120,   832},
        {1280,   720},
        {1280,   800},
        {1280,   960},
        {1280,  1024},
        {1400,  1050},
        {1440,   900},
        {1440,   960},
        {1600,   720},
        {1600,   900},
        {1600,  1200},
        {1640,   720},
        {1680,  1050},
        {1920,   720},
        {1920,  1080},
        {1920,  1200},
        {2048,  1152},
        {2048,  1536},
        {2400,  1080},
        {2408,  1080},   // poco 5
        {2560,  1440},
        {2560,  1600},
        {2880,  1600},
        {2560,  2048},
        {3200,  1800},
        {3200,  2048},
        {3200,  2400},
        {3840,  2160},
        {3840,  2400},
        {4096,  2160},
        {4096,  3072},
        {5120,  2880},
        {5120,  3200},
        {5120,  4096},
        {6400,  4096},
        {6400,  4800},
        {7680,  4320},
        {7680,  4800},
        {15360, 8640},
        // Добавьте здесь другие распространенные разрешения при необходимости
};

std::string getAbsolutePath(const std::string &relative_path) {
    try {
        // Получаем абсолютный путь
        std::filesystem::path absolute_path = std::filesystem::absolute(relative_path);
        // Возвращаем абсолютный путь в виде std::string
        return absolute_path.string();
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return "";
    }
}

std::string get_filename_from_path(const std::string &path) {
    try {
        std::filesystem::path fs_path(path);
        return fs_path.filename().string();
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return "";
    }
}

// Функция для получения имени файла без расширения
std::string get_filename_without_extension(const std::string &path) {
    try {
        std::filesystem::path fs_path(path);
        return fs_path.stem().string();  // Возвращает имя файла без расширения
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return "";
    }
}

// Функция для получения пути без расширения файла
std::string get_path_without_extension(const std::string &path) {
    try {
        std::filesystem::path fs_path(path);
        return fs_path.parent_path().string() + "/" + fs_path.stem().string();
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return "";
    }
}

// Функция для получения расширения файла
std::string get_file_extension(const std::string &path) {
    try {
        std::filesystem::path fs_path(path);
        return fs_path.extension().string();  // Возвращает расширение файла
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return "";
    }
}

void copyFile(const std::string &source_path, const std::string &destination_path) {
    try {
        // Если целевой файл уже существует, удаляем его
        if (std::filesystem::exists(destination_path)) std::filesystem::remove(destination_path);
        // Копируем файл
        std::filesystem::copy_file(source_path, destination_path, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(-1);
    }
}

std::string replacetxt(std::string str, std::string oldSubstring, std::string newSubstring) {
    size_t pos = str.find(oldSubstring);
    if (pos != std::string::npos) {
        str.replace(pos, oldSubstring.length(), newSubstring);
    } else {
        std::cout << "Substring not found" << std::endl;
    }
    return str;
}

// Функция для распаковки данных
std::vector<unsigned char> unpack_zlib(const std::vector<char> &compressed_data) {
    std::vector<unsigned char> uncompressed_data_tmp(3000 * 3000 * 4, 0); // Увеличиваем размер буфера для распакованных данных
    uLongf destLen = uncompressed_data_tmp.size();
    int result = uncompress(reinterpret_cast<Bytef *>(uncompressed_data_tmp.data()), &destLen, reinterpret_cast<const Bytef *>(compressed_data.data()), compressed_data.size());
    uncompressed_data_tmp.resize(destLen); // Уменьшаем размер до оригинального
    std::vector<unsigned char> uncompressed_data = uncompressed_data_tmp;
    if (!(result == Z_OK)) {
        std::cerr << "Error: zlib decompression failed with error code " << result << std::endl;
        exit(-1);
    }
    return uncompressed_data;
}

std::vector<unsigned char> pack_zlib(const std::vector<unsigned char> &decompressed_data) {
    // Создаем вектор для сжатых данных
    std::vector<unsigned char> compressed_data;

    // Размер буфера для сжатия
    uLong source_len = decompressed_data.size();
    uLong dest_len = compressBound(source_len);

    // Выделяем память для буфера сжатых данных
    compressed_data.resize(dest_len);

    // Сжимаем данные с максимальным уровнем сжатия
    int result = compress2(&compressed_data[0], &dest_len, &decompressed_data[0], source_len, Z_BEST_COMPRESSION);
    if (result != Z_OK) {
        std::cerr << "Compression failed: " << result << std::endl;
        return {};
    }

    // Уменьшаем размер вектора до фактического размера сжатых данных
    compressed_data.resize(dest_len);

    return compressed_data;
}

void copy_part_file(const std::string &logo_file, const std::string &output_dir, const int bytes_to_copy) {
// Открываем входной файл для чтения в бинарном режиме
    std::ifstream input_file(logo_file, std::ios::binary);

    if (!input_file) {
        std::cerr << "Error: Unable to open input file " << logo_file <<
                  std::endl;
        exit(-1);
    }
    std::string output_filename = output_dir + "/header";
// Открываем выходной файл для записи в бинарном режиме
    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file) {
        std::cerr << "Error: Unable to open output file " << output_filename << std::endl;
        exit(-1);
    }

// Читаем первые bytes_to_copy байт из входного файла
    char buffer[bytes_to_copy];
    input_file.read(buffer, bytes_to_copy);

    if (!input_file) {
        std::cerr << "Error: Unable to read from input file " << logo_file << std::endl;
        exit(-1);
    }

// Записываем прочитанные байты в выходной файл
    output_file.write(buffer, bytes_to_copy);

    if (!output_file) {
        std::cerr << "Error: Unable to write to output file " << output_filename << std::endl;
        exit(-1);
    }

// Закрываем файлы
    input_file.close();
    output_file.close();
}

// Функция для получения текущего времени в формате "YYYYMMDDHHMMSS"
std::string get_current_time_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
    return ss.str();
}

// Функция для чтения файла в вектор байт
std::vector<unsigned char> read_file_to_vector(const std::string &file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        throw std::runtime_error("Could not read file: " + file_path);
    }

    return buffer;
}

// Функция для упаковки std::vector<unsigned char> в ZIP архив
/*
 std::string entry_name = "hello.txt";
    try {
        pack_to_zip(data, entry_name);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
 */

void pack_to_zip(const std::vector<unsigned char> &data, const std::string &entry_name) {
    std::string zip_filename = "logo_" + get_current_time_str() + ".zip";

    zipFile zf = zipOpen(zip_filename.c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        throw std::runtime_error("Could not open zip archive for writing");
    }

    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zip_fileinfo));

    int err = zipOpenNewFileInZip(zf, entry_name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION);
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not open new file in zip archive");
    }

    err = zipWriteInFileInZip(zf, data.data(), data.size());
    if (err != ZIP_OK) {
        zipCloseFileInZip(zf);
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not write data to zip archive");
    }

    zipCloseFileInZip(zf);
    zipClose(zf, nullptr);

    std::cout << "Данные успешно упакованы в " << zip_filename << std::endl;
}

// Функция для распаковки ZIP архива
void extract_zip(const std::string &zip_path, const std::string &extract_dir) {
    unzFile uf = unzOpen(zip_path.c_str());
    if (uf == nullptr) {
        throw std::runtime_error("Could not open zip archive for reading");
    }

    if (unzGoToFirstFile(uf) != UNZ_OK) {
        unzClose(uf);
        throw std::runtime_error("Could not read first file in zip archive");
    }

    do {
        char filename_inzip[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(uf);
            throw std::runtime_error("Could not get file info in zip archive");
        }

        if (unzOpenCurrentFile(uf) != UNZ_OK) {
            unzClose(uf);
            throw std::runtime_error("Could not open file in zip archive");
        }

        std::vector<char> buffer(file_info.uncompressed_size);
        if (unzReadCurrentFile(uf, buffer.data(), buffer.size()) < 0) {
            unzCloseCurrentFile(uf);
            unzClose(uf);
            throw std::runtime_error("Could not read file in zip archive");
        }

        std::string output_path = extract_dir + "/" + filename_inzip;
        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            unzCloseCurrentFile(uf);
            unzClose(uf);
            throw std::runtime_error("Could not create output file: " + output_path);
        }
        out_file.write(buffer.data(), buffer.size());
        out_file.close();

        unzCloseCurrentFile(uf);
    } while (unzGoToNextFile(uf) == UNZ_OK);

    unzClose(uf);
    std::cout << "Files extracted to " << extract_dir << std::endl;
}

// Функция для добавления файла в ZIP архив
void add_file_in_zip(const std::string &zip_path, const std::string &file_path, const std::string &entry_name) {
    // Открываем zip файл для создания нового архива
    zipFile zf = zipOpen(zip_path.c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        throw std::runtime_error("Could not open zip archive for writing: " + zip_path);
    }

    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zip_fileinfo));

    // Читаем содержимое файла в вектор
    std::vector<unsigned char> data = read_file_to_vector(file_path);

    // Открываем новый файл в zip архиве
    int err = zipOpenNewFileInZip(zf, entry_name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION);
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not open new file in zip archive: " + entry_name);
    }

    // Записываем данные файла в zip архив
    err = zipWriteInFileInZip(zf, data.data(), data.size());
    if (err != ZIP_OK) {
        zipCloseFileInZip(zf);
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not write data to zip archive: " + entry_name);
    }

    // Закрываем текущий файл в zip архиве
    zipCloseFileInZip(zf);

    // Закрываем zip архив
    zipClose(zf, nullptr);

    std::cout << "File " << file_path << " added to zip archive as " << entry_name << std::endl;
}

// Функция для вывода списка файлов в ZIP архиве
void list_files_in_zip(const std::string &zip_path) {
    unzFile uf = unzOpen(zip_path.c_str());
    if (uf == nullptr) {
        throw std::runtime_error("Could not open zip archive for reading");
    }

    if (unzGoToFirstFile(uf) != UNZ_OK) {
        unzClose(uf);
        throw std::runtime_error("Could not read first file in zip archive");
    }

    do {
        char filename_inzip[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(uf);
            throw std::runtime_error("Could not get file info in zip archive");
        }

        std::cout << "File: " << filename_inzip << " | Size: " << file_info.uncompressed_size << " bytes" << std::endl;

    } while (unzGoToNextFile(uf) == UNZ_OK);

    unzClose(uf);
}

// Функция для создания папки в ZIP архиве
void add_folder_to_zip(const std::string &zip_path, const std::string &folder_name) {
    zipFile zf = zipOpen(zip_path.c_str(), APPEND_STATUS_ADDINZIP);
    if (zf == nullptr) {
        throw std::runtime_error("Could not open zip archive for writing");
    }

    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zip_fileinfo));

    std::string folder_entry_name = folder_name;
    if (folder_entry_name.back() != '/') {
        folder_entry_name += '/';
    }

    int err = zipOpenNewFileInZip(zf, folder_entry_name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, 0, 0);
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not create folder in zip archive");
    }

    zipCloseFileInZip(zf);
    zipClose(zf, nullptr);

    std::cout << "Folder " << folder_name << " added to " << zip_path << std::endl;
}

// Функция для распаковки файлов из ZIP архива
void extract_files_from_zip(const std::string &zip_path, const std::string &extract_dir, const std::vector<std::string> &files_to_extract) {
    unzFile uf = unzOpen(zip_path.c_str());
    if (uf == nullptr) {
        throw std::runtime_error("Could not open zip archive for reading");
    }

    for (const auto &file_name: files_to_extract) {
        if (unzLocateFile(uf, file_name.c_str(), 0) != UNZ_OK) {
            std::cerr << "File " << file_name << " not found in zip archive" << std::endl;
            continue;
        }

        if (unzOpenCurrentFile(uf) != UNZ_OK) {
            throw std::runtime_error("Could not open file in zip archive");
        }

        char filename_inzip[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzCloseCurrentFile(uf);
            throw std::runtime_error("Could not get file info in zip archive");
        }

        std::vector<char> buffer(file_info.uncompressed_size);
        if (unzReadCurrentFile(uf, buffer.data(), buffer.size()) < 0) {
            unzCloseCurrentFile(uf);
            throw std::runtime_error("Could not read file in zip archive");
        }

        std::string output_path = extract_dir + "/" + filename_inzip;
        std::filesystem::create_directories(std::filesystem::path(output_path).parent_path());
        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            unzCloseCurrentFile(uf);
            throw std::runtime_error("Could not create output file: " + output_path);
        }
        out_file.write(buffer.data(), buffer.size());
        out_file.close();

        unzCloseCurrentFile(uf);
        std::cout << "File " << filename_inzip << " extracted to " << output_path << std::endl;
    }

    unzClose(uf);
}

// Функция для добавления файла в ZIP архив
void add_file_to_zip(zipFile zf, const std::string &file_path, const std::string &entry_name) {
    std::vector<unsigned char> data = read_file_to_vector(file_path);

    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zip_fileinfo));

    int err = zipOpenNewFileInZip(zf, entry_name.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION);
    if (err != ZIP_OK) {
        throw std::runtime_error("Could not open new file in zip archive");
    }

    err = zipWriteInFileInZip(zf, data.data(), data.size());
    if (err != ZIP_OK) {
        zipCloseFileInZip(zf);
        throw std::runtime_error("Could not write data to zip archive");
    }

    zipCloseFileInZip(zf);
    std::cout << "File " << file_path << " added to zip archive as " << entry_name << std::endl;
}

// Функция для добавления нескольких файлов или папок в ZIP архив
void add_files_to_zip(const std::string &zip_path, const std::vector<std::string> &files, std::string &base_path) {
    zipFile zf = zipOpen(zip_path.c_str(), APPEND_STATUS_ADDINZIP);
    if (zf == nullptr) {
        throw std::runtime_error("Could not open zip archive for writing");
    }

    for (const auto &file_path: files) {
        std::string entry_name = base_path.empty() ? file_path : base_path + "/" + file_path;
        add_file_to_zip(zf, file_path, entry_name);
    }

    zipClose(zf, nullptr);
}

// Функция для записи строки в файл внутри ZIP архива
void write_string_to_zip(const std::string& zip_filename, const std::string& file_inside_zip, const std::string& content) {
    zipFile zf = zipOpen(zip_filename.c_str(), APPEND_STATUS_ADDINZIP);
    if (zf == nullptr) {
        throw std::runtime_error("Could not open zip archive for writing: " + zip_filename);
    }

    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zip_fileinfo));

    int err = zipOpenNewFileInZip(zf, file_inside_zip.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_BEST_COMPRESSION);
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not open file in zip archive: " + file_inside_zip);
    }

    err = zipWriteInFileInZip(zf, content.c_str(), content.size());
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not write to file in zip archive: " + file_inside_zip);
    }

    err = zipCloseFileInZip(zf);
    if (err != ZIP_OK) {
        zipClose(zf, nullptr);
        throw std::runtime_error("Could not close file in zip archive: " + file_inside_zip);
    }

    zipClose(zf, nullptr);
}

// Функция для угадывания разрешения изображения на основе размера файла и глубины цвета
Resolution guess_resolution(size_t file_size) {
    // Предполагаемое количество пикселей в изображении (ширина * высота)
    size_t total_pixels = file_size / 4; // 4 байта на пиксель (32 бита)

    // Предполагаемая сторона изображения (ширина и высота)
    int assumed_side = static_cast<int>(sqrt(total_pixels));

    // Ищем подходящее разрешение из списка распространенных разрешений
    for (const auto &resolution: common_resolutions) {
        // Проверяем, что ширина и высота соответствуют сторонам изображения
        if (resolution.width * resolution.height == total_pixels) {
            return resolution;
        }
    }

    // Если не удалось найти точное соответствие, вернем предполагаемое разрешение
    return {assumed_side, assumed_side};
}

uint16_t rgba2rgb565(uint32_t color32) {
    uint16_t r = ((color32 & 0xF8000000) >> 16) & 0b11111;
    uint16_t g = ((color32 & 0x00FC0000) >> 13) & 0b111111;
    uint16_t b = ((color32 & 0x0000F800) >> 11) & 0b11111;
    return (r << 11) | (g << 5) | b;
}

uint32_t rgb5652rgba(uint16_t color16) {
    uint32_t r = (static_cast<uint32_t>(color16 & 0xF800) << 16) & 0xFF000000;
    uint32_t g = (static_cast<uint32_t>(color16 & 0x07E0) << 13) & 0x00FF0000;
    uint32_t b = (static_cast<uint32_t>(color16 & 0x001F) << 11) & 0x0000FF00;
    return r | g | b | 0x000000FF;
}

// Helper function to convert RGBA to BGRA
uint32_t rgba2bgra(uint32_t color32) {
    uint8_t r = (color32 >> 24) & 0xFF;
    uint8_t g = (color32 >> 16) & 0xFF;
    uint8_t b = (color32 >> 8) & 0xFF;
    uint8_t a = color32 & 0xFF;
    return (b << 24) | (g << 16) | (r << 8) | a;
}

// Helper function to convert RGBA to BGRA Big-Endian
uint32_t rgba2bgra_be(uint32_t color32) {
    uint8_t r = (color32 >> 24) & 0xFF;
    uint8_t g = (color32 >> 16) & 0xFF;
    uint8_t b = (color32 >> 8) & 0xFF;
    uint8_t a = color32 & 0xFF;
    return (b << 24) | (g << 16) | (r << 8) | a;
}

// Helper function to convert RGBA to BGRA Little-Endian
uint32_t rgba2bgra_le(uint32_t color32) {
    uint8_t r = (color32 >> 24) & 0xFF;
    uint8_t g = (color32 >> 16) & 0xFF;
    uint8_t b = (color32 >> 8) & 0xFF;
    uint8_t a = color32 & 0xFF;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Функция для конвертации RGBA в BGRA
std::vector<unsigned char> rgba_to_bgra(const std::vector<unsigned char>& rgba) {
    if (rgba.size() % 4 != 0) {
        throw std::invalid_argument("Input data size is not a multiple of 4");
    }

    std::vector<unsigned char> bgra(rgba.size());

    for (size_t i = 0; i < rgba.size(); i += 4) {
        bgra[i] = rgba[i + 2];     // B
        bgra[i + 1] = rgba[i + 1]; // G
        bgra[i + 2] = rgba[i];     // R
        bgra[i + 3] = rgba[i + 3]; // A
    }

    return bgra;
}

// Функция для конвертации BGRA в RGBA
std::vector<unsigned char> bgra_to_rgba(const std::vector<unsigned char>& bgra) {
    if (bgra.size() % 4 != 0) {
        throw std::invalid_argument("Input data size is not a multiple of 4");
    }

    std::vector<unsigned char> rgba(bgra.size());

    for (size_t i = 0; i < bgra.size(); i += 4) {
        rgba[i] = bgra[i + 2];     // R
        rgba[i + 1] = bgra[i + 1]; // G
        rgba[i + 2] = bgra[i];     // B
        rgba[i + 3] = bgra[i + 3]; // A
    }

    return rgba;
}

// Function to convert RGBA byte buffer to BGRA Big-Endian
std::vector<unsigned char> rgba_to_bgra_be(const std::vector<unsigned char>& rgba_data, uint32_t w, uint32_t h) {
    size_t pixels = w * h;
    std::vector<unsigned char> bgra_data(pixels * 4);

    for (size_t i = 0; i < pixels; ++i) {
        uint32_t color32;
        std::memcpy(&color32, &rgba_data[i * 4], sizeof(color32));
        color32 = __builtin_bswap32(color32); // Ensure RGBA is in BigEndian
        uint32_t bgra_color = rgba2bgra_be(color32);
        std::memcpy(&bgra_data[i * 4], &bgra_color, sizeof(bgra_color));
    }

    return bgra_data;
}

// Function to convert RGBA byte buffer to BGRA Little-Endian
std::vector<unsigned char> rgba_to_bgra_le(const std::vector<unsigned char>& rgba_data, uint32_t w, uint32_t h) {
    size_t pixels = w * h;
    std::vector<unsigned char> bgra_data(pixels * 4);

    for (size_t i = 0; i < pixels; ++i) {
        uint32_t color32;
        std::memcpy(&color32, &rgba_data[i * 4], sizeof(color32));
        uint32_t bgra_color = rgba2bgra_le(color32);
        std::memcpy(&bgra_data[i * 4], &bgra_color, sizeof(bgra_color));
    }

    return bgra_data;
}

// Function to convert RGBA byte buffer to RGBA Big-Endian
std::vector<unsigned char> rgba_to_rgba_be(const std::vector<unsigned char>& rgba_data, uint32_t w, uint32_t h) {
    size_t pixels = w * h;
    std::vector<unsigned char> rgba_be_data(pixels * 4);

    for (size_t i = 0; i < pixels; ++i) {
        uint32_t color32;
        std::memcpy(&color32, &rgba_data[i * 4], sizeof(color32));
        color32 = __builtin_bswap32(color32); // Swap to Big-Endian
        std::memcpy(&rgba_be_data[i * 4], &color32, sizeof(color32));
    }

    return rgba_be_data;
}

std::vector<uint8_t> u32be_to_u32le(std::istream& reader, size_t words) {
    std::vector<uint8_t> rgbale;
    rgbale.reserve(words * sizeof(uint32_t));
    
    for (size_t i = 0; i < words; ++i) {
        // Read color32 in big endian.
        uint32_t color32;
        reader.read(reinterpret_cast<char*>(&color32), sizeof(uint32_t));
        
        // Convert color32 from big endian to little endian and from RGBA to BGRA.
        uint32_t bgra;
        std::memcpy(&bgra, &color32, sizeof(uint32_t));
        bgra = rgba2bgra(bgra);
        
        // Write bgra as little endian into rgbale.
        for (size_t j = 0; j < sizeof(uint32_t); ++j) {
            rgbale.push_back(reinterpret_cast<uint8_t*>(&bgra)[j]);
        }
    }
    return rgbale;
}

template <typename O, typename R>
std::vector<uint8_t> rgba_to_rgb565(R& reader, uint32_t w, uint32_t h) {
    size_t pixels = static_cast<size_t>(w * h);
    std::vector<uint8_t> rgb565;
    rgb565.reserve(pixels * 2);
    
    for (size_t i = 0; i < pixels; ++i) {
        // 'pivot' rgba is always BigEndian.
        uint32_t color32;
        reader.read(reinterpret_cast<char*>(&color32), sizeof(uint32_t));
        
        // Convert color32 from RGBA to RGB565.
        uint16_t rgb565_color = rgba2rgb565(color32);
        
        // Write rgb565_color into rgb565 vector.
        for (size_t j = 0; j < sizeof(uint16_t); ++j) {
            rgb565.push_back(reinterpret_cast<uint8_t*>(&rgb565_color)[j]);
        }
    }
    
    return rgb565;
}

template <typename B>
std::vector<uint8_t> rgb565_to_rgba(const uint8_t* data, uint32_t w, uint32_t h) {
    size_t pixels = static_cast<size_t>(w * h);
    std::vector<uint8_t> rgba;
    rgba.reserve(pixels * 4);
    
    const uint16_t* data_reader = reinterpret_cast<const uint16_t*>(data);
    for (size_t i = 0; i < pixels; ++i) {
        uint16_t color16 = data_reader[i];
        uint32_t color32 = rgb5652rgba(color16);
        
        // Write color32 into rgba vector.
        for (size_t j = 0; j < sizeof(uint32_t); ++j) {
            rgba.push_back(reinterpret_cast<uint8_t*>(&color32)[j]);
        }
    }
    
    return rgba;
}

void read_png_file(const char *file_name, std::vector<unsigned char> &image) {
    png_byte header[8];
    int width;
    int height;
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        std::cerr << "Error: File " << file_name << " could not be opened for reading" << std::endl;
        return;
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        std::cerr << "Error: " << file_name << " is not a PNG file" << std::endl;
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        std::cerr << "Error: png_create_read_struct failed" << std::endl;
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        std::cerr << "Error: png_create_info_struct failed" << std::endl;
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        std::cerr << "Error: Error during init_io" << std::endl;
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    image.resize(rowbytes * height);

    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; y++)
        row_pointers[y] = &image[y * rowbytes];

    png_read_image(png_ptr, row_pointers.data());

    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

}

void write_png_file(const std::string &filename, int width, int height, std::vector<unsigned char> &data) {
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cerr << "Error: Couldn't open " << filename << " for writing." << std::endl;
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        std::cerr << "Error: png_create_write_struct failed" << std::endl;
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fclose(fp);
        png_destroy_write_struct(&png_ptr, nullptr);
        std::cerr << "Error: png_create_info_struct failed" << std::endl;
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        std::cerr << "Error: PNG error during writing" << std::endl;
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = data.data() + y * width * 4; // 4 channels (RGBA)
    }

    png_write_image(png_ptr, row_pointers.data());
    png_write_end(png_ptr, nullptr);

    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

// Функция для чтения структуры MTK_logo из файла
bool read_MTK_logo_from_file(const std::string& filename, MTK_logo& data) {
    // Открываем файл для чтения в бинарном режиме
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Couldn't open " << filename << std::endl;
        return false;
    }

    // Считываем данные из файла и сохраняем их в структуру MTK_logo
    file.read(reinterpret_cast<char*>(&data), sizeof(data));

    // Проверяем, что чтение прошло успешно
    if (!file) {
        std::cerr << "Error: Couldn't read data from " << filename << std::endl;
        return false;
    }

    // Закрываем файл
    file.close();

    return true;
}

void unpack_logo(const std::string &logo_file, const std::string &output_dir) {
    std::ifstream logo_bin(logo_file, std::ios::binary);
    if (!logo_bin.is_open()) {
        std::cerr << "Error: Couldn't open " << logo_file << std::endl;
        return;
    }

    int picture_count;
    int block_size;
    // Создаем экземпляр структуры MTK_logo для хранения данных
    MTK_logo data;

    // Читаем структуру MTK_logo из файла
    if (read_MTK_logo_from_file(logo_file, data)) {
        std::cout << "Found \"" << data.header.magic << "\" signature at offset 0x08" << std::endl;
        picture_count = data.count_pictures[0];
        std::cout << "File contains " << picture_count << " pictures!" << std::endl;
        block_size = data.total_blocks[0];
        std::cout << "Total block size (8 bytes + map + pictures): " << std::to_string(block_size) << " bytes" << std::endl;
    }else
    {
        std::cout << "Unsupport logo.bin" <<std::endl;
        exit(-1);
    }

    std::map<int, int> offsets;
    std::map<int, int> sizes;
    logo_bin.seekg(sizeof(MTK_logo)); //offset 0x208: starts of pictures offset's
    for (int i = 0; i < picture_count; ++i) {
        int offset;
        logo_bin.read(reinterpret_cast<char *>(&offset), SIZE_INT);
        offsets[i] = offset;
    }

    for (int i = 0; i < picture_count - 1; ++i) {
        sizes[i] = offsets[i + 1] - offsets[i];
    }
    sizes[picture_count - 1] = block_size - offsets[picture_count - 1];

    std::cout << std::endl;

    printf("--------------------------------------------------------\n");
    printf("%8s  |  %10s  |  %6s |  %s\n", "NUMBER", "OFFSET", "SIZE (bytes)", "FILE");
    printf("--------------------------------------------------------\n");

    int images_size = 0;

    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directories(output_dir);
    }
    std::string real_folder = getAbsolutePath(output_dir);

    for (int i = 0; i < picture_count; ++i) {
        std::vector<char> image_z(sizes[i]);
        logo_bin.read(image_z.data(), sizes[i]);
        images_size += sizes[i];
        std::printf("    %03d   |   0x%06X   |   %010d  |  img-%03d.png\n", i + 1, offsets[i], sizes[i], i + 1);
        std::fflush(stdout);
        char filename_l[256];
        std::string prefix = output_dir + "/img-";
        std::sprintf(filename_l, "%s%03d%s", prefix.c_str(), i+1, ".png");
        std::string filename = filename_l;

        std::vector<unsigned char> data = unpack_zlib(image_z);
        // Угадываем разрешение изображения
        Resolution guessed_resolution = guess_resolution(data.size());

        // Вычисляем ширину и высоту изображения
        int width = guessed_resolution.width; // Замените это на фактическую ширину изображения
        int height = guessed_resolution.height; // Замените это на фактическую высоту изображения

        // Создаем файл PNG
        //write_png_file(filename.data(), width, height, data);

        try {
            // Конвертируем данные RGBA в данные BGRA BE
            //std::vector<unsigned char> bgra_data = rgba_to_bgra(data, width, height);
            std::vector<unsigned char> bgra_data = rgba_to_bgra(data);
            // Сохраняем данные BGRA в PNG файл
            write_png_file(filename, width, height, bgra_data);
        } catch (const std::exception& e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
            exit(-1);
        }

    }

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "                             " << images_size << std::endl;
    std::cout << std::endl;
    //
    copy_part_file(logo_file, output_dir, sizeof(MTK_logo) - 8);
    printf("All files are extracted to the folder: \n\"%s\"\n", real_folder.c_str());
}

void pack_logo(const std::string &output_dir, const std::string &logo_file) {
    std::vector<std::string> images;
    std::map<int, int> offsets;
    std::map<int, int> sizes;
    std::string real_path_logo = getAbsolutePath(logo_file);
    for (const auto &entry: std::filesystem::directory_iterator(output_dir)) {
        std::string file = entry.path().filename().string();
        if (file.substr(0, 4) == "img-" && file.substr(file.size() - 4) == ".png") {
            images.push_back(file);
        }
    }
    std::sort(images.begin(), images.end()); //отсортируем по порядку, на всякий случай

    std::string header_file = output_dir + "/header";
    if (std::filesystem::exists(header_file) && std::filesystem::is_regular_file(header_file)) {
        // Копируем файл с новым именем
        copyFile(header_file, logo_file);
    } else {
        std::cerr << "Error: File " << header_file << " does not exist or is not a regular file." << std::endl;
        exit(-1);
    }

    int bytes_written = 0;
    std::fstream new_logo(logo_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!new_logo.is_open()) {
        std::cerr << "Error: Could not open file " << logo_file << " for writing" << std::endl;
        exit(-1);
    }
    new_logo.seekp(0, std::ios_base::end); //переходим в конец заголовочного файла
    int picture_count = images.size();
    int bloc_size = (SIZE_INT * 2) + (SIZE_INT * picture_count);
    std::cout << "Writing MTK header to file \"" << logo_file << "\" (" << SIZE_MTK_HEADER << " bytes)..." << std::endl;
    //new_logo.write(mtk_header, SIZE_MTK_HEADER);
    bytes_written += SIZE_MTK_HEADER;
    std::cout << "Writing pictures count, which is " << picture_count << " (" << SIZE_INT << " bytes)..." << std::endl;
    new_logo.write(reinterpret_cast<const char *>(&picture_count), SIZE_INT);
    bytes_written += SIZE_INT;
    new_logo.write(std::string(4, '\0').c_str(), 4); //запишем 4 байта, потом по верху запишем общий размер
    bytes_written += SIZE_INT;

    // Read and compress images, Write offsets map.
    std::cout << "Writing offsets map (" << picture_count << " * " << SIZE_INT << " = " << picture_count * SIZE_INT << " bytes)" << std::endl;
    std::cout << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << " IMAGE FILE  |   OFFSET   | SIZE (bytes) " << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    int i = 1;
    std::vector<unsigned char> tmp_images_z;
    for (const auto &img: images) {

        std::vector<unsigned char> image_png;
        std::string inpute_file = output_dir + "/" + img;
        read_png_file(inpute_file.c_str(), image_png);
        if (image_png.empty()) {
            std::cerr << "Error: Failed to read PNG file" << std::endl;
        }
        std::vector<unsigned char> image = bgra_to_rgba(image_png);
        std::vector<unsigned char> image_z = pack_zlib(image);
        tmp_images_z.insert(tmp_images_z.end(), image_z.begin(), image_z.end()); //добавляем в конец вектора, чтобы потом всё вывалить
        sizes[i] = image_z.size();

        if (i == 1) {
            offsets[i] = bloc_size;
        } else {
            offsets[i] = offsets[i - 1] + sizes[i - 1];
        }
        new_logo.write(reinterpret_cast<const char *>(&offsets[i]), SIZE_INT);
        printf(" %s | 0x%08X | %010d\n", img.c_str(), offsets[i], sizes[i]);
        std::fflush(stdout);
        bytes_written += SIZE_INT;
        i++;
    }
    bloc_size += tmp_images_z.size();
    new_logo.write(reinterpret_cast<const char *>(tmp_images_z.data()), tmp_images_z.size());
    bytes_written += tmp_images_z.size();

    std::cout << "---------------------------------------" << std::endl;
    printf("                              %zu\n", tmp_images_z.size());
    std::cout << std::endl;

    std::cout << "Total block size (8 bytes + map + pictures): " << bloc_size << " bytes" << std::endl;
    std::cout << "Writing total block size (" << SIZE_INT << " bytes)..." << std::endl;
    new_logo.seekp(SIZE_INT, std::ios_base::beg);
    new_logo.write(reinterpret_cast<const char *>(&bloc_size), SIZE_INT);
    new_logo.seekp(516, std::ios_base::beg);
    new_logo.write(reinterpret_cast<const char *>(&bloc_size), SIZE_INT);
    try {
        std::string zip_file = get_path_without_extension(logo_file) + "_" + get_current_time_str() + ".zip";
        std::string entry_name = get_filename_from_path(logo_file);
        add_file_in_zip(zip_file, logo_file, entry_name);
        std::string updater_script_path = "META-INF/com/google/android/updater-script";
        std::string updater_script_content = R"(ui_print("Flashing logo...");
show_progress(0.200000, 0);
package_extract_file(")" + entry_name + R"(, "/dev/block/platform/mtk-msdc.0/by-name/logo");
ui_print("Patched!");
ui_print("");
ui_print("Now you can reboot your phone");
)";
        // Записываем данные в updater-script
        write_string_to_zip(zip_file, updater_script_path, updater_script_content);

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    //std::cout << "Done! The file was successfully created at path: \n" << real_path_logo << std::endl;
    printf("Done! The file was successfully created at path:\n%s\n", real_path_logo.c_str());
}

// Помощь по параметрам командной строки
void LOGO_MTK::show_help() {
    fprintf(stderr, R"EOF(
logo_mtk

Usage:

    logo_mtk <unpack/pack> <file/folder> <folder/file>)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult LOGO_MTK::parse_cmd_line(int argc, char *argv[]) {
    if (argc < 3) {
        show_help();
        return ParseResult::wrong_option;
    }
    mode = argv[0];
    if (mode == "unpack") {
        logo_file = argv[1];
        dir = argv[2];
    }else if (mode == "pack") {
        logo_file = argv[2];
        dir = argv[1];
    }else {
        show_help();
        return ParseResult::wrong_option;
    }
    return ParseResult::ok;
}

// Основная функция
ProcessResult LOGO_MTK::process() {

    if (mode == "unpack") {
        unpack_logo(logo_file, dir);
    }

    if (mode == "pack") {
        pack_logo(dir, logo_file);
    }

    return ProcessResult::ok;
}