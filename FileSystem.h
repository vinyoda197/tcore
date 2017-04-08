#pragma once
class CFileSystem
{
public:
	CFileSystem(void);
	~CFileSystem(void);

	typedef vector<string>				FileList;
	typedef vector<string>::iterator	ITFileList;
	typedef vector<unsigned char>		FileContent;	

	static void setCurrentDirectory(string);
	static string currentFolder();
	static void createFolder(string folder);
	static FileList getFolderFiles(string folder, string filterExt="");

	static bool file_exists(string filename);

	static FileContent readFile(string fname);
	static bool writeFile(string fname, string data);
};

