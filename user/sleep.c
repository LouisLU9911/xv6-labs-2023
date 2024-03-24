#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc <= 1) {
      char * errMsg = "Please input an integer.";
      write(2, errMsg, strlen(errMsg));
  }

  char * secStr = argv[1];
  int sec = atoi(secStr);
  sleep(sec);
  exit(0);
}
