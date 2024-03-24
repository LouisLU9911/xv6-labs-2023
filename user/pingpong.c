#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pP2C[2];
  int pC2P[2];
  pipe(pP2C);
  pipe(pC2P);

  if (fork() == 0) {
    char cBuf[1];
    close(pP2C[1]);
    read(pP2C[0], cBuf, 1);
    close(pP2C[0]);
    int cPid = getpid();
    fprintf(1, "%d: received ping\n", cPid);
    close(pC2P[0]);
    write(pC2P[1], cBuf, 1);
    close(pC2P[1]);
  } else {
    char pBuf[1];
    close(pP2C[0]);
    write(pP2C[1], "\n", 1);
    close(pP2C[1]);
    close(pC2P[1]);
    read(pC2P[0], pBuf, 1);
    close(pC2P[0]);
    int pPid = getpid();
    fprintf(1, "%d: received pong\n", pPid);
  }
  exit(0);
}
