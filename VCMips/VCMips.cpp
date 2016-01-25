// VCMips.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "VCProject.h"
#include "MCS2Project.h"
#include "CMD5.h"
#include "FileSystem.h"
#include "CommandLine.h"

int _tmain(int argc, _TCHAR* argv[])
{


   CCommandLine Cmd;
   TCHAR TestStr[50];
   TCHAR *Str=TestStr;
   _stprintf(TestStr,_T("   adsd   \t\t  "));
//   Cmd.TrimSpaces(Str);
   Cmd.ParseLine(_T("-g -mips32 -EL -O2  -fno-delayed-branch -fno-builtin -c -Wa,-ahl -Wa,-mdebug   -Wall   -o \"$(IntDir)\\C ld.o\"  \"Cld.S\" >> \"$(IntDir)\\Cld.lst\""));
   auto Line=Cmd();
   /*
	auto Data=CMD5("Md5Test").Get();

	CFileSystem fs;
  // auto fis=fs.GetDirectoryInfo(_T("C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\Tools"));
   BOOL IsC=fs.CopyDir(_T("C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\Tools\\ProjectComponents"),_T("ProjectComponents"));
   printf("IsC=%i\n",IsC);
   return 0;
	auto res=fs.CompareDirs(_T("D:\\Work\\VCMips\\ProjectComponents"), _T("C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\Common7\\Tools\\ProjectComponents"));
	auto fi=fs.GetDirectoryInfo(_T("D:\\Work\\VCMips3"));


   */
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (hr != S_OK)
	{
		_ftprintf(stderr, _T("COM Itinialization failed.\n"));
		return 1;
	}
	CVCProject *VCProj;
	try
	{
		VCProj = new CVCProject;
	}
	catch (const CFormatedException &Exc)
	{
		_ftprintf(stderr, _T("Error: %s\n"),Exc());
		return 1;
	}
	catch (...)
	{
		fprintf(stderr, "Unknown exception\n");
		return 1;
	}
//	VCProj->CreateProject(_T("D:\\Stuff\\Progs\\VSProjReader\Test.vcxproj"));
	VCProj->OpenProject(_T("D:\\Stuff\\Progs\\VSProjReader\\EmptyProj\\EmptyProj.vcxproj"));
	//VCProj->OpenProject(_T("D:/Stuff/Progs/VCMips/TestPrj_2013/TestPrj_2013.vcxproj"));
  // VCProj->OpenProject(argv[1]);
  // VCProj->ForEachFile(NULL);
   delete VCProj;
	

//	CMCS2Project MCSProj;
	CoUninitialize();
	return 0;
}

