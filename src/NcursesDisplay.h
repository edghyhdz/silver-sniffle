
#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H
#define FIELD_MAX_CHARS 32

#include <curses.h>
#include <mutex>
#include <string>
#include <vector>

class Client; 

struct _viewwin {
	std::vector<std::string> _fields; 
};

namespace NcursesDisplay {
typedef struct _viewwin viewwin;

void Display(char *&ipAddress, char *&portNum);
void DisplayMessages(WINDOW *window, std::string number, viewwin *view, std::vector<std::string> responses, std::string *sendMsg); 
void TextBox(viewwin *view);

} // namespace NcursesDisplay

#endif