#include "../include/main.hpp"
// Помощь по параметрам командной строки
void FstabFix::show_help()
{
    std::cout <<
    R"***(
fstab_fix

Usage:
  utils fstab_fix <directory1> <directory2> ...
)***";
}

// Структура для хранения данных строки в файле fstab
struct FstabEntry
{
    std::string line;
    std::string source;
    std::string mountPoint;
    std::string type;
    size_t type_pos;  // позиция type в line
    std::string mountFlags;
    size_t mountFlags_pos;  // позиция mountFlags в line
    std::string fsMgrFlags;

    // Конструктор по умолчанию
    FstabEntry() = default;

    // Конструктор для удобного создания FstabEntry из строки
    FstabEntry(std::string line) {
        ss << line;
        ss >> source;
        if (!source.starts_with("#")) {
            ss >> mountPoint >> type;
            type_pos = static_cast<size_t>(ss.tellg()) - type.size();
            ss >> mountFlags;
            mountFlags_pos = static_cast<size_t>(ss.tellg()) - mountFlags.size();
            ss >> fsMgrFlags;
        } else {
            source.clear();
        }
        this->line = std::move(line);
    }

private:
    std::stringstream ss;
};

// Специализация std::hash для двух строк
template<>
struct std::hash<std::pair<std::string, std::string>>
{
    std::size_t operator()(const std::pair<std::string, std::string>& s) const noexcept
    {
        std::size_t h1 = std::hash<std::string>{}(s.first);
        std::size_t h2 = std::hash<std::string>{}(s.second);
        return h1 ^ (h2 << 1);
    }
};

// Функция замены прав ro -> rw для всех типов, кроме erofs
void updateRights(FstabEntry& entry)
{
    if (entry.type == "erofs") { return; }
    if (entry.mountFlags.starts_with("ro")) {
        entry.line[entry.mountFlags_pos + 1] = 'w';  // в начале строки
    } else if (entry.mountFlags.ends_with("ro")) {
        entry.line[entry.mountFlags_pos + entry.mountFlags.size() - 1] = 'w';  // в конце строки
    } else {
        size_t pos = entry.mountFlags.find(",ro,");
        if (pos != std::string::npos) {
            entry.line[entry.mountFlags_pos + pos + 2] = 'w';  // в середине строки
        }
    }
}
void progress(const std::string &filename){
    std::cout << std::flush;
    std::cout << "\rUpdating fstab file: ";
    std::cout << filename;
    std::cout << std::string(15, ' '); // Очистка строки до конца
    std::cout << "\r"; // Возвращаем курсор в начало строки
    std::cout << std::flush;
}
// Функция для обновления файла fstab, возвращает флаг успеха
UpdateResult updateFstab(const std::string &filename)
{
    // Вектор для хранения исходных строки и записей fstab
    std::vector<FstabEntry> entries;
    // Множество для хранения путей + точек монтирования (только для ext4)
    std::unordered_set<std::pair<std::string, std::string>> ext4Set;

    // Открываем файл
    std::fstream file(filename, std::ios::in);
    if (!file.is_open()) {
        return UpdateResult::openError;
    }

    // Читаем строки из файла в вектор entries, обновляем ext4Set
    std::string line;
    while (std::getline(file, line)) {
        FstabEntry entry = std::move(line);
        if (entry.type == "ext4") {
            ext4Set.emplace(std::make_pair(entry.source, entry.mountPoint));
        }
        entries.emplace_back(std::move(entry));
    }
    file.close();

    // Записываем строки обратно в файл, заменяя ro -> rw и дублируя erofs -> ext4 при необходимости
    file.open(filename, std::ios::out);
    if (!file.is_open()) {
        return UpdateResult::createError;
    }
    for (auto& entry : entries) {
        if (entry.type == "erofs" && !ext4Set.contains(std::make_pair(entry.source, entry.mountPoint))) {
            if (!(file << entry.line << '\n')) {
                return UpdateResult::writeError;
            }
            entry.line[entry.type_pos+1] = 'x';
            entry.line[entry.type_pos+2] = 't';
            entry.line[entry.type_pos+3] = '4';
            entry.line[entry.type_pos+4] = ' ';
            entry.type.clear();  // entry.type = "ext4"
        }
        updateRights(entry);
        if (!(file << entry.line << '\n')) {
            return UpdateResult::writeError;
        }
    }

    return UpdateResult::ok;
}

// Рекурсивная функция для обхода файловой системы и обновления файлов fstab
void updateFstabRecursively(const std::string &path) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().filename().string().find("fstab") != std::string::npos) {

            progress(entry.path().string());

            switch (updateFstab(entry.path().string())) {
                case UpdateResult::ok:
                    std::cout << "File updated successfully.";
                    break;
                case UpdateResult::openError:
                    std::cerr << "Unable to open file: " << entry.path().string();
                    break;
                case UpdateResult::createError:
                    std::cerr << "Unable to create file: " << entry.path().string();
                    break;
                case UpdateResult::writeError:
                    std::cerr << "Unable to write file: " << entry.path().string();
                    break;
            }
        }
    }
}

// Парсить командную строку
ParseResult FstabFix::parse_cmd_line(int argc, char* argv[])
{
    if(argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    // Обновляем файлы fstab в указанных директориях и всех их поддиректориях
    for (int i = 0; i < argc; ++i) {
        directories.push_back(argv[i]);

    }
    return ParseResult::ok;
}

ProcessResult FstabFix::process()
{
    for(auto dir : directories)
    {
        updateFstabRecursively(dir);
    }
    return ProcessResult::ok;
}