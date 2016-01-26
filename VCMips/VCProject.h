#ifndef VCProject_h__
#define VCProject_h__
#include "BaseProject.h"
#include "FileSystem.h"


#define VCPImportSettings raw_interfaces_only raw_native_types named_guids
//raw_interfaces_only named_guids raw_native_types

//#import <dte80a.olb>				VCPImportSettings rename( "GetObject", "GetObjectX" )rename( "FindText", "FindTextX" ) rename( "SearchPath", "SearchPathX" ) rename( "ReplaceText", "ReplaceTextX" )
//#import <dte80.olb>			VCPImportSettings
//#import <dte90.olb>			VCPImportSettings
//#import <dte90a.olb>			VCPImportSettings
//#import <dte100.olb>			VCPImportSettings
  #import <vcpb2.tlb>				VCPImportSettings rename( "GetObject", "GetObjectX" ) rename( "PropertySheet", "PropertySheetX" ) rename( "event", "eventX" )
// #import <vcproject.dll>		VCPImportSettings
// #import <vcpkg.dll>			VCPImportSettings
// #import <vslangproj.olb>	VCPImportSettings
 

class CFormatedException :public std::exception
{
public:
   /*
	CFormatedException(const TCHAR *Msg)
	{
		//int Len = _tcslen(Msg) + 1;
		//MsgBuffer = new TCHAR[Len];
		//_tcscpy_s(MsgBuffer,Len, Msg);
      _tcscpy_s(MsgBuffer,Msg);
	}
   */
   CFormatedException(const TCHAR *pFormat,...)
   {
      va_list marker;
      int maxLen=sizeof(MsgBuffer)/sizeof(TCHAR)-1;

      va_start( marker, pFormat );
      int len = _vsntprintf_s( MsgBuffer, maxLen, pFormat, marker );
      va_end( marker );

      // Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
      if( len < 0 || len >= maxLen )
      {
         len = maxLen;
         MsgBuffer[maxLen-1] = 0;
      }
   }

	const TCHAR *operator()() const
	{
		return const_cast<const TCHAR*>(MsgBuffer);
	}
private:
	TCHAR MsgBuffer[3000]; 
};




class CProjectConfiguration;
class CVCProject :public CBaseProject
{
public:
	CVCProject();
	~CVCProject();
	BOOL OpenProject(const TCHAR *szProjectFile);
	BOOL CreateProject(const TCHAR *ProjectName) ;//NotImplemented(FALSE);
	BOOL SaveProject();
   BOOL CloseProject();
   void ForEachFile(pfnForEachFileFunc Func);
   const TCHAR *GetProjectName();
private:
   BOOL GetComponentsBase();
   BOOL CopyComponents();
   BOOL DeleteComponents();
private:
   
	CComPtr<VCProjectEngineLibrary::VCProjectEngine> VCEngine;
	CComQIPtr<VCProjectEngineLibrary::VCProject>	    VCProject;
   TCHAR szComponentsDir[MAX_PATH];
   TCHAR szCurWD[MAX_PATH];
   TCHAR szComponentsInternalDir[MAX_PATH];
   TCHAR ProjectFile[MAX_PATH];
   TCHAR *ProjectName;
   BOOL bChanged;
   CFileSystem Fs;
   std::vector<CProjectConfiguration*> Configs;
};



typedef struct UserMacro_s
{
   TCHAR *szName;
   TCHAR *szVal;
   BOOL bEnvSet;
   TCHAR *szFilePath;
}UserMacro_t;



class CFileInfo
{
public:
   CFileInfo();
   ~CFileInfo();
};

class CProjectConfiguration
{
public:
   CProjectConfiguration(const TCHAR *PlatformName,const TCHAR *ConfigName);
   ~CProjectConfiguration();
   void AddUserMacro(const TCHAR *Name,const TCHAR *Value,BOOL EnvSet,const TCHAR *FilePath);
   const TCHAR *GetMacroValue(const TCHAR* Name,BOOL *EnvSet=NULL,const TCHAR **FilePath=NULL);

private:
   TCHAR *szPlatName;
   TCHAR *szConfigName;
   TCHAR *szUserMacroFile;
   std::vector<UserMacro_t*> UserMacros;
   std::vector<CFileInfo*> ProjectFiles;
};
#endif // VCProject_h__
