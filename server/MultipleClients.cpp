/*
Server class declaration
*/

#include "MultipleClients.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

bool findWord(std::string &sentence, std::string &&word) {
  int pos = 0;
  while (true) {
    pos = sentence.find(word, pos++);
    if (pos != std::string::npos) {
      pos++;
    } else {
      return true;
    }
    break;
  }
  return false;
}

Server::Server() { this->initServer(); }

Server::~Server() {
  std::cout << "Closing the server" << std::endl; 
  FD_CLR(_listening, &_master);
  close(_listening);
}

template<typename T>
void Server::updateDictionary(int key, T value, std::map<int, T> *dictionary){

  typename std::map<int, T>::iterator it = dictionary->find(key);

  // Updates key if found
  if (it != dictionary->end()){
    it->second = value; 
  }
  // Else creates new key/value pair
  else {
    dictionary->insert(std::make_pair(key, value));
  }
}

bool Server::userFirstMessage(int k) {
  std::map<int, bool>::iterator it = _userFirstMessage.find(k);
  if (it == _userFirstMessage.end()) {
    // Did not find the key
    return true;
  } else {
    return it->second;
  }
}

void Server::getPK(int key, std::string message){
  bool foundKey = findWord(message, "-----BEGIN RSA PUBLIC KEY-----");

  if (foundKey){
    this->updateDictionary(key, message, &_userToPK); 
  }
  else {
    // ? IF KEY NOT FOUND?
  }

}

int Server::initServer() {
  this->_listening = socket(AF_INET, SOCK_STREAM, 0);

  if (_listening == -1) {
    std::cerr << "Can't create a socket!";
    return -1;
  }

  // Bind the socket to an IP / Port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(_listening, (sockaddr *)&hint, sizeof(hint)) == -1) {
    std::cerr << "Can't bind to IP/Port";
    return -2;
  }
  // Mark socket for listening in
  if (listen(_listening, SOMAXCONN) == -1) {
    std::cerr << "Can't listen";
    return -3;
  }

  FD_ZERO(&_master);
  // Add to set
  FD_SET(_listening, &_master);
}

void Server::runServer() {
  // Servers may only accept connection and receive a message
  while (true) {
    // copies all
    auto copy = _master;
    int socketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);

    for (int sock = 0; sock <= FD_SETSIZE - 1; ++sock) {
      if (!FD_ISSET(sock, &copy))
        continue;
      sockaddr_in req_addr;

      if (sock == _listening) {

        // Accept new connection
        auto client = accept(_listening, nullptr, nullptr);

        // Add new connection to the list of connected clients
        FD_SET(client, &_master);
        _loggedUsers.insert(client);
        std::string all_users;
        for (int user : _loggedUsers) {
          all_users += std::to_string(user) + ";";
        }

        // Send welcome message
        send(client, all_users.c_str(), all_users.size() + 1, 0);

        // Have tell you who has joined the chat
        for (int outSock = 0; outSock <= FD_SETSIZE - 1; ++outSock) {
          if (outSock != _listening && outSock != sock) {
            std::ostringstream ss;
            ss << "USER #" << client << " has joined the chat\r\n";
            std::string strOut = ss.str();
            send(outSock, strOut.c_str(), strOut.size() + 1, 0);
          }
        }
      } else {
        // Accept new message
        char buf[4096];
        memset(buf, 0, 4096);

        // Receive message
        int bytesIn = recv(sock, buf, 4096, 0);
        if (bytesIn <= 0) {
          // drop client
          close(sock);
          FD_CLR(sock, &_master);

          // Send message to other clients that user has left the chat
          for (int outSock = 0; outSock <= FD_SETSIZE - 1; ++outSock) {
            if (outSock != _listening && outSock != sock) {
              std::ostringstream ss;
              ss << "USER #" << sock << " has left the chat";

              // TODO: erase them all in private method
              _loggedUsers.erase(sock);
              _userFirstMessage.erase(sock);
              _userToPK.erase(sock);

              std::string strOut = ss.str();
              send(outSock, strOut.c_str(), strOut.size() + 1, 0);
            }
          }

        } else {
          std::ostringstream ss;
          ss << "USER #" << sock << ": " << buf;
          std::string strOut = ss.str();

          // Check if it's users' first message
          // First message is sent via the client containing the pk
          std::string test_message = ""; 
          if (this->userFirstMessage(sock)) {
            std::cout << "did it go here? message: " << strOut << std::endl;
            // Set to false afterwards
            this->updateDictionary(sock, false, &this->_userFirstMessage);
            this->getPK(sock, strOut);
            test_message = "GOT PUBLIC KEY"; 
            // send(outSock, test_message.c_str(), test_message.size() + 1, 0);
          }

          // send message to other clients, and not listening socket
          for (int outSock = 0; outSock <= FD_SETSIZE - 1; ++outSock) {
            if (outSock != _listening && outSock != sock) {

              if (test_message == ""){
                send(outSock, strOut.c_str(), strOut.size() + 1, 0);
              }
              else{
              // If users' first message, send back publick key
              
                send(outSock, test_message.c_str(), test_message.size() + 1, 0);
              }
              // if (!this->userFirstMessage(sock)) {

              //   std::string pk{"test publick key :-D"}; 
              //   send(outSock, pk.c_str(), pk.size() + 1, 0);
              // }
            }
          }
        }
      }
    }
  }
}
