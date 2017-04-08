#include "StdAfx.h"
#include "Crypto.h"

#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#ifdef USEPTREE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;
#define BOOST_SPIRIT_THREADSAFE
#endif

#ifdef USERAPIDJS
#include "rapidjson/document.h"	
#include "rapidjson/writer.h"	
#include "rapidjson/prettywriter.h"	// for stringify JSON
#include "rapidjson/filestream.h"	// wrapper of C stream for prettywriter as output
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
#endif


#ifdef USEJSONSPIRIT
#include "json_spirit.h"
using namespace json_spirit;
#endif


EVP_CIPHER_CTX CCrypto::en_ctx;
EVP_CIPHER_CTX CCrypto::de_ctx;


///////////////////////////////////////
//
//  initialize Encryption and decryption context
//  using OpenSSL
//
int INITIALIZEAES(
	unsigned char *KeyData, 
	int KeyLength, 
	unsigned char *Salt, 
	EVP_CIPHER_CTX *en_ctx,                                                       
	EVP_CIPHER_CTX *de_ctx)
{
	int rc;
	int Rounds = 5;
	unsigned char aes_key[32], aes_iv[32];   

	///////////////////////////////////////////
	//
	//
	// Cipher Block chaining mode , SHA1 
	rc = EVP_BytesToKey(
		EVP_aes_256_cbc(),  // Cryptographic mode
		EVP_sha1(),         // SHA1 
		Salt,               // a fuzzifier
		KeyData,            
		KeyLength,
		Rounds,             // more rounds  
		aes_key, aes_iv);   // return buffers

	EVP_CIPHER_CTX_init(en_ctx);
	EVP_EncryptInit_ex(en_ctx, 
		EVP_aes_256_cbc(), 
		NULL, 
		aes_key, 
		aes_iv);   

	EVP_CIPHER_CTX_init(de_ctx);
	EVP_DecryptInit_ex(de_ctx, 
		EVP_aes_256_cbc(), 
		NULL, aes_key, aes_iv);

	return 0;

}

#define AES_BLOCK_SIZE 16

unsigned char *Encrypt(
	EVP_CIPHER_CTX *en_ctx, 
	unsigned char *msg, 
	int *msglen)
{
	int buffer_length = *msglen + AES_BLOCK_SIZE;
	int final_length = 0;
	unsigned char *ciphermsg = (unsigned char *)malloc(buffer_length );

	EVP_EncryptInit_ex(en_ctx, NULL, NULL, NULL, NULL);
	EVP_EncryptUpdate(en_ctx, ciphermsg, &buffer_length, msg, *msglen);
	EVP_EncryptFinal_ex(en_ctx, ciphermsg+buffer_length, &final_length);
	*msglen = buffer_length + final_length;
	return ciphermsg;
}

unsigned char *DeCrypt(EVP_CIPHER_CTX *de_ctx,
	unsigned char *ciphertext, 
	int *len)
{
	int p_len = *len, f_len = 0;
	unsigned char *plaintext =(unsigned char *)malloc(p_len);

	EVP_DecryptInit_ex(de_ctx, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(de_ctx, plaintext, &p_len, ciphertext, *len);        
	EVP_DecryptFinal_ex(de_ctx, plaintext+p_len, &f_len);
	*len = p_len + f_len;
	return plaintext;
}



CCrypto::CCrypto(void)
{
}


CCrypto::~CCrypto(void)
{
}


bool CCrypto::initAES()
{
	unsigned int Salt[] = {12345, 54321};
	unsigned char *KeyData;
	int KeyDataLen;

	KeyData = (unsigned char *)SALTNPEPPA;
	KeyDataLen = strlen((const char*)KeyData) + 1;

	if (INITIALIZEAES(KeyData, KeyDataLen, 
		(unsigned char *)&Salt, &(CCrypto::en_ctx), &(CCrypto::de_ctx))) 
	{
		return false;
	}
	else
	{
		return true;
	}
}


string CCrypto::aes_encrypt(string str)
{
	char *msgtoencrypt = new char[str.size()];
	str.copy(msgtoencrypt, str.size());

	unsigned char *ciphermsg;
	int inlen;
	inlen = strlen(msgtoencrypt)+1;

	ciphermsg = Encrypt(&(CCrypto::en_ctx), 
		(unsigned char *)msgtoencrypt, &inlen);	

	string sret;
	sret.assign((char *)ciphermsg, inlen);
	free(ciphermsg);
	return sret;
}


string CCrypto::aes_decrypt(string str)
{
	char *msgtodecrypt = new char[str.size()];
	str.copy(msgtodecrypt, str.size());

	char *msg;
	int outlen;
	outlen = str.size();
	msg = (char *)DeCrypt(&(CCrypto::de_ctx), (unsigned char*)msgtodecrypt , &outlen);
	string sRet = string(msg);
	free(msg);
	return sRet;
}


string CCrypto::aes_encryptData(string str)
{
	string sub;
	string sum = "";
	while(str.size())
	{
		sub = str.substr(0,16);
		sum += CCrypto::aes_encrypt(sub);
		str.erase(0, 16);
	}
	return sum;
}


string CCrypto::aes_decryptData(string str)
{
	string sub;
	string sum = "";
	while(str.size())
	{
		sub = str.substr(0,32);
		sum += CCrypto::aes_decrypt(sub);
		str.erase(0, 32);
	}
	return sum;
}


void CCrypto::ComputeHMAC(const char* data, string key, char* result)
{
	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);

	// Using sha1 hash engine here.
	HMAC_Init_ex(&ctx, key.c_str(), key.size() , EVP_sha1(), NULL);
	HMAC_Update(&ctx, (unsigned char*)data, strlen(data));

	unsigned len;// = 20;
	HMAC_Final(&ctx, (unsigned char*)result, &len);
	HMAC_CTX_cleanup(&ctx);  
}


