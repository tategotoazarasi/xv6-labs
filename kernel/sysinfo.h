#ifndef SYSINFO_H
#define SYSINFO_H
struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of process
};
#endif