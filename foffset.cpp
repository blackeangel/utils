#include "main.hpp"

// Помощь по параметрам командной строки
void Foffset::show_help()
{
    std::cout <<
R"***(Usage:
  utils foffset file hexstring [option...]
where option is:
  -s offset - start offset (DEC or 0xHEX), negative value for offset from end of file
  -l length - search region length (DEC or 0xHEX)
  -n num - maximum number of offsets (DEC or 0xHEX)
  -b size - size of buffer in KB (DEC or 0xHEX), default is 1024
  -r - reverse search
  -d - display offsets in decimal (default is hex)
  -hu - display offsets in uppercase hex (only if -d option is not specified)
  -hp - display offset 0x prefix for hex (only if -d option is not specified)
  -hup - combination of -hu and -hp options
  -o - display offsets in one string separated by space
  -q - no statistic message
hexstring digit pairs that can be separated using space, period(.), comma(,), dash(-) or colon(:), e.g. 41.35:AA.55
)***";
}

// Парсить командную строку
ParseResult Foffset::parse_cmd_line(int argc, char* argv[])
{
    //if (argc < 2) { return ParseResult::not_enough; }
    if(argc < 2) {show_help();}

    filename = argv[0];
    
    // Hexstring
    char prev = char(-1);
    for (char ch : std::string_view(argv[1])) {
        if (ch >= '0' && ch <= '9') { ch -= '0'; }
        else if (ch >= 'a' && ch <= 'f') { ch -= 'a' - 10; }
        else if (ch >= 'A' && ch <= 'F') { ch -= 'A' - 10; }
        else if ((ch != ' ' && ch != '.' && ch != ',' && ch != '-' && ch != ':') || prev != char(-1)) { return ParseResult::wrong_hex; }
        if (prev == char(-1)) { if (ch < 16) { prev = ch; } }
        else {
            search_str.push_back(prev << 4 | ch);
            prev = char(-1);
        }
    }
    if (prev != char(-1) || search_str.size() == 0) { return ParseResult::wrong_hex; }
    
    // Опции
    for (int i = 2; i < argc; ++i) {
        if (argv[i] == "-s"sv) {
            if (++i >= argc || !parse_llong(argv[i], start_offset)) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-l"sv) {
            if (++i >= argc || !parse_llong(argv[i], search_size) || search_size < 0) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-n"sv) {
            if (++i >= argc || !parse_long(argv[i], max_count) || max_count < 1) { return ParseResult::wrong_option; }
        }
        else if (argv[i] == "-b"sv) {
            if (++i >= argc || !parse_llong(argv[i], buf_size) || buf_size < 1) { return ParseResult::wrong_option; }
            buf_size *= 1024;
            if (buf_size < (long long)search_str.size()) { return ParseResult::wrong_option; }
        } else if (argv[i] == "-r"sv) { reverse_search = true; }
        else if (argv[i] == "-d"sv) { hex_offsets = false; }
        else if (argv[i] == "-hu"sv) { hex_uppercase = true; }
        else if (argv[i] == "-hp"sv) { hex_prefix = true; }
        else if (argv[i] == "-hup"sv) { hex_uppercase = hex_prefix = true; }
        else if (argv[i] == "-o"sv) { sep_char = ' '; }
        else if (argv[i] == "-q"sv) { show_stat = false; }
        else { return ParseResult::wrong_option; }
    }
    return ParseResult::ok;
}

