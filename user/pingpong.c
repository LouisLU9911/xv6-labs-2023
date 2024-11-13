#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // [read, write]
    int p[2];
    pipe(p);

    if (fork() == 0) {
        // child
        char cBuf[1];

        // read from parent
        read(p[0], cBuf, 1);
        close(p[0]);

        int cPID = getpid();
        fprintf(1, "%d: received ping\n", cPID);

        // send back to parent
        write(p[1], cBuf, 1);
        close(p[1]);
    } else {
        // parent
        char pBuf[1];

        // send to child
        write(p[1], pBuf, 1);
        close(p[1]);

        // read from child
        read(p[0], pBuf, 1);
        close(p[0]);

        int pPID = getpid();
        fprintf(1, "%d: received pong\n", pPID);
  }
  exit(0);
}
