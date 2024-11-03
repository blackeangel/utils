#include "../include/main.hpp"

// Помощь по параметрам командной строки
void KerVer::show_help()
{
    fprintf(stderr, R"EOF(
kerver

Usage:

    kerver <file>)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult KerVer::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    kernelFile = argv[0];
    return ParseResult::ok;
}


// Универсальная структура для хранения любого заголовка
union BootHeader {
    boot_img_hdr_v0 v0;
    boot_img_hdr_v1 v1;
    boot_img_hdr_v2 v2;
    boot_img_hdr_v3 v3;
    boot_img_hdr_v4 v4;
    vendor_boot_img_hdr_v3 vendor3;
    vendor_boot_img_hdr_v4 vendor4;
};

bool checkFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filePath << std::endl;
        return false;
    }

    std::streampos fileSize = file.tellg();
    const std::streampos oneGB = 1e9;

    if (fileSize >= oneGB) {
        std::cerr << "Error: File size is greater than or equal to 1 GB." << std::endl;
        return false;
    }

    return true;
}

// Функция для загрузки файла
bool loadFile(const std::string &filename, std::vector<uint8_t> &data) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(size);
    if (!file.read(reinterpret_cast<char *>(data.data()), size)) {
        std::cerr << "Error: Could not read file " << filename << "\n";
        return false;
    }

    return true;
}

// Определение версии заголовка
int detectBootHeaderVersion(const std::vector<uint8_t> &data) {
    // Проверка magic
    if (std::strncmp(reinterpret_cast<const char *>(data.data()), "ANDROID!", 8) != 0 &&
        std::strncmp(reinterpret_cast<const char *>(data.data()), "VNDRBOOT", 8) != 0) {
        std::cerr << "Error: Invalid boot or vendor boot image magic\n";
        return -1;
    }

    // Определяем версию по размеру данных или по конкретному полю
    const auto *headerV1 = reinterpret_cast<const boot_img_hdr_v1 *>(data.data());
    if (headerV1->header_version >= 4) return 4;
    if (headerV1->header_version >= 3) return 3;
    if (headerV1->header_version == 2) return 2;
    if (headerV1->header_version == 1) return 1;

    return 0;  // По умолчанию версия 0
}

// Парсинг заголовка
bool parseBootImageHeader(const std::vector<uint8_t> &data, BootHeader &header, int version) {
    switch (version) {
        case 0:
            std::memcpy(&header.v0, data.data(), sizeof(boot_img_hdr_v0));
            break;
        case 1:
            std::memcpy(&header.v1, data.data(), sizeof(boot_img_hdr_v1));
            break;
        case 2:
            std::memcpy(&header.v2, data.data(), sizeof(boot_img_hdr_v2));
            break;
        case 3:
            std::memcpy(&header.v3, data.data(), sizeof(boot_img_hdr_v3));
            break;
        case 4:
            std::memcpy(&header.v4, data.data(), sizeof(boot_img_hdr_v4));
            break;
        default:
            std::cerr << "Error: Unsupported boot image version\n";
            return false;
    }
    return true;
}

// Проверка формата boot_img
bool isBootImage(const std::vector<uint8_t> &data) {
    return data.size() >= 8 && std::memcmp(data.data(), "ANDROID!", 8) == 0;
}

// Проверка формата vendor_boot
bool isVendorBootImage(const std::vector<uint8_t> &data) {
    return data.size() >= 12 && std::memcmp(data.data(), "VNDRBOOT", 8) == 0;
}

// Проверка на gzip-сжатие
bool isGzipCompressed(const std::vector<uint8_t> &data) {
    return data.size() > 2 && data[0] == 0x1F && data[1] == 0x8B;
}

// Функция для поиска заголовка gzip архива
std::streampos findGzipHeader(const std::vector<uint8_t> &data) {
    const std::vector<uint8_t> gzipHeader = {0x1F, 0x8B, 0x08}; // Заголовок gzip
    auto it = std::search(data.begin(), data.end(), gzipHeader.begin(), gzipHeader.end());

    if (it != data.end()) {
        return std::distance(data.begin(), it); // Возвращаем позицию заголовка
    } else {
        return -1; // Если заголовок не найден, возвращаем -1
    }
}

// Функция для чтения данных от смещения до конца файла
std::vector<uint8_t> readFromOffset(const std::vector<uint8_t> &data, std::streampos offset) {
    if (offset == -1) {
        std::cerr << "Error: Offset not found." << std::endl;
        return {};
    }

    // Приводим offset к типу size_t для корректного использования
    auto offsetSize = static_cast<std::size_t>(offset);
    return std::vector<uint8_t>(data.begin() + offsetSize, data.end());
}

// Распаковка gzip в память
bool decompressGzip(const std::vector<uint8_t> &compressedData, std::vector<uint8_t> &decompressedData) {
    z_stream strm = {};
    strm.next_in = const_cast<Bytef *>(compressedData.data());
    strm.avail_in = compressedData.size();

    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
        std::cerr << "Error: Failed to initialize zlib for decompression\n";
        return false;
    }

    decompressedData.clear();
    decompressedData.resize(compressedData.size() * 2);

    int ret;
    do {
        strm.next_out = decompressedData.data() + strm.total_out;
        strm.avail_out = decompressedData.size() - strm.total_out;

        ret = inflate(&strm, Z_SYNC_FLUSH);
        if (ret == Z_OK || ret == Z_STREAM_END) {
            if (strm.avail_out == 0) {
                decompressedData.resize(decompressedData.size() * 2);
            }
        } else {
            std::cerr << "Error: Decompression failed with code " << ret << "\n";
            inflateEnd(&strm);
            return false;
        }
    } while (ret != Z_STREAM_END);
    decompressedData.resize(strm.total_out);
    inflateEnd(&strm);
    return true;
}

