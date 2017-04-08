#include "StdAfx.h"
#include "Dispatcher.h"


CDispatcher::CDispatcher(int ithread) : 
										_m_imaxThread(ithread)
{
}

CDispatcher::~CDispatcher(void)
{
}

void CDispatcher::post(Callback fcallback)
{
	this->m_io.post(fcallback);
}

void CDispatcher::dispatch(Callback fcallback)
{
	this->m_io.dispatch(fcallback);	
}

void CDispatcher::startDispatcher()
{
	// add work to io service
	this->_m_pWork.reset(new boost::asio::io_service::work(this->_m_io));

	for(int i = _m_imaxThread; i > 0; --i)
	{
		//this->_m_g.create_thread(boost::bind(&boost::asio::io_service::run, &this->_m_io));
		this->_m_vTh.push_back(boost::thread(boost::bind(&boost::asio::io_service::run, &this->_m_io)));
	}

}

void CDispatcher::dispatch_1(boost::function<string(void)> f, boost::function<void(string)> fcallback)
{	
	this->_m_g.create_thread(boost::bind(&CDispatcher::_dispatch_t1, this, f, fcallback));
}


void CDispatcher::dispatch_p1(boost::function<string(void)> f, boost::function<void(string)> fcallback)
{
	this->_m_io.post(boost::bind(&CDispatcher::_dispatch_t1, this, f, fcallback));
}
 
void CDispatcher::_postedStop()
{
	this->post(boost::bind(&boost::asio::io_service::stop, &this->_m_io));
}

void CDispatcher::Shutdown()
{	
	this->_m_pWork.reset();
	this->_postedStop();
	this->debug_("wait for thread group to join all");	
	this->_m_g.join_all();

	CDispatcher::ITVTHREAD it = this->_m_vTh.begin();
	while(it != this->_m_vTh.end())
	{
		//it->timed_join(boost::posix_time::seconds(10));
		it->join();
		++it;
	}

	this->debug_("thread group joined all");
	CCore::Shutdown();
}

void CDispatcher::_dispatch_t1(boost::function<string(void)> f, 
								boost::function<void(string)> fcallback)								
{
	try
	{	
		this->debug_( string("Thread Started. Num Thread : ") + _ts(this->numThreads()));
		string fs = f();
		this->m_io.post(boost::bind(fcallback, fs));
		this->debug_("Thread Ended.");
	}
	catch(...)
	{
		this->debug_("Error in running thread.");
	}
}


