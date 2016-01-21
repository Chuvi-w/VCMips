// VCMips.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "VCProject.h"
#include "MCS2Project.h"



int _tmain(int argc, _TCHAR* argv[])
{

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
	
	//VCProj->OpenProject(_T("D:\\Stuff\\Progs\\VSProjReader\\EmptyProj\\EmptyProj.vcxproj"));
   VCProj->OpenProject(_T("D:\\USK\\USK50_PO\\USK50_PO-D_MG\\USK50_PO-D_MG.vcxproj"));
  // VCProj->ForEachFile(NULL);
   delete VCProj;
	

//	CMCS2Project MCSProj;
	CoUninitialize();
	return 0;
}

