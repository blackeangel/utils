#include "../include/main.hpp"

#if defined(_WIN32) || defined(_WIN64)
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

namespace fs = std::filesystem;

void FileExplorer::show_help()
{
    std::cout <<
              R"***(
file_explorer

Usage:

  file_explorer [directory] [options] [filters]
    Explore files and directories in the specified directory.\n"
                 Options:
                 -d    Show only directories, ignoring filters.
                 -f    Show only files, applying filters.
                 Filters:
                 By default, the following filters are applied to files: .img, .dat, .br, .list.
                 Additional filters can be specified after the directory path.
                 "Example: file_explorer /path/to/directory .txt .pdf
)***";
}

void clearScreen()
{
    printf("\033c");     //Reset terminal
    printf("\033[2J");   // очистка экрана
    printf("\033[0;0f"); // перемещение курсора в верхний левый угол
}

bool comparePaths(const fs::directory_entry &a, const fs::directory_entry &b)
{
    // Получаем имена файлов для сравнения без учета регистра
    std::string filenameA = a.path().filename().string();
    std::string filenameB = b.path().filename().string();
    std::transform(filenameA.begin(), filenameA.end(), filenameA.begin(), ::tolower);
    std::transform(filenameB.begin(), filenameB.end(), filenameB.begin(), ::tolower);

    // Сравниваем имена файлов без учета регистра
    if (fs::is_directory(a) && !fs::is_directory(b))
    {
        return true;
    }
    else if (!fs::is_directory(a) && fs::is_directory(b))
    {
        return false;
    }
    else
    {
        return filenameA < filenameB;
    }
}

std::string getFileType(const std::string &filename)
{
    if (filename.size() >= 4 && (filename.substr(filename.size() - 4) == ".img" || filename.substr(filename.size() - 4) == ".dat"))
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Unable to open file: " << filename << std::endl;
            return "";
        }

        const int maxBytesToRead = 4194304; // Читаем только первые несколько килобайт файла
        std::vector<char> content(maxBytesToRead);
        file.read(content.data(), maxBytesToRead);

        // Получаем количество прочитанных байт
        std::streamsize bytesRead = file.gcount();

        // Проверяем тип файла на Sparse
        if (is_sparse(content))
        {
            return "Android Sparse Image";
        }

        // Проверяем тип файла на Android Boot Image
        if (is_boot(content))
        {
            return "Android Boot Image";
        }

        // Проверяем тип файла на EROFS
        if (is_erofs(content))
        {
            return "EROS File System (EROFs)";
        }

        // Проверяем тип файла на Ext4
        if (is_ext4(content))
        {
            return "Ext4 File System";
        }

        // Если не удалось определить тип файла
        return "Unknown";
    }
    else
    {
        return "";
    }
}

