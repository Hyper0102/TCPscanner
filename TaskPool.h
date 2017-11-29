#ifndef _TASKPOOL_H_
#define _TASKPOOL_H_

template <void __TaskFunc(void *p), int __NTASK>
class TASKPOOL_FUNC
{  private:
   
   typedef struct 
   {  void *Param;
      int Number;
      TASKPOOL_FUNC<__TaskFunc, __NTASK> *TaskPool;
      CRITICAL_SECTION Active;
   } TASKINFO;

   TASKINFO TaskInfo[__NTASK];

   CRITICAL_SECTION Select;
   HANDLE Wait;
   int Number;
   HANDLE Send;
   HANDLE Done;
   HANDLE ThreadHndl[__NTASK];
      
   static DWORD WINAPI TaskThread(  LPVOID lpParameter)
   {  TASKINFO *pInfo=(TASKINFO *) lpParameter;
      TASKPOOL_FUNC<__TaskFunc, __NTASK> *pTaskPool=pInfo->TaskPool;

      while(1)
      {  EnterCriticalSection(&pTaskPool->Select);
         pTaskPool->Number=pInfo->Number;
         //SetEvent(pTaskPool->Wait);
         ReleaseSemaphore(pTaskPool->Wait,1,NULL);

         WaitForSingleObject(pTaskPool->Send, INFINITE);
         EnterCriticalSection(&pInfo->Active);
         SetEvent(pTaskPool->Done);
         LeaveCriticalSection(&pTaskPool->Select);
         __TaskFunc(pInfo->Param);
         LeaveCriticalSection(&pInfo->Active);
      }

      return 0;
   }

   public:
   TASKPOOL_FUNC()
	{  
	   InitializeCriticalSection(&Select);
      //Wait=CreateEvent(NULL,false,false,NULL);
      Wait=CreateSemaphore(NULL,0, 10000, NULL);
      Send=CreateEvent(NULL,false,false,NULL);
      Done=CreateEvent(NULL,false,false,NULL);

      for(int i=0; i<__NTASK; i++)
      {  InitializeCriticalSection(&TaskInfo[i].Active);
      }
      
      for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].TaskPool=this;
      } 

      for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].Number=i;
      } 
      
      for(int i=0; i<__NTASK; i++)
      {   DWORD tid; 
          ThreadHndl[i]=CreateThread(NULL, 0, TaskThread , &TaskInfo[i], 0, &tid);
      }
	}

   bool WaitForAnyReady(int *pNtask)
   {  DWORD n_res=WaitForSingleObject(Wait, INFINITE);
      
		if (n_res == WAIT_OBJECT_0 )
		{  if (pNtask!=NULL)
         {  *pNtask = Number;
         }
         return true;
		}
		else
		{	return false;
		}
   }

   void WaitForAllComplete()
	{	for(int i=0; i<__NTASK; i++)
      {  EnterCriticalSection(&TaskInfo[i].Active);
      }
      for(int i=0; i<__NTASK; i++)
      {  LeaveCriticalSection(&TaskInfo[i].Active);
      }
	}

   void Start(void *p)
   {  TaskInfo[Number].Param=p;
      SetEvent(Send);
      WaitForSingleObject(Done, INFINITE);
   }

	~TASKPOOL_FUNC()
	{  
      for(int i=0; i<__NTASK; i++)
		{  CloseHandle(ThreadHndl[i]);
		}

      for(int i=0; i<__NTASK; i++)
      {  DeleteCriticalSection(&TaskInfo[i].Active);
      }

      CloseHandle(Done);
      CloseHandle(Send);
      CloseHandle(Wait);
      DeleteCriticalSection(&Select);
	}
};


//--------------------------------------//

template <class _class_T, int __NTASK>
class TASKPOOL_CLASS
{  private:
   
   typedef struct 
   {  void *Param;
      int Number;
      _class_T Obj;
      TASKPOOL_CLASS<_class_T, __NTASK> *TaskPool;
      CRITICAL_SECTION Active;
   } TASKINFO;

   TASKINFO TaskInfo[__NTASK];

   CRITICAL_SECTION Select;
   HANDLE Wait;
   int Number;
   HANDLE Send;
   HANDLE Done;

   HANDLE ThreadHndl[__NTASK];
      
   static DWORD WINAPI TaskThread(  LPVOID lpParameter)
   {  TASKINFO *pInfo=(TASKINFO *) lpParameter;
      TASKPOOL_CLASS<_class_T, __NTASK> *pTaskPool=pInfo->TaskPool;

      while(1)
      {  EnterCriticalSection(&pTaskPool->Select);
         pTaskPool->Number=pInfo->Number;
         SetEvent(pTaskPool->Wait);
         WaitForSingleObject(pTaskPool->Send, INFINITE);
         EnterCriticalSection(&pInfo->Active);
         SetEvent(pTaskPool->Done);
         LeaveCriticalSection(&pTaskPool->Select);
         pInfo->Obj.TaskProc(pInfo->Param);
         LeaveCriticalSection(&pInfo->Active);
      }

      return 0;
   }

   public:
   TASKPOOL_CLASS()
	{  
	   InitializeCriticalSection(&Select);
      Wait=CreateEvent(NULL,false,false,NULL);
      Send=CreateEvent(NULL,false,false,NULL);
      Done=CreateEvent(NULL,false,false,NULL);

      for(int i=0; i<__NTASK; i++)
      {  InitializeCriticalSection(&TaskInfo[i].Active);
      }
      
      for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].TaskPool=this;
      } 

      for(int i=0; i<__NTASK; i++)
      {  TaskInfo[i].Number=i;
      } 
      
      for(int i=0; i<__NTASK; i++)
      {   DWORD tid; 
          ThreadHndl[i]=CreateThread(NULL, 0, TaskThread , &TaskInfo[i], 0, &tid);
      }
	}

   bool WaitForAnyReady(int *pNtask)
   {  DWORD n_res=WaitForSingleObject(Wait, INFINITE);
      
		if (n_res == WAIT_OBJECT_0 )
		{  if (pNtask!=NULL)
         {  *pNtask = Number;
         }
         return true;
		}
		else
		{	return false;
		}
   }

   void WaitForAllComplete()
	{	for(int i=0; i<__NTASK; i++)
      {  EnterCriticalSection(&TaskInfo[i].Active);
      }
      for(int i=0; i<__NTASK; i++)
      {  LeaveCriticalSection(&TaskInfo[i].Active);
      }
	}

   void Start(void *p)
   {  TaskInfo[Number].Param=p;
      SetEvent(Send);
      WaitForSingleObject(Done, INFINITE);
   }

	~TASKPOOL_CLASS()
	{  
      for(int i=0; i<__NTASK; i++)
		{  CloseHandle(ThreadHndl[i]);
		}

      for(int i=0; i<__NTASK; i++)
      {  DeleteCriticalSection(&TaskInfo[i].Active);
      }

      CloseHandle(Done);
      CloseHandle(Send);
      CloseHandle(Wait);
      DeleteCriticalSection(&Select);
	}
};

#endif

