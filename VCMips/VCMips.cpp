// VCMips.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "VCProject.h"
#include "MCS2Project.h"
#include "CMD5.h"
#include "FileSystem.h"
#include "CommandLine.h"
#include "ProcessRunner.h"


class CComInitializer
{
public:
   CComInitializer()
   {
      HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
      if (hr != S_OK)
      {
         _ftprintf(stderr, _T("COM Itinialization failed.\n"));
      }
   }
   ~CComInitializer()
   {
      CoUninitialize();
   }
};


CComInitializer ComInitializer;


int _tmain(int argc, _TCHAR* argv[])
{
  
   CProcessRunner CR;
   CR.SetStartupDir(_T("C:\\MCStudio_2\\Tools4_RT2\\bin"));
   CR.SetProgram(_T("mipsel-elf32-gcc.exe"));
   CR.Run();

 


   /*
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


	VCProj->OpenProject(_T("D:\\Stuff\\LV_OLU\\CppProj\\CppProj.vcxproj"));
   delete VCProj;
	
   */

	
	return 0;
}

