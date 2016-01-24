#include "stdafx.h"
#include "FileSystem.h"



CFileSystem::CFileSystem()
{
}


CFileSystem::~CFileSystem()
{
}

BOOL CFileSystem::DirExists(TCHAR *Directory,BOOL *isEmpty)
{
	if (!Directory)
	{
		return FALSE;
	}
	if (!PathIsDirectory(Directory))
	{
		return FALSE;
	}
	if (isEmpty)
	{
		*isEmpty = PathIsDirectoryEmpty(Directory);
	}
	return TRUE;
}

std::vector<FileInfo_t> CFileSystem::GetDirectoryInfo(TCHAR *Directory, std::vector<FileInfo_t> *fi)
{
	std::vector<FileInfo_t> fiBase;
	if (!Directory || !DirExists(Directory))
	{
		return fiBase;
	}

	if (!fi)
	{
		fi = &fiBase;
	}
	TCHAR SearchDir[MAX_PATH];
	PathCombine(SearchDir, Directory, _T("*"));
	WIN32_FIND_DATA f;
	
	HANDLE h = FindFirstFile(SearchDir, &f);
	FileInfo_t Info;
	do
	{
		if (
			!_tcscmp(f.cFileName, _T("."))
			|| !_tcscmp(f.cFileName, _T(".."))
			)
		{
			continue;
		}


		if (f.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			PathCombine(SearchDir, Directory, f.cFileName);
			GetDirectoryInfo(SearchDir, fi);
		}
		else
		{
			memset(&Info, 0, sizeof(Info));

			_tcscpy_s(Info.File, f.cFileName);
			PathCombine(Info.FullPath, Directory, f.cFileName);
			Info.MD5f = CMD5(reinterpret_cast<FileName_t*>(Info.FullPath)).Get();
			Info.ftCreationTime = f.ftCreationTime;
			Info.ftLastAccessTime = f.ftLastAccessTime;
			Info.ftLastWriteTime = f.ftLastWriteTime;
			fi->push_back(Info);
		}
	}
	while (FindNextFile(h, &f));

	return fiBase;
}

BOOL CFileSystem::CompareDirs(TCHAR *Dir1, TCHAR *Dir2)
{
	if (!Dir1 || !Dir2)
	{
		return FALSE;
	}
	auto fi1 = GetDirectoryInfo(Dir1);
	auto fi2 = GetDirectoryInfo(Dir2);
	BOOL Valid = TRUE;
	size_t Len = 0;
	if (fi1.size() == fi2.size())
	{
		Len = fi1.size();
		for (size_t i = 0; i < Len; i++)
		{
			if (_tcscmp(fi1[i].File, fi2[i].File))
			{
				Valid = FALSE;
				break;
			}
			if (!CMD5(fi1[i].MD5f).Compare(fi2[i].MD5f))
			{
				Valid = FALSE;
				break;
			}

		}	
	}
	else
	{
		Valid = FALSE;
	}
	fi1.empty();
	fi2.empty();
	return Valid;
}
