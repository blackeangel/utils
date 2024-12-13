#include "../include/main.hpp"

// Поиск строки (вектора символов) search_str в файле filename и вызов функции callback для каждого вхождения.
// Подробное описание см. в файле main.hpp.
std::pair<ProcessResult, long long> find_in_file(std::function<ProcessResult(std::fstream&, long long, long long)> callback,
    const char* filename, const std::vector<char>& search_str, bool read_only, bool forward,
    long long start_offset, long long search_size, long long max_count, long long buf_size)
{
    // Открытие файла и получение размера
    auto mode = std::ios::binary | std::ios::in;
    if (!read_only) { mode |= std::ios::out; }
    std::fstream file(filename, mode);
    if (!file) { return {ProcessResult::open_error, 0}; }
    if (!file.seekg(0, std::ios::end)) { return {ProcessResult::seek_error, 0}; }
    long long file_size = file.tellg();

    // Перемещение на начальную позицию
    if (start_offset >= 0) {
        if (!file.seekg(start_offset)) { return {ProcessResult::seek_error, 0}; }
    } else if (start_offset < 0) {
        if (!file.seekg(start_offset, std::ios::end)) { return {ProcessResult::seek_error, 0}; }
    }

    // Проверка размера файла
    start_offset = file.tellg();
    search_size = std::min<long long>(file_size - start_offset, search_size);
    if (search_size < (long long)search_str.size()) {
        return {ProcessResult::ok, 0};  // файл меньше длины строки
    }
    buf_size = std::min(buf_size, search_size);

    // Поиск
    std::vector<char> buf(buf_size);
    long long found_count = 0;
    if (forward) {
        // Поиск вперёд
        long long end_offset = start_offset + search_size;
        long long buf_read_pos = 0;
        long long file_pos = start_offset;
        do {
            // Читаем
            long long buf_file_offset = file_pos - buf_read_pos;
            long long read_size = std::min<long long>(buf_size - buf_read_pos, end_offset - file_pos);
            if (read_size <= 0) { break; }
            if (!file.seekg(file_pos)) { return {ProcessResult::seek_error, found_count}; }
            file.read(buf.data() + buf_read_pos, read_size);
            if (!file/* && !file.eof()*/) { return {ProcessResult::read_error, found_count}; }
            file_pos += file.gcount();
            // Ищем
            long long count = file.gcount();
            auto start = buf.data();
            auto end = start + (buf_read_pos + count);
            while (true) {
                auto pos = std::search(start, end, search_str.begin(), search_str.end());
                if (pos == end) { break; }
                auto result = callback(file, buf_file_offset + (pos - buf.data()), ++found_count);
                if (result != ProcessResult::ok) { return {result, found_count}; }
                if (found_count == max_count) { return {ProcessResult::ok, found_count}; };
                start = pos + 1;
                if (end - start < (long long)search_str.size()) { break; }
            }
            // Копируем search_str.size() - 1 байтов из конца буфера в начало
            if (search_str.size() > 1) {
                buf_read_pos = search_str.size() - 1;
                std::copy(end - buf_read_pos, end, buf.data());
            }
        } while (!file.eof());
    } else {
        // Поиск назад
        long long end_offset = std::min(start_offset + search_size, file_size);
        long long read_offset = end_offset;
        long long tail_size = 0;
        do {
            // Читаем
            long long read_size = std::min(buf_size - tail_size, read_offset - start_offset);
            read_offset -= read_size;
            if (!file.seekg(read_offset)) { return {ProcessResult::seek_error, found_count}; }
            auto start = buf.data() + (buf_size - tail_size - read_size);
            if (!file.read(start, read_size) || file.gcount() != read_size) { return {ProcessResult::read_error, found_count}; }
            // Ищем
            auto end = start + (read_size + tail_size);
            while (true) {
                auto pos = std::find_end(start, end, search_str.begin(), search_str.end());
                if (pos == end) { break; }
                auto result = callback(file, read_offset + (pos - start), ++found_count);
                if (result != ProcessResult::ok) { return {result, found_count}; }
                if (found_count == max_count) { return {ProcessResult::ok, found_count}; };
                end = pos + search_str.size() - 1;
                if (end - start < (long long)search_str.size()) { break; }
            }
            // Копируем search_str.size() - 1 байтов из начала буфера в конец
            if (search_str.size() > 1) {
                tail_size = search_str.size() - 1;
                std::copy_backward(buf.data(), buf.data() + tail_size, buf.data() + buf_size);
            }
        } while (read_offset > start_offset);
    }
    return {ProcessResult::ok, found_count};
}

