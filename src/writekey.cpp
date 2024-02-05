#include "../include/main.hpp"

// Помощь по параметрам командной строки
void WriteKey::show_help()
{
    fprintf(stderr, R"EOF(
writekey

Usage:
    writekey <image_file> <offset> <-fhb> <file_key_value>
        Where:
        -b backup
        -f file key
        -h hex value
)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult WriteKey::parse_cmd_line(int argc, char* argv[])
{
    if(argc != 4){show_help();}
    image_file = argv[0];
    foffset = argv[1];
    key = argv[2];
    file_value = argv[3];
    return ParseResult::ok;
}

// Основная функция
ProcessResult WriteKey::process()
{
    std::string keys(key);
    if (!std::filesystem::exists(image_file))
    {
        std::cerr << image_file << " not found!" << std::endl;
        exit(2);
    }
    if (keys == "-b"||keys == "-fb"||keys == "-bf"||keys == "-hb"||keys == "-bh")
    {
        std::string fname(image_file.string());
        std::filesystem::copy(image_file, fname + "_backup");
    }
    std::string tmpstr = viravn_offset(foffset);
    long position = string2long(tmpstr);
    size_t size_img = std::filesystem::file_size(image_file);
    std::fstream output_img(image_file, std::ios::in | std::ios::out | std::ios::binary);
    output_img.seekp(position);
    if (keys == "-f"||keys == "-bf"||keys == "-fb")
    {
        std::filesystem::path file_key = file_value;
        if (!std::filesystem::exists(file_key))
        {
            std::cerr << file_key << " not found!" << std::endl;
            exit(2);
        }
        uint8_t *data;
        size_t size = std::filesystem::file_size(file_key);
        data = (uint8_t *)malloc(size); //выделили память
        if ((position + size) > size_img)
        {
            std::cout << "Offset + size key out of file size!" << std::endl;
            exit(-1);
        }

        //read all key -->
        std::ifstream key_file(file_key);
        key_file.seekg(0);
        key_file.read((char *)data, size);
        key_file.close();
        //read all key <--

        //open img and seekp and write -->
        output_img.write((char *)data, size);
        free(data);
        //open img and seekp and write <--
    }
    if(keys == "-h"||keys == "-bh"||keys == "-hb"){
        std::string tmpstr = viravn_offset(file_value);
        std::vector<char> data = hex2byte(tmpstr.c_str());
        //open img and seekp and write -->
        output_img.write(data.data(), data.size());
        //free(data);
        //open img and seekp and write <--
    }
    output_img.close();
    std::cout << "Done!" << std::endl;

    return ProcessResult::ok;
}
