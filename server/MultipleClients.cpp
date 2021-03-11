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

Server::Server() { this->initServer(); }

Server::~Server() {
  FD_CLR(_listening, &_master);
  close(_listening);
}

void Server::updateDictionary(int key, std::string value, std::map<int, std::string> *dictionary){
  std::map<int, std::string>::iterator it = dictionary->find(key); 

  // Updates key if found
  if (it != dictionary->end()){
    it->second = value; 
  }
  // Else creates new key/value pair
  else {
    dictionary->insert(std::make_pair(key, value));
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
              _loggedUsers.erase(sock);
              std::string strOut = ss.str();
              send(outSock, strOut.c_str(), strOut.size() + 1, 0);
            }
          }

        } else {
          // send message to other clients, and not listening socket
          for (int outSock = 0; outSock <= FD_SETSIZE - 1; ++outSock) {
            if (outSock != _listening && outSock != sock) {
              std::ostringstream ss;
              ss << "USER #" << sock << ": " << buf;
              std::string strOut = ss.str();
              send(outSock, strOut.c_str(), strOut.size() + 1, 0);
            }
          }
        }
      }
    }
  }
}
