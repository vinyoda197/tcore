#include "StdAfx.h"
#include "S3.h"


CS3::CS3(void)
{
}


CS3::~CS3(void)
{
}


string CS3::PutFileObject(string fname)
{
	// read file
	ifstream ifs(fname, ios::binary);
	vector<unsigned char> content( (std::istreambuf_iterator<char>(ifs) ),
						   (std::istreambuf_iterator<char>()    ) );
		
	// prepare headers
	string mime = this->mimeType(fname);

	this->addHeader(string("Content-Type: ") + mime);

	// setup time for use in signature
	char canonData[100]; 
	time_t now = time(NULL);		
	tm gmtm;
	gmtime_s(&gmtm, &now);
	strftime(canonData, 100, "%a, %d %b %Y %H:%M:%S GMT", &gmtm); 
		
	this->addHeader(string("Date: ") + canonData);

	// make public
	this->addHeader("x-amz-acl: public-read");

	string stringToSign("PUT\n\n");	
	stringToSign += mime + "\n";
	stringToSign += canonData;
	stringToSign += "\n";
	stringToSign += "x-amz-acl:public-read\n";
	stringToSign += this->_bucket;

	string fn = this->prepareFile(fname);		
	stringToSign += fn;
	
	char auth[160/8];
	this->ComputeHMAC(stringToSign.c_str(), this->_keySecret, auth);
	string b64 = this->__b64_encode((unsigned char const*) auth, sizeof(auth));
	
	string strSig	= "Authorization: AWS ";
	strSig	+= this->_key;
	strSig	+= ":";
	strSig	+= b64;

	this->addHeader(strSig.c_str());

	string fEP = this->_server + this->_bucket + fn;

	string response = this->postData(fEP, reinterpret_cast<void*> (&content[0]), content.size());

	this->clearHeaders();
	ifs.close();

	return response;
}


string CS3::mimeType(string fname)
{
	size_t pos = fname.rfind(".");
	if(pos != fname.npos)
	{
		string ext = fname.substr(pos);
		if(ext == "png") return "image/png";
		if(ext == "jpg") return "image/jpeg";
	}
	return "unknown";
}


string CS3::prepareFile(string fname)
{
	size_t pos = fname.rfind("/");
	if(pos != fname.npos)
	{
		fname = fname.substr(pos);
		fname.replace(fname.find("_"), 1, "/");
		fname.replace(fname.find("_"), 1, "/");
	}
	return fname;
}