void exploreDirectory(const std::string &folder_path, const std::vector<std::string> &filters, bool showDirectories, bool showFiles, std::stack<std::string> &directory_stack)
{
    try
    {
        clearScreen();
        std::cout << "\033[1;32m";
        std::cout << "Текущая директория: " << folder_path << std::endl;
        std::cout << "\033[0m";

        std::vector<fs::directory_entry> entries;
        for (const auto &entry : fs::directory_iterator(folder_path))
        {
            if (fs::is_regular_file(entry))
            {
                std::string extension = entry.path().extension().string();
                bool matched = false;
                for (const auto &filter : filters)
                {
                    if (extension == filter)
                    {
                        matched = true;
                        break;
                    }
                }
                if (matched && showFiles)
                {
                    entries.push_back(entry);
                }
            }
            else if (fs::is_directory(entry) && showDirectories)
            {
                entries.push_back(entry);
            }
        }

        std::sort(entries.begin(), entries.end(), comparePaths);

        int index = 1;
        for (const auto &entry : entries)
        {
            std::cout << index << " ";
            std::string filename = entry.path().filename().string();
            if (fs::is_directory(entry))
            {
                std::cout << "\033[93m";
                std::cout << filename << "/";
            }
            else
            {
                std::cout << filename;
                if (filename.size() >= 4 && (filename.substr(filename.size() - 4) == ".img" || filename.substr(filename.size() - 4) == ".dat"))
                {
                    std::string type = getFileType(entry.path().string());
                    if (!type.empty())
                    {
                        std::cout << " (" << type << ")";
                    }
                }
            }
            std::cout << "\033[0m" << std::endl;
            ++index;
        }

        std::cout << "\033[1;31m";
        std::cout << "b: назад  \n";
        std::cout << "c: выбрать текущую директорию  \n";
        std::cout << "e: выход\n"
                  << std::endl;
        std::cout << "\033[0m";

        std::string input;
        while (true)
        {
            std::cout << "Введите номер выбранного элемента: ";
            std::cin >> input;

            if (input == "b" || input == "c" || input == "e")
            {
                break;
            }
            else
            {
                try
                {
                    int selected_item = std::stoi(input);
                    if (selected_item >= 1 && selected_item < index)
                    {
                        break;
                    }
                    else
                    {
                        exploreDirectory(folder_path, filters, showDirectories, showFiles, directory_stack);
                    }
                }
                catch (const std::exception &e)
                {
                    exploreDirectory(folder_path, filters, showDirectories, showFiles, directory_stack);
                }
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if (input == "b")
        {
            if (directory_stack.size() > 1)
            {
                directory_stack.pop();
                exploreDirectory(directory_stack.top(), filters, showDirectories, showFiles, directory_stack);
            }
            else
            {
                // Если стек пустой, заполняем его относительно корневой папки и делаем шаг назад
                fs::path root_path = "/";
                directory_stack.push(folder_path);
                while (directory_stack.top() != root_path.string())
                {
                    directory_stack.push(fs::absolute(directory_stack.top()).parent_path().string());
                }
                //directory_stack.pop(); // Удаляем последний элемент, который будет "/"
                exploreDirectory(directory_stack.top(), filters, showDirectories, showFiles, directory_stack);
            }
            return;
        }
        else if (input == "c")
        {
            clearScreen();
            std::cout << "\033[1;32m";
            std::cout << "Текущая директория: " << folder_path << std::endl;
            std::cout << "\033[0m";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            exploreDirectory(folder_path, filters, showDirectories, showFiles, directory_stack);
            return;
        }
        else if (input == "e")
        {
            return;
        }

        int selected_item = std::stoi(input);
        auto it = entries.begin();
        std::advance(it, selected_item - 1);
        std::string selected_path = it->path().string();

        if (fs::is_directory(*it))
        {
            directory_stack.push(selected_path);
            exploreDirectory(selected_path, filters, showDirectories, showFiles, directory_stack);
        }
        else
        {
            clearScreen();
            std::cout << "\033[1;32m";
            std::cout << "Полный путь к выбранному файлу: " << selected_path << std::endl;
            std::cout << "\033[0m";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            exploreDirectory(folder_path, filters, showDirectories, showFiles, directory_stack);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}


ParseResult FileExplorer::parse_cmd_line(int argc, char* argv[])
{
    //std::cout << argc << std::endl;
    if(argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    /*if (argc == 1)
    {
        if(fs::exists(argv[0]))
        {
            initial_directory = argv[0];
        }
    }*/
    // Проверяем наличие аргументов командной строки
    if (argc >= 1)
    {
        int last_iter = 0;
        // Перебираем аргументы
        for (int i = 0; i < argc; ++i)
        {
            if(fs::exists(argv[i]))
            {
                initial_directory = argv[i];
                last_iter += 1;
                continue;
            }
            // Проверяем ключи
            if (std::string(argv[i]) == "-d")
            {
                showFiles = false;
                showDirectories = true;
                last_iter += 1;
                continue;
            }
           if (std::string(argv[i]) == "-f")
            {
                showFiles = true;
                showDirectories = false;
                last_iter += 1;
                continue;
            }
        }
        // Собираем фильтры из оставшихся аргументов командной строки
        if ((argc - last_iter) > 0)
        {
            filters.clear();
            for (int i = last_iter; i < argc; ++i)
            {
                filters.emplace_back(argv[i]);
            }
        }
    }



    return ParseResult::ok;
}

ProcessResult FileExplorer::process()
{
    // Создаем стек директорий
    std::stack<std::string> directory_stack;
    // Добавляем начальную и текущию дерриктории
    directory_stack.push(absolute(initial_directory.root_path()).string());
    directory_stack.push(absolute(initial_directory).string());

    // Запускаем обход директории
    exploreDirectory(initial_directory.string(), filters, showDirectories, showFiles, directory_stack);

    return ProcessResult::ok;
}
