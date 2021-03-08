#ifndef FIND_WORD_H
#define FIND_WORD_H

#include <string>

namespace Utils {
bool messageFromYou(std::string &sentence) {
  int pos = 0;
  while (true) {
    pos = sentence.find("YOU: ", pos++);
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