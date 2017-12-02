
#ifndef _TASKPOOL_H_
#define _TASKPOOL_H_

template <void __TaskFunc(void *p), int __NTASK>
class TASKPOOL_FUNC
{  private:
   
   typedef struct 
   {  HANDLE Done;
      HANDLE Active;
      void *Param;
   } TASKINFO;

   HANDLE HndlDone[__NTASK];
   TASKINFO TaskInfo[__NTASK];
   HANDLE ThreadHndl[__NTASK];


   static DWORD WINAPI Thread(  LPVOID lpParameter)
   {  TASKINFO *p=(TASKINFO *)lpParameter;
      while(1)
      {  WaitForSingleObject(p->Active, INFINITE);
         __TaskFunc(p->Param);
         SetEvent(p->Done);
      }
      return 0;
   }

   public:
   TASKPOOL_FUNC()
	{  for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].Active=CreateEvent(NULL,false,false,NULL);
         HndlDone[i]=CreateEvent(NULL,true,true,NULL);
         TaskInfo[i].Done=HndlDone[i];
      }
      for(int i=0; i<__NTASK; i++)
      {   DWORD tid; 
          ThreadHndl[i]=CreateThread(NULL, 0, Thread , &TaskInfo[i], 0, &tid);
      }
	}

   bool WaitForAnyReady(int *pNtask,  DWORD dwMilliseconds)
   {  int stop=1;
      DWORD n_res=WaitForMultipleObjects(__NTASK, HndlDone, false, dwMilliseconds);
		if ((n_res >= WAIT_OBJECT_0) && (n_res < WAIT_OBJECT_0+__NTASK))
		{  DWORD ntask=n_res - WAIT_OBJECT_0;
         ResetEvent(HndlDone[n_res]);
         *pNtask = n_res;
         return true;
		}
		else
		{	return false;
		}
   }

   void WaitForAllComplete(DWORD dwMilliseconds)
	{	WaitForMultipleObjects(__NTASK, HndlDone, true, dwMilliseconds); 
	}

   void Start(int ntask, void *p)
   {  TaskInfo[ntask].Param=p;
      SetEvent(TaskInfo[ntask].Active);
   }

	~TASKPOOL_FUNC()
	{  for(int i=0; i<__NTASK; i++)
		{  CloseHandle(ThreadHndl[i]);
         CloseHandle(TaskInfo[i].Done);
         CloseHandle(TaskInfo[i].Active);
		}
	}
};


//-----------------------------------------------------------------------//

template <class _class_T, int __NTASK>
class TASKPOOL_CLASS
{  public:
   
   typedef struct 
   {  HANDLE Done;
      HANDLE Active;
      void *Param;
     _class_T Obj;  
   } TASKINFO;

   HANDLE HndlDone[__NTASK];
   
   HANDLE ThreadHndl[__NTASK];
   
   public:
   TASKINFO TaskInfo[__NTASK];

   private:

   static DWORD WINAPI Thread( LPVOID lpParameter)
   {  TASKINFO *p=(TASKINFO *)lpParameter;
      while(1)
      {  WaitForSingleObject(p->Active, INFINITE);
         p->Obj.TaskProc(p->Param);
         SetEvent(p->Done);
      }
      return 0;
   }

   public:
   TASKPOOL_CLASS()
	{  for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].Active=CreateEvent(NULL,false,false,NULL);
         HndlDone[i]=CreateEvent(NULL,true,true,NULL);
         TaskInfo[i].Done=HndlDone[i];
      }
      for(int i=0; i<__NTASK; i++)
      {   DWORD tid; 
          ThreadHndl[i]=CreateThread(NULL, 0, Thread , &TaskInfo[i], 0, &tid);
          if (ThreadHndl[i]==NULL)
          {  int err=GetLastError();
             printf("%s %d:CreateThread Error=%d i=%d\n",__FILE__,__LINE__, err, i);
          }
      }
	}

   bool WaitForAnyReady(int *pNtask,  DWORD dwMilliseconds)
   {  int stop2=1;
      DWORD n_res=WaitForMultipleObjects(__NTASK, HndlDone, false, dwMilliseconds);
		if ((n_res >= WAIT_OBJECT_0) && (n_res < WAIT_OBJECT_0+__NTASK))
		{  DWORD ntask=n_res - WAIT_OBJECT_0;
         ResetEvent(HndlDone[n_res]);
         *pNtask = n_res;
         return true;
		}
		else
		{	return false;
		}
   }

   void WaitForAllComplete(DWORD dwMilliseconds)
	{	WaitForMultipleObjects(__NTASK, HndlDone, true, dwMilliseconds); 
	}

   void Start(int ntask, void *p)
   {  int stop=3;
      TaskInfo[ntask].Param=p;
      SetEvent(TaskInfo[ntask].Active);
   }

	~TASKPOOL_CLASS()
	{  for(int i=0; i<__NTASK; i++)
		{  CloseHandle(ThreadHndl[i]);
         CloseHandle(TaskInfo[i].Done);
         CloseHandle(TaskInfo[i].Active);
		}
	}
};


#endif
