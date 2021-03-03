
#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

namespace NcursesDisplay {
void Display();

void DisplayMessages(WINDOW *window);
void SendText(WINDOW *window);

} // namespace NcursesDisplay

#endif