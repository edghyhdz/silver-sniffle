/*
Reference: https://www.programmersought.com/article/37955188510/
Reference certificates server
https://stackoverflow.com/questions/11705815/client-and-server-communication-using-ssl-c-c-ssl-protocol-dont-works
*/
#include <fstream>
#include <iostream>
#include <openssl/pem.h>
#include <stdio.h>
#include "RSAEncrypt.h"
#include <fstream> 
#include <sstream>

/*
Class declaration
*/

RSAEncrypt::RSAEncrypt(){
    // Load keys to private members
    this->loadKeys();

}

// As per default, it will load keys from
// the ./certificates folder
void RSAEncrypt::loadKeys(){

    std::ifstream sKeyRaw("./certificates/private.pem"); 
    std::string sKey;
    if (sKeyRaw){
        std::string line; 
        while (getline(sKeyRaw, line)){
            sKey += line + "\n"; 
        }
        // Remove trailing space
        sKey = sKey.substr(0, sKey.size() - 1);
        this->_secretKey = sKey; 
    }

    std::ifstream pKeyRaw("./certificates/public.pem"); 
    std::string pKey;
    if (pKeyRaw){
        std::string line; 
        while (getline(pKeyRaw, line)){
            pKey += line + "\n"; 
        }
        // Remove trailing space
        pKey = pKey.substr(0, pKey.size() - 1);
        this->_publicKey = pKey; 
    }
}
