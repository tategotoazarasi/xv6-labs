#include "kernel/fcntl.h"
#include "user.h"

#define N 4097

int main() {
  int fd = open("/mmap.dur", O_RDWR);
  void *p = mmap(0, N, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  // printf("%p\n", p);
  char *p2 = (char *)p;
  for (int j = 0; j < N; j++) {
    printf("%d ", p2[j]);
  }

  /*char content[1];
  for (int j = 0; j < 4096; j++) {
    read(fd, content, 1);
    printf("%d ", content[0]);
  }
  printf("\n");*/

  munmap(p, N);
  exit(0);
}