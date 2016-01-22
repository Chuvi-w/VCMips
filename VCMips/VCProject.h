#ifndef VCProject_h__
#define VCProject_h__
#include "BaseProject.h"


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
	CFormatedException(const TCHAR *Msg)
	{
		int Len = _tcslen(Msg) + 1;
		MsgBuffer = new TCHAR[Len];
		_tcscpy_s(MsgBuffer,Len, Msg);
	}
	const TCHAR *operator()() const
	{
		return const_cast<const TCHAR*>(MsgBuffer);
	}
private:
	TCHAR *MsgBuffer; 
};



class CVCProject :public CBaseProject
{
public:
	CVCProject();
	~CVCProject();
	BOOL OpenProject(const TCHAR *ProjectName);
	BOOL CreateProject(const TCHAR *ProjectName) NotImplemented(FALSE);
	BOOL SaveProject();
   BOOL CloseProject();
   void ForEachFile(pfnForEachFileFunc Func);
private:
   BOOL CopyComponents();
   BOOL DeleteComponents();
private:
	CComPtr<VCProjectEngineLibrary::VCProjectEngine> VCEngine;
	CComQIPtr<VCProjectEngineLibrary::VCProject>	    VCProject;
   TCHAR szCurWD[MAX_PATH];
   TCHAR szComponentsInternalDir[MAX_PATH];
   BOOL bChanged;
};

#endif // VCProject_h__
