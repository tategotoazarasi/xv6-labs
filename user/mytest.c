#include "user.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    return -argc;
  }
  int status = symlink(argv[1], argv[2]);
  if(status!=0){
    exit(status);
  }
  //status = open(argv[1],0);
  //status = open(argv[2],0);
  //if(status!=0){
  //  exit(status);
  //}
  exit(0);
}