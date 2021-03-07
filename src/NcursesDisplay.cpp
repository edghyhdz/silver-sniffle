#include "NcursesDisplay.h"
#include <algorithm>
#include <chrono>
#include <form.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <assert.h>
#include <string.h>
#include "utils/utility.h"

static std::mutex mtx;
 
void NcursesDisplay::DisplayMessages(WINDOW *window, std::string number, viewwin view) {
  mvwprintw(window, 4, 4, number.c_str());
  std::string test = view.field_1 + "."; 
  std::string test2 = view.field_2 + "."; 
  mvwprintw(window, 7, 7, test.c_str());
  mvwprintw(window, 10, 10, test2.c_str());
}

void NcursesDisplay::TextBox(viewwin *view) {
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  /* Figure out where to put the window */
  int ym, xm;
  getmaxyx(stdscr, ym, xm);

  /* Create a new window and draw the form */
  WINDOW *fwin = newwin(ym * (1 - 0.7), xm * 0.7, ym * 0.7, 0);
  // WINDOW *fwin = newwin(wheight, wwidth, wy, wx);
  keypad(fwin, TRUE);
  werase(fwin);

  wattron(fwin, A_BOLD);
  //   box(fwin, 0, 0);
  std::string send_text = "[ENTER] SEND";
  mvwprintw(fwin, 1, 1, send_text.c_str());
  wattroff(fwin, A_BOLD);

  /* Create the form fields */
  FIELD *fields[4];
  int max_chars = fwin->_maxx - 10; 
  char printbuf[max_chars + 1];
  fields[0] = NULL;
  for (int i = 0; i < 4; i++) {
    fields[i] = new_field(1, max_chars, i, 0, 0, 0);
    field_opts_off(fields[i], O_AUTOSKIP | O_STATIC);
    set_max_field(fields[i], 20);
  }
  
  snprintf(printbuf, max_chars + 1, "%s", "-");
  set_field_buffer(fields[0], 0, printbuf);
  set_max_field(fields[0], 20);
  snprintf(printbuf, max_chars+ 1, "%s", "");
  set_field_buffer(fields[1], 0, printbuf);
  snprintf(printbuf, max_chars + 1, "%s", "");
  set_field_buffer(fields[2], 0, printbuf);
  snprintf(printbuf, max_chars + 1, "%s", "");
  set_field_buffer(fields[3], 0, printbuf);

  fields[4] = NULL;

  /* Create a subwindow for the form fields */
  WINDOW *fsub = derwin(fwin, 0, max_chars + 5, 2, 1);
  keypad(fsub, TRUE);

  /* Create the actual form */
  FORM *f = new_form(fields);
  
  set_form_win(f, fwin);
  set_form_sub(f, fsub);
  post_form(f);
  wrefresh(fwin);
  wrefresh(fsub);
  curs_set(1);

  /* Handle input */
  int savewin = 1;
  int exitloop = 0;
  int ch;
  size_t leneff;

  while (!exitloop) {
    mtx.lock();
    box(fwin, 0, 0);
    mtx.unlock();
    switch (ch = wgetch(fwin)) {
    case KEY_DOWN:
      mtx.lock();
      form_driver(f, REQ_NEXT_FIELD);
      form_driver(f, REQ_END_LINE);
      mtx.unlock();
      break;

    case KEY_UP:
      mtx.lock();
      form_driver(f, REQ_PREV_FIELD);
      form_driver(f, REQ_END_LINE);
      mtx.unlock();
      break;

    case KEY_LEFT:
      mtx.lock();
      form_driver(f, REQ_PREV_CHAR);
      mtx.unlock();
      break;

    case KEY_RIGHT:
      mtx.lock();
      form_driver(f, REQ_NEXT_CHAR);
      mtx.unlock();
      break;

    // Delete the char before cursor
    case KEY_BACKSPACE:
    case 127:
      mtx.lock();
      if (f->current == fields[0] && f->curcol == 0) {
        mtx.unlock(); 
        break; 
      }
      if (f->curcol == 0) {
        form_driver(f, REQ_PREV_FIELD);
        form_driver(f, REQ_END_LINE);
        form_driver(f, REQ_DEL_PREV);
      } else {
        form_driver(f, REQ_DEL_PREV);
      }
      mtx.unlock();
      break;

    // Delete the char under the cursor
    case KEY_DC:
      mtx.lock();
      form_driver(f, REQ_DEL_CHAR);
      mtx.unlock();
      break;
    case '\n':
      mtx.lock();
      form_driver(f, REQ_VALIDATION);
      snprintf(printbuf, strlen(field_buffer(fields[0], 0)) + 1, "%s",
               field_buffer(fields[0], 0));
      view->field_1 = Utils::trim(printbuf);
      set_field_buffer(fields[0], 0, "");
      set_field_buffer(fields[1], 0, "");
      set_field_buffer(fields[2], 0, "");
      set_field_buffer(fields[3], 0, "");
      form_driver(f, REQ_FIRST_FIELD);
      mtx.unlock();
    default:
      mtx.lock();
      if ((f->current == fields[3]) && (f->curcol == max_chars - 2)){
        mtx.unlock(); 
        break; 
      }
      form_driver(f, ch);
      form_driver(f, REQ_VALIDATION);
      snprintf(printbuf, max_chars + 1, "%s",
               field_buffer(f->current, 0));
      leneff = Utils::trim(printbuf).size();
      if (leneff >= max_chars - 1) {
        form_driver(f, REQ_NEXT_FIELD);
      }
      mtx.unlock();
      break;
    }   
  }

  // if (savewin) {
  //         form_driver(f, REQ_VALIDATION);
  //         view->first_coin = field_buffer(fields[0], 0);
  //         view->second_coin = field_buffer(fields[1], 0);
  //         view->third_coin = field_buffer(fields[2], 0);
  //         view->window_range = std::stoi(field_buffer(fields[3], 0));
  //       }

  /* Clean up */
  curs_set(0);
  unpost_form(f);
  free_form(f);
  for (int i = 0; i < 4; i++)
    free_field(fields[i]);
  delwin(fsub);
  delwin(fwin);
  refresh();
  // return savewin;
}

