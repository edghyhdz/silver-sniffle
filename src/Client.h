#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <mutex>
#include <curses.h>
#include <vector>

// Auxiliary class to queue messages in a thread-safe manner
class ArrivingMessages {
public:
  // getters / setter
  int getSize();
  void pushBack(std::string response);
  void setMessage(std::string message); 
  std::vector<std::string> getResponses();
  std::string getMessage(); 
  bool messageIsEmpty(); 
  void clearMessage(); 

private:
  std::vector<std::string> _responses;
  std::string _message; 
  std::mutex _mutex;
};

class Client {

public:
  Client(char *&ipAddress, char *&portNum);
  int createConnection();
  void runClient();
  void runSendMessage(); 

  // getters / setters related to arrivingMessages list
  void setMessage(std::string message); 

  std::vector<std::string> getResponses(); 
  void clearMessage(); 
  void pushBack(std::string message); 


private:
  char *&_ipAddress;
  char *&_portNum;
  int _sockFD;
  int _connectR;
  ArrivingMessages _arrivingMessages; 
  bool _updatedResponse;
  std::mutex *_mtx; 
};

#endif