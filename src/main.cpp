/*
g++ src/main.cpp src/NcursesDisplay.h src/NcursesDisplay.cpp src/utils/utility.h src/utils/trim.h -lcurses -lform -pthread
*/

#include "NcursesDisplay.h"

int main() {
    NcursesDisplay::Display(); 
}