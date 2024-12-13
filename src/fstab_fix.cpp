#include "../include/main.hpp"

// Помощь по параметрам командной строки
void FstabFix::show_help() {
    std::cout <<
              R"***(
fstab_fix

Usage:

  fstab_fix [-rw] <folder1> <folder2>....<folderN>
    Where:
        -rw - replace 'ro' to 'rw' in lines with ext4
        <folder1>...<folderN> - folder where finding fstab's files
)***";
}

// Структура для хранения данных строки в файле fstab
struct FstabEntry {
    std::string line;
    std::string source;
    std::string mountPoint;
    std::string type;
    size_t type_pos{};  // позиция type в line
    std::string mountFlags;
    size_t mountFlags_pos{};  // позиция mountFlags в line
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
struct std::hash<std::pair<std::string, std::string>> {
    std::size_t operator()(const std::pair<std::string, std::string> &s) const noexcept {
        std::size_t h1 = std::hash<std::string>{}(s.first);
        std::size_t h2 = std::hash<std::string>{}(s.second);
        return h1 ^ (h2 << 1);
    }
};

// Функция замены прав ro -> rw для ext4
void updateRights(FstabEntry &entry) {
    if (entry.type != "ext4") { return; }
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

// Функция для обновления файла fstab, возвращает флаг успеха
UpdateResult updateFstab(const std::string &filename, bool rw) {
    // Вектор для хранения исходных строки и записей fstab
    std::vector<FstabEntry> entries;
    // Множество для хранения путей + точек монтирования (только для ext4)
    std::unordered_set<std::pair<std::string, std::string>> ext4Set;
    // Множество для хранения путей + точек монтирования (только для erofs)
    std::unordered_set<std::pair<std::string, std::string>> erofsSet;

    // Открываем файл
    std::fstream file(filename, std::ios::in);
    if (!file.is_open()) {
        return UpdateResult::openError;
    }

    // Читаем строки из файла в вектор entries, обновляем ext4Set и erofsSet
    std::string line;
    while (std::getline(file, line)) {
        FstabEntry entry = std::move(line);
        if (entry.type == "ext4") {
            ext4Set.emplace(std::make_pair(entry.source, entry.mountPoint));
        } else if (entry.type == "erofs") {
            erofsSet.emplace(std::make_pair(entry.source, entry.mountPoint));
        }
        entries.emplace_back(std::move(entry));
    }
    file.close();

    // Записываем строки обратно в файл, заменяя ro -> rw и дублируя ext4 -> erofs при необходимости
    file.open(filename, std::ios::out);
    if (!file.is_open()) {
        return UpdateResult::createError;
    }
    for (auto &entry: entries) {
        if (entry.type == "ext4" && !erofsSet.contains(std::make_pair(entry.source, entry.mountPoint))) {
            if (!(file << entry.line << '\n')) {
                return UpdateResult::writeError;
            }
            entry.line[entry.type_pos + 1] = 'r';
            entry.line[entry.type_pos + 2] = 'o';
            entry.line[entry.type_pos + 3] = 'f';
            entry.line[entry.type_pos + 4] = 's';
            entry.type.clear();  // entry.type = "erofs"
        } else if (entry.type == "erofs" && !ext4Set.contains(std::make_pair(entry.source, entry.mountPoint))) {
            if (!(file << entry.line << '\n')) {
                return UpdateResult::writeError;
            }
            entry.line[entry.type_pos + 1] = 'x';
            entry.line[entry.type_pos + 2] = 't';
            entry.line[entry.type_pos + 3] = '4';
            entry.line[entry.type_pos + 4] = ' ';
            entry.type.clear();  // entry.type = "ext4"
        }
        if (rw) {
            updateRights(entry);
        }
        if (!(file << entry.line << '\n')) {
            return UpdateResult::writeError;
        }
    }

    return UpdateResult::ok;
}

// Рекурсивная функция для обхода файловой системы и обновления файлов fstab
void updateFstabRecursively(const std::string &path, bool rw) {
    if (std::filesystem::is_symlink(path)) { return; }
    int count_find = 0;
    for (const auto &entry: std::filesystem::recursive_directory_iterator(path)) {
        std::string fname = entry.path().filename().string();
        if (entry.is_regular_file() && !entry.is_symlink() && (fname.find(".fstab") != std::string::npos || fname.find("fstab.") != std::string::npos || fname == "fstab")) {
            count_find++;
            std::string name = entry.path().string();
            std::cout << "Updating fstab file: " << name << std::endl;
            switch (updateFstab(name, rw)) {
                case UpdateResult::ok:
                    std::cout << "File updated successfully." << std::endl;
                    break;
                case UpdateResult::openError:
                    std::cerr << "Unable to open file." << std::endl;
                    break;
                case UpdateResult::createError:
                    std::cerr << "Unable to create file." << std::endl;
                    break;
                case UpdateResult::writeError:
                    std::cerr << "Unable to write file." << std::endl;
                    break;
            }
        }
    }
    if (count_find == 0) {
        std::cerr << "No files found in " << path << std::endl;
    }
}

// Парсить командную строку
ParseResult FstabFix::parse_cmd_line(int argc, char *argv[]) {
    if (argc < 1) {
        show_help();
        return ParseResult::wrong_option;
    }
    int i = 0;
    if (std::string(argv[0]) == "-rw") {
        rw = true;
        i = 1;
    }
    // Обновляем файлы fstab в указанных директориях и всех их поддиректориях
    for (; i < argc; ++i) {
        directories.emplace_back(argv[i]);
    }
    return ParseResult::ok;
}

ProcessResult FstabFix::process() {
    for (const auto &dir: directories) {
        updateFstabRecursively(dir, rw);
    }
    return ProcessResult::ok;
}