// Поиск строки (вектора символов) search_str в векторе source и вызов функции callback для каждого вхождения.
// Подробное описание см. в файле main.hpp.
std::pair<ProcessResult, long long> find_in_vector(std::function<ProcessResult(std::vector<char> &, long long,
                                                                               long long)> callback,
                                                   std::vector<char>& source, const std::vector<char>& search_str, bool forward,
                                                   long long start_offset, long long search_size, long long max_count)
{
    // Корректировака начального и конечного смещения
    auto source_size = static_cast<long long>(source.size());
    auto search_str_size = static_cast<long long>(search_str.size());
    if (start_offset < 0) {
        start_offset = std::max(source_size + start_offset, 0LL);
    }
    if (search_size > source_size) { search_size = source_size; }
    long long end_offset = std::min(source_size, start_offset + search_size);

    // Поиск
    long long found_count = 0;
    if (start_offset + search_str_size <= end_offset) {
        if (forward) {
            // Поиск вперёд
            auto start_iter = source.begin() + start_offset;
            auto end_iter = source.begin() + end_offset;
            while (found_count < max_count) {
                auto pos_iter = search(start_iter, end_iter, search_str.cbegin(), search_str.cend());
                if (pos_iter == end_iter) { break; }
                auto result = callback(source, pos_iter - source.begin(), ++found_count);
                start_iter = pos_iter + 1;  // следующая позиция поиска
                if (result != ProcessResult::ok) { return {result, found_count}; }
            }
        } else {
            // Поиск назад
            auto start_iter = source.rbegin() + (source_size - end_offset);
            auto end_iter = source.rend() - start_offset;
            while (found_count < max_count) {
                auto pos_iter = search(start_iter, end_iter, search_str.crbegin(), search_str.crend());
                if (pos_iter == end_iter) { break; }
                auto result = callback(source, pos_iter.base() - source.begin() - search_str_size, ++found_count);
                start_iter = pos_iter + 1;  // следующая позиция поиска
                if (result != ProcessResult::ok) { return {result, found_count}; }
            }
        }
    }
    return {ProcessResult::ok, found_count};
}


// Преобразовать hex-строку в вектор символов, вернуть флаг успеха
bool parse_hexstring(const char* hexstring, std::vector<char>& result)
{
    result.clear();
    char prev = char(-1);
    for (char ch : std::string_view(hexstring)) {
        if (ch >= '0' && ch <= '9') { ch -= '0'; }
        else if (ch >= 'a' && ch <= 'f') { ch -= 'a' - 10; }
        else if (ch >= 'A' && ch <= 'F') { ch -= 'A' - 10; }
        else if ((ch != ' ' && ch != '.' && ch != ',' && ch != '-' && ch != ':') || prev != char(-1)) {
            return false;
        }
        if (prev == char(-1)) { if (ch < 16) { prev = ch; } }
        else {
            result.push_back(prev << 4 | ch);
            prev = char(-1);
        }
    }
    return (prev == char(-1) && result.size() > 0);
}

// Парсить число long из строки
bool parse_long(const char* s, long& result)
{
    char* pos;
    errno = 0;
    result = std::strtol(s, &pos, 0);
    return (*pos == '\0' && errno == 0);
}

// Парсить число long long из строки
bool parse_llong(const char* s, long long& result)
{
    char* pos;
    errno = 0;
    result = std::strtoll(s, &pos, 0);
    return (*pos == '\0' && errno == 0);
}

std::string &replace(std::string &str, const std::string &from, const std::string &to)
{
    if (!from.empty())
    {
        size_t start_pos = str.find(from);
        if (start_pos != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
        }
    }
    return str;
}

std::string &replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (!from.empty())
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
    return str;
}

std::vector<std::string> recursive_dir(const std::vector<std::string> &path)
{
    std::vector<std::string> folder_files;
    for (auto &&path_item : path)
    {
        for (const auto &p : std::filesystem::recursive_directory_iterator(path_item))
        {
            folder_files.push_back(p.path().string());
        }
    }
    return folder_files;
}

std::vector<std::string> list_dir(const std::filesystem::path &path)
{
    std::vector<std::string> names;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (is_directory(entry.path()))
        {
            names.push_back(entry.path().stem().string());
        }
    }

    return names;
}

