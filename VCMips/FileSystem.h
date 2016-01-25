#ifndef FileSystem_h__
#define FileSystem_h__
#include "CMD5.h"
#pragma once


typedef struct FileInfo_s
{
	TCHAR File[MAX_PATH];
	TCHAR FullPath[MAX_PATH];
   TCHAR RootPath[MAX_PATH];
	MD5Data_t MD5f;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
   BOOL IsDir;
   std::vector<FileInfo_s> SubDir;
}FileInfo_t;

class CFileSystem
{
public:
	CFileSystem();
	~CFileSystem();
	BOOL DirExists(TCHAR *Directory,BOOL *isEmpty=NULL);
	BOOL CompareDirs(TCHAR *Dir1, TCHAR *Dir2);
	BOOL CopyDir(TCHAR *Src, TCHAR *Dst);
   BOOL RemoveDir(TCHAR *Dir,BOOL MakeEmpty=FALSE);
	std::vector<FileInfo_t> GetDirectoryInfo(TCHAR *Directory, std::vector<FileInfo_t> *fi = NULL);
private:

};

#endif // FileSystem_h__
