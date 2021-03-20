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
#include <fstream>


/*
Class declarations -> ArrivingMessages and Client classes

-------------------------------------------------------------------------------
ArrivingMessages class declaration
-------------------------------------------------------------------------------
*/ 

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

  // Encrypt message with all users' public keys
  // and append it into one single message
  std::string encryptedMsg, msgLength; 

  for (int user : this->_users){
    std::map<int, std::string>::iterator it = _userToPK.find(user);

    if (it != _userToPK.end()) {
      std::string pK = it->second;
      if (pK != "-----NOT FOUND-----"){
        // Encrypt message and append user at the beginning of the message
        std::string user_header = std::to_string(user) + "_";
        encryptedMsg += user_header + this->rsa.encryptWithPK(Utils::trim(message), it->second);
        encryptedMsg += "-----NEWMESSAGE-----";
      }
    }
  }

  // Get message length to append to encrypted msg
  // This is information relevant to the server
  msgLength = "-----BEGIN\n" + std::to_string(encryptedMsg.length())+ "\nEND-----"; 

  _message = msgLength + encryptedMsg;
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

void ArrivingMessages::setUserData(std::string data) {
  std::lock_guard<std::mutex> lock(_mutex);
  std::istringstream sline(data);
  int n;
  char c;
  std::string users_string; 
  while (sline >> n >> c && c == ';') {
    this->_users.push_back(n);
    users_string += std::to_string(n) + ";";
  }

  // Get public keys
  std::istringstream pkeys(data);

  while (pkeys.good()) {
    std::string pKRaw;
    getline(pkeys, pKRaw, ',');
    if (pKRaw != "" && pKRaw != users_string) {
      // Split string to user/rsa_pk pairs
      std::vector<std::string> userToPK = Utils::split(pKRaw, ":"); 
      int user = stoi(userToPK[0]);
      std::string pK = userToPK[1];

      // update userToPK maps
      Utils::updateDictionary(user, pK, &_userToPK); 
    }
  }
}

std::vector<int> ArrivingMessages::getUsers(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _users;
}

std::map<int, std::string> ArrivingMessages::getPKeys(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _userToPK;
}

