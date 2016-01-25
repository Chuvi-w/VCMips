#ifndef CommandLine_h__
#define CommandLine_h__
#include <vector>
#pragma once
class CCommandLine
{
public:
   CCommandLine(void);
   ~CCommandLine(void);
   void AddParam(const TCHAR *Param);
   void RemoveParam(const TCHAR *Param);
   void ParseLine(TCHAR *Line);
   TCHAR *operator()();
private:
   TCHAR* TrimSpaces(const TCHAR *Str);
   std::vector<TCHAR *> Params;
   TCHAR *CmdBuf;
};

#endif // CommandLine_h__
