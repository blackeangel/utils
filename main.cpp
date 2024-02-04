#include "main.hpp"

constexpr const char* COMMON_CMDLINE_HELP =
R"***(Usage:
  utils function [option...]
where function is:
  help - show this help
  help function - show function help

  delgaaps <folder> <file_list>
        Where:
        <folder> - path to the folder where the image was unpacked
        <file_list> - a file with a list to remove gapps and more
        For example:
        delgaaps "/sdcard/Unpacker" "/sdcard/gappslist.txt"

  sdat2img <transfer_list> <new_dat_file> <output_img_file>
        Where <output_img_file> path to output img file, optionaly

  writekey <image_file> <offset> <-fhb> <file_key_value>
        Where:
        -b backup
        -f file key
        -h hex value

  cut <file> <offset> <offset or length> <cute_dir>

  copy <file> <offset> <offset or length> <copy_dir>

  insert <file> <offset> <insert_file>

  foffset <file> <hexstring> [option...]
        where option is:
        -s <offset> - start offset (DEC or 0xHEX), negative value for offset from end of file
        -l <length> - search region length (DEC or 0xHEX)
        -n <num> - maximum number of offsets (DEC or 0xHEX)
        -b <size> - size of buffer in KB (DEC or 0xHEX), default is 1024
        -r - reverse search
        -d - display offsets in decimal (default is hex)
        -hu - display offsets in uppercase hex (only if -d option is not specified)
        -hp - display offset 0x prefix for hex (only if -d option is not specified)
        -hup - combination of -hu and -hp options
        -o - display offsets in one string separated by space
        -q - no statistic message
        hexstring digit pairs that can be separated using:
            space, period(.), comma(,), dash(-) or colon(:), e.g. 41.35:AA.55

  hexpatch <file> <what_find in hex> <what_replace in hex> <way>
      Where way as integer:
      way = 0 - first find from begin file, by default
      way = 1 - all finds in file
      way = -1 - reverse first find, from end file

  kerver <kernel file>
        kernel file may be compressed to gzip
)***";

std::unique_ptr<UtilBase> make_object(const char* name)
{
    if (name == "delgaaps"sv) {
        return std::make_unique<DelGapps>();
    }
    if (name == "sdat2img"sv) {
        return std::make_unique<Sdat2Img>();
    }
    if (name == "writekey"sv) {
        return std::make_unique<WriteKey>();
    }
    if (name == "cut"sv) {
        return std::make_unique<Cut>();
    }
    if (name == "copy"sv) {
        return std::make_unique<Copy>();
    }
    if (name == "insert"sv) {
        return std::make_unique<Insert>();
    }
    if (name == "foffset"sv) {
        return std::make_unique<Foffset>();
    }
    if (name == "hexpatch"sv) {
        return std::make_unique<HexPatch>();
    }
    if (name == "kerver"sv) {
        return std::make_unique<KerVer>();
    }
    return nullptr;
}

int main(int argc, char* argv[])
{
//std::cout << sizeof(long long) << std::endl;
    // Парсинг названия функции или помощи функции
    if (argc < 2 || argv[1] == "help"sv) {
        std::unique_ptr<UtilBase> obj = nullptr;
        if (argc > 2)
        {
            obj = make_object(argv[2]);
        }
        if (obj == nullptr)
        {
           std::cout << COMMON_CMDLINE_HELP;
        }
        else
        {
           obj->show_help();
        }
        return EXIT_FAILURE;
    }
    auto obj = make_object(argv[1]);
    if (obj == nullptr) {
        std::cerr << "Wrong function name is specified!\n";
        return EXIT_FAILURE;
    }
    // Парсинг параметров конкретной функции
    switch (obj->parse_cmd_line(argc - 2, argv + 2))
    {
        case ParseResult::not_enough:
            obj->show_help();
//            std::cerr << "Not enough command line parameters!\n";
            return EXIT_FAILURE;
        case ParseResult::wrong_hex:
            std::cerr << "Wrong hexstring is specified!\n";
            return EXIT_FAILURE;
        case ParseResult::wrong_option:
            std::cerr << "Wrong option or option value is specified!\n";
            return EXIT_FAILURE;
        case ParseResult::ok:
            break;
        default:
            return EXIT_FAILURE;  // вообще, такого быть не должно, просто на всякий случай!
    }
    // Поиск
    switch (obj->process()) {
        case ProcessResult::open_error:
            std::cerr << "File open error!\n";
            return EXIT_FAILURE;
        case ProcessResult::seek_error:
            std::cerr << "File seek error!\n";
            return EXIT_FAILURE;
        case ProcessResult::read_error:
            std::cerr << "File read error!\n";
            return EXIT_FAILURE;
        case ProcessResult::write_error:
            std::cerr << "File write error!\n";
            return EXIT_FAILURE;
        case ProcessResult::ok:
            break;
        default:
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
