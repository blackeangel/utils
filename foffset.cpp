#include "main.hpp"

// Помощь по параметрам командной строки
void Foffset::show_help()
{
    std::cout <<
R"***(Usage:
  utils foffset file hexstring [option...]
where option is:
  -s offset - start offset (DEC or 0xHEX), negative value for offset from end of file
  -l length - search region length (DEC or 0xHEX)
  -n num - maximum number of offsets (DEC or 0xHEX)
  -b size - size of buffer in KB (DEC or 0xHEX), default is 1024
  -r - reverse search
  -d - display offsets in decimal (default is hex)
  -hu - display offsets in uppercase hex (only if -d option is not specified)
  -hp - display offset 0x prefix for hex (only if -d option is not specified)
  -hup - combination of -hu and -hp options
  -o - display offsets in one string separated by space
  -q - no statistic message
hexstring digit pairs that can be separated using space, period(.), comma(,), dash(-) or colon(:), e.g. 41.35:AA.55
)***";
}

// Парсить командную строку
ParseResult Foffset::parse_cmd_line(int argc, char* argv[])
{
    if (argc < 2) { return ParseResult::not_enough; }

    filename = argv[0];
    
    // Hexstring
    if (!parse_hexstring(argv[1], search_str)) { return ParseResult::wrong_hex; }
    
    // Опции
    for (int i = 2; i < argc; ++i) {
        if (argv[i] == "-s"sv) {
            if (++i >= argc || !parse_llong(argv[i], start_offset)) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-l"sv) {
            if (++i >= argc || !parse_llong(argv[i], search_size) || search_size < 0) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-n"sv) {
            if (++i >= argc || !parse_llong(argv[i], max_count) || max_count < 1) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-b"sv) {
            if (++i >= argc || !parse_llong(argv[i], buf_size) || buf_size < 1) { return ParseResult::wrong_option; }
            buf_size *= 1024;
            if (buf_size < (long long)search_str.size()) { return ParseResult::wrong_option; }
        } else if (argv[i] == "-r"sv) { forward = false; }
        else if (argv[i] == "-d"sv) { hex_offsets = false; }
        else if (argv[i] == "-hu"sv) { hex_uppercase = true; }
        else if (argv[i] == "-hp"sv) { hex_prefix = true; }
        else if (argv[i] == "-hup"sv) { hex_uppercase = hex_prefix = true; }
        else if (argv[i] == "-o"sv) { sep_char = ' '; }
        else if (argv[i] == "-q"sv) { show_stat = false; }
        else { return ParseResult::wrong_option; }
    }
    return ParseResult::ok;
}

// Искать строку в файле
ProcessResult Foffset::process()
{
    // Режим вывода чисел
    if (hex_offsets) {
        std::cout << std::hex;
        if (hex_uppercase) { std::cout << std::uppercase; }
    }

    auto result = find_in_file(
        [&](std::fstream& file, long long offset, long long number)
        {
            if (number > 1) { std::cout << sep_char; }
            if (hex_offsets && hex_prefix) { std::cout << "0x"; }
            std::cout << offset;
            return ProcessResult::ok;
        }, filename, search_str, true, forward, start_offset, search_size, max_count, buf_size);
    if (result.second > 0) { std::cout << '\n'; }
    if (result.first == ProcessResult::ok && show_stat) {
        std::cout << "Offsets found: " << std::dec << result.second << '\n';
    }
    return result.first;
}
