#include "main.hpp"

// Помощь по параметрам командной строки
void Cut::show_help()
{
    fprintf(stderr, R"EOF(
cut

Usage:

    cut <file> <offset> <offset or length> <cute_dir>
)EOF");
    fprintf(stderr, "\n");

}

// Парсить командную строку
ParseResult Cut::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 3 || argc > 4){show_help();}
    image_file = argv[0];
    start_offset =  argv[1];
    end_offset_length = argv[2];

    if (argc == 3)
    {
        output_dir = image_file.parent_path();
    }else if (argc == 4)
    {
        output_dir = argv[3];
    }else{return ParseResult::not_enough;}
    return ParseResult::ok;
}

// Основная функция
ProcessResult Cut::process()
{
    if (!std::filesystem::exists(image_file))
    {
        std::cerr << image_file << " not found!" << std::endl;
        exit(2);
    }
    long start_position;
    long end_position;
    std::string tmpstr(start_offset);
    size_t start_pos = tmpstr.find("0x");
    if (start_pos != std::string::npos)
    {
        tmpstr = tmpstr.substr(2, tmpstr.length());
    }
    start_position = string2long(tmpstr);

    tmpstr = end_offset_length;
    start_pos = tmpstr.find("0x");

    if (start_pos != std::string::npos)
    {
        tmpstr = tmpstr.substr(2, tmpstr.length());
        end_position = string2long(tmpstr);
    }
    else
    {
        long tmplong = atol(end_offset_length);
        end_position = start_position + tmplong;
    }

    std::string new_image_name = image_file.stem().string() + "_copied" + image_file.extension().string();
    std::filesystem::path output_file = output_dir;
    output_file /= new_image_name;
    std::ofstream out(output_file, std::ios::binary);
    std::ifstream in(image_file, std::ios::binary);
    if (in.is_open() && out.is_open())
    {
        auto rd = partialCopy(in, out, start_position, end_position, 4096);
        std::cout << rd << " bytes was copied\n";
    }

    return ProcessResult::ok;
}
