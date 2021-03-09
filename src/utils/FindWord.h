#ifndef FIND_WORD_H
#define FIND_WORD_H

#include <string>

namespace Utils {
inline bool findWord(std::string &sentence, std::string &&word) {
  int pos = 0;
  while (true) {
    pos = sentence.find(word, pos++);
    if (pos != std::string::npos) {
      pos++;
    } else {
      return true;
    }
    break;
  }
  return false;
}
} // namespace Utils
#endif