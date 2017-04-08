#pragma once
#include "curl.h"
#include "Crypto.h"

class CS3 :
			public CCurl,
			public CCrypto
{
public:
	CS3(void);
	~CS3(void);

	void setBucket(string bucket) { this->_bucket = bucket; };
	void setKey(string k) { this->_key = k; };
	void setKSecret(string ks) { this->_keySecret = ks; };
	void setServer(string server) { this->_server = server; };

	string PutFileObject(string fname);

private:
	string mimeType(string fname);
	string prepareFile(string fname);

	string _bucket;
	string _key;
	string _keySecret;
	string _server;
};

