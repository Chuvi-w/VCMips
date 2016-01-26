#include "StdAfx.h"
#include "ProcessRunner.h"


CProcessRunner::CProcessRunner(void)
{
  Reset();
}


CProcessRunner::~CProcessRunner(void)
{
}

BOOL CProcessRunner::Reset()
{
   ZeroVar(StartupDir);
   ZeroVar(ExePath);
   ZeroVar(ExeFullPath);
   return TRUE;
}

BOOL CProcessRunner::SetStartupDir(const TCHAR *Dir)
{
   if(!Dir||!Dir[0])
   {
      _tgetcwd(StartupDir,MAX_PATH-1);
      return TRUE;
   }
   if(PathIsRelative(Dir))
   {
      TCHAR TmpPath[MAX_PATH];
      _tgetcwd(TmpPath,MAX_PATH-1);
      PathAppend(TmpPath,Dir);
      PathCanonicalize(StartupDir,TmpPath);
   }

   if(!FS.DirExists(Dir))
   {
      _ftprintf(stderr, _T("Directory \"%s\" not exists.\n"),Dir);
      return FALSE;
   }
   _tcscpy_s(StartupDir,Dir);
   return TRUE;
}

BOOL CProcessRunner::SetProgram(const TCHAR *Prog)
{
   if(!Prog)
   {
      return FALSE;
   }
   if(!PathIsRelative(Prog))
   {
      if(!PathFileExists(Prog))
      {
         _ftprintf(stderr, _T("Path \"%s\" not exists.\n"),Prog);
         return FALSE;
      }
      _tcscpy_s(ExePath,Prog);
      _tcscpy_s(ExeFullPath,Prog);
   }
   else
   {
      _tcscpy_s(ExePath,Prog);
   }
   return TRUE;
}

int CProcessRunner::Run()
{
  
   if(!ExePath[0])
   {
      _ftprintf(stderr, _T("ExePath is NULL!!!\n"));
      return 1;
   }
   BOOL ExePathContainsSeparator=FALSE;
   size_t ExePathSeparatorPos=NULL;
   TCHAR tmpChar;
   ExePathContainsSeparator=FS.PathContainsSeparator(ExePath,&ExePathSeparatorPos);

   if(!StartupDir[0])
   {
      SetStartupDir(NULL);
   }
   HANDLE g_hChildStd[ST_Max][H_Max];//={{NULL,NULL},{NULL,NULL}};
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   ZeroMemory(&g_hChildStd,sizeof(g_hChildStd));
   ZeroMemory( &si, sizeof(si) );
   ZeroMemory( &pi, sizeof(pi) );
	SECURITY_ATTRIBUTES saAttr; 
   TCHAR *FullCmdLine=NULL;
   size_t FullCmdLineLen=NULL;
   auto Cleanup=[this,&pi,&si,&g_hChildStd,&FullCmdLine]()
   {
      //уборка
      if(pi.hThread)
      {
         CloseHandle(pi.hThread);
         pi.hThread=NULL;
      }
      if(pi.hProcess)
      {
          CloseHandle(pi.hProcess);
          pi.hProcess=NULL;
      }
      
      for(int st=0;st<ST_Max;st++)
      {
         for(int ht=0;ht<H_Max;ht++)
         {
            if(g_hChildStd[st][ht])
            {
               CloseHandle(g_hChildStd[st][ht]);
               g_hChildStd[st][ht]=NULL;
            }
         }
      }
      if(FullCmdLine)
      {
         delete [] FullCmdLine;
         FullCmdLine=NULL;
      }
   };

   
	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 
   if (!CreatePipe(&g_hChildStd[ST_Out][H_Read], &g_hChildStd[ST_Out][H_Write], &saAttr, 0))
   {
      _ftprintf(stderr, _T("Failed to create StdOut pipe.\n"));
      Cleanup();
      return 1;
   }

   if ( ! CreatePipe(&g_hChildStd[ST_Err][H_Read], &g_hChildStd[ST_Err][H_Write], &saAttr, 0) ) 
   {
      _ftprintf(stderr, _T("Failed to create StdErr pipe.\n"));
      Cleanup();
      return 1;
   }

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(g_hChildStd[ST_Out][H_Read], HANDLE_FLAG_INHERIT, 0) )
   {
      _ftprintf(stderr, _T("Failed to SetHandleInformation to StdOut pipe.\n"));
      Cleanup(); 
      return 1;
   }
	
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(g_hChildStd[ST_Err][H_Read], HANDLE_FLAG_INHERIT, 0) )
   {
      _ftprintf(stderr, _T("Failed to SetHandleInformation to StdErr pipe.\n"));
      Cleanup(); 
      return 1;
   }
	

	si.cb = sizeof(si);
	si.hStdError = g_hChildStd[ST_Err][H_Write];
	si.hStdOutput = g_hChildStd[ST_Out][H_Write];
	si.dwFlags |= STARTF_USESTDHANDLES;
	
