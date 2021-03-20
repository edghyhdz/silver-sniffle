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
#include "openssl/rsa.h"
#include <string.h>
#include "Variables.h"

/*
TODO: Remove this workaround

*/

/*
Class declaration
*/

RSAEncrypt::RSAEncrypt(){
    // Load keys to private members
    this->loadKeys();
}

std::string RSAEncrypt::decryptWithPK(const std::string &message, const std::string & pK)
{
  std::string decrypt_text;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)pK.c_str(), -1);
	RSA* rsa = RSA_new();
	
	 // Note-------Use the public key in the first format for decryption
	rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);
	 // Note-------Use the public key in the second format for decryption (we use this format as an example)
	// rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	if (!rsa)
	{
		BIO_free_all(keybio);
        return "LOOOOL"; 
        // return decrypt_text;
	}
 
	 // Get the maximum length of RSA single processing
	int len = RSA_size(rsa);
	char *sub_text = new char[len + 1];
	memset(sub_text, 0, len + 1);
	int ret = 0;
	std::string sub_str;
	int pos = 0;

  int counter = 0; 
	// Decrypt the ciphertext in segments
	while (pos < message.length()) {
		sub_str = message.substr(pos, len);
		memset(sub_text, 0, len + 1);
		ret = RSA_public_decrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
		if (ret >= 0) {
			decrypt_text.append(std::string(sub_text, ret));
			pos += len;
		}
    counter++; 
    if (counter>5000){
      break; 
    }
	}
 
	// release memory  
	delete sub_text;
	BIO_free_all(keybio);
	RSA_free(rsa);
 
	return decrypt_text;
}

std::string RSAEncrypt::encryptWithSK(const std::string &message, const std::string &sK)
{
	std::string encrypt_text;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)sK.c_str(), -1);
	RSA* rsa = RSA_new();
	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	if (!rsa)
	{
		BIO_free_all(keybio);
		return "NOTHING!";
	}
 
	 // Get the maximum length of the data block that RSA can process at a time
	int key_len = RSA_size(rsa);
	 int block_len = key_len-11; // Because the filling method is RSA_PKCS1_PADDING, so you need to subtract 11 from the key_len
 
	 // Apply for memory: store encrypted ciphertext data
	char *sub_text = new char[key_len + 1];
	memset(sub_text, 0, key_len + 1);
	int ret = 0;
	int pos = 0;
	std::string sub_str;
	 // Encrypt the data in segments (the return value is the length of the encrypted data)
	while (pos < message.length()) {
		sub_str = message.substr(pos, block_len);
		memset(sub_text, 0, key_len + 1);
		ret = RSA_private_encrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
		if (ret >= 0) {
			encrypt_text.append(std::string(sub_text, ret));
		}
		pos += block_len;
	}
	
	 // release memory  
	delete sub_text;
	BIO_free_all(keybio);
	RSA_free(rsa);
 
	return encrypt_text;
}

std::string RSAEncrypt::encryptWithPK(const std::string &message, const std::string &pK) {
  std::string encrypt_text;
  BIO *keybio = BIO_new_mem_buf((unsigned char *)pK.c_str(), -1);
  RSA *rsa = RSA_new();
  // Note the public key in the first format
  rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);

  // Get the maximum length of the data block that RSA can process at a
  // time
  int key_len = RSA_size(rsa);
  int block_len = key_len - 11;
  // Because the filling method is RSA_PKCS1_PADDING, so
  // you need to subtract 11 from the key_len

  // Apply for memory: store encrypted ciphertext data
  char *sub_text = new char[key_len + 1];
  memset(sub_text, 0, key_len + 1);
  int ret = 0;
  int pos = 0;
  std::string sub_str;
  // Encrypt the data in segments (the return value is the length of the
  // encrypted data)
  while (pos < message.length()) {
    sub_str = message.substr(pos, block_len);
    memset(sub_text, 0, key_len + 1);
    ret = RSA_public_encrypt(sub_str.length(),
                             (const unsigned char *)sub_str.c_str(),
                             (unsigned char *)sub_text, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0) {
      encrypt_text.append(std::string(sub_text, ret));
    }
    pos += block_len;
  }

  // release memory
  BIO_free_all(keybio);
  RSA_free(rsa);
  delete[] sub_text;

  return encrypt_text;
}

