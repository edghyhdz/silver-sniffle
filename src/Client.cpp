#include "Client.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <thread>
#include <curses.h>
#include <vector>
#include <sstream>


// Class definitions

int ArrivingMessages::getSize(){
  std::lock_guard<std::mutex> lock(_mutex); 
  return _responses.size(); 
}

void ArrivingMessages::pushBack(std::string response){
  std::lock_guard<std::mutex> lock(_mutex); 
  _responses.push_back(response); 
}

std::vector<std::string> ArrivingMessages::getResponses(){
  std::lock_guard<std::mutex> lock(_mutex); 
  return _responses; 
}

void ArrivingMessages::setMessage(std::string message){
  std::lock_guard<std::mutex> lock(_mutex); 
  _message = message; 
}

std::string ArrivingMessages::getMessage(){
  std::lock_guard<std::mutex> lock(_mutex); 
  return _message; 
}

bool ArrivingMessages::messageIsEmpty(){
  std::lock_guard<std::mutex> lock(_mutex); 
  return _message.empty();
}

void ArrivingMessages::clearMessage(){
  std::lock_guard<std::mutex> lock(_mutex); 
  _message.clear(); 
}

void Client::pushBack(std::string message){
  this->_arrivingMessages.pushBack(message); 
}

void Client::setMessage(std::string message){
  this->_arrivingMessages.setMessage(message); 
}

std::vector<std::string> Client::getResponses(){
  return this->_arrivingMessages.getResponses(); 
}

Client::Client(char *&ipAddress, char *&portNum) : _ipAddress(ipAddress), _portNum(portNum){
  this->_mtx = new std::mutex();
  this->_updatedResponse = false; 
  this->createConnection(); 
}

void Client::runClient(){

  // While loop:
  char buf[4096];
  std::string userInput;

  int counter = 0; 
  do {
    // Wait for response
    memset(buf, 0, 4096);
    int bytesReceived = recv(_sockFD, buf, 4096, 0);
    if (bytesReceived == -1) {
      std::cout << "There was an error getting response from server\r\n";
    } else {
      // Get response into output string
      std::ostringstream ss;
      ss << buf;
      this->_arrivingMessages.pushBack(ss.str()); 
    }
    } while(true);

    //	Close the socket
    close(_sockFD);
} 

// TODO: There are lots of shared resources
// Something could easily go wrong
void Client::runSendMessage(){
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(2)); 
      // If there is a new message
      if (!this->_arrivingMessages.messageIsEmpty()){
        // Send to server
        std::string tempMessage = this->_arrivingMessages.getMessage(); 
        int sendRes = send(_sockFD, tempMessage.c_str(), tempMessage.size() + 1, 0);
        this->_arrivingMessages.clearMessage(); 
      }
      // if (!_sendMsg->empty()){
      //   //		Send to server
      //   int sendRes = send(_sockFD, (*_sendMsg).c_str(), (*_sendMsg).size() + 1, 0);
      //   _sendMsg->clear(); 
      // }
    } while (true);
}

int Client::createConnection() {

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return 1;
  }

  addrinfo hints, *p;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int gAddRes = getaddrinfo(this->_ipAddress, this->_portNum, &hints, &p);
  if (gAddRes != 0) {
    std::cerr << gai_strerror(gAddRes) << "\n";
    return -2;
  }

  if (p == NULL) {
    std::cerr << "No addresses found\n";
    return -3;
  }

  // socket() call creates a new socket and returns it's descriptor
  _sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  if (_sockFD == -1) {
    std::cerr << "Error while creating socket\n";
    return -4;
  }

  _connectR = connect(_sockFD, p->ai_addr, p->ai_addrlen);
  if (_connectR == -1) {
    close(_sockFD);
    std::cerr << "Error while connecting socket\n";
    return -5;
  }

  return 0; 
}
