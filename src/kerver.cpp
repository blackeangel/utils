#include "../include/main.hpp"

// Цвета для вывода
#ifdef _WIN32

    const std::string RED = "\x1B[31m";
    const std::string GREEN = "\x1B[32m";
    const std::string YELLOW = "\x1B[33m";
    const std::string BLUE = "\x1B[34m";
    const std::string MAGENTA = "\x1B[35m";
    const std::string CYAN = "\x1B[36m";
    const std::string WHITE = "\x1B[37m";
    const std::string RESET = "\x1B[0m";
#else
    const std::string RED = "\033[1;31m";
    const std::string GREEN = "\033[1;32m";
    const std::string YELLOW = "\033[1;33m";
    const std::string BLUE = "\033[1;34m";
    const std::string MAGENTA = "\033[1;35m";
    const std::string CYAN = "\033[1;36m";
    const std::string WHITE = "\033[1;37m";
    const std::string RESET = "\033[0m";
#endif

// Помощь по параметрам командной строки
void KerVer::show_help()
{
    fprintf(stderr, R"EOF(
Usage:

    kerver <file>)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult KerVer::parse_cmd_line(int argc, char* argv[])
{
    if(argc != 1){show_help();}
    kernelFile = argv[0];
    return ParseResult::ok;
}


// Функция для проверки размера файла
bool checkFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << RED << "Error: Unable to open file: " << filePath << RESET << std::endl;
        return false;
    }

    std::streampos fileSize = file.tellg();
    const std::streampos oneGB = 1e9;

    if (fileSize >= oneGB) {
        std::cerr << RED << "Error: File size is greater than or equal to 1 GB." << RESET << std::endl;
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
        std::cerr << RED << "Error: Offset not found." << RESET << std::endl;
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
        std::cerr << RED << "Error: Failed to initialize zlib." << RESET << std::endl;
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
                std::cout << GREEN << "Architecture: 64 bit" << RESET << std::endl;
            } else if (target.find("arm") != std::string::npos) {
                std::cout << GREEN << "Architecture: 32 bit" << RESET << std::endl;
            }
            return;
        }
    }
    std::cerr << RED << "Architecture not found." << RESET << std::endl;
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
        std::cerr << RED << "Error: Unable to open file: " << filePath << RESET << std::endl;
        return;
    }

    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Поиск и чтение 8 байт после совпадения "Linux version "
    std::string linuxVersion = findLinuxVersion(data);

    // Если результат не пуст, выводим его
    if (!linuxVersion.empty()) {
        std::cout << GREEN << "Linux version: " << linuxVersion << RESET << std::endl;
        // Поиск сразу нескольких значений и определение битности ядра
        findArchitectureAndBitness(data);
        return;
    }

    // Поиск заголовка gzip архива и вывод смещения
    std::streampos gzipOffset = findGzipHeader(data);
    std::cout << GREEN << "Gzip header found at offset: " << gzipOffset  << RESET << std::endl;

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
        std::cout << GREEN << "Linux version: " << result << RESET << std::endl;
    } else {
        std::cerr << RED << "No Linux version found." << RESET << std::endl;
    }
}

// Основная функция
ProcessResult KerVer::process()
{

    std::string file = kernelFile.string();

    processFile(file);

    return ProcessResult::ok;
}
