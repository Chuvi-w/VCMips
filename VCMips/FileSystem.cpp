#include "stdafx.h"
#include "FileSystem.h"



CFileSystem::CFileSystem()
{
}


CFileSystem::~CFileSystem()
{
}

BOOL CFileSystem::DirExists(const TCHAR *Directory,BOOL *isEmpty/*=NULL*/)
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

BOOL CFileSystem::RemoveDir(const TCHAR *Dir,BOOL MakeEmpty/*=FALSE*/)
{
   BOOL IsEmpty=FALSE;
   if(!DirExists(Dir,&IsEmpty))
   {
      return TRUE;
   }
   if(!IsEmpty)
   {
      auto di=GetDirectoryInfo(Dir);
      for(auto a=di.begin();a<di.end();++a)
      {
         if((*a).IsDir)
         {
            RemoveDir((*a).FullPath);
         }
         else
         {
            DeleteFile((*a).FullPath);
         }
         
      }
      di.clear();
   }
   if(MakeEmpty)
   {
      return TRUE;
   }
   
   //return RemoveDirectory(Dir);
   return _trmdir(Dir);
}

BOOL CFileSystem::PathContainsSeparator(TCHAR *Path,size_t *LastSeparatorPos/*=NULL*/)
{
   if(!Path||!Path[0])
   {
      return FALSE;
   }

   for(auto c=&Path[_tcslen(Path)];c>=Path;--c)
   {
      if(*c==_T('\\')||*c==_T('/'))
      {
         if(LastSeparatorPos)
         {
            *LastSeparatorPos=(c-Path);
         }
         return TRUE;
      }
   }
return FALSE;
}


std::vector<FileInfo_t> CFileSystem::GetDirectoryInfo(const TCHAR *Directory, std::vector<FileInfo_t> *fi /*= NULL*/)
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

      memset(&Info, 0, sizeof(Info));
      _tcscpy_s(Info.File, f.cFileName);
      PathCombine(Info.FullPath, Directory, f.cFileName);
      _tcscpy_s(Info.RootPath,Directory);

		if (f.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			PathCombine(SearchDir, Directory, f.cFileName);
         _tcscpy_s(Info.File, f.cFileName);
         PathCombine(Info.FullPath, Directory, f.cFileName);
         Info.IsDir=TRUE;
			//GetDirectoryInfo(SearchDir, fi);
         fi->push_back(Info);
         GetDirectoryInfo(SearchDir, &fi->back().SubDir);
        
		}
		else
		{
         Info.IsDir=FALSE;
			Info.MD5f = CMD5(reinterpret_cast<FileName_t*>(Info.FullPath)).Get();
			Info.ftCreationTime = f.ftCreationTime;
			Info.ftLastAccessTime = f.ftLastAccessTime;
			Info.ftLastWriteTime = f.ftLastWriteTime;
			fi->push_back(Info);
		}
	}
	while (FindNextFile(h, &f));
   FindClose(h);
	return fiBase;
}

BOOL CFileSystem::CompareDirs(const TCHAR *Dir1, const TCHAR *Dir2)
{
	if (!Dir1 || !Dir2)
	{
		return FALSE;
	}
	auto fi1 = GetDirectoryInfo(Dir1);
	auto fi2 = GetDirectoryInfo(Dir2);
	BOOL Valid = TRUE;
   
	if (fi1.size() == fi2.size())
	{
		for (auto a = fi1.begin(), b=fi2.begin(); a<fi1.end();++a,++b)
		{
         if (_tcscmp((*a).File, (*b).File))
         {
            Valid = FALSE;
            break;
         }

         if((*a).IsDir)
         {
            if((*b).IsDir)
            {
               if(!CompareDirs((*a).FullPath,(*b).FullPath))
               {
                  Valid=FALSE;
                  break;
               }
            }
            else
            {
               Valid=FALSE;
               break;
            }
         }
			
			if (!CMD5((*a).MD5f).Compare((*b).MD5f))
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
	fi1.clear();
	fi2.clear();
	return Valid;
}

BOOL CFileSystem::CopyDir(const TCHAR *Src, const TCHAR *Dst)
{
   if(!Src||!Dst||!DirExists(Src))
   {
      return FALSE;
   }
   if(DirExists(Dst))
   {
      if(CompareDirs(Src,Dst))
      {
         return TRUE;
      }
      RemoveDir(Dst,TRUE);
   }
   else
   {
      if(!CreateDirectory(Dst,NULL))
      {
         printf("%u\n",GetLastError());
         return FALSE;
      }
   }

   
   /*
   
   */
   auto SrcInf=GetDirectoryInfo(Src);
   TCHAR DstPath[MAX_PATH];
   BOOL Res=TRUE;
   for(auto f=SrcInf.begin();f<SrcInf.end();++f)
   {
      PathCombine(DstPath,Dst,(*f).File);
      if((*f).IsDir)
      {
         if(!CopyDir((*f).FullPath,DstPath))
         {
            Res=FALSE;
            break;
         }
         
      }
      else
      {
         if(!CopyFile((*f).FullPath,DstPath,FALSE))
         {
            Res=FALSE;
            break;
         }
      }
      
   }
   SrcInf.clear();
   return CompareDirs(Dst,Src);
}
