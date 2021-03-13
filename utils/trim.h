#ifndef TRIM_H
#define TRIM_H
#include <algorithm>
#include <string>

namespace Utils {

const std::string WHITESPACE = " \n\r\t\f\v";

// Reference:
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
inline std::string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
inline std::string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

inline std::string trim(const std::string &s) { return rtrim(ltrim(s)); }
} // namespace Utils

#endif