#include "main.hpp"

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
    for (auto item : src_set)
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
        result.push_back(std::make_pair(num_set[i], num_set[i + 1]));
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

        std::unique_ptr<char> readBuf(new char[bufSize]);

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

