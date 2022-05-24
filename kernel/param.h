#ifndef _PARAM_H_
#define _PARAM_H_
/// \brief maximum number of processes
#define NPROC 64
/// \brief maximum number of CPUs
#define NCPU 8
/// \brief open files per process
#define NOFILE 16
/// \brief open files per system
#define NFILE 100
/// \brief maximum number of active i-nodes
#define NINODE 50
/// \brief maximum major device number
#define NDEV 10
/// \brief device number of file system root disk
#define ROOTDEV 1
/// \brief max exec arguments
#define MAXARG 32
/// \brief max # of blocks any FS op writes
#define MAXOPBLOCKS 10
/// \brief max data blocks in on-disk log
#define LOGSIZE (MAXOPBLOCKS * 3)
/// \brief size of disk block cache
#define NBUF (MAXOPBLOCKS * 3)
/// \brief size of file system in blocks
#define FSSIZE 1000
/// \brief maximum file path name
#define MAXPATH 128
#endif