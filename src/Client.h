#ifndef CLIEN_H
#define CLIEN_H
#include <string>
#include <mutex>
#include <curses.h>
#include <vector>


class Client {

public:
  Client(char *&ipAddress, char *&portNum, std::string *sendMsg, std::vector<std::string> *responses);
  
//   ~Client(); 
//   Client(Client &&o) = default;
  int createConnection();
  void runClient();
  void runSendMessage(); 

  void setResponse(std::string resonse); 
  std::string getResponse(); 

private:
  char *&_ipAddress;
  char *&_portNum;
  int _sockFD;
  int _connectR;
  std::string * _sendMsg; 
  std::string _response;
  bool _updatedResponse;
  std::mutex *_mtx; 
  std::vector<std::string> *_responses; 
};

#endif