/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/FileInformation.h"
#include "watchman/portability/WinError.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_time.h"

namespace watchman {

#ifndef _WIN32
FileInformation::FileInformation(const struct stat& st)
    : mode(st.st_mode),
      size(static_cast<uint64_t>(st.st_size)),
      uid(st.st_uid),
      gid(st.st_gid),
      ino(st.st_ino),
      dev(st.st_dev),
      nlink(st.st_nlink) {
  memcpy(&atime, &st.WATCHMAN_ST_TIMESPEC(a), sizeof(atime));
  memcpy(&mtime, &st.WATCHMAN_ST_TIMESPEC(m), sizeof(mtime));
  memcpy(&ctime, &st.WATCHMAN_ST_TIMESPEC(c), sizeof(ctime));
}
#endif

#ifdef _WIN32
FileInformation::FileInformation(uint32_t dwFileAttributes)
    : fileAttributes(dwFileAttributes) {
  if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
    mode = 0444;
  } else {
    mode = 0666;
  }
  if (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    // Report it as a symlink. This is used by source control
    // to detect symlinks.
    mode |= S_IFLNK;
  } else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    mode |= _S_IFDIR | 0111 /* executable/searchable */;
  } else {
    mode |= _S_IFREG;
  }
}
#endif

DType FileInformation::dtype() const {
#ifdef DTTOIF
  return static_cast<DType>(IFTODT(mode));
#else
  // Windows, Solaris
  if (isSymlink()) {
    return DType::Symlink;
  }
  if (isDir()) {
    return DType::Dir;
  }
  if (isFile()) {
    return DType::Regular;
  }
  return DType::Unknown;
#endif
}

bool FileInformation::isSymlink() const {
#ifdef _WIN32
  // We treat all reparse points as equivalent to symlinks
  return fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
#else
  return S_ISLNK(mode);
#endif
}

bool FileInformation::isDir() const {
#ifdef _WIN32
  // Note that junctions have both DIRECTORY and REPARSE_POINT set,
  // so we have to check both bits to make sure that we only report
  // this as a dir if it isn't a junction, otherwise we will fail to
  // opendir.
  return (fileAttributes &
          (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
      FILE_ATTRIBUTE_DIRECTORY;
#else
  return S_ISDIR(mode);
#endif
}

bool FileInformation::isFile() const {
#ifdef _WIN32
  // We can't simply test for FILE_ATTRIBUTE_NORMAL as that is only
  // valid when no other bits are set.  Instead we check for the absence
  // of DIRECTORY and REPARSE_POINT to decide that it is a regular file.
  return (fileAttributes &
          (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0;
#else
  return S_ISREG(mode);
#endif
}

FileInformation FileInformation::makeDeletedFileInformation() {
  FileInformation info;
#ifndef _WIN32
  info.mode = S_IFREG;
#else
  info.mode = _S_IFREG;
#endif
  return info;
}
} // namespace watchman

#ifdef _WIN32

bool w_path_exists(const char* path) {
  auto wpath = w_string_piece(path).asWideUNC();

  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &data)) {
    DWORD err = GetLastError();
    errno = map_win32_err(err);
    return false;
  }
  return true;
}

#endif