// Искать строку в файле
ProcessResult Foffset::process()
{
    // Режим вывода чисел
    if (hex_offsets) {
        std::cout << std::hex;
        if (hex_uppercase) { std::cout << std::uppercase; }
    }

    // Открытие файла и получение размера
    std::ifstream file(filename, std::ios::binary);
    if (!file) { return ProcessResult::open_error; }
    if (!file.seekg(0, std::ios::end)) { return ProcessResult::read_error; }
    file_size = file.tellg();

    // Перемещение на начальную позицию
    if (start_offset >= 0) {
        if (!file.seekg(start_offset)) { return ProcessResult::read_error; }
    } else if (start_offset < 0) {
        if (!file.seekg(start_offset, std::ios::end)) { return ProcessResult::read_error; }
    }

    // Проверка размера файла
    start_offset = file.tellg();
    search_size = std::min<long long>(file_size - start_offset, search_size);
    if (search_size < (long long)search_str.size()) {
        return ProcessResult::ok;  // файл меньше длины строки
    }
    buf_size = std::min(buf_size, search_size);

    // Поиск и вывод статистики
    ProcessResult result;
    if (!reverse_search) {
        result = find_forward(file);
    } else {
        result = find_backward(file);
    }
    if (found_count > 0) { std::cout << '\n'; }
    if (result == ProcessResult::ok && show_stat) {
        std::cout << "Offsets found: " << std::dec << found_count << '\n';
    }
    return result;
}

// Искать строку вперёд
ProcessResult Foffset::find_forward(std::ifstream& file)
{
    std::vector<char> buf(buf_size);
    long long end_offset = start_offset + search_size;
    long long buf_read_pos = 0;
    do {
        // Читаем
        long long file_pos = file.tellg();
        long long buf_file_offset = file_pos - buf_read_pos;
        long long read_size = std::min<long long>(buf_size - buf_read_pos, end_offset - file_pos);
        if (read_size <= 0) { break; }
        file.read(buf.data() + buf_read_pos, read_size);
        if (!file && !file.eof()) { return ProcessResult::read_error; }
        // Ищем
        long long count = file.gcount();
        auto start = buf.data();
        auto end = start + buf_read_pos + count;
        while (true) {
            auto pos = std::search(start, end, search_str.begin(), search_str.end());
            if (pos == end) { break; }
            if (!new_offset(buf_file_offset + pos - buf.data())) { return ProcessResult::ok; };
            start = pos + 1;
            if (end - start < (long long)search_str.size()) { break; }
        }
        // Копируем search_str.size() - 1 байтов из конца буфера в начало
        if (search_str.size() > 1) {
            buf_read_pos = search_str.size() - 1;
            std::copy(end - buf_read_pos, end, buf.data());
        }
    } while (!file.eof());
    return ProcessResult::ok;
}

// Искать строку назад
ProcessResult Foffset::find_backward(std::ifstream& file)
{
    std::vector<char> buf(buf_size);
    long long end_offset = std::min(start_offset + search_size, file_size);
    long long read_offset = end_offset;
    long long tail_size = 0;
    do {
        // Читаем
        long long read_size = std::min(buf_size - tail_size, read_offset - start_offset);
        read_offset -= read_size;
        if (!file.seekg(read_offset)) { return ProcessResult::read_error; }
        auto start = buf.data() + buf_size - tail_size - read_size;
        if (!file.read(start, read_size) || file.gcount() != read_size) { return ProcessResult::read_error; }
        // Ищем
        auto end = start + read_size + tail_size;
        while (true) {
            auto pos = std::find_end(start, end, search_str.begin(), search_str.end());
            if (pos == end) { break; }
            if (!new_offset(read_offset + pos - start)) { return ProcessResult::ok; };
            end = pos + search_str.size() - 1;
            if (end - start < (long long)search_str.size()) { break; }
        }
        // Копируем search_str.size() - 1 байтов из начала буфера в конец
        if (search_str.size() > 1) {
            tail_size = search_str.size() - 1;
            std::copy_backward(buf.data(), buf.data() + tail_size, buf.data() + buf_size);
        }
    } while (read_offset > start_offset);
    return ProcessResult::ok;
}

// Вывести смещение и увеличить кол-во
// Если кол-во достигло предельного значения, возвращается false
bool Foffset::new_offset(long long offset)
{
    if (found_count > 0) { std::cout << sep_char; }
    if (hex_offsets && hex_prefix) { std::cout << "0x"; }
    std::cout << offset;
    return (++found_count < max_count);
}
