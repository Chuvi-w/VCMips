#ifndef CompilerRunner_h__
#define CompilerRunner_h__
#include <vector>
#include "CommandLine.h"
#include "FileSystem.h"
class CCompilerRunner
{
public:
   CCompilerRunner(void);
   ~CCompilerRunner(void);
   BOOL Init();
   BOOL SetStartupDir(const TCHAR *Dir);
   BOOL SetProgram(const TCHAR *Prog);
   BOOL SetStdOutFile(const TCHAR *FilePath);
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
   TCHAR ExeName[MAX_PATH];
};

#endif // CompilerRunner_h__
