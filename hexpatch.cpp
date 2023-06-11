#include "main.hpp"

// Помощь по параметрам командной строки
void HexPatch::show_help()
{
    fprintf(stderr, R"EOF(
hexpatch

Usage:
  hexpatch <file> <what_find in hex> <what_replace in hex> <way>
      Where way as integer:
      way = 0 - first find from begin file, by default
      way = 1 - all finds in file
      way = -1 - reverse first find, from end file
)EOF");
    fprintf(stderr, "\n");

}

// Парсить командную строку
ParseResult HexPatch::parse_cmd_line(int argc, char* argv[])
{
    if(argc ==0){show_help();}
    if (argc == 3)
    {
        image_file = argv[0];
        whatfind = argv[1];
        whatreplace = argv[2];
    }else if (argc == 4)
    {
        image_file = argv[0];
        whatfind = argv[1];
        whatreplace = argv[2];
        way = atoi(argv[3]);
    }else{ return ParseResult::wrong_option;}

    return ParseResult::ok;
}

// Основная функция
ProcessResult HexPatch::process()
{
    std::string whatfindstr = viravn_offset(whatfind);
    std::string whatreplacestr = viravn_offset(whatreplace);
    if (whatfindstr.length() != whatreplacestr.length())
    {
        std::cout << "Length " << whatfind << " more than length " << whatreplace << std::endl;
        return static_cast<ProcessResult>(-1);
    }
    std::vector<char> data = hex2byte(whatfindstr.c_str());
    std::vector<char> data1 = hex2byte(whatreplacestr.c_str());
    std::vector<std::size_t> off;
    std::FILE *fin = std::fopen(image_file.string().c_str(), "rb");
    if (fin)
    {
        if (way == 0 || way == 1)
        {
            off = findOffsetInFile(fin, 256, (char *)data.data(), data.size(), way);
        }
        if (way == -1)
        {
            std::size_t offs = findBackwardOffsetInFile(fin, 256, (char *)data.data(), data.size());
            off.push_back(offs);
        }
    }
    fclose(fin);
    std::fstream img(image_file, std::ios::in | std::ios::out | std::ios::binary);
    for (auto &&offset : off)
    {
        img.seekp(offset);
        img.write(data1.data(), data1.size());
    }
    img.close();

    return ProcessResult::ok;
}

