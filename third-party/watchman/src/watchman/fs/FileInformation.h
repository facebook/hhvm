/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/stat.h>
#include "watchman/watchman_system.h"

#ifndef _WIN32
#include <dirent.h>
#endif

namespace watchman {

#ifdef _WIN32
using mode_t = int;
using dev_t = int;
using gid_t = int;
using uid_t = int;
using ino_t = unsigned int;
using nlink_t = unsigned int;

/**
 * Convertion between st_mode and d_type on Windows. On Windows the 4th nibble
 * of mode contains the type of directory entry. Right shifting by 12 bits to
 * form a d_type.
 */
static_assert(S_IFMT == 0xF000, "The S_IFMT on Windows should be 0xF000");

#define DT_UNKNOWN 0
#define DT_FIFO ((_S_IFIFO) >> 12)
#define DT_CHR ((_S_IFCHR) >> 12)
#define DT_DIR ((_S_IFDIR) >> 12)
#define DT_REG ((_S_IFREG) >> 12)
#define S_IFLNK 0xA000

#endif

/** Represents the type of a filesystem entry.
 *
 * This is the same type and intent as the d_type field of a dirent struct.
 *
 * We provide an explicit type to make it clearer when we're working
 * with this value.
 *
 * https://www.daemon-systems.org/man/DTTOIF.3.html
 *
 * Not all systems have a dtype concept so we have some conditional
 * code here to compensate.
 */
enum class DType {
  Unknown = DT_UNKNOWN,
  Fifo = DT_FIFO,
  Char = DT_CHR,
  Dir = DT_DIR,
  Regular = DT_REG,

#ifdef DT_BLK
  Block = DT_BLK,
#else
  Block,
#endif

#ifdef DT_LNK
  Symlink = DT_LNK,
#else
  Symlink,
#endif

#ifdef DT_SOCK
  Socket = DT_SOCK,
#else
  Socket,
#endif

#ifdef DT_WHT
  Whiteout = DT_WHT,
#else
  Whiteout,
#endif
};

struct FileInformation {
  // On POSIX systems, the complete mode information.
  // On Windows, this is lossy wrt. symlink information,
  // so it is preferable to use isSymlink() rather than
  // S_ISLNK() on the mode value.
  mode_t mode{0};
  uint64_t size{0};

  // On Windows systems, these fields are approximated
  // from cheaply available information in a way that is
  // consistent with msvcrt which is widely used by many
  // native win32 applications (including python).
  uid_t uid{0};
  gid_t gid{0};
  ino_t ino{0};
  dev_t dev{0};
  nlink_t nlink{0};

#ifdef _WIN32
  uint32_t fileAttributes{0};
#endif

  struct timespec atime {
    0, 0
  };
  struct timespec mtime {
    0, 0
  };
  struct timespec ctime {
    0, 0
  };

  // Returns the directory entry type for the file.
  DType dtype() const;

  // Returns true if this file information references
  // a symlink, false otherwise.
  bool isSymlink() const;

  // Returns true if this file information references
  // a directory, false otherwise.
  bool isDir() const;

  // Returns true if this file information references
  // a regular file, false otherwise.
  bool isFile() const;

#ifndef _WIN32
  explicit FileInformation(const struct stat& st);
#else
  // Partially initialize the common fields.
  // There are a number of different forms of windows specific data
  // types that hold the rest of the information and we don't want
  // to pollute the headers with them, so those are populated
  // externally by the APIs declared elsewhere in this header file.
  explicit FileInformation(uint32_t dwFileAttributes);
#endif
  FileInformation() = default;

  // Construct a placeholder FileInformation instance that represents
  // a file that has been deleted.  This is used in a very specific
  // circumstance in Source Control Aware query responses to represent
  // files that were deleted between two revisions.
  static FileInformation makeDeletedFileInformation();
};

} // namespace watchman

#ifndef _WIN32
static inline bool w_path_exists(const char* path) {
  return access(path, F_OK) == 0;
}
#else
bool w_path_exists(const char* path);
#endif
