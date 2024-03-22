#include "../include/main.hpp"

// Помощь по параметрам командной строки
void HexPatch::show_help()
{
    std::cout <<
R"***(
hexpatch

Usage:
  utils hexpatch <file> <hexstring_to_find> <hexstring_to_replace> <way>
where way is:
  0 - first find from begin file, by default
  1 - all finds in file
  -1 - reverse first find, from end file
)***";
}

// Парсить командную строку
ParseResult HexPatch::parse_cmd_line(int argc, char* argv[])
{
    if (argc < 3) { return ParseResult::not_enough; }
    image_file = argv[0];
    if (!parse_hexstring(argv[1], what_find) ||
        !parse_hexstring(argv[2], what_replace)) { return ParseResult::wrong_hex; }
    if (argc > 3) { way = atoi(argv[3]); }
    if (argc > 4 || way < -1 || way > 3) { return ParseResult::wrong_option; }
    return ParseResult::ok;
}

// Основная функция
ProcessResult HexPatch::process()
{
    std::cout << std::hex;
    auto result = find_in_file(
        [&](std::fstream& file, long long offset, long long number)
        {
            if (!file.seekp(offset)) { return ProcessResult::seek_error; }
            if (!file.write(what_replace.data(), what_replace.size()) || !file.flush()) { return ProcessResult::write_error; }
            std::cout << offset << "\n";
            return ProcessResult::ok;
        }, image_file, what_find, false, way != -1, 0, LLONG_MAX, way == 1 ? LLONG_MAX : 1);
    if (result.first != ProcessResult::ok) { --result.second; }
    std::cout << "Replaces: " << std::dec << result.second << '\n';
    return result.first;
}
