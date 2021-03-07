
#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H
#define FIELD_MAX_CHARS 32

#include <curses.h>
#include <mutex>
#include <string>

struct _viewwin {
	std::string field_1, field_2, field_3, field_4;
};

namespace NcursesDisplay {
typedef struct _viewwin viewwin;

void Display();
void DisplayMessages(WINDOW *window, std::string number, viewwin view); 
void TextBox(viewwin *view);

} // namespace NcursesDisplay

#endif