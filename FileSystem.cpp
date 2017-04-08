#include "StdAfx.h"
#include "FileSystem.h"

namespace fs = boost::filesystem;

CFileSystem::CFileSystem(void)
{
}


CFileSystem::~CFileSystem(void)
{
}


bool CFileSystem::file_exists(string filename)
{
	fs::path p(filename);
	return fs::exists(p);
}


string CFileSystem::currentFolder()
{
	fs::path fp = fs::current_path();
	return fp.generic_string();
}


void CFileSystem::createFolder(string folder)
{
	try
	{
		fs::path someDir(folder);
		if ( !fs::exists(someDir) )
		{
			fs::create_directories(someDir);
		}
	}
	catch(...)
	{
		CApp::Engine()->debug_(string("Failed creating folder : ") + folder);
	}
}


CFileSystem::FileList CFileSystem::getFolderFiles(string folder, string filterExt)
{
	fs::path someDir(folder);
	fs::directory_iterator end_iter;
	
	FileList vs;

	if ( fs::exists(someDir) && fs::is_directory(someDir))
	{
	  for( fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
	  {
		if (fs::is_regular_file(dir_iter->status()) )
		{	
			string fPath = dir_iter->path().generic_string();
			if(filterExt != "")
			{
				if(fPath.find(filterExt) != fPath.npos)
				{
					vs.push_back(fPath);
				}
			}
			else
			{
				vs.push_back(fPath);
			}
		}
	  }
	}
	return vs;
}


CFileSystem::FileContent CFileSystem::readFile(string fname)
{
	ifstream ifs(fname, ios::binary);
	if(ifs.is_open())
	{
		vector<unsigned char> content( (std::istreambuf_iterator<char>(ifs) ),
										 (std::istreambuf_iterator<char>()    ) );
		ifs.close();
		return content;
	}
	return vector<unsigned char>();
}


bool CFileSystem::writeFile(string fname, string data)
{	/*
	ofstream file(fname);
	if(file.is_open())
	{
		file << data;
		file.close();
		return true;
	}
	return false;
	*/
	FILE *f = fopen(fname.c_str(), "wb");
	if (f == NULL)
	{
		return false;
	}
	
	char* cstr = new char[data.size()+1];
	memcpy ( cstr, data.data(), data.size() );	
	fwrite ((char *)data.c_str() , sizeof(char), data.size(), f);
	delete(cstr);
	cstr = NULL;
	fclose(f);
	return true;
}