void ArrivingMessages::removeUser(std::string message){
  std::lock_guard<std::mutex> lock(_mutex);
  // remove user from _users;
  bool hasNotLeft = Utils::findWord(message, "has left the chat");

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
    
    // Remove users' public key
    _userToPK.erase(value); 
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

void ArrivingMessages::updatePK(int user, std::string pK){
  std::lock_guard<std::mutex> lock(_mutex);
  Utils::updateDictionary(user, pK, &_userToPK); 
}

// Sends public key as first message
std::string ArrivingMessages::sendPublicKey(){
  // Load public key
  // return it and send it
  return this->rsa.getPK(); 
}

void ArrivingMessages::decryptMessage(int index, int user, std::string encryptedMsg){
  std::lock_guard<std::mutex> lock(_mutex);
  std::map<int, std::string>::iterator it = _userToPK.find(user);
  std::string message = _responses[index];

  // If there is a RSA PUBLIC KEY -> do not decrypt
  bool isNotPK = Utils::findWord(message, "-----BEGIN RSA PUBLIC KEY-----");

  // Decrypt message if found
  if (it != _userToPK.end() && isNotPK) {
    // Fetch public key
    std::string pK = it->second;
    if (pK != "-----NOT FOUND-----") {
      // Decrypt message with users private key
      _responses[index] = "USER #" + std::to_string(user) + ": " + this->rsa.decryptWithSK(encryptedMsg, this->rsa.getSK());
    } else {
      std::string new_message = Utils::trim(message).c_str();
      _responses[index] = "|EXTERNAL| " + new_message;
    }
  }
}


/*
-------------------------------------------------------------------------------
Client class declaration
-------------------------------------------------------------------------------
*/ 

bool Client::generateKeys(){
  return this->_arrivingMessages.rsa.generateKeys(); 
}

void Client::decryptMessage(std::string message, int index){
  bool notUser = Utils::findWord(message, "USER #");
  std::string key, encryptedMsg, user_string;
  int user;
  char c; 

  // If we have a 'USER' in the beginning of the message
  if (!notUser) {
    std::string message_copy = message; 
    std::replace(message.begin(), message.end(), ' ', '_');
    std::replace(message.begin(), message.end(), '#', ' ');
    std::replace(message.begin(), message.end(), '_', ' ');

    std::istringstream sline(message);
    sline >> key >> user >> c;

    user_string = "USER #" + std::to_string(user);

    // if user number is followed by `:`
    // it requires decryption
    if (c == ':') {
      size_t p = -1;
      std::string tempWord = user_string + " ";
      while ((p = message_copy.find(user_string)) != std::string::npos){
        message_copy.replace(p, tempWord.length(), "");
      } 
      
      // Keep only the part without the facing 'USER #'
      encryptedMsg = message_copy.substr(1, message_copy.size());

      // Decrypt received message
      this->_arrivingMessages.decryptMessage(index, user, encryptedMsg);
    }
  }
}

// This function will just be superficial
// Will apend new users to be displayed in the users window
int Client::addUser(std::string message){

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

// Auxiliary method -> Add public key if found
// Only used if user logs into chat at a later stage
bool Client::addPK(std::string message){

  bool notPK = Utils::findWord(message, "-----BEGIN RSA PUBLIC KEY-----");

  std::string key;
  int value; 
  std::string pK;

  if (!notPK) {
    std::replace(message.begin(), message.end(), ' ', '$');
    std::replace(message.begin(), message.end(), '\n', ';');
    std::replace(message.begin(), message.end(), '#', ' ');

    std::istringstream sline(message);
    sline >> key >> value >> pK;

    std::replace(pK.begin(), pK.end(), '$', ' ');
    std::replace(pK.begin(), pK.end(), ';', '\n');
    // Delete first two characters
    pK = pK.substr(2, pK.size());
    this->updatePK(value, pK);

    return true;
  } 
  return false; 
}

void Client::processMessages(){
  // Check for new users or new users' public keys
  // Decrypting happens also here, it is included in
  std::vector<std::string> responses = this->getResponses(); 

  if (_prevResponses.size() > 0) {
    int diff = responses.size() - _prevResponses.size();
    // If there are new messages
    // Get the last <diff> items
    if (diff > 0) {
      for (int i = responses.size() - 1; i > _prevResponses.size() - 1; i--) {
        // Check if this response is a new user or not
        std::string tempResponse = responses[i];
        int userID = addUser(tempResponse);

        // check if it needs decryption
        this->decryptMessage(tempResponse, i); 

        // Add private key if any
        bool addedKey = addPK(tempResponse); 
        if (userID != -1) {
          this->appendUser(userID); 
        }
      }
      // In case first messages is the new user
    } else if (responses.size() <= 2) {
      for (int i = 0; i < responses.size(); i++) {
        std::string tempResponse = responses[i];
        int userID = addUser(tempResponse);

        // check if it needs decryption
        this->decryptMessage(tempResponse, i); 

        // Add private key if any
        bool addedKey = addPK(tempResponse);
        if (userID != -1) {
          this->appendUser(userID);
        }
      }
    }
  }

  // Save snapshot of previous responses
  _prevResponses = responses; 

}

void Client::updatePK(int user, std::string pK){
  this->_arrivingMessages.updatePK(user, pK); 
}

void Client::appendUser(int user){
  this->_arrivingMessages.appendUser(user); 
}

std::map<int, std::string> Client::getPKeys() {
  return this->_arrivingMessages.getPKeys();
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

int Client::getCountFM(){
  std::lock_guard<std::mutex> lock (*this->_mtx); 
  return this->_firstMessages; 
}

void Client::addCountFM(){
  std::lock_guard<std::mutex> lock (*this->_mtx); 
  this->_firstMessages++; 
}

Client::Client(char *&ipAddress, char *&portNum) : _ipAddress(ipAddress), _portNum(portNum){
  // Generate private and public keys
  this->generateKeys(); 
  this->_mtx = new std::mutex();
  this->_firstMessages = 0; 
  this->createConnection();
}

Client::~Client(){
  //	Close the socket
  close(_sockFD);
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
      std::string encryptedMsg; 

      // We need this count, since the first two messages may
      // Contain some information that comes without encryption
      if (this->getCountFM() <= 2){
        this->addCountFM(); 
      }

      // First message contains user information
      // Like user socket and user's rsa public key
      if (firstMessage) {
        this->_arrivingMessages.setUserData(ss.str());
        firstMessage = false;
      } else {
        // All other messages
        // Check if message is related to user leaving chat
        this->_arrivingMessages.removeUser(ss.str());

        std::string temp_string = ss.str(); 
        int userID = this->addUser(temp_string);
        userID = (userID == -1) ? 1 : userID;
        std::string user_header = "USER #" + std::to_string(userID) + ": "; 

        bool hasNotLeft = Utils::findWord(temp_string, "has joined the chat");
        bool isNotPK = Utils::findWord(temp_string, "-----BEGIN RSA PUBLIC KEY-----");

        if (hasNotLeft && isNotPK) {
          char *sendbuf = const_cast<char *>(buf);
          for (int i = 0; i < user_header.length() + 256; i++) {
            encryptedMsg.push_back(sendbuf[i]);
          }
          this->_arrivingMessages.pushBack(encryptedMsg);
        } else {
          this->_arrivingMessages.pushBack(temp_string);
        }
      }
    }
  } while (true);
} 

// Send messages to server
void Client::runSendMessage(){
    bool firstMessage = true; 
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(2)); 

      // First message to server includes public key of client
      if (firstMessage){
        std::string pK = this->_arrivingMessages.sendPublicKey(); 
        send(_sockFD, pK.c_str(), pK.size() + 1, 0); 
        firstMessage = false; 
      }

      // If there is a new message
      if (!this->_arrivingMessages.messageIsEmpty()){
        // Send to server
        std::string tempMessage = this->_arrivingMessages.getMessage();

        // This is the only work around i found to go from string to char
        // without loosing data due to encryption
        // using c_str() did not work due to '\0' termination
        char *sendbuf = new char[tempMessage.length()];
        for(int i = 0; i < tempMessage.length(); i++)
        {
            sendbuf[i] = tempMessage[i];
        }

        int sendRes = send(_sockFD, sendbuf, tempMessage.size() + 1, 0);
        this->_arrivingMessages.clearMessage(); 
        delete sendbuf; 
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
