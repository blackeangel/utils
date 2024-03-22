#pragma once
#include <cstdlib>
#include <climits>
#include <cstdio>
#include <cstring>
#include <string.h> // memmem: GNU SOURCE macro needed
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <chrono>
#include <iterator>
#include <memory>
#include <functional>
#include <unordered_set>
#include "../src/zlib/zlib.h"
#include "../src/e2fsdroid/ext2fs/ext2_fs.h" // Для работы с ext4
#include "../src/sparse/src/sparse_format.h" // Для работы с Android sparse

using namespace std::string_view_literals;

constexpr int BLOCK_SIZE = 4096;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Результаты парсинга
enum class ParseResult
{
    not_enough,    // недостаточно параметров
    wrong_hex,     // неверная 16-ричная строка
    wrong_option,  // неверная опция или неверное использование опции
    ok             // всё верно
};

// Результаты основной работы
enum class ProcessResult
{
    open_error,   // ошибка открытия файла
    seek_error,   // ошибка позиционирования
    read_error,   // ошибка чтения
    write_error,  // ошибка записи
    breaked,      // прервано
    ok            // успешно
};

// Результат работы функции
enum class UpdateResult
{
    ok,
    openError,
    createError,
    writeError
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UtilBase
{
public:
    // Помощь по параметрам командной строки
    virtual void show_help() = 0;
    // Парсить командную строку
    virtual ParseResult parse_cmd_line(int argc, char* argv[]) = 0;
    // Основная работа
    virtual ProcessResult process() = 0;
    // Виртуальный деструктор
    virtual ~UtilBase() = default;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DelGapps : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path folder_path;
    std::filesystem::path file_path;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sdat2Img : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::pair<int, std::vector<std::pair<int, int>>> parse_transfer_list_file(std::string path);
    // Create empty image with Correct size;
    void initOutputFile(std::string output_file);

    std::vector<std::pair<int, int>> all_block_sets;
    unsigned int max_file_size;

    std::filesystem::path transf_file;
    std::filesystem::path new_dat_file;
    std::filesystem::path img_file;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WriteKey : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path image_file;
    const char *foffset;
    const char *file_value;
    const char *key;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Cut : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path image_file;
    const char *start_offset;
    const char *end_offset_length;
    std::filesystem::path output_dir;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Copy : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path image_file;
    const char *start_offset;
    const char *end_offset_length;
    std::filesystem::path output_dir;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Insert : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path image_file;
    const char *start_offset;
    std::filesystem::path insert_file;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Foffset : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    const char* filename;               // имя файла
    std::vector<char> search_str;       // искомая строка
    long long start_offset = 0;         // начальное смещение
    long long search_size = LLONG_MAX;  // размер блока поиска
    long long max_count = LLONG_MAX;    // максимальное кол-во смещений
    bool forward = true;                // поиск с начала (иначе с конца)
    bool hex_offsets = true;            // 16-ричный вывод
    bool hex_uppercase = false;         // 16-ричный вывод в верхнем регистре
    bool hex_prefix = false;            // добавлять префикс для 16-ричных чисел
    char sep_char = '\n';               // разделитель смещений
    bool show_stat = true;              // вывод статистики
    long long buf_size = 1024 * 1024;   // размер буфера

    // Вывести смещение
    bool show_offset(std::fstream& file, long long offset, long long number);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HexPatch : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    const char* image_file;
    std::vector<char> what_find;
    std::vector<char> what_replace;
    int way = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class KerVer : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    // Искать строку в файле
    ProcessResult process() override;

private:
    std::filesystem::path kernelFile;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FstabFix : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;
    ProcessResult process() override;

private:
    std::vector<std::string> directories;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SharedBlockDetector : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;

    ProcessResult process() override;

private:
    std::filesystem::path filename;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FileExplorer : public UtilBase
{
public:
    // Помощь по параметрам командной строки
    void show_help() override;
    // Парсить командную строку
    ParseResult parse_cmd_line(int argc, char* argv[]) override;

    ProcessResult process() override;

private:
    std::filesystem::path initial_directory = "/sdcard";
    std::vector<std::string> filters = {".img", ".dat", ".br", ".list"};
    bool showFiles = true;
    bool showDirectories = true;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Поиск строки (вектора символов) search_str в файле filename и вызов функции callback для каждого вхождения.
// Возвращает результат операции (в т.ч. возвращаемый функцией callback) и кол-во найденных вхождений.
// read_only - открыть только для чтения (true, если не предполагается замена), forward - направление (true - вперёд),
// start_offset - начальное смещение (отрицательное - с конца), search_size - размер области поиска,
// max_count - максимальное кол-во вхождений, buf_size - размер буфера.
// P.S. start_offset - это не смещение, с которого начинается поиск, а начало блока размером search_size.
// Т.е. при forward = false поиск начинается не с позиции start_offset, а с start_offset + search_size.
// Функция callback должна иметь объявление: bool callback(std::fstream& file, long long offset, long long number);
// Здесь file - объект открытого файла, offset - смещение, по которому была найдена строка, number - номер вхождения.
// Функция возвращает ProcessResult::ok, если нужно продолжить поиск, другие значения в противном случае false.
// Текущая позиция файла обычно не совпадает со смещением (offset). При необходимости выполнить замену нужно сначала
// переместить указатель в нужную позицию (file.seekp(offset)). Восстанавливать прежнюю позицию нет необходимости.
std::pair<ProcessResult, long long> find_in_file(std::function<ProcessResult(std::fstream&, long long, long long)> callback,
    const char* filename, const std::vector<char>& search_str, bool read_only, bool forward = true,
    long long start_offset = 0, long long search_size = LLONG_MAX, long long max_count = INT_MAX,
    long long buf_size = 1024*1024);

// Преобразовать hex-строку в вектор символов, вернуть флаг успеха
bool parse_hexstring(const char* hexstring, std::vector<char>& result);

// Парсить число long из строки
bool parse_long(const char* s, long& result);

// Парсить число long long из строки
bool parse_llong(const char* s, long long& result);

std::string &replace(std::string &str, const std::string &from, const std::string &to);

std::string &replaceAll(std::string &str, const std::string &from, const std::string &to);

std::vector<std::string> recursive_dir(const std::vector<std::string> &path);

std::vector<std::string> list_dir(const std::filesystem::path &path);

std::vector<std::string> readlines(const std::string &file);

std::vector<std::string> split(const std::string &s, char delim);

std::vector<std::pair<int, int>> rangeset(std::string src);

long string2long(const std::string s);

std::vector<char> hex2byte(const char *hex);

size_t partialCopy(std::ifstream &in, std::ofstream &out, size_t start, size_t finish, size_t bufSize);

void *memmem(const void *l, size_t l_len, const void *s, size_t s_len);

std::vector<size_t> findOffsetInFile(std::FILE *file, std::size_t size, char const *bytes, std::size_t len, int metod_find = 0);

std::size_t findBackwardOffsetInFile(std::FILE *file, std::size_t size, char const *bytes, std::size_t len);

std::string viravn_offset(const char *offset);

//std::string find_offset_fn(const std::filesystem::path image_file, const char *whatfind, int way = 0);
