#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc<2){
    write(1,"no argument",12);
    exit(1);
  }
  int tick = atoi(argv[1]);
  sleep(tick);
  exit(0);
}