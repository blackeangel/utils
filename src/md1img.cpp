// md1img.cpp — UtilBase wrapper for md1img_repacker v0.5
// Full logic lives in src/md1img/{pack,unpack,compress,decompress,utils}.cpp

#include "../include/main.hpp"
#include "md1img/main.h"

void process_file(const std::string& input_path, const std::string& output_dir);
void pack_files(const std::string& input_dir, const std::string& output_file);

void MD1IMG::show_help() {
    std::cout <<
        "\nmd1img\n\n"
        "Usage:\n\n"
        "  md1img <pack|unpack> <input> [output_dir]\n\n"
        "  unpack <md1img.img> [output_dir]\n"
        "      Unpack a md1img/modemd image into a directory.\n"
        "      output_dir defaults to <input_basename> next to the image.\n\n"
        "  pack <input_dir> [output_file]\n"
        "      Pack a directory back into an md1img/modemd image.\n"
        "      output_file defaults to <dir_name>-new.img next to the directory.\n\n"
        "  Supported compression: gz, xz (XZ container), lzma (raw stream)\n\n";
}

ParseResult MD1IMG::parse_cmd_line(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return ParseResult::not_enough;
    }

    mode       = argv[0];
    input_path = argv[1];

    if (mode == "unpack") {
        if (argc > 2) {
            output_dir = argv[2];
        } else {
            std::filesystem::path p(input_path);
            output_dir = (p.parent_path() / p.stem()).string();
        }
    } else if (mode == "pack") {
        if (argc > 2) {
            output_file = argv[2];
        }
    } else {
        std::cerr << "Invalid mode: '" << mode << "'. Use 'pack' or 'unpack'.\n";
        show_help();
        return ParseResult::wrong_option;
    }

    return ParseResult::ok;
}

ProcessResult MD1IMG::process() {
    if (mode == "unpack") {
        try {
            process_file(input_path, output_dir);
        } catch (const std::exception& e) {
            std::cerr << "md1img unpack error: " << e.what() << "\n";
            return ProcessResult::breaked;
        }
        return ProcessResult::ok;
    }

    std::filesystem::path dir_path(input_path);
    if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
        std::cerr << "Error: '" << input_path << "' does not exist or is not a directory.\n";
        return ProcessResult::open_error;
    }

    std::string out = output_file;
    if (out.empty()) {
        out = (dir_path.parent_path() / (dir_path.stem().string() + "-new.img")).string();
    }

    try {
        pack_files(input_path, out);
    } catch (const std::exception& e) {
        std::cerr << "md1img pack error: " << e.what() << "\n";
        return ProcessResult::write_error;
    }
    return ProcessResult::ok;
}
