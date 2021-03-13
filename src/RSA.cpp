/*
Reference: https://www.programmersought.com/article/37955188510/
Reference certificates server
https://stackoverflow.com/questions/11705815/client-and-server-communication-using-ssl-c-c-ssl-protocol-dont-works
*/
#include <fstream>
#include <iostream>
#include <openssl/pem.h>
#include <stdio.h>
#include "RSA.h"

/*
Class declaration
*/

// As per default, it will load keys from
// ./certificates folder
void RSA::loadKeys(){

}