std::vector<std::string> readlines(const std::string &file)
{
    std::vector<std::string> lines;
    std::ifstream open_file(file, std::ios::in);
    for (std::string line; std::getline(open_file, line);)
        lines.push_back(line);
    open_file.close();
    return lines;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim))
    {
        tokens.push_back(item);
    }
    return tokens;
}

std::vector<std::pair<int, int>> rangeset(std::string src)
{
    std::vector<std::string> src_set = split(src, ',');
    std::vector<int> num_set;
    for (auto& item : src_set)
    {
        num_set.push_back(stoi(item));
    }
    if (num_set.size() != num_set[0] + 1)
    {
        std::cerr << "Error on parsing following data to rangeset:\n"
                  << src << std::endl;
        exit(1);
    }
    std::vector<std::pair<int, int>> result;
    for (int i = 1; i < num_set.size(); i += 2)
    {
        result.emplace_back(num_set[i], num_set[i + 1]);
    }
    return result;
}

long string2long(const std::string s)
{
    unsigned long value;
    std::istringstream iss(s);
    iss >> std::hex >> value;
    return value;
}

std::vector<char> hex2byte(const char *hex)
{
    char high, low;
    std::vector<char> buf(strlen(hex) / 2);
    for (int i = 0, length = strlen(hex); i < length; i += 2)
    {
        high = toupper(hex[i]) - '0';
        low = toupper(hex[i + 1]) - '0';
        buf[i / 2] = ((high > 9 ? high - 7 : high) << 4) + (low > 9 ? low - 7 : low);
    }
    return buf;
}

size_t partialCopy(std::ifstream &in, std::ofstream &out, size_t start, size_t finish, size_t bufSize)
{
    if (in.seekg(start))
    {
        if (start > finish)
        {
            std::cout << "Error! Finish smaller then start!" << std::endl;
            exit(-1);
        }

        size_t totalNeedToRead = finish - start;

        std::unique_ptr<char[]> readBuf(new char[bufSize]);

        size_t totalRead = 0;

        while (totalRead < totalNeedToRead)
        {
            size_t bytesLeft = totalNeedToRead - totalRead;

            if (!in.read(readBuf.get(), std::min(bytesLeft, bufSize)))
                break;

            auto rdSize = in.gcount();

            if (!out.write(readBuf.get(), rdSize))
                break;

            totalRead += rdSize;
        }
        return totalRead;
    }
    return 0;
}

void *memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
    const char *cur, *last;
    const char *cl = (const char *)l;
    const char *cs = (const char *)s;

    /* we need something to compare */
    if (l_len == 0 || s_len == 0)
    {
        return NULL;
    }

    /* "s" must be smaller or equal to "l" */
    if (l_len < s_len)
    {
        return NULL;
    }

    /* special case where s_len == 1 */
    if (s_len == 1)
    {
        return memchr((void *)l, (unsigned char)*cs, l_len);
    }

    /* the last position where its possible to find "s" in "l" */
    last = cl + l_len - s_len;

    for (cur = cl; cur <= last; cur++)
    {
        if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
        {
            return (void *)cur;
        }
    }

    return NULL;
}

