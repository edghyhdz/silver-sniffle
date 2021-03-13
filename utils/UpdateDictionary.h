#ifndef UPDATE_DICT_H
#define UPDATE_DICT_H
#include <map>
#include <vector>
#include <string>

namespace Utils {

template <typename T>

inline void updateDictionary(int key, T value, std::map<int, T> *dictionary) {

  typename std::map<int, T>::iterator it = dictionary->find(key);

  // Updates key if found
  if (it != dictionary->end()) {
    it->second = value;
  }
  // Else creates new key/value pair
  else {
    dictionary->insert(std::make_pair(key, value));
  }
}

inline std::vector<std::string> split(const std::string &str, const std::string &delim) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == std::string::npos)
      pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

} // namespace Utils

#endif