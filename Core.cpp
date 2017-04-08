#include "StdAfx.h"
#include "Core.h"


CCore::CCore(void) :
						_running(false),
						_enable_debug(false)
{	
	this->m_debugCallback = NULL;
}


CCore::~CCore(void)
{
}

void CCore::Start()
{
	this->m_pWork.reset(new boost::asio::io_service::work(this->m_io));
	this->_m_main = boost::thread(boost::bind(&CCore::_run, this));
	this->_running = !(bool)this->_m_main.timed_join(boost::posix_time::millisec(0));
}

bool CCore::ReStart()
{
	// let us check if main thread did indeed finish or interrupted
	if(this->_m_main.timed_join(boost::posix_time::millisec(0)))
	{
		this->Start();
		return true;
	}
	return false;
}

void CCore::Shutdown()
{
	this->m_pWork.reset();
	//this->m_io.stop();
	this->m_io.post(boost::bind(&boost::asio::io_service::stop, &this->m_io)); // can we possibly let the io_service kill itself??
	this->_m_main.timed_join(boost::posix_time::seconds(30));
	this->_debug("main service thread joined");
}

void CCore::addDebugHandler(boost::function<void(string)> f)
{
	this->m_debugCallback = f;
}

void CCore::debug_(string message)
{
	if( ! this->_enable_debug)
		return;
	try
	{
		string s = boost::lexical_cast<std::string>(boost::this_thread::get_id());
		s += " : ";
		s += message;
		this->m_io.post(boost::bind(&CCore::_debug, this, s));
	}
	catch(...)
	{

	}
}

void CCore::_debug(string message)
{
	cout << message << "\n";
	if(this->m_debugCallback != NULL)
	{
		(this->m_debugCallback)(message);
	}
}

void CCore::_run()
{
	try
	{
		this->debug_("Main service Thread");
		this->m_io.run();
		this->_debug("io service ended");
	}
	catch(...)
	{
		this->_debug("error in io_service run.");
	}
}