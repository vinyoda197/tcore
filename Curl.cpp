#include "StdAfx.h"
#include "Curl.h"


CCurl::CCurl(void) :
						m_headers(nullptr),
						m_failCount(0),
						m_fc(NULL)
{
}


CCurl::~CCurl(void)
{
	curl_global_cleanup();
}

bool CCurl::init()
{
	return 0 == curl_global_init(CURL_GLOBAL_DEFAULT);
}

void CCurl::addHeader(string header)
{
	this->m_headers = curl_slist_append(this->m_headers, header.c_str());
}

void CCurl::clearHeaders()
{
	curl_slist_free_all (this->m_headers);
	this->m_headers = nullptr;   // important
}

void CCurl::saveLastRequestInfo(CURL *curl)
{
	long cc;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &cc );
	if(cc != 200)
	{
		//curl failed
		this->m_lastInfo["TotalTime"] = "<unknown>";
	}
	else
	{	
		double totalT;
		curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalT );
		this->m_lastInfo["TotalTime"] = SSTRS(totalT);
	}
}

string CCurl::getInfo(string info)
{
	if(this->m_lastInfo.find(info) != this->m_lastInfo.end())
	{
		return this->m_lastInfo[info];
	}
	return "";
}


string CCurl::postFile(string epUrl, string fname)
{
	CURL *curl;
	CURLcode res;

	responsedata chunk; 
	chunk.memory = (char *)malloc(1);
	chunk.size = 0;

	string result("error : curl failed");
 
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	static const char buf[] = "Expect:";	
 
	/* Fill in the file upload field */ 
	curl_formadd(&formpost,
				&lastptr,
				CURLFORM_COPYNAME, "sendfile",
				CURLFORM_FILE, fname.c_str(),
				CURLFORM_END);
 
	/* Fill in the filename field */ 
	curl_formadd(&formpost,
				&lastptr,
				CURLFORM_COPYNAME, "filename",
				CURLFORM_COPYCONTENTS, fname.c_str(),
				CURLFORM_END); 

	headerlist = curl_slist_append(headerlist, buf);
	 
	curl = curl_easy_init();	
	
	if(curl) {

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		
		curl_easy_setopt(curl, CURLOPT_URL, epUrl.c_str());	
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		
		res = curl_easy_perform(curl);
		
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

		this->saveLastRequestInfo(curl);
 
		/* always cleanup */ 
		curl_easy_cleanup(curl);	
		curl_formfree(formpost);		
		curl_slist_free_all (headerlist);
	}
		
	if(res == CURLE_OK)
	{
		result = string(chunk.memory);
	}
	else
	{
		if(this->m_fc)
			this->m_fc(SSTRS(res));
	}
	
	if(chunk.memory)
	{
		free(chunk.memory);
	}

	return result;
}


string CCurl::post(string epUrl, string params)
{
	CURL *curl;
	CURLcode res;

	responsedata chunk; 
	chunk.memory = (char *)malloc(1);	/* will be grown as needed by the realloc above */ 
	chunk.size = 0;

	string resp("error : curl failed!");

	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, epUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 1);		
		curl_easy_setopt( curl, CURLOPT_POSTFIELDS, params.c_str() );

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);   
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); 

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");		

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));	
		
		if(res == CURLE_OK)
		{
			resp = string(chunk.memory);			
		}

		if(this->m_fc)
			this->m_fc(SSTRS(res));

		this->saveLastRequestInfo(curl);
		
		if(chunk.memory)
			free(chunk.memory);

		curl_easy_cleanup(curl);		
	}

	return resp;
}

string CCurl::get(string epUrl)
{
	CURL *curl;
	CURLcode res;
	string resp("error : curl failed!");

	responsedata chunk; 
	chunk.memory = (char *)malloc(1);
	chunk.size = 0;					

	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, epUrl.c_str());  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); 
  		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		res = curl_easy_perform(curl);

		if(res == CURLE_OK)
		{
			resp = string(chunk.memory);
		}

		if(this->m_fc)
			this->m_fc(SSTRS(res));

		if(chunk.memory)
		{
			free(chunk.memory);
		}
		
		curl_easy_cleanup(curl);  
	}

	return resp;
}


string CCurl::postData(string epUrl, void * data, long datalen)
{
	responsedata chunk; 
	chunk.memory = (char *)malloc(1);
	chunk.size = 0;
	string resp("error : curl failed");

	CURLcode res;

	CURL *curl = curl_easy_init();
	
	if(curl)
	{
		postdata pd;
		pd.data = data;
		pd.body_pos = 0;
		pd.body_size = datalen;

		curl_easy_setopt(curl, CURLOPT_URL, epUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

		if(this->m_headers)
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, this->m_headers);

		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)datalen);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	
		curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&pd);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		curl_easy_setopt(curl, CURLOPT_HEADER, 1);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		res = curl_easy_perform(curl);

		this->saveLastRequestInfo(curl);
		curl_easy_cleanup(curl);
	}

	if(res == CURLE_OK)
	{
		resp = string(chunk.memory);		
	}
	
	if(this->m_fc)
		this->m_fc(SSTRS(res));
		
	if(chunk.memory)
	{
		free(chunk.memory);
	}

	return resp;
}


// ----------------------------------------------------------//
size_t CCurl::Canceled(size_t data, bool stat)
{
	static volatile bool state = false;
	if(state == false && stat == false)
		return data;
	state = stat;
	return CURL_READFUNC_ABORT;
}


size_t 
CCurl::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if (stream)
    { 
        postdata *ud = (postdata*) stream; 

        int available = (ud->body_size - ud->body_pos);

        if (available > 0)
        { 
            int written = min<int>(size * nmemb, available);
            memcpy(ptr, ((char*)(ud->data)) + ud->body_pos, written); 
            ud->body_pos += written;
			return CCurl::Canceled(written); 
        } 
    }

    return 0; 
}

 
size_t
CCurl::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  responsedata *mem = (responsedata *)userp;
 
  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  } 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
///////////////////////////////////////////////////////////////
