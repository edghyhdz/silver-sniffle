#include "NcursesDisplay.h"
#include <chrono>
#include <string>
#include <thread>
#include <iostream>

void NcursesDisplay::Display() {
  initscr();     // start ncurses
  noecho();      // do not print input values
  cbreak();      // terminate ncurses on ctrl + c
  start_color(); // enable color

  int x_max{getmaxx(stdscr)};
  int y_max{getmaxy(stdscr)};
  double text_x{0.70}, text_y{0.70}; 

  WINDOW *text_window = newwin(y_max * text_y, x_max * text_x, 0, 0);
  WINDOW *chat_window = newwin(y_max * (1 - text_y), x_max * text_x, y_max *text_y, 0);
  WINDOW *users_window = newwin(y_max - 1, x_max * (1 - text_x), 0, x_max *text_x);

  // Send keys timeout
  keypad(text_window, true);
  wtimeout(text_window, 10);

  switch (wgetch(text_window)) {
  case KEY_F0 + 1:
    std::string test{"LOOOOOOOOOL"};
    std::cout << "IS IT WOKING?" << std::endl;
    mvwprintw(text_window, 0, 0, test.c_str());
    break;
  }

  while (1) {
    // switch(wgetch(system_window)){
    //   case KEY_F0 + 1:
    //   editViewWindow(&view);
    //   // orchestrator.setCoinToPlot(view.first_coin);
    //   setCoinsVector(&coins_vector, view);
    //   orchestrator.setCoinsToPlot(coins_vector);
    //   orchestrator.setWindowRange(view.window_range);
    //   break;
    // }


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
    wclear(text_window);
    wclear(users_window);

    box(chat_window, 0, 0);
    box(text_window, 0, 0);
    box(users_window, 0, 0);

    wrefresh(chat_window);
    wrefresh(text_window);
    wrefresh(users_window);
    refresh();

    // Speed in which it is refreshed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  endwin();
}