
#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#define FIELD_MAX_CHARS 32


#include <curses.h>
#include <mutex>

namespace NcursesDisplay {
void Display();
void DisplayMessages(WINDOW *window, std::string number);
void TextBox();

} // namespace NcursesDisplay

#endif