/*
Reference: https://www.programmersought.com/article/37955188510/
Reference certificates server
https://stackoverflow.com/questions/11705815/client-and-server-communication-using-ssl-c-c-ssl-protocol-dont-works
*/

#ifndef RSAEncrypt_H
#define RSARSAEncrypt_H

#include "openssl/rsa.h"
#include <string>

/*
Class definition
*/

class RSAEncrypt {
public:
  RSAEncrypt(); 
  void loadKeys();      // Load secret key
  std::string encryptWithPK(const std::string &message, const std::string &pK); // Encrypt using public key
  std::string encryptWithSK(const std::string &message, const std::string &sK); // Encrypt with secret key
  std::string decryptWithSK(const std::string &message, const std::string &sK); // Decrypt with secret key
  std::string decryptWithPK(const std::string &message, const std::string &pK); // Decrypt with public key
  bool generateKeys(); 

  std::string getPK(){ return _publicKey; }
  std::string getSK(){ return _secretKey; } 

private:
  std::string _secretKey;
  std::string _publicKey;
};

#endif