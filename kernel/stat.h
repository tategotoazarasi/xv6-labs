#ifndef _STAT_H_
#define _STAT_H_
#include "types.h"
/// \brief Directory
#define T_DIR 1
/// \brief File
#define T_FILE 2
/// \brief Device
#define T_DEVICE 3

struct stat {
  int dev;     ///< File system's disk device
  uint ino;    ///< Inode number
  short type;  ///< Type of file
  short nlink; ///< Number of links to file
  uint64 size; ///< Size of file in bytes
};
#endif