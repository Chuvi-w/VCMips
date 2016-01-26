#include "stdafx.h"
#include "VCProject.h"
#include <cerrno>

#define DispVar(var) _disp_##var
#define ColVar(var) var##Collection
#define DispColVar(var) _disp_##var##Collection
#define ColCountVar(var) _##var##Collection_Count

#define FromDisp(var) var=DispVar(var)
#define FromCDisp(var) ColVar(var)=DispColVar(var)
#define QiPtrD(type,var)\
   CComPtr<IDispatch> DispVar(var)=0;\
   CComQIPtr<type> var;

#define QiPtrDC(type,var)\
   QiPtrD(IVCCollection,ColVar(var));\
   QiPtrD(type,var);\
   long ColCountVar(var)=NULL;



#define IfSafeCallFails(Func)\
   hr=Func;\
   if(FAILED(hr))

#define IfSafeCallFailsArgs(Func,...)\
   hr=Func;\
   if(FAILED(hr)||__VA_ARGS__)


#define IfSafeCallSucceeded(Func)\
   hr=Func;\
   if(SUCCEEDED(hr))

#define IfSafeCallSucceededArgs(Func,...)\
   hr=Func;\
   if(SUCCEEDED(hr)&&__VA_ARGS__)




#define SafeGetCollectionCountFails(var)\
   IfSafeCallFailsArgs(ColVar(var)->get_Count(&ColCountVar(var)),!ColCountVar(var))

#define SafeGetCollectionItemFails(var,num)\
   IfSafeCallFailsArgs(ColVar(var)->Item(num,&DispVar(var)),!DispVar(var))


#define SafeGetCollectionCountSucceeded(var)\
   IfSafeCallSucceededArgs(ColVar(var)->get_Count(&ColCountVar(var)),ColCountVar(var))

#define SafeGetCollectionItemSucceeded(var,num)\
   IfSafeCallSucceededArgs(ColVar(var)->Item(num,&DispVar(var)),DispVar(var))


using namespace VCProjectEngineLibrary;
CVCProject::CVCProject() :CBaseProject(this)
{
	HRESULT hr;
   _tgetcwd(szCurWD,MAX_PATH-1);
   _sntprintf_s(szComponentsInternalDir,MAX_PATH-1,_T("%s\\ProjectComponents"),szCurWD);
   if(!GetComponentsBase())
   {
      throw CFormatedException(_T("Failed to find ProjectComponents dir."));
      return;
   }
   
   if(!CopyComponents())
   {
      throw CFormatedException(_T("Failed to copy ProjectComponents dir."));
      return;
   }

	IfSafeCallFailsArgs
   (
      CoCreateInstance
      (
         CLSID_VCProjectEngineObject,
         nullptr, 
         CLSCTX_INPROC_SERVER, 
         IID_VCProjectEngine,
         reinterpret_cast<void**>(&VCEngine)
      ),
      !VCEngine
   )
	{
		throw CFormatedException(_T("Unable to create VCProjectEngine Instance."));
      return;
	}
 
   ProjectName=NULL;
   bChanged=FALSE;
}


CVCProject::~CVCProject()
{
   CloseProject();
   DeleteComponents();
   
}

