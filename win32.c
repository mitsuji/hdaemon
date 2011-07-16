#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <HsFFI.h>

#include "HDaemon/Foreign_stub.h"

int g_argc;
char** g_argv;

char* SERVICE_NAME = "HDaemon";
SERVICE_STATUS_HANDLE serviceStatusHandle;
HANDLE terminateEvent = NULL;

extern void __stginit_HDaemonziForeign(void);
HsStablePtr handle = NULL;



/**
 *
 */
void alloc_args(int f_argc, char** f_argv, char*** t_argv )
{
  int i;

  (*t_argv) = (char**)malloc( sizeof(char*) * f_argc);

  for( i=0; i<f_argc; i++)
    {
      (*t_argv)[i] = (char*)malloc( sizeof(char) * ( strlen(f_argv[i]) +1 ) );
      strcpy((*t_argv)[i], f_argv[i]);
    }

}


/**
 *
 */
void free_args(int argc, char** argv)
{
  int i; 

  for(i=0; i< argc; i++)
    {
      free(argv[i]);
    }

  free(argv);
}


/**
 *
 */
BOOL InitService()
{
   
  hs_init(&g_argc, &g_argv);
  hs_add_root(__stginit_HDaemonziForeign);

  handle = foreignStart();

  return TRUE;
  
}


/**
 *
 */
VOID StopService()
{
  foreignStop(handle);

  hs_exit();

  SetEvent(terminateEvent);
}


/**
 *
 */
BOOL SendStatusToSCM (
		      DWORD dwCurrentState,
		      DWORD dwWin32ExitCode,
		      DWORD dwServiceSpecificExitCode,
		      DWORD dwCheckPoint,
		      DWORD dwWaitHint)
{
  BOOL success;
  SERVICE_STATUS serviceStatus;

  serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  serviceStatus.dwCurrentState = dwCurrentState;
  
  if (dwCurrentState == SERVICE_START_PENDING)
    serviceStatus.dwControlsAccepted = 0;
  else
    serviceStatus.dwControlsAccepted =
       SERVICE_ACCEPT_STOP
      |SERVICE_ACCEPT_PAUSE_CONTINUE
      |SERVICE_ACCEPT_SHUTDOWN
      ;

  if (dwServiceSpecificExitCode == 0)
    serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
  else
    serviceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;

  serviceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
  serviceStatus.dwCheckPoint = dwCheckPoint;
  serviceStatus.dwWaitHint = dwWaitHint;
  
  success = SetServiceStatus (serviceStatusHandle, &serviceStatus);
  if (!success)
    StopService();

  return success;
}



/**
 *
 */
VOID ServiceCtrlHandler (DWORD controlCode)
{
  DWORD currentState = 0;
  BOOL success;

  switch(controlCode)
    {
    case SERVICE_CONTROL_STOP:
      currentState = SERVICE_STOP_PENDING;
      success = SendStatusToSCM(SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 5000);
      StopService();
      return;

    case SERVICE_CONTROL_INTERROGATE:
      break;

    case SERVICE_CONTROL_SHUTDOWN:
      return;

    default:
      break;
    }

  SendStatusToSCM(currentState, NO_ERROR, 0, 0, 0);

}


/**
 *
 */
VOID terminate(DWORD error)
{
  if (terminateEvent)
    CloseHandle(terminateEvent);

  if (serviceStatusHandle)
    SendStatusToSCM(SERVICE_STOPPED, error, 0, 0, 0);
  
  free_args(g_argc, g_argv);

}


/**
 *
 */
VOID ServiceMain(DWORD argc, LPTSTR *argv)
{
  BOOL success;
  
  serviceStatusHandle
    = RegisterServiceCtrlHandler( SERVICE_NAME,
				  (LPHANDLER_FUNCTION)ServiceCtrlHandler );

  if (!serviceStatusHandle)
    {
      terminate(GetLastError());
      return;
    }
  
  success = SendStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 1, 5000);
  if (!success)
    {
      terminate(GetLastError());
      return;
    }

  terminateEvent = CreateEvent (0, TRUE, FALSE, 0);
  if (!terminateEvent)
    {
      terminate(GetLastError());
      return;
    }
  
  success = SendStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 2, 1000);
  if (!success)
    {
      terminate(GetLastError());
      return;
    }
  
  success = InitService(argc, argv);
  if (!success)
    {
      terminate(GetLastError());
      return;
    }
  
  success = SendStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 0, 0);
  if (!success)
    {
      terminate(GetLastError());
      return;
    }
  
  
  WaitForSingleObject (terminateEvent, INFINITE);
  terminate(0);
}


/**
 *
 *
 */
VOID main(int argc, char* argv[])
{
  SERVICE_TABLE_ENTRY serviceTable[] =
    {
      { SERVICE_NAME,
	(LPSERVICE_MAIN_FUNCTION) ServiceMain},
      { NULL, NULL }
    };

  // preserve argv for hs_init
  g_argc = argc;
  alloc_args(argc, argv, &g_argv);

  if (!StartServiceCtrlDispatcher(serviceTable))
    {
      printf("Error: StartServiceCtrlDispatcher");
      ExitProcess(GetLastError());
    }

}
