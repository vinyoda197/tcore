#pragma once
class CHooks
{
public:
	CHooks(void);
	~CHooks(void);
	void Set();
	void Unset();
	bool checkOutState();

	static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
	HHOOK mousehook;
	HHOOK keyboardhook;

	static bool m_mstate;
	static bool m_kstate;

	static boost::mutex m_mmutex;
	static boost::mutex m_kmutex;
};

