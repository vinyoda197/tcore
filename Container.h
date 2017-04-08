#pragma once
class CContainer
{
public:
	CContainer(void);
	~CContainer(void);

	typedef map<string, string>				mCData;
	typedef map<string, string>::iterator	ITmCData;

	bool initFromJson(string data);
	void setData(mCData md) { this->m_mdata = md; };
	string Get(string dataItem);
	mCData Get() { return this->m_mdata; };
	void Clear() { this->m_mdata.clear(); };
	bool Empty() { return this->m_mdata.empty(); };

private:
	mCData m_mdata;
};

