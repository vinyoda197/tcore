#pragma once
/*
*	This dispatcher layer handles the suppose spawning of separate worker threads, threads such as for 
*	curl calls, etc. It also leverages the asio io_service capability of using a thread pool. A process
*	will be dispatched through this class io_service running on the thread pool, and results will be posted 
*	or dispatched back to the parent core io_service, again ensuring that all data handling and logic be done 
*	on a single core thread.
*/
#include "core.h"
class CDispatcher :
	public CCore
{
public:
	CDispatcher(int ithread = 3);
	~CDispatcher(void);

	typedef boost::function<void()> Callback;
	typedef boost::function<void(string)> VSCallback;
	typedef boost::function<string(void)> SVCallback;

	typedef vector<boost::thread>	VTHREAD;
	typedef vector<boost::thread>::iterator ITVTHREAD; 
	
	void dispatch(Callback fcallback);
	void post(Callback fcallback);

	void dispatch_1(SVCallback f, VSCallback fcallback);
	void dispatch_p1(SVCallback f, VSCallback fcallback);	

	void startDispatcher();

	int numThreads() { return this->_m_g.size(); };
	void Shutdown();

private:
	void _dispatch_t(Callback fcallback);

	void _dispatch_t1(SVCallback f, 
							VSCallback fcallback);
	void _postedStop();

	VTHREAD _m_vTh;

	int _m_imaxThread;
	boost::thread_group _m_g;
	boost::asio::io_service _m_io;
	boost::shared_ptr<boost::asio::io_service::work> _m_pWork;
};

