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
#include <memory>
#include <sstream>
#include <assert.h>
#include "Client.h"
#include "utility.h"


static std::mutex mtx;

// Works as a copy of the chat messages
// Used as comparison basis to search for new logged users
static std::vector<std::string> prevMessages; 

// This function will just be superficial
// Will apend new users to be displayed in the users window
int addUser(std::string message){

  bool hasNotLeft = Utils::findWord(message, "has joined the chat");

  std::string key;
  int value;

  if (!hasNotLeft) {
    std::replace(message.begin(), message.end(), ' ', '_');
    std::replace(message.begin(), message.end(), '#', ' ');
    std::replace(message.begin(), message.end(), '_', ' ');

    std::istringstream sline(message);
    sline >> key >> value;
    return value; 
  }
  return -1; 
}

void NcursesDisplay::DisplayUsers(WINDOW *window, std::shared_ptr<Client> client){
  const int column{1}; 
  int row{1}; 

  std::vector<int> users = client->getUsers();

  wattron(window, A_BOLD);
  wattron(window, COLOR_PAIR(4));
  mvwprintw(window, row++, column, "Logged Users:");
  wattroff(window, COLOR_PAIR(4));
  wattroff(window, A_BOLD);

  for (int user : users){
    std::string temp_user = "User: " + std::to_string(user); 
    mvwprintw(window, row++, column, temp_user.c_str());
  }
}
 
void NcursesDisplay::DisplayMessages(WINDOW *window, viewwin *view, std::shared_ptr<Client> client) {
  std::string message_text; 

  // Format message_text to fit max window size
  // This is the message that was input to the console
  std::string checker = "";  
  mtx.lock(); 
  for (std::string &text : view->_fields){
    message_text += text + " "; 
    checker += text; 
    text.clear(); 
  }
  mtx.unlock(); 

  // Get vector with responses from server < get this out
  std::vector<std::string> responses = client->getResponses(); 

  // TODO: Get publicKey into map and delete it from response dictionary
  // It should come in the first server message if any user has already logged in

  // Check for new users
  if (prevMessages.size() > 0) {
    int diff = responses.size() - prevMessages.size();
    // If there are new messages
    // Get the last <diff> items
    if (diff > 0) {
      for (int i = responses.size() - 1; i > prevMessages.size() - 1; i--) {
        // Check if this response is a new user or not
        std::string tempResponse = responses[i];
        int userID = addUser(tempResponse);
        if (userID != -1) {
          client->appendUser(userID); 
        }
      }
      // In case first message is the new user
    } else if (responses.size() <= 2) {
      std::string tempResponse = responses[0];
      int userID = addUser(tempResponse);
      if (userID != -1) {
        client->appendUser(userID);
      }
    }
  }

  // TODO: Check new responses for users that might have logged in
  prevMessages = responses; 

  if (checker != "") {
    long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    
    // Set message to be sent to the server
    client->setMessage(message_text); 

    // Add your message to be also displayed into this window
    client->pushBack("YOU: " + message_text); 
  }

  int response_counter = 1; 
  int diff = 0; 
  int counter = 0; 

  if (responses.size() >= window->_maxy - 1) {
    diff = responses.size() - window->_maxy + 1; 
  }

  for (std::string &response : responses) {
    if (counter >= diff) {
      int color_print = Utils::findWord(response, "YOU: ") ? 0 : 4;
      wattron(window, COLOR_PAIR(color_print));
      mvwprintw(window, response_counter++, 1, (Utils::trim(response)).c_str());
      wattroff(window, COLOR_PAIR(color_print));
    }
    counter++;
  }
}

void NcursesDisplay::TextBox(viewwin *view) {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  /* Figure out where to put the window */
  int ym, xm;
  getmaxyx(stdscr, ym, xm);

  /* Create a new window and draw the form */
  WINDOW *fwin = newwin(ym * (1 - 0.7), xm * 0.7, ym * 0.7, 0);
  keypad(fwin, TRUE);
  werase(fwin);

  wattron(fwin, A_BOLD);
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
  
  snprintf(printbuf, max_chars + 1, "%s", "//");
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
      for (int i = 0; i < 4; i++) {
        snprintf(printbuf, max_chars + 1, "%s", field_buffer(fields[i], 0));
        view->_fields[i] = Utils::trim(printbuf);
        // Empty field once pressing enter
        set_field_buffer(fields[i], 0, "");
      }
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
      snprintf(printbuf, max_chars + 1, "%s", field_buffer(f->current, 0));
      leneff = Utils::trim(printbuf).size();
      if (leneff >= max_chars - 1) {
        form_driver(f, REQ_NEXT_FIELD);
      }
      mtx.unlock();
      break;
    }   
  }
}

void NcursesDisplay::Display(char *&ipAddress, char *&portNum) {
  initscr();     // start ncurses
  noecho();      // do not print input values
  cbreak();      // terminate ncurses on ctrl + c
  start_color(); // enable color

  std::vector<std::thread> threads;

  int x_max{getmaxx(stdscr)};
  int y_max{getmaxy(stdscr)};
  double text_x{0.70}, text_y{0.70};

  WINDOW *chat_window = newwin(y_max * text_y, x_max * text_x, 0, 0);
  WINDOW *users_window = newwin(y_max - 1, x_max * (1 - text_x), 0, x_max * text_x);
  viewwin view; 
  for (int i=0; i < 4; i++){
    view._fields.push_back(""); 
  }
  std::shared_ptr<Client> client = std::make_shared<Client>(ipAddress, portNum); 
  
  threads.emplace_back(std::thread(NcursesDisplay::TextBox, &view));
  threads.emplace_back(std::thread(&Client::runClient, client)); 
  threads.emplace_back(std::thread(&Client::runSendMessage, client)); 

  while (1) {
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

    DisplayMessages(chat_window,  &view, client);
    DisplayUsers(users_window, client); 

    mtx.lock();
    wrefresh(chat_window);
    wrefresh(users_window);
    refresh();
    mtx.unlock();

    // TODO: Speed should be refreshed based on messagequeue from server
    // Speed in which it is refreshed 
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  threads.front().join();
  std::for_each(threads.begin(), threads.end(), [](std::thread &t) { t.join(); });
  endwin();
}