std::string RSAEncrypt::decryptWithSK(const std::string &message, const std::string &sK){
  std::string decrypt_text;
  RSA *rsa = RSA_new();
  BIO *keybio;
  keybio = BIO_new_mem_buf((unsigned char *)sK.c_str(), -1);

  rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
  if (rsa == nullptr) {
    return std::string();
  }

  // Get the maximum length of RSA single processing
  int key_len = RSA_size(rsa);
  char *sub_text = new char[key_len + 1];
  memset(sub_text, 0, key_len + 1);
  int ret = 0;
  std::string sub_str;
  int pos = 0;
  int counter = 0; 
  // Decrypt the ciphertext in segments
  while (pos < message.length()) {
    sub_str = message.substr(pos, key_len);
    memset(sub_text, 0, key_len + 1);
    ret = RSA_private_decrypt(
        sub_str.length(), (const unsigned char *)sub_str.c_str(),
        (unsigned char *)sub_text, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0) {
      decrypt_text.append(std::string(sub_text, ret));
      pos += key_len;
    }
    counter++;
    // Really bad workaround, but just for now
    // To avoid encryption get stucked forever
    if (counter > 5000){
      break; 
    }
  }
  // release memory
  delete[] sub_text;
  BIO_free_all(keybio);
  RSA_free(rsa);

  return decrypt_text;
}

// As per default, it will load keys from
// the ./certificates folder
void RSAEncrypt::loadKeys(){
  std::string path = CERTIFICATES_PATH;
  std::ifstream sKeyRaw(path + "private.pem");
  std::string sKey;
  if (sKeyRaw) {
    std::string line;
    while (getline(sKeyRaw, line)) {
      sKey += line + "\n";
    }
    // Remove trailing space
    sKey = sKey.substr(0, sKey.size() - 1);
    this->_secretKey = sKey;
  }

  std::ifstream pKeyRaw(path + "public.pem");
  std::string pKey;
  if (pKeyRaw) {
    std::string line;
    while (getline(pKeyRaw, line)) {
      pKey += line + "\n";
    }
    // Remove trailing space
    pKey = pKey.substr(0, pKey.size() - 1);
    this->_publicKey = pKey;
  }
}

/*
Reference:
https://www.dynamsoft.com/codepool/how-to-use-openssl-generate-rsa-keys-cc.html

*/
bool RSAEncrypt::generateKeys(){
  std::string path = CERTIFICATES_PATH; 
  int ret = 0;
  RSA *r = NULL;
  BIGNUM *bne = NULL;
  BIO *bp_public = NULL, *bp_private = NULL;

  int bits = 2048;
  unsigned long e = RSA_F4;

  // 1. generate rsa key
  bne = BN_new();
  ret = BN_set_word(bne, e);
  if (ret != 1) {
    goto free_all;
  }

  r = RSA_new();
  ret = RSA_generate_key_ex(r, bits, bne, NULL);
  if (ret != 1) {
    goto free_all;
  }

  // 2. save public key
  bp_public = BIO_new_file((path + "public.pem").c_str(), "w+");
  ret = PEM_write_bio_RSAPublicKey(bp_public, r);
  if (ret != 1) {
    goto free_all;
  }

  // 3. save private key
  bp_private = BIO_new_file((path + "private.pem").c_str(), "w+");
  ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

  // 4. free
free_all:

  BIO_free_all(bp_public);
  BIO_free_all(bp_private);
  RSA_free(r);
  BN_free(bne);

  return (ret == 1);
}