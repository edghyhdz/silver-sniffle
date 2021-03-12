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
#include <algorithm>

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

void ArrivingMessages::setUsers(std::string users) {
  std::lock_guard<std::mutex> lock(_mutex);
  std::istringstream sline(users);
  int n;
  char c;
  while (sline >> n >> c && c == ';') {
    this->_users.push_back(n);
  }
}

std::vector<int> ArrivingMessages::getUsers(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _users;
}

void ArrivingMessages::removeUser(std::string message){
  std::lock_guard<std::mutex> lock(_mutex);
  // remove user from _users;
  bool hasNotLeft = Utils::findWord(message, "has left the chat");
  // bool hasNotLeft = true; 

  std::string key;
  int value;

  if (!hasNotLeft) {
    std::replace(message.begin(), message.end(), ' ', '_');
    std::replace(message.begin(), message.end(), '#', ' ');
    std::replace(message.begin(), message.end(), '_', ' ');

    std::istringstream sline(message);
    sline >> key >> value;

    // Remove user that's left
    _users.erase(std::remove(_users.begin(), _users.end(), value),
                 _users.end());
  }
}

void ArrivingMessages::appendUser(int user){
  /*  Append from client in NcursesDisplay
      to _users vector
      user:   int user number
  */ 
  std::lock_guard<std::mutex> lock(_mutex);
  // Only append if item is not there
  if ( std::find(_users.begin(), _users.end(), user) == _users.end()){
    _users.push_back(user);
  }
}

void Client::appendUser(int user){
  this->_arrivingMessages.appendUser(user); 
}

std::vector<int> Client::getUsers(){
  return this->_arrivingMessages.getUsers();
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
  this->createConnection(); 
}

void Client::runClient(){

  // While loop:
  char buf[4096];
  bool firstMessage = true; 
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
      // First message contains user information
      if (firstMessage) {
        this->_arrivingMessages.setUsers(ss.str());
        firstMessage = false;
      } else { // All other messages
        // Check if message is related to user leaving chat
        this->_arrivingMessages.removeUser(ss.str()); 
        // Append message to chat vector
        this->_arrivingMessages.pushBack(ss.str());
      }
    }
  } while (true);

  //	Close the socket
  close(_sockFD);
} 

// Send messages to server
void Client::runSendMessage(){
    bool firstMessage = true; 
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(2)); 

      // First message to server includes public key of client
      if (firstMessage){
        std::string pK = this->getPublicKey(); 
        send(_sockFD, pK.c_str(), pK.size() + 1, 0); 
        firstMessage = false; 
      }

      // If there is a new message
      if (!this->_arrivingMessages.messageIsEmpty()){
        // Send to server
        std::string tempMessage = this->_arrivingMessages.getMessage(); 
        int sendRes = send(_sockFD, tempMessage.c_str(), tempMessage.size() + 1, 0);
        this->_arrivingMessages.clearMessage(); 
      }
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

// Sends public key as first message
std::string Client::getPublicKey(){
  // Load public key
  // return it and send it
  return "-----BEGIN RSA PUBLIC KEY-----"; 

}