/*
Main file multiclient server
g++ MultipleClients.h MultipleClients.cpp server.cpp ../utils/utility.h -o server
*/

#include "MultipleClients.h"

int main(){
    Server server = Server(); 
    server.runServer(); 
}