void NcursesDisplay::Display() {
  initscr();     // start ncurses
  noecho();      // do not print input values
  cbreak();      // terminate ncurses on ctrl + c
  start_color(); // enable color

  std::vector<std::thread> threads;
  int x_max{getmaxx(stdscr)};
  int y_max{getmaxy(stdscr)};
  double text_x{0.70}, text_y{0.70};

  WINDOW *chat_window = newwin(y_max * text_y, x_max * text_x, 0, 0);
  // WINDOW *text_window = newwin(y_max * (1 - text_y), x_max * text_x, y_max
  // *text_y, 0);
  WINDOW *users_window = newwin(y_max - 1, x_max * (1 - text_x), 0, x_max * text_x);
  viewwin view; 
  view.field_1 = ""; 
  view.field_2 = ""; 
  view.field_3 = ""; 
  view.field_4 = ""; 

  std::thread t(NcursesDisplay::TextBox, &view);

  int counter = 0;
  while (1) {

    // std::lock_guard<std::mutex> lock(mtx);
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_WHITE);
    init_pair(8, COLOR_WHITE, COLOR_GREEN);
    init_pair(9, COLOR_WHITE, COLOR_BLACK);
    wclear(chat_window);
    wclear(users_window);

    box(chat_window, 0, 0);
    box(users_window, 0, 0);

    DisplayMessages(chat_window, std::to_string(counter), view);
    DisplayMessages(users_window, std::to_string(counter), view);

    mtx.lock();
    wrefresh(chat_window);
    // wrefresh(text_window);
    wrefresh(users_window);
    refresh();
    mtx.unlock();

    // Speed in which it is refreshed
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    counter++;
  }
  t.join();
  endwin();
}