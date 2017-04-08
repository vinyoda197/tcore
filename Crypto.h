#pragma once
#include "MD5.h"

#include <openssl\evp.h>

#define	USEJSONSPIRIT
//#define USEPTREE
//#define USERAPIDJS

class CCrypto
{
public:
	CCrypto(void);
	~CCrypto(void);

	typedef map<string, string> assocA;
	typedef map<string, assocA> assocAA;
	typedef map<string, string>::iterator ITassocA;
	typedef map<string, assocA>::iterator ITassocAA;

	string	__b64_encode(const unsigned char *input, int length);
	void	ComputeHMAC(const char* data, string key, char* result); // for S3
	static string	mapToPostVars( assocA params );

	//we'll just include json here also	

	static assocA json_read(string);
	static assocAA json_read2(string);

	static string json_write(assocA);

	// and aes of course
	static bool initAES();
	static string aes_encrypt(string str);
	static string aes_encryptData(string str);
	static string aes_decrypt(string str);
	static string aes_decryptData(string str);
	static EVP_CIPHER_CTX en_ctx;
	static EVP_CIPHER_CTX de_ctx;

#ifdef USEJSONSPIRIT
	static assocA json_readJS(string);
	static string json_writeJS(assocA);
#endif
#ifdef USEPTREE
	static assocA json_readPT(string);
	static string json_writePT(assocA);
#endif
#ifdef USERAPIDJS
	static assocA json_readRJ(string);
	static string json_writeRJ(assocA);
#endif

	static string url_encode(char *str);
	static string url_decode(char *str);

	static char to_hex(char code);
	static char from_hex(char ch);

	static string extractTail(string& sbase, string selector);

};

