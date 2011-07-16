#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <HsFFI.h>

#include "HDaemon/Daemon_stub.h"

extern void __stginit_HDaemonziDaemon(void);

int save_pid()
{
  pid_t pid = getpid();
  FILE* file = fopen("/var/run/hdaemon.pid","w");
  char buff[20];
  sprintf(buff,"%d",pid);
  fputs(buff,file);
  fclose(file);
  return 0;
}


int main(int argc, char* argv[])
{
  int r = daemon(0,0);
  int p = save_pid();

  hs_init(&argc, &argv);
  hs_add_root(__stginit_HDaemonziDaemon);

  foreignMain();

  hs_exit();

}

