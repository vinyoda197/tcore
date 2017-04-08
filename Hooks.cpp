#include "StdAfx.h"
#include "Hooks.h"

bool CHooks::m_mstate = false;
bool CHooks::m_kstate = false;
boost::mutex CHooks::m_mmutex;
boost::mutex CHooks::m_kmutex;

CHooks::CHooks(void) :
						mousehook(NULL),
						keyboardhook(NULL)						
{
}


CHooks::~CHooks(void)
{
}


void CHooks::Set()
{
	//this->mousehook = SetWindowsHookEx(WH_MOUSE_LL, CHooks::MouseHookProc, GetModuleHandle(NULL), 0);
	this->keyboardhook = SetWindowsHookEx(WH_KEYBOARD_LL, CHooks::KeyboardHookProc, GetModuleHandle(NULL), 0);
}


void CHooks::Unset()
{
	if(this->mousehook != NULL)
	{
		UnhookWindowsHookEx(this->mousehook);
	}

	if(this->keyboardhook != NULL)
	{
		UnhookWindowsHookEx(this->keyboardhook);
	}
}


bool CHooks::checkOutState()
{
	bool ret = false;
	static long lastX = 0;
	static long lastY = 0;

	POINT p;
	if (GetCursorPos(&p))
	{
		if(p.x != lastX || p.y != lastY)
		{
			ret = true;
			lastX = p.x;
			lastY = p.y;
		}
	}

	if(ret) 
		return ret;	

	ret = CHooks::m_kstate;
	CHooks::m_kstate = false;
	
	return ret;
}


LRESULT CALLBACK CHooks::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0) 
  {
    if (wParam == WM_RBUTTONDOWN 		
		|| wParam == WM_LBUTTONDOWN		
		|| wParam == WM_MOUSEMOVE
		|| wParam == WM_MOUSEWHEEL)
	{			
		CHooks::m_mstate = true; 
	}   
  }
  return CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CALLBACK CHooks::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0)
  {
    if (wParam == WM_KEYUP)
	{		
		CHooks::m_kstate = true;
	}   
  }
  return CallNextHookEx(0, nCode, wParam, lParam);
}
