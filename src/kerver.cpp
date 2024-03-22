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


// Функция для проверки размера файла
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

// Функция для поиска заголовка gzip архива
std::streampos findGzipHeader(const std::vector<char>& data) {
    const std::string gzipHeader = "\x1F\x8B\x08";
    auto it = std::search(data.begin(), data.end(), gzipHeader.begin(), gzipHeader.end());

    if (it != data.end()) {
        return std::distance(data.begin(), it);
    } else {
        return -1;
    }
}

// Функция для чтения данных от смещения до конца файла
std::vector<char> readFromOffset(const std::vector<char>& data, std::streampos offset) {
    if (offset == -1) {
        std::cerr << "Error: Offset not found." << std::endl;
        return {};
    }

    return std::vector<char>(data.begin() + offset, data.end());
}

// Функция для распаковки данных
std::vector<char> decompressData(const std::vector<char>& data) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    stream.avail_in = static_cast<uInt>(data.size());

    // Используем deflateInit2 с параметрами 16 + MAX_WBITS для обработки gzip
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        std::cerr << "Error: Failed to initialize zlib." << std::endl;
        return {};
    }

    std::vector<char> decompressedData(data.size() * 2, 0);  // Увеличиваем размер буфера
    stream.next_out = reinterpret_cast<Bytef*>(decompressedData.data());
    stream.avail_out = static_cast<uInt>(decompressedData.size());

    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);

    // Уменьшаем размер вектора до фактически распакованных данных
    decompressedData.resize(stream.total_out);

    return decompressedData;
}

// Функция для поиска и чтения 8 байт после совпадения "Linux version "
std::string findLinuxVersion(const std::vector<char>& data) {
    const std::string searchStr = "Linux version ";
    const size_t searchStrLen = searchStr.size();
    const size_t dataSize = data.size();

    size_t pos = 0;

    while (pos < dataSize) {
        auto foundPos = std::search(data.begin() + pos, data.end(), searchStr.begin(), searchStr.end());

        if (foundPos != data.end()) {
            pos = std::distance(data.begin(), foundPos) + searchStrLen;
            if (pos + 8 <= dataSize) {
                std::string resultStr(data.begin() + pos, data.begin() + pos + 8);

                // Проверка наличия "%s" в строке
                if (resultStr.find("%s") == std::string::npos) {
                    return resultStr;
                }
            }
        } else {
            break;  // прерываем, если достигнут конец файла
        }
    }

    return "";
}

// Функция для поиска сразу нескольких значений и определения битности ядра
void findArchitectureAndBitness(const std::vector<char>& data) {
    const std::vector<std::string> targets = {"arch/arm64", "Linux/arm64", "arm64","arch/arm", "Linux/arm"};
    for (const auto& target : targets) {
        auto it = std::search(data.begin(), data.end(), target.begin(), target.end());
        if (it != data.end()) {
            if (target.find("arm64") != std::string::npos) {
                std::cout << "Architecture: 64 bit" << std::endl;
            } else if (target.find("arm") != std::string::npos) {
                std::cout << "Architecture: 32 bit" << std::endl;
            }
            return;
        }
    }
    std::cerr << "Architecture not found." << std::endl;
}

// Главная функция, объединяющая остальные шаги
void processFile(const std::string& filePath) {
    // Проверка размера файла
    if (!checkFileSize(filePath)) {
        return;
    }

    // Чтение файла в память
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filePath << std::endl;
        return;
    }

    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Поиск и чтение 8 байт после совпадения "Linux version "
    std::string linuxVersion = findLinuxVersion(data);

    // Если результат не пуст, выводим его
    if (!linuxVersion.empty()) {
        std::cout << "Linux version: " << linuxVersion << std::endl;
        // Поиск сразу нескольких значений и определение битности ядра
        findArchitectureAndBitness(data);
        return;
    }

    // Поиск заголовка gzip архива и вывод смещения
    std::streampos gzipOffset = findGzipHeader(data);
    std::cout << "Gzip header found at offset: " << gzipOffset  << std::endl;

    // Чтение данных от смещения до конца файла
    std::vector<char> readData = readFromOffset(data, gzipOffset);

    // Распаковка данных
    std::vector<char> decompressedData = decompressData(readData);

    // Поиск и чтение 8 байт после совпадения "Linux version "
    std::string result = findLinuxVersion(decompressedData);
    
    // Поиск сразу нескольких значений и определение битности ядра
    findArchitectureAndBitness(decompressedData);

    // Вывод результатов
    if (!result.empty()) {
        std::cout << "Linux version: " << result << std::endl;
    } else {
        std::cerr << "No Linux version found." << std::endl;
    }
}

// Основная функция
ProcessResult KerVer::process()
{

    std::string file = kernelFile.string();

    processFile(file);

    return ProcessResult::ok;
}
