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
#include "../utils/utility.h"

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
  bool missingKey = Utils::findWord(message, "-----BEGIN RSA PUBLIC KEY-----");

  if (!missingKey){
    std::cout << "Key socket #" << std::to_string(key) << message << std::endl; 
    this->updateDictionary(key, message, &_userToPK); 
  }
  else {
    // ? IF KEY NOT FOUND?
    std::string empty_key{"-----NOT FOUND-----"}; 
    this->updateDictionary(key, empty_key, &_userToPK); 
  }
}

std::string Server::getPK(int key){
  // If key was not found should return empty string
  return _userToPK.at(key); 
}

void Server::eraseMaps(int sock){
  _loggedUsers.erase(sock);
  _userFirstMessage.erase(sock);
  _userToPK.erase(sock);
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

        std::set<int> loggedUsersTemp = _loggedUsers; 
        _loggedUsers.insert(client);
        std::string all_users;
        for (int user : _loggedUsers) {
          all_users += std::to_string(user) + ";";
        }    

        // Append public keys from other logged users
        for (int user : _loggedUsers){
          if (user != client){
            std::string temp_pk = _userToPK.at(user); 
            all_users += "," + std::to_string(user) + ":" + temp_pk + ","; 
          }
        }

        std::cout << all_users << std::endl;   

        // Send welcome message containing users logged in and pkeys
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

              // Remove user from maps
              this->eraseMaps(sock); 

              std::string strOut = ss.str();
              send(outSock, strOut.c_str(), strOut.size() + 1, 0);
            }
          }

        } else {
          std::ostringstream ss;
          ss << buf;

          // Get string where we will get message length information
          std::string getLength = ss.str();  
          int msgLength;
          std::string temp_string, removeHeader;

          std::replace(getLength.begin(), getLength.end(), '\n', ' ');

          // Get message length, included in the beginning of the package
          std::istringstream sline(getLength);
          sline >> temp_string >> msgLength;

          // Needed get message length and then remove it from final message
          removeHeader = "-----BEGIN\n" + std::to_string(msgLength) + "\nEND-----";

          // Append full message
          std::string strMessage;
          char *strbuf = const_cast<char *>(buf);
          for (int i = 0; i < msgLength + removeHeader.length(); i++) {
            strMessage.push_back(strbuf[i]);
          }

          // Remove length information
          size_t p = -1;
          std::string tempWord = removeHeader + "";
          while ((p = strMessage.find(removeHeader)) != std::string::npos) {
            strMessage.replace(p, tempWord.length(), "");
          }

          // Get messages for every user where we have a valid public key
          std::vector<std::string> messages = Utils::split(strMessage, "-----NEWMESSAGE-----");

          // User header to be added to final message
          std::string user_header = "USER #" + std::to_string(sock) + ": ";

          // Get encrypted messages for all client users
          std::map<int, char *> userToMessage;  
          for (std::string msg : messages) {
            int usr;
            std::string tempString, getUser;
            getUser = msg; 

            std::replace(getUser.begin(), getUser.end(), '_', ' ');
            std::istringstream sline(getUser);
            sline >> usr;
            
            // Remove user info from original message
            size_t p = -1;
            std::string tempWord = std::to_string(usr) + "_";
            while ((p = msg.find(std::to_string(usr) + "_")) != std::string::npos) {
              msg.replace(p, tempWord.length(), "");
            }

            // Concatenate both char * manually
            char *tempbuf = new char[256 + user_header.length()];
            int j = 0;
            for (int i = 0; i < 256 + user_header.length(); i++) {
              if (i < user_header.length()) {
                tempbuf[i] = user_header[i];
              } else {
                tempbuf[i] = msg[j];
                j++;
              }
            }
            // Create message to be sent to each logged user
            userToMessage.insert(std::make_pair(usr, tempbuf));
          }

          // Check if it's users' first message
          // First message is sent via the client containing the pk
          std::string pK = ""; 
          if (this->userFirstMessage(sock)) {
            // Set to false afterwards
            this->updateDictionary(sock, false, &this->_userFirstMessage);
            this->getPK(sock, ss.str());
            pK = this->getPK(sock);
          }
          
          // send message to other clients, and not listening socket
          for (int outSock = 0; outSock <= FD_SETSIZE - 1; ++outSock) {
            if (outSock != _listening && outSock != sock) {
              if (pK == ""){
                std::map<int, char*>::iterator it = userToMessage.find(outSock);
                if (it != userToMessage.end()) {
                  send(outSock, it->second, 256 + user_header.length() + 1, 0);
                  // Release buffer memory
                  delete it->second; 
                } else {
                  // If user is not logged with official client, send giberish
                  send(outSock, ss.str().c_str(), ss.str().length() + 1, 0);
                }
              }
              else{
                // If users' first message, send back publick key
                std::string header_pk = user_header + pK; 
                send(outSock, header_pk.c_str(), header_pk.size() + 1, 0);
              }
            }
          }
        }
      }
    }
  }
}