BOOL CVCProject::OpenProject(const TCHAR *szProjectFileName)
{
   USES_CONVERSION;
   if(VCProject)
   {
      CloseProject();
   }
   CComPtr<IDispatch> DispVar(VCProject) = VCProject;
   HRESULT hr = S_FALSE;
   IfSafeCallFailsArgs(VCEngine->LoadProject(CComBSTR(szProjectFileName), &DispVar(VCProject)),!DispVar(VCProject))
   {
      _ftprintf(stderr, _T("Error: unable to load \"%s\" (%x).\n"),szProjectFileName,HRESULT_CODE(hr));
      return FALSE;
   }
   _tcscpy_s(ProjectFile,szProjectFileName);
   BSTR bsProjectName=NULL;
 
   
   FromDisp(VCProject);
   bChanged=FALSE;


   QiPtrDC(VCConfiguration,PrjConfig);
   QiPtrDC(VCPropertySheet,PropSheet);
   QiPtrD(VCPlatform,Platform);
   QiPtrDC(VCUserMacro,UserMacro);

   size_t PrjNameLen=0;
   IfSafeCallSucceededArgs(VCProject->get_Name(&bsProjectName),bsProjectName)
   {
      PrjNameLen=_tcslen(OLE2CT(bsProjectName))+1;
      ProjectName=new TCHAR[PrjNameLen];
      _tcscpy_s(ProjectName,PrjNameLen,OLE2CT(bsProjectName));
   }


   IfSafeCallFailsArgs(VCProject->get_Configurations(&DispColVar(PrjConfig)),!DispColVar(PrjConfig))
   {
      _ftprintf(stderr, _T("Error: unable to get configurations from \"%s\" (%x).\n"),GetProjectName(),HRESULT_CODE(hr));
      return FALSE;
   }
   
   FromCDisp(PrjConfig);
   long NumConfigs=0;
   SafeGetCollectionCountFails(PrjConfig)
   {
      _ftprintf(stderr, _T("Error: Failed to count configurations in \"%s\" (%x).\n"),GetProjectName(),HRESULT_CODE(hr));
      return FALSE;
   }


   BSTR CfgName;
   BSTR PlatformName;
   BSTR PropSheetFile;
   BSTR PropSheetName;
   BSTR MacroName;
   BSTR MacroValue;
   VARIANT_BOOL vbMacroEnvSet;
   CProjectConfiguration *gConfItem;


   for(int CfgNum=1;CfgNum<=ColCountVar(PrjConfig);CfgNum++)
   {
      DispVar(PrjConfig)=NULL;
      DispVar(PropSheet)=NULL;
      DispVar(Platform)=NULL;
      CfgName=NULL;
      PlatformName=NULL;

      SafeGetCollectionItemFails(PrjConfig,CComVariant(CfgNum))
      {
         continue;
      }
      FromDisp(PrjConfig);
      IfSafeCallFails(PrjConfig->get_ConfigurationName(&CfgName))
      {
         _ftprintf(stderr, _T("Warning: Failed to get name in \"%s\" for configuration item %i. (Err=%x).\n"),GetProjectName(),CfgNum,HRESULT_CODE(hr));
         continue;
      }
      IfSafeCallFailsArgs(PrjConfig->get_Platform(&DispVar(Platform)),!DispVar(Platform))
      {
         _ftprintf(stderr, _T("Warning: Failed to get platform item for %s configuration in \"%s\" (%x).\n"),CfgName,GetProjectName(),HRESULT_CODE(hr));
      }
      else
      {
         FromDisp(Platform);
         IfSafeCallFails(Platform->get_Name(&PlatformName))
         {
            _ftprintf(stderr, _T("Warning: Failed to get platform name for %s configuration in \"%s\" (%x).\n"),CfgName,GetProjectName(),HRESULT_CODE(hr));
         }
      }
      gConfItem=new CProjectConfiguration(OLE2CT(PlatformName),OLE2CT(CfgName));

      IfSafeCallFailsArgs(PrjConfig->get_PropertySheets(&DispColVar(PropSheet)),!DispColVar(PropSheet))
      {
         _ftprintf(stderr, _T("Warning: Failed to get property sheets in %s|%s configuration in \"%s\" (%x).\n"),CfgName,PlatformName,GetProjectName(),HRESULT_CODE(hr));
      }
      else
      {
        
         FromCDisp(PropSheet);
         SafeGetCollectionCountFails(PropSheet)
         {
            _ftprintf(stderr, _T("Warning: Failed to count property sheets in %s|%s configuration in \"%s\" (%x).\n"),CfgName,PlatformName,GetProjectName(),HRESULT_CODE(hr));
         }
         else
         {
            _tprintf(_T(" %s Num ps=%i\n"),OLE2CT(CfgName),ColCountVar(PropSheet));

            for(int PropSheetNum=1;PropSheetNum<=ColCountVar(PropSheet);PropSheetNum++)
            {
               DispVar(PropSheet)=NULL;
               DispVar(UserMacro)=NULL;
               DispColVar(UserMacro)=NULL;
               PropSheetName=NULL;
               PropSheetFile=NULL;

               SafeGetCollectionItemFails(PropSheet,CComVariant(PropSheetNum))
               {
               _ftprintf(stderr, _T("Warning: Failed to get property sheet item %i in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,CfgName,PlatformName,GetProjectName(),HRESULT_CODE(hr));
               }
               else
               {
                  FromDisp(PropSheet);
                  IfSafeCallFails(PropSheet->get_Name(&PropSheetName))
                  {
                     _ftprintf(stderr, _T("Warning: Failed to get property sheet item(%i) name in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,CfgName,PlatformName,GetProjectName(),HRESULT_CODE(hr));
                  }
                  IfSafeCallFails(PropSheet->get_PropertySheetFile(&PropSheetFile))
                  {
                     _ftprintf(stderr, _T("Warning: Failed to get property sheet item(%i) file in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,CfgName,PlatformName,GetProjectName(),HRESULT_CODE(hr));
                  }
                  _ftprintf(stderr, _T("Warning: \"%s\" \"%s\"\n"),PropSheetName,PropSheetFile);
                  IfSafeCallSucceededArgs(PropSheet->get_UserMacros(&ColVar(UserMacro)),ColVar(UserMacro))
                  {
                     SafeGetCollectionCountSucceeded(UserMacro)
                     {
                        for(int UserMacroNum=0;UserMacroNum<=ColCountVar(UserMacro);UserMacroNum++)
                        {
                           //  printf("Um %i\n",UserMacroNum);
                           DispVar(UserMacro)=NULL;
                           SafeGetCollectionItemSucceeded(UserMacro,CComVariant(UserMacroNum))
                           {
                              FromDisp(UserMacro);
                              MacroName=NULL;
                              MacroValue=NULL;
                              vbMacroEnvSet=VARIANT_FALSE;

                              IfSafeCallSucceededArgs(UserMacro->get_Name(&MacroName),MacroName)
                              {
                                 UserMacro->get_Value(&MacroValue);
                                 UserMacro->get_PerformEnvironmentSet(&vbMacroEnvSet);
                                 gConfItem->AddUserMacro(OLE2CT(MacroName),OLE2CT(MacroValue),vbMacroEnvSet!=VARIANT_FALSE,OLE2CT(PropSheetFile));
                              }
                           }
                           UserMacro.Release();
                           DispVar(UserMacro).Release();
                        }
                     }
                  }

                  ColVar(UserMacro).Release();
                  PropSheet.Release();
               }
            }
            ColVar(PropSheet).Release();
         }
         DispColVar(PropSheet).Release();
      }
      PrjConfig.Release();
      Configs.push_back(gConfItem);
   }
   if(!Configs.size())
   {
      _ftprintf(stderr, _T("Error: Failed to load any configuration from \"%s\" (%x).\n"),szProjectFileName,HRESULT_CODE(hr));
      return FALSE;
   }
   return TRUE;
}


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
}

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
   
   for(auto conf=Configs.begin();conf<Configs.end();++conf)
   {
      delete (*conf);
   }
   Configs.clear();

   delete [] ProjectName;
   ProjectName=NULL;
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

const TCHAR * CVCProject::GetProjectName()
{
   return ProjectName?ProjectName:_T("__NAME_NOT_EXISTS__");
}

BOOL CVCProject::GetComponentsBase()
{

   TCHAR EnvName[60];
   auto GetEnvName=[&EnvName](int Version)->TCHAR*
   {
      _stprintf_s(EnvName,_T("VS%iCOMNTOOLS"),Version);
      return EnvName;
   };
   auto GetComponenstPath=[this,GetEnvName](int Version)->TCHAR*
   {
      size_t RetSize=0;
     auto err= _tgetenv_s(&RetSize,this->szComponentsDir,GetEnvName(Version));
     if(err||!RetSize)
     {
        return NULL;
     }
     PathAppend(this->szComponentsDir,_T("ProjectComponents"));
     return this->szComponentsDir;
   };

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
   switch(_MSC_VER)
   {
   default:
      {
         if(_MSC_VER<1400)
         {
            _ftprintf(stderr,_T("Your Visual Studio version (%i) is too old. Throw it away.\n"),_MSC_VER);
         }
         else if(_MSC_VER>1900)
         {
            _ftprintf(stderr,_T("Add your Visual Studio version (%i) to switch below.\n"),_MSC_VER);
         }
         else
         {
            _ftprintf(stderr,_T("Unknown Visual Studio version (%i)\n"),_MSC_VER);
         }
         return FALSE;
      }
   case 1400:
      {
         if(!GetComponenstPath(80))
         {
            return FALSE;
         }
         break;
      }
   case 1500:
      {
         if(!GetComponenstPath(90))
         {
            return FALSE;
         }
         break;
      }
   case 1600:
      {
         if(!GetComponenstPath(100))
         {
            return FALSE;
         }
         break;
      }
   case 1700:
      {
         if(!GetComponenstPath(110))
         {
            return FALSE;
         }
         break;
      }
   case 1800:
      {
         if(!GetComponenstPath(120))
         {
            return FALSE;
         }
         break;
      }
   case 1900:
      {
         if(!GetComponenstPath(140))
         {
            return FALSE;
         }
         break;
      }
   }

   BOOL Empty=FALSE;
   if(!Fs.DirExists(szComponentsDir,&Empty))
   {
      _ftprintf(stderr,_T("%s not exists\n"),szComponentsDir);
      return FALSE;
   }
   if(Empty)
   {
       _ftprintf(stderr,_T("%s is empty\n"),szComponentsDir);
   }
   return TRUE;
}

BOOL CVCProject::CopyComponents()
{
   return Fs.CopyDir(szComponentsDir,szComponentsInternalDir);
   
}

BOOL CVCProject::DeleteComponents()
{
   return Fs.RemoveDir(szComponentsInternalDir);

}




CProjectConfiguration::CProjectConfiguration(const TCHAR *PlatformName,const TCHAR *ConfigName)
{
    size_t len=0;
   szPlatName=NULL;
   szConfigName=NULL;
   szUserMacroFile=NULL;
   if(PlatformName)
   {
      len=_tcslen(PlatformName)+1;
      szPlatName=new TCHAR[len];
      _tcscpy_s(szPlatName,len,PlatformName);
   }

   if(ConfigName)
   {
      len=_tcslen(ConfigName)+1;
      szConfigName=new TCHAR[len];
      _tcscpy_s(szConfigName,len,ConfigName);
   }
   ProjectFiles.clear();
   UserMacros.clear();
}

CProjectConfiguration::~CProjectConfiguration()
{
   delete [] szPlatName;
   delete [] szConfigName;
   delete [] szUserMacroFile;

   for(auto pf=ProjectFiles.begin();pf<ProjectFiles.end();++pf)
   {
      delete (*pf);
   }
   for(auto pf=UserMacros.begin();pf<UserMacros.end();++pf)
   {
      delete (*pf)->szVal;
      delete (*pf)->szName;
      delete (*pf)->szFilePath;
      delete (*pf);
   }
   ProjectFiles.clear();
   UserMacros.clear();
}


void CProjectConfiguration::AddUserMacro(const TCHAR *Name,const TCHAR *Value,BOOL EnvSet,const TCHAR *FilePath)
{

   const TCHAR *tFilePath,*tVal;
   tVal=GetMacroValue(Name,NULL,&tFilePath);
   if(tVal)
   {
      _ftprintf(stderr,_T("Warning: macro \"%s\" from \"%s\" already exists in \"%s\" \n"),Name,FilePath,tFilePath);
      return;
   }

   UserMacro_t *Macro=new UserMacro_t;

   size_t len=0;

   Macro->szName=NULL;
   Macro->szVal=NULL;
   Macro->szFilePath=NULL;
   if(Name)
   {
      len=_tcslen(Name)+1;
      Macro->szName=new TCHAR[len];
      _tcscpy_s(Macro->szName,len,Name);
   }

   if(Value)
   {
      len=_tcslen(Value)+1;
      Macro->szVal=new TCHAR[len];
      _tcscpy_s(Macro->szVal,len,Value);
   }
   if(FilePath)
   {
      len=_tcslen(FilePath)+1;
      Macro->szFilePath=new TCHAR[len];
      _tcscpy_s(Macro->szFilePath,len,FilePath);
   }
   Macro->bEnvSet=EnvSet;
   UserMacros.push_back(Macro);
    _ftprintf(stderr,_T("Warning: macro \"%s\" \"%s\ added \n"),Name,Value);
}

const TCHAR * CProjectConfiguration::GetMacroValue(const TCHAR* Name,BOOL *EnvSet/*=NULL*/,const TCHAR **FilePath/*=NULL*/)
{
   for(auto a=UserMacros.begin();a<UserMacros.end();++a)
   {
      if(!_tcscmp((*a)->szName,Name))
      {
         if(EnvSet)
         {
            *EnvSet=(*a)->bEnvSet;
         }
         if(FilePath)
         {
            *FilePath=(*a)->szFilePath;
         }
         return (*a)->szVal;
      }
   }
   return NULL;
}



CFileInfo::~CFileInfo()
{

}
