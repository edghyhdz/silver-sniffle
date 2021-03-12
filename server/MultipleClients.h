/*
Server class definition
*/
#include <set>
#include <map>

class Server {
public:
  Server();
  ~Server();
  int initServer();
  void runServer();
  template<typename T>
  void updateDictionary(int key, T value, std::map<int, T> *dictionary);
  bool userFirstMessage(int key); 
  void getPK(int key, std::string message); 


private:
  int _listening;
  std::set<int> _loggedUsers; 
  // Key containing public keys
  std::map<int, std::string> _userToPK; 
  std::map<int, bool> _userFirstMessage; 
  fd_set _master;
};