#if 0
	

	if( !CreateProcess( NULL, // Нет имени модуля (используется командная строка).
		Prog,     // Командная строка.
		NULL,                 // Дескриптор процесса не наследуемый.
		NULL,                 // Дескриптор потока не наследуемый.
		TRUE,                // Установим наследование дескриптора в FALSE.
		0,                    // Флажков создания нет.
		NULL,                 // Используйте блок конфигурации родителя.
		NULL,                 // Используйте стартовый каталог родителя.
		&si,                  // Указатель на структуру STARTUPINFO.
		&pi )                 // Указатель на структуру PROCESS_INFORMATION.
		)
		{
			fprintf(stderr,"Error:Unable to start %s\n",Prog);
			Cleanup();
         return 1;
		}
     
     /*fwrite(Prog,sizeof(byte),strlen(Prog),stderr);
     fwrite("\r",sizeof(byte),strlen("\r"),stderr);*/
	unsigned long exit=0;  //код завершения процесса
	unsigned long bread;   //кол-во прочитанных байт
	unsigned long avail;   //кол-во доступных байт
	char *buf,*buf2;
	int strnum,strnum2;
	char *NumEnd,*NumEnd2;
     int EndPos;
	char *FileIncludedStr;
	int InFileIncStrLen=strlen("In file included from");
   bool WasStdErrUsed=false;
	for(;;)      //основной цикл программы
	{
		GetExitCodeProcess(pi.hProcess,&exit);
		if (exit != STILL_ACTIVE)
			break;

		PeekNamedPipe(g_hChildStd_ERR_Rd,nullptr,NULL,&bread,&avail,NULL);
          int len;
		if (avail != 0)
		{
			buf=new char[avail+1];
			ReadFile(g_hChildStd_ERR_Rd,buf,avail,&bread,NULL);
               buf[avail]=0;
			for(unsigned long i=0;i<avail-1;i++)
			{
				 if(buf[i]==':')
				 {
				   strnum=strtoul(&(buf[i+1]),&NumEnd,10);
				   if(&(buf[i+1])<NumEnd)
                       {
                            //avail+=2;
                            strnum2=strtoul(&(NumEnd[1]),&NumEnd2,10);  
                            buf[i]='\0';
                            if(NumEnd2>NumEnd&&strnum2)
                            {
                                 avail=printf("%s(%i,%i)%s",buf,strnum,strnum2,NumEnd2)+1;
                                 buf2=new char[avail];
                                 len=sprintf_s(buf2,avail,"%s(%i,%i)%s",buf,strnum,strnum2,NumEnd2);
                                 EndPos=(NumEnd2-buf);

                            }
                            else
                            {
                                 avail=printf("%s(%i)%s",buf,strnum,NumEnd)+1;
                                 buf2=new char[avail];
                                 len=sprintf_s(buf2,avail,"%s(%i)%s",buf,strnum,NumEnd);
                                 EndPos=(NumEnd-buf);
                            }
                            
                           
                           
                            delete [] buf;
                            buf=buf2;
                            i=EndPos+1;
                       }
                       /*
				   {
					   buf[i]='(';
					   *NumEnd=')';
                            avail++;
                            buf2=new char[avail];
                            memcpy(buf2,buf,NumEnd-buf+1);
                            buf2[NumEnd-buf+1]=':';
                            EndPos=NumEnd-buf+1;
                            memcpy((char*)((long)buf2+(long)NumEnd-(long)buf+2),(char*)((long)NumEnd+1),avail-(NumEnd-buf)-1);
                            delete [] buf;
                            buf=new char[avail];
                            memcpy(buf,buf2,avail);
                            delete [] buf2;
                         //   i=EndPos;
                            //strnum=strtoul(&(buf[EndPos+1]),&NumEnd,10);
					   // *(NumEnd+1)=':';
				   }
                       */
				 }
				 if(buf[i]=='\r'&&buf[i+1]=='\n')
				 {
					memcpy(&(buf[i]),&(buf[i+1]),avail-i-1);
					avail--;
				 }
 
				 FileIncludedStr=strstr(&(buf[i]),"In file included from ");
				 if(FileIncludedStr==&(buf[i]))
				 {
				   buf[i+InFileIncStrLen]='\n';
				 }
			}
            fprintf(stderr,"_____________________________________\rSTART: %s\r",Prog);
			fwrite(buf,sizeof(byte),avail,stderr);
               WasStdErrUsed=true;
			delete [] buf;	
			
		}

		

		PeekNamedPipe(g_hChildStd_OUT_Rd,NULL,NULL,&bread,&avail,NULL);
		if (avail != 0)
		{
			buf=new char[avail];
			ReadFile(g_hChildStd_OUT_Rd,buf,avail,&bread,NULL);
			for(unsigned long i=0;i<avail-1;i++)
			{
				if(buf[i]=='\r'&&buf[i+1]=='\n')
				{
					memcpy(&(buf[i]),&(buf[i+1]),avail-i-1);
					avail--;
				}
			}
			fwrite(buf,sizeof(byte),avail,stdout);
			delete [] buf;
		}
	}

	 Cleanup();
    if(WasStdErrUsed)
        fprintf(stderr,"\rFinished with code: %i\r___________________________________________________\r",exit);

    
	return exit;
#endif
   return 0;
}
