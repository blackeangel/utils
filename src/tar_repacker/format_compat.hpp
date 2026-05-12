#pragma once
// format_compat.hpp — std::format fallback for NDK / older libc++
// NDK r25c (Clang 14) ships libc++ without <format>.
// We provide a minimal variadic replacement that covers the patterns used
// in tar_repacker: format("{}", str), format("{}: {}", str, str), etc.

#if defined(__has_include) && __has_include(<format>)
#  include <format>
#else
#  include <string>
#  include <sstream>
#  include <stdexcept>

namespace std {

namespace _fmt_detail {
    // Append next {} placeholder with the value, recurse on the rest.
    inline void _apply(std::string& out, const char* tmpl) {
        out += tmpl;   // no more placeholders
    }
    template<typename T, typename... Rest>
    inline void _apply(std::string& out, const char* tmpl, T&& val, Rest&&... rest) {
        const char* p = tmpl;
        while (*p) {
            if (p[0] == '{' && p[1] == '}') {
                std::ostringstream ss;
                ss << val;
                out += ss.str();
                _apply(out, p + 2, std::forward<Rest>(rest)...);
                return;
            }
            out += *p++;
        }
    }
} // namespace _fmt_detail

template<typename... Args>
inline std::string format(const char* tmpl, Args&&... args) {
    std::string out;
    out.reserve(128);
    _fmt_detail::_apply(out, tmpl, std::forward<Args>(args)...);
    return out;
}
template<typename... Args>
inline std::string format(const std::string& tmpl, Args&&... args) {
    return format(tmpl.c_str(), std::forward<Args>(args)...);
}

} // namespace std
#endif
