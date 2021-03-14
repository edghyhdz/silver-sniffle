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
  void encryptWithPK(); // Encrypt using public key
  void encryptWithSK(); // Encrypt with secret key
  void decryptWithSK(); // Decrypt with secret key
  void decryptWithPK(); // Decrypt with public key
private:
  std::string _secretKey;
  std::string _publicKey;
};

#endif