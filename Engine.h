#pragma once
#include "timer.h"
class CEngine :
	public CTimer
{
public:
	CEngine(void);
	~CEngine(void);

	void StartEngine();
	void PauseEngine();
	void Shutdown();



};

