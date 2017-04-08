#pragma once
#include <Gdiplus.h>
#include <GdiPlusImageCodec.h>

using namespace Gdiplus;

class CCapture
{
public:
	CCapture(void);
	~CCapture(void);

	void getScreenshot(string fname);
	void getSystemProcesses(string fname, string status, string debug = "" );
	string getFormattedList();
	void createThumbs(string fname, string _tm = "_tm", string _prev = "_prev");
	string getScreenshotWin(string fname);

private:
	
	void _getScreenshotWinAll(string fname);
	int _SaveImage(Bitmap *bm, string sFilename, int maxWidth = 0);
	int _SaveImageW(HBITMAP hBmp, string sFilename, int maxWidth = 0);
	int _GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	Bitmap* _ResizeClone(Bitmap *bmp, INT width, INT height);

	vector<string> getProcesses();
	vector<string> getWindows();
	string getWindowsList();
	string fileMinusX(string&);

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData );

	int _standardWidth;
	ULONG_PTR _gdiplusToken;

	static map<unsigned long, string>	mProcessIdName;
	static map<string, string>			mWNamePName;
};

