#pragma once
/*
*	This is the dispatcher of the main io_service thread. It will dispatch the io_service into a 
*	separate thread. All events, callbacks will then be dispatched through this io_service run() to ensure
*	that all events and callbacks are handled within this same thread in an asynchronous manner.
*/
using namespace std;
class CCore
{
public:
	CCore(void);
	~CCore(void);

	void Start();
	bool ReStart();
	void Shutdown();
	void debug_(string);
	void addDebugHandler(boost::function<void(string)> f);
	bool running() { return this->_running; };
	void enable_debug(bool d=true) { this->_enable_debug = d; };

protected:
	string _ts(int x) { return boost::lexical_cast<string>( x ); };
	boost::asio::io_service m_io;

private:
	void _run();
	void _debug(string);	

	bool _running;
	bool _enable_debug;
	
	boost::shared_ptr<boost::asio::io_service::work> m_pWork;
	boost::thread _m_main;
	boost::function<void(string)> m_debugCallback;
};

