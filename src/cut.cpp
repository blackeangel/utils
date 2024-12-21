#include "../include/main.hpp"

// Помощь по параметрам командной строки
void Cut::show_help() {
    fprintf(stderr, R"EOF(
cut

Usage:

    cut <source_file> -d|-h|-o <start_position> -d|-h|-o <length_or_offset> [<output_file>]
                    -d: Specify value in decimal format
                    -h: Specify value in hexadecimal format
                    -o: Specify value as an offset
)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult Cut::parse_cmd_line(int argc, char *argv[]) {
    if (argc != 5 && argc != 6) {
        show_help();
        return ParseResult::not_enough;
    }
    // Разбор аргументов
    image_file = argv[0];
    startFlag = argv[1];
    start_offset = argv[2];
    lengthFlag = argv[3];
    end_offset_length = argv[4];

    if (argc == 6) {
        output_file = argv[5];
    } else {
        output_file = image_file.parent_path() / (image_file.stem().string() + "_cuted" + image_file.extension().string());
    }

    return ParseResult::ok;
}


void cutBinaryData(const std::string& sourcePath, size_t startPos, size_t length, const std::string& removedFilePath) {
    // Открытие исходного файла
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile) {
        throw std::runtime_error("Failed to open source file: " + sourcePath);
    }

    // Проверка размера файла
    sourceFile.seekg(0, std::ios::end);
    size_t fileSize = sourceFile.tellg();
    if (startPos >= fileSize) {
        throw std::out_of_range("Start position is out of range.");
    }

    size_t endPos = startPos + length;
    if (endPos > fileSize) {
        endPos = fileSize;
    }

    sourceFile.seekg(0, std::ios::beg);

    // Создание временного файла
    std::filesystem::path tempFilePath = sourcePath + ".tmp";
    std::ofstream tempFile(tempFilePath, std::ios::binary);
    if (!tempFile) {
        throw std::runtime_error("Failed to create temporary file: " + tempFilePath.string());
    }

    // Создание файла для сохранения удалённых данных
    std::ofstream removedFile(removedFilePath, std::ios::binary);
    if (!removedFile) {
        throw std::runtime_error("Failed to create removed data file: " + removedFilePath);
    }

    // Чтение данных до `startPos` и запись их во временный файл
    std::vector<char> buffer(8 * 1024 * 1024); // 8 MB buffer
    size_t bytesWritten = 0;

    while (sourceFile.tellg() < static_cast<std::streamsize>(startPos)) {
        size_t bytesToRead = std::min(buffer.size(), startPos - bytesWritten);
        sourceFile.read(buffer.data(), bytesToRead);
        size_t bytesRead = sourceFile.gcount();
        tempFile.write(buffer.data(), bytesRead);
        bytesWritten += bytesRead;

        if (bytesRead == 0) break;
    }

    // Копирование данных из удаляемого участка в новый файл
    size_t removedBytes = 0;
    while (removedBytes < length) {
        size_t bytesToRead = std::min(buffer.size(), length - removedBytes);
        sourceFile.read(buffer.data(), bytesToRead);
        size_t bytesRead = sourceFile.gcount();
        removedFile.write(buffer.data(), bytesRead);
        removedBytes += bytesRead;

        if (bytesRead == 0) break;
    }

    // Чтение данных после `endPos` и запись их во временный файл
    while (sourceFile) {
        sourceFile.read(buffer.data(), buffer.size());
        size_t bytesRead = sourceFile.gcount();
        if (bytesRead > 0) {
            tempFile.write(buffer.data(), bytesRead);
        }
    }

    sourceFile.close();
    tempFile.close();
    removedFile.close();

    // Замена оригинального файла временным
    std::filesystem::rename(tempFilePath, sourcePath);
}

// Основная функция
ProcessResult Cut::process() {
    if (!std::filesystem::exists(image_file)) {
        std::cerr << image_file << " not found!" << std::endl;
        exit(2);
    }
    try {
        // Определение форматов
        bool startIsDec = false;
        bool lengthIsDec = false;
        bool startIsHex = false;
        bool lengthIsHex = false;
        bool startIsOffset = false;
        bool lengthIsOffset = false;
        if (startFlag == "-d") { startIsDec = true; }
        if (lengthFlag == "-d") { lengthIsDec = true; }
        if (startFlag == "-h") { startIsHex = true; }
        if (lengthFlag == "-h") { lengthIsHex = true; }
        if (startFlag == "-o") { startIsOffset = true; }
        if (lengthFlag == "-o") { lengthIsOffset = true; }

        // Преобразование значений
        size_t startPos;
        if (startIsHex || startIsOffset) {
            startPos = parseSize(start_offset, true);
        } else {
            startPos = parseSize(start_offset, false);
        }

        size_t length;

        if (lengthIsOffset) {
            size_t offset = parseSize(end_offset_length, lengthIsOffset);
            if (offset <= startPos) {
                throw std::invalid_argument("Offset must be greater than the start position.");
            }
            length = offset; //offset - startPos;
        } else {
            length = startPos + parseSize(end_offset_length, lengthIsHex);
        }

        // Вырезание данных
        cutBinaryData(image_file.string(), startPos, length, output_file.string());
        std::cout << "Data successfully cut from " << image_file << " and saved to " << output_file << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return ProcessResult::breaked;
    }
    return ProcessResult::ok;
}
