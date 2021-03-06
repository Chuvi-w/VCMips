#include "StdAfx.h"
#include "CommandLine.h"


CCommandLine::CCommandLine(void)
{
   CmdBuf=NULL;
   Params.clear();
}


CCommandLine::~CCommandLine(void)
{
   for(auto a=Params.begin();a<Params.end();++a)
   {
      delete [] (*a);
   }
   Params.clear();
   if(CmdBuf)
   {
      delete [] CmdBuf;
      CmdBuf=NULL;
   }
}

BOOL CCommandLine::AddParam(const TCHAR *Param)
{
   
   auto tParam=TrimSpaces(Param);
   if(!tParam)
   {
      return FALSE;
   }
   int len=_tcslen(tParam);
   Params.push_back(new TCHAR[len+1]);
   _tcscpy_s(Params.back(),len+1,tParam);
   delete [] tParam;
   return TRUE;
}

BOOL CCommandLine::RemoveParam(const TCHAR *Param)
{
   
   auto tParam=TrimSpaces(Param);
   if(!tParam)
   {
      return FALSE;
   }
   size_t Cur=0;
   //auto Cur = Params.begin();
   BOOL Removed=FALSE;
   while (Cur<Params.size())
   {
      if (!_tcsicmp((Params[Cur]),tParam))
      {
         delete Params[Cur];
         Params.erase(std::remove(Params.begin(), Params.end(),Params[Cur]), Params.end());
         Removed=TRUE;
      }
      else
      {
         ++Cur;
      }
   }
   delete [] tParam;
   return Removed;
}

BOOL CCommandLine::ParseLine(const TCHAR *Line)
{
   auto TLine=TrimSpaces(Line);
   if(!TLine)
   {
      return FALSE;
   }
   TCHAR *CurParam=TLine;
   TCHAR *c=TLine;
   BOOL NQuot=FALSE;
   do 
   {
      if(*c==_T('"'))
      {
         NQuot=!NQuot;
      }
      if( _istspace(*c)&&!NQuot)
      {
         *c=_T('\0');
         AddParam(CurParam);
         c++;
         CurParam=c;
      }
      else
      {
         c++;
      }
      
      if(!*c)
      {
         AddParam(CurParam);
      }
      
   } while (*c);

   BOOL RemoveFlag=FALSE;
   size_t Cur=0;
   while(Cur<Params.size())
   {
      if(_tcsicmp(Params[Cur],_T(">>"))==0||_tcsicmp(Params[Cur],_T(">"))==0)
      {
         RemoveFlag=TRUE;
      }
      if(RemoveFlag)
      {
         RemoveParam(Params[Cur]);
      }
      else
      {
         ++Cur;
      }
   }

   return Params.size()>0;
}

size_t CCommandLine::GetLength()
{
   int BuffLen=0;
   for(auto a=Params.begin();a<Params.end();++a)
   {
      BuffLen+=_tcslen((*a))+1;
   }
   //BuffLen--;
   return BuffLen;
}

TCHAR * CCommandLine::operator()()
{
   if(!Params.size())
   {
      return NULL;
   }
   if(CmdBuf)
   {
      delete [] CmdBuf;
      CmdBuf=0;
   }
   size_t BuffLen=GetLength();
   CmdBuf=new TCHAR[BuffLen];
   /*
   int Len=0;
   for(auto a=Params.begin();a<Params.end();++a)
   {
      _tcscpy_s(&CmdBuf[Len],BuffLen-Len,(*a));
      Len+=_tcslen((*a));
      CmdBuf[Len]=_T(' ');
      Len++;
   }
   Len--;
   CmdBuf[Len]=_T('\0');
   */
   size_t TLen=(*this)(CmdBuf,BuffLen);
   return CmdBuf;
}

size_t CCommandLine::operator()(TCHAR *szParamsLine,size_t szInSymbols)
{
   if(!szParamsLine||!szInSymbols)
   {
      return NULL;
   }
   if(!Params.size())
   {
      *szParamsLine=_T('\0');
      return NULL;
   }
   ZeroMemory(szParamsLine,szInSymbols*sizeof(TCHAR));
   size_t Len=0;
   size_t ParamLen=NULL;
   for(auto a=Params.begin();a<Params.end();++a)
   {
      ParamLen=_tcslen((*a));
      if(Len+ParamLen+2>szInSymbols)
      {
         break;
      }
      
      if(szParamsLine[0])
      {
         szParamsLine[Len]=_T(' ');
         Len++;
      }
      //_tcscat_s(szParamsLine,ParamLen+2,(*a));
      _tcscpy_s(&szParamsLine[Len],ParamLen+1/*szInSymbols-Len*/,(*a));
      Len+=ParamLen;
   }
   //Len--;
   //CmdBuf[Len]=_T('\0');
   return _tcslen(szParamsLine);
}

TCHAR* CCommandLine::TrimSpaces(const TCHAR *Str)
{
   if(!Str||!*Str)
   {
      return NULL;
   }
   int len=_tcslen(Str);
   if(!len)
   {
      return NULL;
   }

   do 
   {
      if(
         _istspace(*Str)||
         *Str==_T(' ')||
         *Str==_T('\r')||
         *Str==_T('\n')||
         *Str==_T('\t'))
      {
         (Str)++;
         len--;
      }
      else
      {
         break;
      }
   } while (*Str||len); 

   if(!len)
   {
      return NULL;
   }

   const TCHAR *c=&Str[len-1];
   do 
   {
      if(
         _istspace(*c)||
         *c==_T(' ')||
         *c==_T('\r')||
         *c==_T('\n')||
         *c==_T('\t'))
      {
         //*c=_T('\0');
         c--;
         len--;
      }
      else
      {
         break;
      }
   } while (len||c>Str);

   TCHAR *Out=new TCHAR[len+1];
   memcpy_s(Out,(len+1)*sizeof(TCHAR),Str,(len)*sizeof(TCHAR));
   Out[len]=0;
   return Out;
}
