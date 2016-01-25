#include "StdAfx.h"
#include "CompilerRunner.h"


CCompilerRunner::CCompilerRunner(void)
{
   memset(StartupDir,0,sizeof(StartupDir));
   memset(ExeName,0,sizeof(ExeName));
}


CCompilerRunner::~CCompilerRunner(void)
{
}

int CCompilerRunner::Run()
{
   if(ExeName[0]==_T('\0'))
   {
      return 1;
   }

   
   HANDLE g_hChildStd[ST_Max][H_Max];//={{NULL,NULL},{NULL,NULL}};
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   ZeroMemory(&g_hChildStd,sizeof(g_hChildStd));
   ZeroMemory( &si, sizeof(si) );
   ZeroMemory( &pi, sizeof(pi) );
	SECURITY_ATTRIBUTES saAttr; 

   auto Cleanup=[this,&pi,&si,&g_hChildStd]()
   {
      //������
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
	

	

	if( !CreateProcess( NULL, // ��� ����� ������ (������������ ��������� ������).
		Prog,     // ��������� ������.
		NULL,                 // ���������� �������� �� �����������.
		NULL,                 // ���������� ������ �� �����������.
		TRUE,                // ��������� ������������ ����������� � FALSE.
		0,                    // ������� �������� ���.
		NULL,                 // ����������� ���� ������������ ��������.
		NULL,                 // ����������� ��������� ������� ��������.
		&si,                  // ��������� �� ��������� STARTUPINFO.
		&pi )                 // ��������� �� ��������� PROCESS_INFORMATION.
		)
		{
			fprintf(stderr,"Error:Unable to start %s\n",Prog);
			Cleanup();
         return 1;
		}
     
     /*fwrite(Prog,sizeof(byte),strlen(Prog),stderr);
     fwrite("\r",sizeof(byte),strlen("\r"),stderr);*/
	unsigned long exit=0;  //��� ���������� ��������
	unsigned long bread;   //���-�� ����������� ����
	unsigned long avail;   //���-�� ��������� ����
	char *buf,*buf2;
	int strnum,strnum2;
	char *NumEnd,*NumEnd2;
     int EndPos;
	char *FileIncludedStr;
	int InFileIncStrLen=strlen("In file included from");
   bool WasStdErrUsed=false;
	for(;;)      //�������� ���� ���������
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

}