// Поиск версии Linux
std::string findLinuxVersion(const std::vector<uint8_t> &data) {
    const std::string searchStr = "Linux version ";
    auto it = std::search(data.begin(), data.end(), searchStr.begin(), searchStr.end());

    if (it != data.end()) {
        auto start = it + searchStr.size();
        auto end = std::find(start, data.end(), '\n');
        std::string versionStr(start, end);

        std::regex versionRegex(R"((\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(versionStr, match, versionRegex)) {
            return match.str(1);
        }
    }

    return "";
}

// Определение архитектуры и битности
std::string findArchitectureAndBitness(const std::vector<uint8_t> &data) {
    const std::vector<std::string> targets = {
            "arch/arm64", "Linux/arm64", "arm64",
            "arch/arm", "Linux/arm"
    };

    for (const auto &target: targets) {
        auto it = std::search(data.begin(), data.end(), target.begin(), target.end());
        if (it != data.end()) {
            if (target.find("arm64") != std::string::npos) {
                return "Architecture: 64 bit";
            } else if (target.find("arm") != std::string::npos) {
                return "Architecture: 32 bit";
            }
        }
    }
    return "Architecture not found.";
}

void processFile(const std::string& filePath) {
    // Проверка размера файла
    if (!checkFileSize(filePath)) {
        return;
    }

    std::vector<uint8_t> data;
    if (!loadFile(filePath, data)) {
        return;
    }
    std::vector<uint8_t> itogKernelData;
    std::string linuxVersion;
    std::string architecture;

    // Проверка на boot_img или vendor_boot
    if (isBootImage(data) || (isVendorBootImage(data))) {
        int version = detectBootHeaderVersion(data);
        if (version < 0) {
            return;
        }

        BootHeader header{};
        if (!parseBootImageHeader(data, header, version)) {
            return;
        }
        // Расчет смещения ядра на основе page_size и kernel_size
        size_t kernel_offset = header.v0.page_size;
        size_t kernel_size = header.v0.kernel_size;

        if (data.size() < kernel_offset + kernel_size) {
            std::cerr << "Error: Kernel size exceeds file boundaries\n";
            return;
        }

        // Извлечение ядра в память
        std::vector<uint8_t> kernel_data(data.begin() + kernel_offset, data.begin() + kernel_offset + kernel_size);
        //читаем просто данные сначала, потом будем искать архив
        // Поиск версии и архитектуры напрямую в бинарных данных
        linuxVersion = findLinuxVersion(kernel_data);
        if (!linuxVersion.empty()) {
            std::cout << "Linux version: " << linuxVersion << std::endl;
            architecture = findArchitectureAndBitness(kernel_data);
            std::cout << architecture << std::endl;
        }else {
            // Поиск заголовка gzip архива и вывод смещения
            std::streampos gzipOffset = findGzipHeader(kernel_data);
            if (gzipOffset >= 0) {
                std::cout << "Gzip header found at offset: " << gzipOffset << std::endl;
                // Чтение данных от смещения до конца файла
                std::vector<uint8_t> readData = readFromOffset(kernel_data, gzipOffset);

                // Распаковка gzip
                if (!decompressGzip(readData, itogKernelData)) {
                    return;
                }
                // Поиск версии и архитектуры напрямую в распакованных бинарных данных
                linuxVersion = findLinuxVersion(itogKernelData);
                std::cout << "Linux version: " << linuxVersion << std::endl;
                architecture = findArchitectureAndBitness(itogKernelData);
                std::cout << architecture << std::endl;
            }
        }
    } else {
        //читаем просто данные сначала, потом будем искать архив
        itogKernelData = data;
        // Поиск версии и архитектуры напрямую в бинарных данных
        linuxVersion = findLinuxVersion(itogKernelData);
        if (!linuxVersion.empty()) {
            std::cout << "Linux version: " << linuxVersion << std::endl;
            architecture = findArchitectureAndBitness(itogKernelData);
            std::cout << architecture << std::endl;
        }else {
            // Поиск заголовка gzip архива и вывод смещения
            std::streampos gzipOffset = findGzipHeader(data);
            if (gzipOffset >= 0) {
                std::cout << "Gzip header found at offset: " << gzipOffset << std::endl;
                // Чтение данных от смещения до конца файла
                std::vector<uint8_t> readData = readFromOffset(data, gzipOffset);

                // Распаковка gzip
                if (!decompressGzip(readData, itogKernelData)) {
                    return;
                }
                // Поиск версии и архитектуры напрямую в распакованных бинарных данных
                linuxVersion = findLinuxVersion(itogKernelData);
                std::cout << "Linux version: " << linuxVersion << std::endl;
                architecture = findArchitectureAndBitness(itogKernelData);
                std::cout << architecture << std::endl;
            }
        }
    }
}

// Основная функция
ProcessResult KerVer::process()
{

    std::string file = kernelFile.string();

    processFile(file);

    return ProcessResult::ok;
}
