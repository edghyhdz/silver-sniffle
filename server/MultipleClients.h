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
  void updateDictionary(int key, std::string value, std::map<int, std::string> *dictionary);

private:
  int _listening;
  std::set<int> _loggedUsers; 
  // Key containing public keys
  std::map<int, std::string> userToPK; 
  fd_set _master;
};