#pragma once
class CCurl
{
public:
	CCurl(void);
	~CCurl(void);

	bool init();
	void addHeader(string header);
	void clearHeaders();
	string getInfo(string);
	void addFailCallback(boost::function<void(string)> fc) { this->m_fc = fc; };

	string postFile(string epUrl, string fname);
	string post(string epUrl, string params);
	string get(string epUrl);
	string postData(string epUrl, void * data, long datalen);

	static size_t Canceled(size_t data, bool stat = false);
protected:
	void saveLastRequestInfo(CURL *curl);

	//----- static struct, callbacks used by curl ----------------/
	typedef struct
	{ 
		void *data; 
		long body_size; 
		long body_pos; 
	} postdata;

	typedef struct
	{
		char *memory;
		size_t size;
	} responsedata;

	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

private:
	curl_slist * m_headers;
	map<string, string> m_lastInfo;
	int m_failCount;
	boost::function<void(string)> m_fc;
};

