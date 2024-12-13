#include "../include/main.hpp"

// Помощь по параметрам командной строки
void Copy::show_help() {
    fprintf(stderr, R"EOF(
copy

Usage:

    copy <source_file> -d|-h|-o <start_position> -d|-h|-o <length_or_offset> [<output_file>]
                    -d: Specify value in decimal format
                    -h: Specify value in hexadecimal format
                    -o: Specify value as an offset
)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult Copy::parse_cmd_line(int argc, char *argv[]) {

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
        output_dir = argv[5];
    } else {
        output_dir = image_file.parent_path();
    }

     return ParseResult::ok;
}

// Основная функция
ProcessResult Copy::process() {
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
        if (startFlag == "-d") {startIsDec = true;}
        if (lengthFlag == "-d") {lengthIsDec = true;}
        if (startFlag == "-h") {startIsHex = true;}
        if (lengthFlag == "-h") {lengthIsHex = true;}
        if (startFlag == "-o") {startIsOffset = true;}
        if (lengthFlag == "-o") {lengthIsOffset = true;}

        // Преобразование значений
        size_t startPos = parseSize(start_offset, startIsHex);
        size_t length;

        if (lengthIsOffset) {
            size_t offset = parseSize(end_offset_length, lengthIsOffset);
            if (offset <= startPos) {
                throw std::invalid_argument("Offset must be greater than the start position.");
            }
            length = offset;//offset - startPos;
        } else {
            length = startPos + parseSize(end_offset_length, lengthIsHex);
        }

        std::string new_image_name = image_file.stem().string() + "_copied" + image_file.extension().string();
        std::filesystem::path output_file = output_dir;
        output_file /= new_image_name;
        std::ofstream out(output_file, std::ios::binary);
        std::ifstream in(image_file, std::ios::binary);
        if (in.is_open() && out.is_open()) {
            auto rd = partialCopy(in, out, startPos, length, 4096);
            std::cout << rd << " bytes was copied\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return ProcessResult::breaked;
    }
    return ProcessResult::ok;
}
