/*
g++ src/main.cpp src/NcursesDisplay.h src/NcursesDisplay.cpp src/utils/utility.h src/utils/trim.h -lcurses -lform -pthread
g++ src/main.cpp src/NcursesDisplay.h src/NcursesDisplay.cpp src/utils/utility.h src/utils/trim.h src/utils/TimeStamp.h src/Client.h src/Client.cpp -lcurses -lform -pthread
*/

#include "NcursesDisplay.h"
#include "Client.h"

int main(int argc, char *argv[]) {

    auto &ipAddress = argv[1];
    auto &portNum   = argv[2];
    NcursesDisplay::Display(ipAddress, portNum); 
}