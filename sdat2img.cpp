#include "main.hpp"

// Помощь по параметрам командной строки
void Sdat2Img::show_help()
{
    fprintf(stderr, R"EOF(
sdat2img

Usage:
    sdat2img <transfer_list> <new_dat_file> <output_img_file>
    Where <output_img_file> path to output img file, optionaly
)EOF");
    fprintf(stderr, "\n");
}

// Парсить командную строку
ParseResult Sdat2Img::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 2 || argc > 3){show_help();}
    if (argc == 2)
    {
        std::string arg4;
        std::string s(argv[1]);
        arg4 = replace(s, "new.dat", "img");
        transf_file = argv[0];
        new_dat_file = argv[1];
        img_file =  arg4;
    }else if (argc == 3)
    {
        transf_file = argv[0];
        new_dat_file = argv[1];
        img_file =  argv[2];
    }else {return ParseResult::not_enough;}
    return ParseResult::ok;
}

// Основная функция
ProcessResult Sdat2Img::process()
{
    std::cout << "\n===================================================================" << std::endl;
    std::cout << "Designed by blackeangel (blackeangel@mail.ru) special for UKA tools" << std::endl;
    std::cout << "Re-written in C++ from xpirt - luxi78 - howellzhu work in python" << std::endl;
    std::cout << "===================================================================" << std::endl;

    std::string TRANSFER_LIST_FILE = transf_file.string();
    std::string NEW_DATA_FILE = new_dat_file.string();
    std::string OUTPUT_IMAGE_FILE = img_file.string();

    std::ifstream transfer_list_file(TRANSFER_LIST_FILE);
    if (!transfer_list_file.is_open())
    {
        std::cerr << TRANSFER_LIST_FILE << " not found!" << std::endl;
        exit(2);
    }
    transfer_list_file.close();

    parse_transfer_list_file(TRANSFER_LIST_FILE);

    std::ifstream new_data_file(NEW_DATA_FILE, std::ios::binary);
    if (!new_data_file.is_open())
    {
        std::cerr << NEW_DATA_FILE << " not found!" << std::endl;
        exit(2);
    }

    initOutputFile(OUTPUT_IMAGE_FILE);

    std::fstream output_img(OUTPUT_IMAGE_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!output_img.is_open())
    {
        std::cerr << OUTPUT_IMAGE_FILE << " not found!" << std::endl;
        exit(2);
    }

    uint8_t *data;
    bool quiet = false;
    for (std::pair<int, int> block : all_block_sets)
    {
        long begin = block.first;
        long end = block.second;
        long block_count = end - begin;
        long blocks = block_count * BLOCK_SIZE;
        //long long position = begin * BLOCK_SIZE;
        long long position = (long long)begin * BLOCK_SIZE;
        unsigned long offset = position % ULONG_MAX;
        int cycles = position / ULONG_MAX;

        data = (uint8_t *)malloc(blocks);
        if (data == NULL)
        {
            std::cerr << "Out of memory error!" << std::endl;
            exit(-1);
        }

        new_data_file.read((char *)data, blocks);

        // in the case of images greater than 4GB if its even possible
        if (cycles > 0)
        {
            output_img.seekp(0, std::ios::beg);
            for (int i = 0; i < cycles; i++)
            {
                output_img.seekp(ULONG_MAX, std::ios::cur);
            }
            output_img.seekp(offset, std::ios::cur);
        }
        else
        {
            output_img.seekp(position);
        }

        if (!quiet)
        {
            //cout << "Copying " << block_count << " blocks into position " << begin << " with " << blocks << " bytes" << endl;
        }

        output_img.write((char *)data, blocks);
        free(data);
    }

    output_img.close();
    new_data_file.close();

    std::cout << "\nDone!" << std::endl;

    return ProcessResult::ok;
}

std::pair<int, std::vector<std::pair<int, int>>> Sdat2Img::parse_transfer_list_file(std::string path)
{
    std::ifstream trans_list(path);
    int version;
    std::string line;
    getline(trans_list, line);
    version = atoi(line.c_str()); // First line in transfer list is the version number
    int new_blocks;
    getline(trans_list, line);
    new_blocks = atoi(line.c_str()); // Second line in transfer list is the total number of blocks we expect to write
    if (version == 1)
    {
        std::cout << "\nAndroid 5.0 detected!\n"
                  << std::endl;
    }
    else if (version == 2)
    {
        std::cout << "\nAndroid 5.1 detected!\n"
                  << std::endl;
    }
    else if (version == 3)
    {
        std::cout << "\nAndroid 6.x detected!\n"
                  << std::endl;
    }
    else if (version == 4)
    {
        std::cout << "\nAndroid 7.0+ detected!\n"
                  << std::endl;
    }
    else
    {
        std::cout << "\nUnknown Android version!\n"
                  << std::endl;
    }
    if (version >= 2)
    {
        trans_list.get(); // Third line is how many stash entries are needed simultaneously
        trans_list.get(); // Fourth line is the maximum number of blocks that will be stashed simultaneously
    }

    // Subsequent lines are all individual transfer commands
    while (getline(trans_list, line))
    {
        std::vector<std::string> line_split = split(line, ' ');
        std::string cmd = line_split[0];
        if (cmd == "new")
        {
            std::vector<std::pair<int, int>> block_sets = rangeset(line_split[1]);
            all_block_sets.insert(all_block_sets.end(), block_sets.begin(), block_sets.end());
        }
    }
    trans_list.close();

    int max_pair = 0;
    for (auto block : all_block_sets)
    {
        max_pair = std::max(max_pair, block.second);
    }
    max_file_size = max_pair * BLOCK_SIZE; // Rezult file size

    return make_pair(max_file_size, all_block_sets);
}

// Create empty image with Correct size;
void Sdat2Img::initOutputFile(std::string output_file)
{
    std::ofstream output_file_obj(output_file, std::ios::binary);
    long long position = max_file_size - 1;
    //cout << "A file will be created with the size " << position << " bytes" <<endl;
    unsigned long offset = position % ULONG_MAX;
    int cycles = position / ULONG_MAX;

    // in the case of images greater than 4GB if its even possible
    if (cycles > 0)
    {
        output_file_obj.seekp(0, std::ios::beg);
        for (int i = 0; i < cycles; i++)
        {
            output_file_obj.seekp(ULONG_MAX, std::ios::cur);
        }
        output_file_obj.seekp(offset, std::ios::cur);
    }
    else
    {
        output_file_obj.seekp(position);
    }

    output_file_obj.put('\0');
    output_file_obj.flush();
    output_file_obj.close();
}