std::vector<size_t> findOffsetInFile(std::FILE *file, std::size_t size, char const *bytes, std::size_t len, int metod_find)
{
    /*
    metod_find=0 - first find
    metod_find=1 - all finds
    metod_find=-1 - reverse first find ?!
     */
    std::unique_ptr<char[]> buffer(new char[size + len]);
    std::vector<size_t> sxs;
    std::size_t tSize = 0; // Total read bytes

    char *sBuf = &buffer[0];
    std::size_t fSize = std::fread(sBuf, 1, size, file);
    if (fSize >= len)
    {
        if (char *pFnd = (char *)::memmem(sBuf, fSize, bytes, len))
        {
            if (metod_find == 0)
            {
                sxs.push_back(std::distance(sBuf, pFnd));
                return sxs;
            }
            if (metod_find == 1)
            {
                sxs.push_back(std::distance(sBuf, pFnd));
            }
        }
        tSize += fSize;
        std::memcpy(sBuf, sBuf + fSize - len, len);

        char *pBuf = &buffer[len];
        while (std::size_t sSize = std::fread(pBuf, 1, size, file))
        {
            // Also here is the processing of the buffer boundary
            char *pFnd = (char *)::memmem(sBuf, sSize + len, bytes, len);
            if (pFnd)
            {
                if (metod_find == 0)
                {
                    sxs.push_back(tSize + std::distance(sBuf, pFnd));
                    break;
                }
                if (metod_find == 1)
                {
                    sxs.push_back(tSize + std::distance(sBuf, pFnd));
                }
            }
            if (sSize < len)
            {
                break; // There's no point to continue lookup
            }
            tSize += sSize;
            std::memcpy(sBuf, pBuf + sSize - len, len);
        }
    }
    return sxs; // Not found or error
}
std::size_t findBackwardOffsetInFile(std::FILE *file, std::size_t size, char const *bytes, std::size_t len)
{
    if (std::fseek(file, 0L, SEEK_END) != 0)
    {
        return -1;
    }
    std::size_t const fileSize = std::ftell(file);
    std::size_t const buffSize = size + len;

    std::unique_ptr<char[]> buffer(new char[buffSize]);

    char *sBuf = &buffer[0];
    std::size_t offset = fileSize - std::min(fileSize, buffSize);

    while (std::fseek(file, offset, SEEK_SET) == 0)
    {
        std::size_t cSize = std::fread(sBuf, 1, buffSize, file);
        if (cSize < len)
        {
            break;
        }
        /*if(char* pFnd = (char*)::memmem(sBuf, cSize, bytes, len))
        {
            std::rewind(file);
            return offset + std::distance(sBuf, pFnd);
        }
        */
        char *it = std::find_end(sBuf, sBuf + cSize, bytes, bytes + len);
        if (it != (sBuf + cSize))
        {
            std::rewind(file);
            return offset + std::distance(sBuf, it);
        }
        if (offset == 0)
        {
            break;
        }
        //offset -= (offset - cSize != 0) ? cSize - len : cSize;
        size_t shrink = std::min(cSize - len, offset);
        offset -= shrink;
    }
    std::rewind(file);
    return -1;
}

std::string viravn_offset(const char *offset)
{
    std::string tmpstr(offset);
    if (tmpstr.find(" ") != std::string::npos)
    { //если есть пробелы - уберём
        tmpstr = replaceAll(tmpstr, " ", "");
    }
    if (tmpstr.find("0x") != std::string::npos)
    {
        tmpstr = tmpstr.substr(2, tmpstr.length());
    }
    if (tmpstr.find("0х") != std::string::npos)
    {
        tmpstr = tmpstr.substr(2, tmpstr.length());
    }
    return tmpstr;
}

bool is_ext4(const std::vector<char>& buffer) {
    // Приводим буфер к типу ext2_super_block, чтобы можно было обращаться к полям
    const auto* superblock = reinterpret_cast<const ext2_super_block*>(buffer.data() + 1024);
    // Проверяем сигнатуру ext4
    if (superblock->s_magic == EXT2_SUPER_MAGIC) {
        return true;
    }
    return false;
}

bool is_erofs(const std::vector<char>& buffer) {
    // Приводим буфер к типу erofs_super_block, чтобы можно было обращаться к полям
    const auto* superblock = reinterpret_cast<const erofs_super_block*>(buffer.data() + EROFS_SUPER_OFFSET);
    if (superblock->magic == EROFS_SUPER_MAGIC_V1) {
        return true;
    }
    return false;
}

bool is_boot(const std::vector<char>& buffer) {
    // Приводим буфер к типу boot_img_hdr, чтобы можно было обращаться к полям
    const auto * superblock = reinterpret_cast<const boot_img_hdr *>(buffer.data());
    std::string temp(reinterpret_cast<char const*>(superblock->magic), sizeof superblock->magic);
    if (temp == BOOT_MAGIC || temp == VENDOR_BOOT_MAGIC) {
        return true;
    }
    return false;
}

// Функция для определения, является ли файл форматом Android sparse
bool is_sparse(const std::vector<char>& buffer) {
    // Приводим буфер к типу sparse_header, чтобы можно было обращаться к полям
    const auto * superblock = reinterpret_cast<const sparse_header *>(buffer.data());
    if (superblock->magic == SPARSE_HEADER_MAGIC) {
        return true;
    }
    return false;
}

// Функция для преобразования строки в число
size_t parseSize(const std::string &str, bool isHex) {
    size_t result;
    std::stringstream ss;

    if (isHex) {
        ss << std::hex << str;
    } else {
        ss << str;
    }

    ss >> result;
    if (ss.fail()) {
        throw std::invalid_argument("Invalid number format: " + str);
    }

    return result;
}
