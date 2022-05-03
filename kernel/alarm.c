#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64 sys_sigalarm(void){
  int ticks;
  if(argint(0, &ticks) < 0)
    return -1;
  uint64 p;
  if(argaddr(1, &p) < 0)
    return -1;
  void (*handler)() = (void (*)())p;
  //printf("handler: %p\n", handler);
  struct proc* proc = myproc();
  proc->alarm_interval = ticks;
  proc->alarm_handler = handler;
  proc->alarm_ticks = 0;
  return 0;
}

uint64 sys_sigreturn(void){
  return 0;
}