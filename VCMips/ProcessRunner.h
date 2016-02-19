#ifndef CompilerRunner_h__
#define CompilerRunner_h__
#include <vector>
#include "CommandLine.h"
#include "FileSystem.h"
class CProcessRunner
{
public:
   CProcessRunner(void);
   ~CProcessRunner(void);
   BOOL Reset();
   BOOL SetStartupDir(const TCHAR *Dir);
   BOOL SetProgram(const TCHAR *Prog);
   BOOL SetStdOutFile(const TCHAR *FilePath);
   BOOL AddCommandLine(TCHAR *CmdLine);
   BOOL AddCmdParam(TCHAR *Param);
   BOOL RemoveCmdParam(TCHAR *Param);
   int Run();
private:
   enum 
   {
      H_Read,
      H_Write,
      H_Max
   };

   enum
   {
      ST_Err,
      ST_Out,
      ST_Max
   };
   CCommandLine Cmd;
   CFileSystem FS;
   TCHAR StartupDir[MAX_PATH];
   TCHAR ExePath[MAX_PATH];
   TCHAR ExeFullPath[MAX_PATH];
   TCHAR szStdOutPath[MAX_PATH];
};

#endif // CompilerRunner_h__
