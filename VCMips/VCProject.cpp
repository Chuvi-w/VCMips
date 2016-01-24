#include "stdafx.h"
#include "VCProject.h"
/*
   MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
   MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
   MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
   MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
   MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
   MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
   MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
   MSVC++ 7.0  _MSC_VER == 1300
   MSVC++ 6.0  _MSC_VER == 1200
   MSVC++ 5.0  _MSC_VER == 1100
*/


using namespace VCProjectEngineLibrary;
CVCProject::CVCProject() :CBaseProject(this)
{
	HRESULT hr;
	/*hr = CoCreateInstance(
									__uuidof(VCProjectEngineObject), 
									NULL, 
									CLSCTX_INPROC_SERVER, 
									__uuidof(VCProjectEngine), 
									reinterpret_cast<void**>(&VCEngine)
								);*/

   _tgetcwd(szCurWD,MAX_PATH-1);
   _sntprintf_s(szComponentsInternalDir,MAX_PATH-1,_T("%s\\ProjectComponents"),szCurWD);

   if(!CopyComponents())
   {
      throw CFormatedException(_T("Failed to copy ProjectComponents dir."));
      return;
   }

	hr = CoCreateInstance(
									CLSID_VCProjectEngineObject,
									nullptr, 
									CLSCTX_INPROC_SERVER, 
									IID_VCProjectEngine,
									reinterpret_cast<void**>(&VCEngine)
								);

	if (FAILED(hr) || !VCEngine)
	{
		throw CFormatedException(_T("Unable to create VCProjectEngine Instance."));
      return;
	}
	
   
}


CVCProject::~CVCProject()
{
   CloseProject();
}



BOOL CVCProject::OpenProject(const TCHAR *ProjectName)
{
   if(VCProject)
   {
      CloseProject();
   }
   CComPtr<IDispatch> spDisp = VCProject;
	BSTR bsPrjName = CComBSTR(ProjectName);
  // HRESULT hr=VCEngine->LoadProject(CComBSTR(ProjectName),&spDisp);
	HRESULT hr = S_FALSE;
  auto llb= LoadLibrary(_T("C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\Tools\\ProjectComponents\\Microsoft.VisualStudio.Project.VisualC.VCProjectEngine.dll"));
   hr=VCEngine->LoadProject(bsPrjName, &spDisp);
/*
   try
   {
		
   }
	catch (const std::exception& e)
   {
		_ftprintf(stderr, _T("Error: unable to load \"%s\" (%x).\n"), ProjectName, HRESULT_CODE(hr));
   }
   catch (...)
   {
		_ftprintf(stderr, _T("Error: unable to load \"%s\" (%x).\n"), ProjectName, HRESULT_CODE(hr));
   }
  */
   if(FAILED(hr)||!spDisp)
   {
		

		
      _ftprintf(stderr, _T("Error: unable to load \"%s\" (%x).\n"),ProjectName,HRESULT_CODE(hr));
      return FALSE;
   }
   VCProject=spDisp;
   bChanged=FALSE;
   CComPtr<IDispatch> DiPrjConfigs;
   CComQIPtr<IVCCollection> PrjConfigs;
   VCProject->get_Configurations(&DiPrjConfigs);
   PrjConfigs=DiPrjConfigs;
   long NumConfs=0,NumPS=0;
   PrjConfigs->get_Count(&NumConfs);
   CComPtr<IDispatch> DiConf;
   CComQIPtr<VCConfiguration> Conf;
   CComBSTR prp;

   CComPtr<IDispatch> DiPSCollection;
   CComQIPtr<IVCCollection> PSCollection;

   CComPtr<IDispatch> DiPS;
   CComQIPtr<VCPropertySheet> PS;
   for(int i=1;i<=NumConfs;i++)
   {
      DiConf=NULL;
      hr=PrjConfigs->Item(CComVariant(i),&DiConf);
      if(FAILED(hr)||!DiConf)
      {
         continue;;
      }
      Conf=DiConf;
      DiPSCollection=NULL;
      hr=Conf->get_PropertySheets(&DiPSCollection);
      if(SUCCEEDED(hr)&&DiPSCollection)
      {
         PSCollection=DiPSCollection;
         PSCollection->get_Count(&NumPS);
         printf("Num ps=%i\n",NumPS);

         for(int j=0;j<=NumPS;j++)
         {
            DiPS=NULL;
            hr=PSCollection->Item(CComVariant(j),&DiPS);
            if(FAILED(hr)||!DiPS)
            {
               continue;
            }
            PS=DiPS;
            CComBSTR psFile;
            PS->get_PropertySheetFile(&psFile);
            _ftprintf(stdout,_T("ps=%s\n"),COLE2T(psFile));
            CComQIPtr<IVCCollection> UMCol;
            hr=PS->get_UserMacros(&UMCol);
            if(SUCCEEDED(hr))
            {
               long nMacr=0;
               UMCol->get_Count(&nMacr);
               printf("Num um=%i\n",nMacr);
            }
            PS.Release();
         }
         PSCollection.Release();
      }

      Conf.Release();
   }
	return TRUE;
}

