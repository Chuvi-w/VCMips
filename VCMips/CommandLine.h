#ifndef CommandLine_h__
#define CommandLine_h__
#include <vector>
#pragma once
class CCommandLine
{
public:
   CCommandLine(void);
   ~CCommandLine(void);
   BOOL AddParam(const TCHAR *Param);
   BOOL RemoveParam(const TCHAR *Param);
   BOOL ParseLine(const TCHAR *Line);
   size_t GetLength();
   TCHAR *operator()();
   size_t operator()(TCHAR *szBuff,size_t BuffSz);
private:
   TCHAR* TrimSpaces(const TCHAR *Str);
   std::vector<TCHAR *> Params;
  std::vector<TCHAR *> OutParams;
   TCHAR *CmdBuf;
};

#endif // CommandLine_h__
