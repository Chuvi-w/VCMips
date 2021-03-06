#include "stdafx.h"
#include "VCProject.h"
#include <cerrno>

#define DispatchVar(var) _disp_##var
#define CollectionVar(var) var##Collection
#define DispCollectionVar(var) _disp_##var##Collection
#define CollectionCountVar(var) _##var##Collection_Count

#define FromDispatch(var) var=DispatchVar(var)
#define FromCollectionDispatch(var) CollectionVar(var)=DispCollectionVar(var)
#define QiPtrDispatch(type,var)\
   CComPtr<IDispatch> DispatchVar(var)=0;\
   CComQIPtr<type> var;

#define QiPtrDispatchCollection(type,var)\
   QiPtrDispatch(IVCCollection,CollectionVar(var));\
   QiPtrDispatch(type,var);\
   long CollectionCountVar(var)=NULL;



#define IfSafeCallFails(Func)\
   if(FAILED(Func))

#define IfSafeCallFailsArgs(Func,...)\
   if(FAILED(Func)||__VA_ARGS__)


#define IfSafeCallSucceeded(Func)\
   if(SUCCEEDED(Func))

#define IfSafeCallSucceededArgs(Func,...)\
   if(SUCCEEDED(Func)&&__VA_ARGS__)




#define IfSafeGetCollectionCountFails(var)\
   IfSafeCallFailsArgs(CollectionVar(var)->get_Count(&CollectionCountVar(var)),!CollectionCountVar(var))

#define IfSafeGetCollectionItemFails(var,num)\
   IfSafeCallFailsArgs(CollectionVar(var)->Item(num,&DispatchVar(var)),!DispatchVar(var))


#define IfSafeGetCollectionCountSucceeded(var)\
   IfSafeCallSucceededArgs(CollectionVar(var)->get_Count(&CollectionCountVar(var)),CollectionCountVar(var))

#define IfSafeGetCollectionItemSucceeded(var,num)\
   IfSafeCallSucceededArgs(CollectionVar(var)->Item(num,&DispatchVar(var)),DispatchVar(var))


using namespace VCProjectEngineLibrary;


