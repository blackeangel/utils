#include "../include/main.hpp"

// Помощь по параметрам командной строки
void Insert::show_help()
{
    fprintf(stderr, R"EOF(
insert

Usage:

    insert <file> <offset> <insert_file>
)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult Insert::parse_cmd_line(int argc, char* argv[])
{
    if(argc != 3){show_help();}
    image_file = argv[0];
    start_offset =  argv[1];
    insert_file = argv[2];
    return ParseResult::ok;
}

// Основная функция
ProcessResult Insert::process()
{
    if (!std::filesystem::exists(image_file))
    {
        std::cerr << image_file << " not found!" << std::endl;
        exit(2);
    }
    size_t size_img = std::filesystem::file_size(image_file);
    long start_position;
    std::string tmpstr(start_offset);
    size_t start_pos = tmpstr.find("0x");
    if (start_pos != std::string::npos)
    {
        tmpstr = tmpstr.substr(2, tmpstr.length());
    }
    start_position = string2long(tmpstr);

    // взвесим всовываемый файл
    size_t size_insert = std::filesystem::file_size(insert_file);
    std::string insert_image_name = image_file.stem().string() + "_inserted" + image_file.extension().string();
    std::filesystem::path output_file = image_file.parent_path();
    output_file /= insert_image_name;
    std::ofstream out(output_file, std::ios::binary);
    out.close();
    out.open(output_file, std::ios::binary | std::ios::app);
    std::ifstream in(image_file, std::ios::binary);
    std::ifstream in1(insert_file, std::ios::binary);
    if (start_position == 0)
    {
        //если 0, то кидаем с начала файла один файл, затем другой
        partialCopy(in1, out, 0, size_insert, BLOCK_SIZE);
        partialCopy(in, out, 0, size_img, BLOCK_SIZE);
    }
    else
    {
        //кидаем сначала 1 часть, потом вставляем нужныц файл, затем пишем хвост
        partialCopy(in, out, 0, start_position, BLOCK_SIZE);
        partialCopy(in1, out, 0, size_insert, BLOCK_SIZE);
        partialCopy(in, out, start_position, size_img, BLOCK_SIZE);
    }
    //а тута надо сделать проверку на дописывание в конец файла

    in.close();
    in1.close();
    out.close();

    return ProcessResult::ok;
}
