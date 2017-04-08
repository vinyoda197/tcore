#include "StdAfx.h"
#include "Engine.h"


CEngine::CEngine(void)
{
}


CEngine::~CEngine(void)
{
}

void CEngine::StartEngine()
{
	this->Start();
}

void CEngine::PauseEngine()
{

}

void CEngine::Shutdown()
{
	CTimer::calcelAllTimer();  
	CDispatcher::Shutdown();	// shutdown of dispatcher will join all threads
}
