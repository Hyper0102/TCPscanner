#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

#include "TASKPOOL.h"


using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

bool ResultTab[65536];

typedef struct 
{  unsigned long ip;
   unsigned short port;
   bool result;
} TASK_PARAM;

const int NTASK=64;
TASK_PARAM Param[NTASK];


#if 1
void Func(void *_p)
{  TASK_PARAM *p=(TASK_PARAM *)_p;
   unsigned long ip=p->ip;
   unsigned short port=p->port;

   SOCKET ConnectSocket = INVALID_SOCKET;
   struct sockaddr_in clientService; 

   ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (ConnectSocket == INVALID_SOCKET) 
   {  ResultTab[port]=false;
      return;
   }

   //----------------------
   // The sockaddr_in structure specifies the address family,
   // IP address, and port of the server to be connected to.
   clientService.sin_family = AF_INET;
   clientService.sin_addr.s_addr = htonl(ip);
   clientService.sin_port = htons( port );

   //----------------------
   // Connect to server.
   int iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
   if (iResult == SOCKET_ERROR) 
   {  closesocket(ConnectSocket);
      ResultTab[port]=false;
      return;
   }
   {
   }
   closesocket(ConnectSocket);
   ResultTab[port]=true;
   printf("open port %d\n",port);
}


TASKPOOL_FUNC<Func, NTASK> TaskPool;
#else
class TASK
{  public: 
   void TaskProc(void *_p)
   {  TASK_PARAM *p=(TASK_PARAM *)_p;
      for(int i=0; i<5000000; i++)
      {  p->count++;
      }
   }
};
TASKPOOL_CLASS<TASK, NTASK> TaskPool;

#endif

int main(int argc, char* argv[])
{ 
int ip[4];
   int port;

/*   
   int res=sscanf(argv[1], "%d.%d.%d.%d:%d", &ip[3], &ip[2], &ip[1], &ip[0],&port);
	if (res!=5)
	{  return 2;
	}
*/
   int res=sscanf(argv[1], "%d.%d.%d.%d:%d", &ip[3], &ip[2], &ip[1], &ip[0],&port);
   if (res<4)
	{  return 2;
	}

   int iResult;
   // Initialize Winsock
   WSADATA wsaData;
   iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
   if (iResult != NO_ERROR) 
   {  return false;
   }
	
   DWORD addr=(ip[3]<<24) | (ip[2]<<16) | (ip[1]<<8) | ip[0];

   for(int i=1; i<65536; i++)
   {  int ntask;
      bool res;
      res=TaskPool.WaitForAnyReady(&ntask,INFINITE);
      if (!res)
      {  printf("error\n");
         return 0;
      }
      Param[ntask].ip=addr;
      Param[ntask].port=i;
      TaskPool.Start(&Param[ntask]); 
   }
   //
    TaskPool.WaitForAllComplete(INFINITE);

   for(int i=1; i<65536; i++)
   {   if(ResultTab[i])
       { // printf("open port %d\n",i);
       }
   }

   printf("All done\n");
   return 0;
}