#define CaseReturnFileType(type) case eFileType##type:{ return _T(#type);}
const TCHAR *FileTypeToString(eFileType FileType)
{
   switch(FileType)
   {
      default: {return _T("UnknownFileType");};
         CaseReturnFileType(Default);
         CaseReturnFileType(CppCode);
         CaseReturnFileType(CppClass);
         CaseReturnFileType(CppHeader);
         CaseReturnFileType(CppForm);
         CaseReturnFileType(CppControl);
         CaseReturnFileType(Text);
         CaseReturnFileType(DEF);
         CaseReturnFileType(IDL);
         CaseReturnFileType(Makefile);
         CaseReturnFileType(RGS);
         CaseReturnFileType(RC);
         CaseReturnFileType(RES);
         CaseReturnFileType(XSD);
         CaseReturnFileType(XML);
         CaseReturnFileType(HTML);
         CaseReturnFileType(CSS);
         CaseReturnFileType(BMP);
         CaseReturnFileType(ICO);
         CaseReturnFileType(Resx);
         CaseReturnFileType(Script);
         CaseReturnFileType(BSC);
         CaseReturnFileType(XSX);
         CaseReturnFileType(CppWebService);
         CaseReturnFileType(Asax);
         CaseReturnFileType(AspPage);
         CaseReturnFileType(Document);
         CaseReturnFileType(Discomap);
         CaseReturnFileType(CSharpFile);
         CaseReturnFileType(ClassDiagram);
         CaseReturnFileType(MHT);
         CaseReturnFileType(PropertySheet);
         CaseReturnFileType(CUR);
         CaseReturnFileType(Manifest);
         CaseReturnFileType(RDLC);
         CaseReturnFileType(Filters);
   }
   return _T("UnknownFileType");
}
CVCProject::CVCProject() :CBaseProject(this)
{
	
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
   bProjectChanged=FALSE;
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
   CComPtr<IDispatch> DispatchVar(VCProject) = VCProject;
   HRESULT hr = S_FALSE;
   IfSafeCallFailsArgs(VCEngine->LoadProject(CComBSTR(szProjectFileName), &DispatchVar(VCProject)),!DispatchVar(VCProject))
   {
      _ftprintf(stderr, _T("Error: unable to load \"%s\" (%x).\n"),szProjectFileName,HRESULT_CODE(hr));
      return FALSE;
   }
   _tcscpy_s(ProjectFile,szProjectFileName);
   BSTR bsProjectName=NULL;
   BSTR bsProjectDir=NULL;
 
   
   FromDispatch(VCProject);
   bProjectChanged=FALSE;


   QiPtrDispatchCollection(VCConfiguration,PrjConfig);
   QiPtrDispatchCollection(VCPropertySheet,PropSheet);
   QiPtrDispatch(VCPlatform,Platform);
   QiPtrDispatchCollection(VCUserMacro,UserMacro);

   size_t PrjNameLen=0;
   size_t PrjDirLen=0;
   IfSafeCallSucceededArgs(VCProject->get_Name(&bsProjectName),bsProjectName)
   {
      PrjNameLen=_tcslen(OLE2CT(bsProjectName))+1;
      ProjectName=new TCHAR[PrjNameLen];
      _tcscpy_s(ProjectName,PrjNameLen,OLE2CT(bsProjectName));
   }

   IfSafeCallSucceededArgs(VCProject->get_ProjectDirectory(&bsProjectDir),bsProjectDir)
   {
      PrjDirLen=_tcslen(OLE2CT(bsProjectDir))+1;
      ProjectDir=new TCHAR[PrjDirLen];
      _tcscpy_s(ProjectDir,PrjDirLen,OLE2CT(bsProjectDir));
   }

   IfSafeCallFailsArgs(VCProject->get_Configurations(&DispCollectionVar(PrjConfig)),!DispCollectionVar(PrjConfig))
   {
      _ftprintf(stderr, _T("Error: unable to get configurations from \"%s\" (%x).\n"),GetProjectName(),HRESULT_CODE(hr));
      return FALSE;
   }
   
   FromCollectionDispatch(PrjConfig);
   IfSafeGetCollectionCountFails(PrjConfig)
   {
      _ftprintf(stderr, _T("Error: Failed to count configurations in \"%s\" (%x).\n"),GetProjectName(),HRESULT_CODE(hr));
      return FALSE;
   }

   QiPtrDispatchCollection(VCFile,PrjFile);
   QiPtrDispatchCollection(VCFileConfiguration,FileConfiguration);
   IfSafeCallFailsArgs(VCProject->get_Files(&DispCollectionVar(PrjFile)),!DispCollectionVar(PrjFile))
   {
      _ftprintf(stderr, _T("Error: unable to get project files from \"%s\".\n"),GetProjectName());
      return FALSE;
   }
   FromCollectionDispatch(PrjFile);
   IfSafeGetCollectionCountFails(PrjFile)
   {
      _ftprintf(stderr, _T("Error: Failed to count files in \"%s\".\n"),GetProjectName());
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
   BSTR bsParamStr;
   BSTR bsEvalutedParamStr;
   for(int ConfigurationNum=0;ConfigurationNum<=CollectionCountVar(PrjConfig);ConfigurationNum++)
   {
      DispatchVar(PrjConfig)=NULL;
      DispatchVar(PropSheet)=NULL;
      DispatchVar(Platform)=NULL;
      CfgName=NULL;
      PlatformName=NULL;
      bsParamStr=NULL;
      bsEvalutedParamStr=NULL;
      IfSafeGetCollectionItemFails(PrjConfig,CComVariant(ConfigurationNum))
      {
         continue;
      }
      FromDispatch(PrjConfig);
      IfSafeCallFails(PrjConfig->get_ConfigurationName(&CfgName))
      {
         _ftprintf(stderr, _T("Warning: Failed to get name in \"%s\" for configuration item %i. (Err=%x).\n"),GetProjectName(),ConfigurationNum,HRESULT_CODE(hr));
         continue;
      }
      IfSafeCallFailsArgs(PrjConfig->get_Platform(&DispatchVar(Platform)),!DispatchVar(Platform))
      {
         _ftprintf(stderr, _T("Warning: Failed to get platform item for %s configuration in \"%s\" (%x).\n"),OLE2CT(CfgName),GetProjectName(),HRESULT_CODE(hr));
      }
      else
      {
         FromDispatch(Platform);
         IfSafeCallFails(Platform->get_Name(&PlatformName))
         {
            _ftprintf(stderr, _T("Warning: Failed to get platform name for %s configuration in \"%s\" (%x).\n"),OLE2CT(CfgName),GetProjectName(),HRESULT_CODE(hr));
         }
      }
      gConfItem=new CProjectConfiguration(OLE2CT(PlatformName),OLE2CT(CfgName),DispatchVar(PrjConfig));
      if(!gConfItem->SetProjectDir(ProjectDir))
      {
          _ftprintf(stderr, _T("Error: Failed to set project directory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
          delete gConfItem;
          continue;
      }

      IfSafeCallFails(PrjConfig->get_OutputDirectory(&bsParamStr))
      {
         _ftprintf(stderr, _T("Error: Failed to get OutputDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }

      IfSafeCallFails(PrjConfig->Evaluate(bsParamStr,&bsEvalutedParamStr))
      {
         _ftprintf(stderr, _T("Error: Failed to evalute  OutputDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }

      if(!gConfItem->SetOutputDir(OLE2CT(bsEvalutedParamStr)))
      {
         _ftprintf(stderr, _T("Error: Failed to set OutputDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }

      IfSafeCallFails(PrjConfig->get_IntermediateDirectory(&bsParamStr))
      {
         _ftprintf(stderr, _T("Error: Failed to get IntermediateDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }
      
      IfSafeCallFails(PrjConfig->Evaluate(bsParamStr,&bsEvalutedParamStr))
      {
         _ftprintf(stderr, _T("Error: Failed to evalute  IntermediateDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }

      if(!gConfItem->SetIntermediateDir(OLE2CT(bsEvalutedParamStr)))
      {
         _ftprintf(stderr, _T("Error: Failed to set IntermediateDirectory for %s configuration in \"%s\".\n"),OLE2CT(CfgName),GetProjectName());
         delete gConfItem;
         continue;
      }


      IfSafeCallFailsArgs(PrjConfig->get_PropertySheets(&DispCollectionVar(PropSheet)),!DispCollectionVar(PropSheet))
      {
         _ftprintf(stderr, _T("Warning: Failed to get property sheets in %s|%s configuration in \"%s\" (%x).\n"),OLE2CT(CfgName),OLE2CT(PlatformName),GetProjectName(),HRESULT_CODE(hr));
      }
      else
      {
        
         FromCollectionDispatch(PropSheet);
         IfSafeGetCollectionCountFails(PropSheet)
         {
            _ftprintf(stderr, _T("Warning: Failed to count property sheets in %s|%s configuration in \"%s\" (%x).\n"),OLE2CT(CfgName),OLE2CT(PlatformName),GetProjectName(),HRESULT_CODE(hr));
         }
         else
         {
            //_tprintf(_T(" %s Num ps=%i\n"),OLE2CT(CfgName),CollectionCountVar(PropSheet));

            for(int PropSheetNum=1;PropSheetNum<=CollectionCountVar(PropSheet);PropSheetNum++)
            {
               DispatchVar(PropSheet)=NULL;
               DispatchVar(UserMacro)=NULL;
               DispCollectionVar(UserMacro)=NULL;
               PropSheetName=NULL;
               PropSheetFile=NULL;

               IfSafeGetCollectionItemFails(PropSheet,CComVariant(PropSheetNum))
               {
                  _ftprintf(stderr, _T("Warning: Failed to get property sheet item %i in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,OLE2CT(CfgName),OLE2CT(PlatformName),GetProjectName(),HRESULT_CODE(hr));
               }
               else
               {
                  FromDispatch(PropSheet);
                  IfSafeCallFails(PropSheet->get_Name(&PropSheetName))
                  {
                     _ftprintf(stderr, _T("Warning: Failed to get property sheet item(%i) name in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,OLE2CT(CfgName),OLE2CT(PlatformName),GetProjectName(),HRESULT_CODE(hr));
                  }
                  IfSafeCallFails(PropSheet->get_PropertySheetFile(&PropSheetFile))
                  {
                     _ftprintf(stderr, _T("Warning: Failed to get property sheet item(%i) file in %s|%s configuration in \"%s\" (%x).\n"),PropSheetNum,OLE2CT(CfgName),OLE2CT(PlatformName),GetProjectName(),HRESULT_CODE(hr));
                  }
                  //_ftprintf(stderr, _T("Warning: \"%s\" \"%s\"\n"),OLE2CT(PropSheetName),OLE2CT(PropSheetFile));
                  IfSafeCallSucceededArgs(PropSheet->get_UserMacros(&CollectionVar(UserMacro)),CollectionVar(UserMacro))
                  {
                     IfSafeGetCollectionCountSucceeded(UserMacro)
                     {
                        for(int UserMacroNum=0;UserMacroNum<=CollectionCountVar(UserMacro);UserMacroNum++)
                        {
                           //  printf("Um %i\n",UserMacroNum);
                           DispatchVar(UserMacro)=NULL;
                           IfSafeGetCollectionItemSucceeded(UserMacro,CComVariant(UserMacroNum))
                           {
                              FromDispatch(UserMacro);
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
                           DispatchVar(UserMacro).Release();
                        }
                     }
                  }

                  CollectionVar(UserMacro).Release();
                  PropSheet.Release();
               }
            }
            CollectionVar(PropSheet).Release();
         }
         DispCollectionVar(PropSheet).Release();
      }
      PrjConfig.Release();
      Configs.push_back(gConfItem);
   }
   if(!Configs.size())
   {
      _ftprintf(stderr, _T("Error: Failed to load any configuration from \"%s\" (%x).\n"),szProjectFileName,HRESULT_CODE(hr));
      return FALSE;
   }

   BSTR bsFileName;
   BSTR bsFullPath;
   BSTR bsRelativePath;
   BSTR bsFileExtension;
   BSTR bsItemType;
   eFileType iFileType;

   CProjectConfiguration *tConf;
   VARIANT_BOOL vbFileExcluded;
   QiPtrDispatch(VCConfiguration,LinkedProjectConfig);
   for(int FileNum=0;FileNum<=CollectionCountVar(PrjFile);++FileNum)
   {

      DispatchVar(PrjFile)=NULL;
      DispatchVar(FileConfiguration)=NULL;
      DispCollectionVar(FileConfiguration)=NULL;
      DispatchVar(FileConfiguration)=NULL;
      bsFileName=NULL;
      bsFullPath=NULL;
      bsFileExtension=NULL;
      bsItemType=NULL;
      bsRelativePath=NULL;
      iFileType=eFileTypeDefault;
      IfSafeGetCollectionItemFails(PrjFile,CComVariant(FileNum))
      {
         continue;
      }
      FromDispatch(PrjFile);

      IfSafeCallFails(PrjFile->get_Name(&bsFileName))
      {
         _ftprintf(stderr, _T("Warning: Failed to get file name in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
         continue;
      }
      IfSafeCallFails(PrjFile->get_FullPath(&bsFullPath))
      {
         _ftprintf(stderr, _T("Warning: Failed to get full path in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
         continue;
      }

      IfSafeCallFails(PrjFile->get_RelativePath(&bsRelativePath))
      {
         _ftprintf(stderr, _T("Warning: Failed to get relative path in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
         continue;
      }

      IfSafeCallFails(PrjFile->get_Extension(&bsFileExtension))
      {
         _ftprintf(stderr, _T("Warning: Failed to get file extension in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
         continue;
      }

      IfSafeCallFails(PrjFile->get_ItemType(&bsItemType))
      {
         _ftprintf(stderr, _T("Warning: Failed to get item type in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
      }

      IfSafeCallFails(PrjFile->get_FileType(&iFileType))
      {
         _ftprintf(stderr, _T("Warning: Failed to get file type in \"%s\" for item %i.\n"),GetProjectName(),FileNum);
      }
     
      IfSafeCallFailsArgs(PrjFile->get_FileConfigurations(&DispCollectionVar(FileConfiguration)),!DispCollectionVar(FileConfiguration))
      {
         _ftprintf(stderr, _T("Warning: Failed to get configurations for %s= configuration in \"%s\".\n"),OLE2CT(bsFileName),GetProjectName());
         continue;
      }
     
      FromCollectionDispatch(FileConfiguration);
      IfSafeGetCollectionCountFails(FileConfiguration)
      {
         _ftprintf(stderr, _T("Error: Failed to count file configurations for %s in \"%s\".\n"),OLE2CT(bsFileName),GetProjectName());
         return FALSE;
      }
    //  _ftprintf(stderr, _T("FileConfigurations=%i\n"),CollectionCountVar(FileConfiguration));
      for(int FileConfigNum=0;FileConfigNum<=CollectionCountVar(FileConfiguration);++FileConfigNum)
      {
         DispatchVar(FileConfiguration)=NULL;
         DispatchVar(LinkedProjectConfig)=NULL;
        
         IfSafeGetCollectionItemFails(FileConfiguration,CComVariant(FileConfigNum))
         {
            continue;
         }
         FromDispatch(FileConfiguration);

         IfSafeCallFails(FileConfiguration->get_ExcludedFromBuild(&vbFileExcluded))
         {
            _ftprintf(stderr, _T("Warning: get_ExcludedFromBuild failed\n"));
            vbFileExcluded=VARIANT_FALSE;
         }

         if(vbFileExcluded==VARIANT_TRUE)
         {
            continue;
         }
         IfSafeCallFailsArgs(FileConfiguration->get_ProjectConfiguration(&DispatchVar(LinkedProjectConfig)),!DispatchVar(LinkedProjectConfig))
         {
             _ftprintf(stderr, _T("Warning: Failed to get linked configuration %i for file %s in \"%s\"\n"),FileConfigNum,OLE2CT(bsFileName),GetProjectName());
             continue;
         }
         tConf=FindConfigurationByDispatch(DispatchVar(LinkedProjectConfig));
         if(!tConf)
         {
            _ftprintf(stderr, _T("Warning: Failed to get linked configuration %i for file %s in \"%s\"\n"),FileConfigNum,OLE2CT(bsFileName),GetProjectName());
            continue;
         }
         tConf->AddFile(DispatchVar(PrjFile),OLE2CT(bsFileName),OLE2CT(bsFullPath),OLE2CT(bsRelativePath),OLE2CT(bsFileExtension),OLE2CT(bsItemType),iFileType);

      }
   }

   for(auto a=Configs.begin();a<Configs.end();++a)
   {
      (*a)->PrintConfigurationData();
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
   if(bProjectChanged)
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
   delete [] ProjectDir;
   ProjectName=NULL;
   ProjectDir=NULL;
   return TRUE;
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




CProjectConfiguration * CVCProject::FindConfigurationByDispatch(CComPtr<IDispatch> pConfigDispatch)
{
   for(auto a=Configs.begin();a<Configs.end();++a)
   {
      if((*a)->GetConfigDispatch()==pConfigDispatch)
      {
         return (*a);
      }
   }
   return NULL;
}

CProjectConfiguration::CProjectConfiguration(const TCHAR *PlatformName,const TCHAR *ConfigName,CComPtr<IDispatch> pConfigDispatch)
{
    size_t len=0;
   szPlatName=NULL;
   szConfigName=NULL;
   szUserMacroFile=NULL;
   szProjectDir=NULL;
   szOutputDir=NULL;
   szIntermediateDir=NULL;
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

   ConfigurationDispatch=pConfigDispatch;
   ProjectFiles.clear();
   UserMacros.clear();
}

CProjectConfiguration::~CProjectConfiguration()
{
   delete [] szPlatName;
   delete [] szConfigName;
   delete [] szUserMacroFile;
   delete [] szOutputDir;
   delete [] szIntermediateDir;
   delete [] szProjectDir;

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

BOOL CProjectConfiguration::SetOutputDir(const TCHAR *sziOutputDir)
{
   if(!sziOutputDir)
   {
      _ftprintf(stderr,_T("ERROR: sziOutputDir is empty! \n"));
      return FALSE;
   }
   if(!szProjectDir&&PathIsRelative(sziOutputDir))
   {
      _ftprintf(stderr,_T("ERROR: szProjectDir is not set! \n"));
      return FALSE;
   }

   TCHAR CombinedPath[MAX_PATH];
   TCHAR CanonicalizePath[MAX_PATH];

   if(PathIsRelative(sziOutputDir))
   {
      PathCombine(CombinedPath,szProjectDir,sziOutputDir);
      PathCanonicalize(CanonicalizePath,CombinedPath);
   }
   else
   {
      PathCanonicalize(CanonicalizePath,sziOutputDir);
   }

   if(!CanonicalizePath[0])
   {
      _ftprintf(stderr,_T("ERROR: sziOutputDir not canonicalized! \n"));
      return FALSE;
   }

  
   if(CanonicalizePath[_tcslen(CanonicalizePath)-1]!=_T('\\'))
   {
      _tcscat_s(CanonicalizePath,_T("\\"));
   }

   size_t len=_tcslen(CanonicalizePath)+1;
   delete [] szOutputDir;
   szOutputDir=new TCHAR[len];
   _tcscpy_s(szOutputDir,len,CanonicalizePath);
   _ftprintf(stderr,_T("INFO: szOutputDir = \"%s\" \n"),szOutputDir);
   return TRUE;
}

BOOL CProjectConfiguration::SetIntermediateDir(const TCHAR *sziIntermediateDir)
{
   if(!sziIntermediateDir)
   {
      _ftprintf(stderr,_T("ERROR: sziIntermediateDir is empty! \n"));
      return FALSE;
   }
   if(!szProjectDir&&PathIsRelative(sziIntermediateDir))
   {
      _ftprintf(stderr,_T("ERROR: szProjectDir is not set! \n"));
      return FALSE;
   }

   TCHAR CombinedPath[MAX_PATH];
   TCHAR CanonicalizePath[MAX_PATH];

   if(PathIsRelative(sziIntermediateDir))
   {
      PathCombine(CombinedPath,szProjectDir,sziIntermediateDir);
      PathCanonicalize(CanonicalizePath,CombinedPath);
   }
   else
   {
      PathCanonicalize(CanonicalizePath,sziIntermediateDir);
   }

   if(!CanonicalizePath[0])
   {
      _ftprintf(stderr,_T("ERROR: sziIntermediateDir not canonicalized! \n"));
      return FALSE;
   }

   if(CanonicalizePath[_tcslen(CanonicalizePath)-1]!=_T('\\'))
   {
      _tcscat_s(CanonicalizePath,_T("\\"));
   }

   size_t len=_tcslen(CanonicalizePath)+1;
   delete [] szIntermediateDir;
   szIntermediateDir=new TCHAR[len];
   _tcscpy_s(szIntermediateDir,len,CanonicalizePath);
  
    _ftprintf(stderr,_T("INFO: szIntermediateDir = \"%s\" \n"),szIntermediateDir);
   return TRUE;
}

BOOL CProjectConfiguration::SetProjectDir(const TCHAR *sziProjectDir)
{
   if(!sziProjectDir)
   {
      _ftprintf(stderr,_T("ERROR: sziProjectDir is empty! \n"));
      return FALSE;
   }
   if(PathIsRelative(sziProjectDir))
   {
      _ftprintf(stderr,_T("ERROR: (\"%s\") is relative!\n"),sziProjectDir);
      return FALSE;
   }
   if(!PathIsDirectory(sziProjectDir))
   {
      _ftprintf(stderr,_T("ERROR: (\"%s\") is not a directory!\n"),sziProjectDir);
      return FALSE;
   }
   
   TCHAR CanonicalizePath[MAX_PATH];
   PathCanonicalize(CanonicalizePath,sziProjectDir);

   if(CanonicalizePath[_tcslen(CanonicalizePath)-1]!=_T('\\'))
   {
      _tcscat_s(CanonicalizePath,_T("\\"));
   }

   size_t len=_tcslen(CanonicalizePath)+1;
   delete [] szProjectDir;
   szProjectDir=new TCHAR[len];
   _tcscpy_s(szProjectDir,len,CanonicalizePath);
   _ftprintf(stderr,_T("INFO: szProjectDir = \"%s\" \n"),szProjectDir);
   return TRUE;
}

const TCHAR * CProjectConfiguration::GetFileIntermediatePath(CFileInfo *File,const TCHAR *Ext)
{
   if(!File)
   {
      return NULL;
   }
   TCHAR szIntPath[MAX_PATH];
   TCHAR szCanonPath[MAX_PATH];
   PathCombine(szIntPath,szIntermediateDir,File->GetRelativePath());
   PathRemoveExtension(szIntPath);
   if(Ext)
   {
      PathAddExtension(szIntPath,Ext);
   }
   PathCanonicalize(szCanonPath,szIntPath);
   return szCanonPath;
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
   // _ftprintf(stderr,_T("Warning: macro \"%s\" \"%s\ added \n"),Name,Value);
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



void CProjectConfiguration::PrintConfigurationData()
{
   _ftprintf(stderr,_T("Config=%s, platform=%s\n"),szConfigName,szPlatName);
   for(auto a=UserMacros.begin();a<UserMacros.end();++a)
   {
       _ftprintf(stderr,_T("\t [%s] \"%s\"=\"%s\" (%i)\n"),(*a)->szFilePath,(*a)->szName,(*a)->szVal,(*a)->bEnvSet);
   }

   for(auto pf=ProjectFiles.begin();pf<ProjectFiles.end();++pf)
   {
      //(*pf)->PrintFileInfo();
      _tprintf(_T("ExtFile->%s\n"),GetFileIntermediatePath((*pf),_T(".o")));
   }
}

BOOL CProjectConfiguration::AddFile(CComPtr<IDispatch> pFileDispatch,const TCHAR *pFileName,const TCHAR *pFullPath,const TCHAR *pRelativePath,const TCHAR *pFileExtension,const TCHAR *pItemType,VCProjectEngineLibrary::eFileType inFileType)
{
   CComQIPtr<VCFile> ProjectFile =pFileDispatch;
   if(!ProjectFile)
   {
      return FALSE;
   }
   auto pFile=FindFile(pFileDispatch);
   if(pFile)
   {
      _ftprintf(stderr,_T("File \"%s\" already exists in project\n"),pFile->GetFileName());
      return FALSE;
   }

   auto prjFile=new CFileInfo(pFileDispatch,pFileName,pFullPath,pRelativePath,pFileExtension,pItemType,inFileType);
   ProjectFiles.push_back(prjFile);
   return TRUE;
}

CFileInfo * CProjectConfiguration::FindFile(CComPtr<IDispatch> dpProjectFile)
{
   for(auto a=ProjectFiles.begin();a<ProjectFiles.end();++a)
   {
      if((*a)->GetFileDispatch()==dpProjectFile)
      {
         return (*a);
      }
   }
   return NULL;
}

CFileInfo::CFileInfo(CComPtr<IDispatch> pFileDispatch,const TCHAR *pFileName,const TCHAR *pFullPath,const TCHAR *pRelativePath,const TCHAR *pFileExtension,const TCHAR *pItemType,VCProjectEngineLibrary::eFileType inFileType)
{
   FileDispatch=pFileDispatch;
   auto CopyStr=[](const TCHAR *In,TCHAR **Out)
   {
      if(!Out)
      {
         return;
      }
      if(!In)
      {
         Out=NULL;
      }
      size_t len=_tcslen(In)+1;
      *Out=new TCHAR[len];
      _tcscpy_s(*Out,len,In);
   };
   CopyStr(pFileName,&szFileName);
   CopyStr(pFullPath,&szFullPath);
   CopyStr(pRelativePath,&szRelativePath);
   CopyStr(pFileExtension,&szFileExtension);
   CopyStr(pItemType,&szItemType);
   FileType=inFileType;


}

CFileInfo::~CFileInfo()
{
   FileDispatch=NULL;
   delete [] szFileName;
   delete [] szFullPath;
   delete [] szFileExtension;
   delete [] szItemType;
   delete [] szRelativePath;
}

void CFileInfo::PrintFileInfo()
{
    _ftprintf(stderr,_T("File \"%s\". Rel=\"%s\" Ext=\"%s\" ItemType=\"%s\" FileType=\"%s\" \n"),szFileName,szRelativePath, szFileExtension,szItemType,FileTypeToString(FileType));
}