string CCrypto::__b64_encode(const unsigned char *input, int length)
{
  BIO *bmem, *b64;
  BUF_MEM *bptr;
  char * buff = NULL;

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, input, length);
  if(BIO_flush(b64)) ; /* make gcc 4.1.2 happy */
  BIO_get_mem_ptr(b64, &bptr);

  buff = (char *)malloc(bptr->length);
  memcpy(buff, bptr->data, bptr->length-1);
  buff[bptr->length-1] = 0;

  BIO_free_all(b64);

  return buff;
}


CCrypto::assocA CCrypto::json_read(string sJson)
{
#ifdef USEJSONSPIRIT
	return CCrypto::json_readJS(sJson);
#endif
#ifdef USEPTREE
	return CCrypto::json_readPT(sJson);
#endif
#ifdef USERAPIDJS
	return CCrypto::json_readRJ(sJson);
#endif	
}


CCrypto::assocAA CCrypto::json_read2(string sJson)
{
	assocAA r;
	Value value;
		
	try
	{
		read_or_throw(sJson, value);
		Object obj = value.get_obj();

		for( Object::size_type i = 0; i != obj.size(); ++i )
		{
			const Pair& pair = obj[i];

			const string& name  = pair.name_;
			const Value&  val = pair.value_;
			if(val.type() == obj_type)
			{
				assocA r2;
				Object obj2 = val.get_obj();
				for(Object::size_type j = 0; j != obj2.size(); ++j)
				{
					const Pair& pair2 = obj2[j];
					const string& name2  = pair2.name_;
					const Value&  val2 = pair2.value_;
					r2[name2] = val2.get_str();
				}
				r[name] = r2;
			}			
		}		
	}
	catch(...)
	{
		CApp::Engine()->debug_(string("Error reading json : ") + sJson );
	}
	
	return r;
}


string CCrypto::json_write(assocA mArray)
{
#ifdef USEJSONSPIRIT
	return CCrypto::json_writeJS(mArray);
#endif
#ifdef USEPTREE
	return CCrypto::json_writePT(mArray);
#endif
#ifdef USERAPIDJS
	return CCrypto::json_writeRJ(mArray);
#endif
}

char CCrypto::from_hex(char ch)
{
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}


char CCrypto::to_hex(char code)
{
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}


