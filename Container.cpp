#include "StdAfx.h"
#include "Container.h"
#include "Crypto.h"


CContainer::CContainer(void)
{
}


CContainer::~CContainer(void)
{
}


bool CContainer::initFromJson(string data)
{
	try
	{
		this->m_mdata = CCrypto::json_read(data);
		return true;
	}
	catch(...)
	{
		return false;
	}
}


string CContainer::Get(string dataItem)
{
	ITmCData it = this->m_mdata.find(dataItem);
	if(it != this->m_mdata.end())
	{
		return it->second;
	}
	return "";
}