/*
BOOL CVCProject::CreateProject(const TCHAR *ProjectName)
{

   CComPtr<IDispatch> spDisp = NULL;
   CComBSTR bstr = "DUMMY DUMMY"; 
    HRESULT hr2= VCEngine->CreateProject( bstr, &spDisp );

	CComPtr<IDispatch> spDispProj = NULL;
	 HRESULT hr = VCEngine->CreateProject(CComBSTR(ProjectName), &spDispProj);
	if (FAILED(hr))
	{
		return FALSE;
	}
	
	return TRUE;
}*/

BOOL CVCProject::SaveProject()
{
   HRESULT hr;
	if(VCProject)
   {
      hr=VCProject->Save();
      if(SUCCEEDED(hr))
      {
         return TRUE;
      }
   }
   return FALSE;
}

BOOL CVCProject::CloseProject()
{
   if(bChanged)
   {
      SaveProject();
   }

   VCProject.Release();
   
   return TRUE;
}

void CVCProject::ForEachFile(pfnForEachFileFunc Func)
{
   if(!VCProject)
   {
      return;
   }
   CComPtr<IDispatch> DiFilesCollection = NULL;
   CComQIPtr<IVCCollection> FilesCollection;
   long NumFiles=NULL;
   CComVariant iFileItem;
   CComPtr<IDispatch> DiFile = NULL;
   CComQIPtr <VCFile> File;
   CComBSTR FileName;

   CComPtr<IDispatch> DiConfigCollection = NULL;
   CComQIPtr<IVCCollection> ConfigsCollection;
   long NumConfigs=NULL;
   CComVariant iConfigItem;
   CComPtr<IDispatch> DiConfig=NULL;
   CComQIPtr <VCFileConfiguration> Config;
   CComBSTR ConfigName;
   

   HRESULT hr = VCProject->get_Files(&DiFilesCollection);
   if(FAILED(hr)||!DiFilesCollection)
   {
      _ftprintf(stderr, _T("Error: unable to get files from project.\n"));
      return;
   }
   FilesCollection=DiFilesCollection;
  
   hr=FilesCollection->get_Count(&NumFiles);
   if(FAILED(hr)||!NumFiles)
   {
      _ftprintf(stderr, _T("Error: unable to count files in project.\n"));
      return;
   }
  
   for(int i=1;i<=NumFiles;i++)
   {
      iFileItem=i;
      DiFile=NULL;
      DiConfigCollection=NULL;
      
      hr=FilesCollection->Item(iFileItem,&DiFile);
      if(FAILED(hr)||!DiFile)
      {
         _ftprintf(stderr, _T("Error: unable to get file item %i.\n"),i);
         continue;
      }

      File=DiFile;
      hr=File->get_ItemName(&FileName);
      if(FAILED(hr))
      {
         _ftprintf(stderr, _T("Error: unable to get name for file %i.\n"),i);
         continue;
      }
      hr=File->get_FileConfigurations(&DiConfigCollection);
      if(FAILED(hr))
      {
         _ftprintf(stderr, _T("Error: unable to get configurations for %s.\n"),COLE2T(FileName));
         continue;
      }
      ConfigsCollection=DiConfigCollection;
      hr=ConfigsCollection->get_Count(&NumConfigs);
      if(FAILED(hr)||!NumConfigs)
      {
         _ftprintf(stderr, _T("Error: unable to count configs for %s.\n"),COLE2T(FileName));
         continue;
      }

      for(int j=1;j<=NumConfigs;j++)
      {
         iConfigItem=j;
         DiConfig=NULL;
         hr=ConfigsCollection->Item(iConfigItem,&DiConfig);
         if(FAILED(hr)||!DiConfig)
         {
            
            _ftprintf(stderr, _T("Error: unable to get config item %i for %s.\n"),j,COLE2T(FileName));
            continue;          
         }
         Config=DiConfig;
         hr=Config->get_Name(&ConfigName);
         if(FAILED(hr))
         {
            _ftprintf(stderr, _T("Error: unable to get name for config %i in %s.\n"),j,COLE2T(FileName));
            continue;
         }
         CComBSTR Ext;
         File->get_Extension(&Ext);

         _ftprintf(stdout,_T("%s\n"),COLE2T(ConfigName));
          _ftprintf(stdout,_T("%s\n"),COLE2T(Ext));
         Config.Release();
      }
      ConfigsCollection.Release();
      File.Release();
      
   }

  // VCProjectConf
}

BOOL CVCProject::CopyComponents()
{
   DeleteComponents();
   TCHAR szBaseComponentsDir[MAX_PATH];
	TCHAR szVSToolsDir[MAX_PATH];
   TCHAR EnvName[50];
  // _tdupenv_s(&szVSToolsDir)
//   szVSToolsDir=_tgetenv(_T("VS120COMNTOOLS"));
   return FALSE;
   
}

BOOL CVCProject::DeleteComponents()
{
   if(PathFileExists(szComponentsInternalDir))
   {
      if(_trmdir(szComponentsInternalDir))
      {
         return FALSE;
      }
   }
   return TRUE;
}