string CCrypto::url_encode(char *str)
{
	if(strlen(str) == 0)
		return "";
	char *pstr = str;
	char *buf = (char *)malloc(strlen(str) * 3 + 1);
	char *pbuf = buf;

	while (*pstr)
	{
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
			*pbuf++ = *pstr;
		else if (*pstr == ' ') 
				*pbuf++ = '+';
			else 
				*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	string r(buf);
	free(buf);
	buf = NULL;
	return r;
}


string CCrypto::url_decode(char *str)
{
	char *pstr = str;
	char *buf = (char *)malloc(strlen(str) + 1);
	char *pbuf = buf;
	while (*pstr)
	{
		if (*pstr == '%')
		{
			if (pstr[1] && pstr[2])
			{
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} 
		else if (*pstr == '+')
				{ 
					*pbuf++ = ' ';
				} 
				else
				{
					*pbuf++ = *pstr;
				}
		pstr++;
	}
	*pbuf = '\0';
	string r(buf);
	free(buf);
	buf = NULL;
	return r;
}


string CCrypto::mapToPostVars( assocA params )
{
	string str = "";
	ITassocA it;
	for(it = params.begin(); it != params.end(); it++)
	{		
		str += it->first;
		str += "=";
		str += it->second;
		str += "&";
	}
	str.pop_back();	
	return str;
}


string CCrypto::extractTail(string& sbase, string selector)
{
	size_t pos = sbase.rfind(selector);
	if(pos != sbase.npos)
	{
		string sret = sbase.substr(pos);
		sbase.erase(pos);
		return sret;
	}
	return "";
}


// -------------------------------------------  JSON PARSING --
#ifdef USEJSONSPIRIT
CCrypto::assocA CCrypto::json_readJS(string sJson)
{
	assocA r;
	Value value;
		
	try
	{
		read_or_throw(sJson, value);
		Object obj = value.get_obj();

		for( Object::size_type i = 0; i != obj.size(); ++i )
		{
			const Pair& pair = obj[i];

			const string& name  = pair.name_;
			const Value&  val = pair.value_;
			if(val.type() == obj_type)
			{
				string v = write(val,0);
				r[name] = v;
				//r.insert(make_pair(name, v));
			}
			else
			{
				r.insert(make_pair(name, val.get_str()));
			}
		}		
	}
	catch(...)
	{
		CApp::Engine()->debug_(string("Error reading json : ") + sJson );
	}
	
	return r;
}


string CCrypto::json_writeJS(assocA mArray)
{
	Object addr_obj;
	try
	{
		for(ITassocA it = mArray.begin(); it != mArray.end(); it++)
		{
			addr_obj.push_back( Pair( it->first, it->second ) );
		}
	}
	catch(...)
	{
		CApp::Engine()->debug_("Error writing json.");
	}

	return write(addr_obj, 0);
}
#endif

#ifdef USEPTREE
CCrypto::assocA CCrypto::json_readPT(string sJson)
{
	assocA r;
	ptree pt, pti;
	ptree::iterator ptIT;

    stringstream ss; 
	ss << sJson;

	try
	{
		read_json(ss, pt);
		for(ptIT = pt.begin(); ptIT != pt.end(); ptIT++)
		{		
			string sec = pt.get<string>(ptIT->first.c_str());
			if(sec=="")
			{	
				ptree ptd = pt.get_child(ptIT->first.c_str());
				std::stringstream ss;
				write_json(ss, ptd);
				sec = ss.str();			
			}
			r.insert(make_pair(ptIT->first, sec));
		}
	}
	catch(...)
	{
		
	}

	return r;
}



string CCrypto::json_writePT(assocA mArray)
{
	ptree ptd;
	std::stringstream ss;

	try
	{
		ITassocA it = mArray.begin();
		for(it = mArray.begin(); it != mArray.end(); it++)
		{
			ptd.put(it->first, it->second);
		}
		write_json(ss, ptd);
	}
	catch(...)
	{
		
	}	
	
	return ss.str();	
}
#endif
#ifdef USERAPIDJS
CCrypto::assocA CCrypto::json_readRJ(string sJson)
{
	Document document;
	size_t s = sJson.size();
	char * buffer = new char[s];
	memcpy(buffer, sJson.c_str(), sJson.size());
	buffer[sJson.size()] = '\0';
	if (document.ParseInsitu<0>(buffer).HasParseError())
	{
		delete[] buffer; buffer=NULL;
		return assocA();
	}
	delete[] buffer; 
	buffer=NULL;
	assocA aas;
	StringBuffer f;
	Writer<StringBuffer> writer(f);		
	Value::MemberIterator mv = document.MemberBegin();
	while(mv != document.MemberEnd())
	{
		string n = mv->name.GetString();
		string v;
		if(mv->value.IsObject())
		{				
			mv->value.Accept(writer);
			v = f.GetString();
			f.Clear();
		}
		else
			v = mv->value.GetString();

		aas[n] = v;
		 ++mv;
	}
	return aas;
}


string CCrypto::json_writeRJ(assocA mArray)
{

	return string();
}
#endif