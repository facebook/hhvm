/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_file.cc
*/

#include "my_config.h"

#include <string.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "sql/malloc_allocator.h"

#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/my_static.h"
#include "mysys/mysys_priv.h"
#include "sql/malloc_allocator.h"
#include "sql/stateless_allocator.h"
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h> /* RLIMIT_NOFILE */
#endif

namespace {
/**
  Set the OS limit on the number of open files. On POSIX systems this
  calls setrlimit(RLIMIT_NOFILE, ...). On Windows there is no
  corresponding api so the requested value is returned. The assumption
  being that the request will never be larger than OS_FILE_LIMIT, @see
  my_set_max_open_files.

  @param max_file_limit	Files to open

  @note The request may not fulfilled because of system limitations

  @return Files available to open. May be more or less than max_file_limit!
*/

uint SetOsLimitMaxOpenFiles(uint max_file_limit) {
  DBUG_TRACE;

#ifndef _WIN32
  rlimit existing;
  if (getrlimit(RLIMIT_NOFILE, &existing) == -1) {
    DBUG_PRINT("warning", ("getrlimit(RLIMIT_NOFILE) failed: %s (%d)",
                           strerror(errno), errno));
    return max_file_limit;
  }

  // If rlim_cur is larger than what is requested, we use that
  // instead, but capped to the largest value an uint can hold,
  // (rlim_t can be 64 bit).
  if (existing.rlim_cur >= max_file_limit) {
    constexpr const rlim_t uim = std::numeric_limits<uint>::max();
    return std::min(existing.rlim_cur, uim);
  }

  // Attempt to modify OS setting
  rlimit request;
  request.rlim_cur = max_file_limit;
  request.rlim_max = max_file_limit;
  if (setrlimit(RLIMIT_NOFILE, &request) == -1) {
    DBUG_PRINT("warning", ("setrlimit(RLIMIT_NOFILE)=%u failed: %s (%d)",
                           max_file_limit, strerror(errno), errno));
    return existing.rlim_cur; /* Use original value */
  }

#ifndef DBUG_OFF
  // Read back new value to check "what we got". Seems overly
  // pessimistic to assume that a successful setrlimit did not
  // actually set the requested values.
  rlimit readback;
  if (getrlimit(RLIMIT_NOFILE, &readback) == -1) {
    DBUG_PRINT("warning",
               ("getrlimit(RLIMIT_NOFILE) (after set)  failed: %s (%d)",
                strerror(errno), errno));
    return max_file_limit;
  }
  DBUG_ASSERT(readback.rlim_cur == request.rlim_cur &&
              readback.rlim_max == readback.rlim_max);
#endif /* DBUG_OFF */
  return request.rlim_cur;
#else  /* not defined(_WIN32) */
  // We don't know the limit.
  DBUG_ASSERT(max_file_limit <= OS_FILE_LIMIT);
  return max_file_limit;
#endif /* not defined _WIN32 */
}

/**
   Rule of 5 class.
   @see https://en.cppreference.com/w/cpp/language/rule_of_three
   @see https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-five
*/
class FileInfo {
  const char *m_name = nullptr;
  file_info::OpenType m_type = file_info::OpenType::UNOPEN;

 public:
  FileInfo() = default;

  FileInfo(const char *n, file_info::OpenType t)
      : m_name{my_strdup(key_memory_my_file_info, n,
                         MYF(MY_WME | ME_FATALERROR))},
        m_type{t} {}

  // Rule of 5 (2)
  FileInfo(const FileInfo &) = delete;

  // Rule of 5 (4)
  FileInfo(FileInfo &&src) noexcept
      : m_name{std::exchange(src.m_name, nullptr)},
        m_type{std::exchange(src.m_type, file_info::OpenType::UNOPEN)} {}

  // Rule of 5 (1)
  ~FileInfo() { my_free(const_cast<char *>(m_name)); }

  // Rule of 5 (3)
  FileInfo &operator=(const FileInfo &) = delete;

  // Rule of 5 (5)
  FileInfo &operator=(FileInfo &&src) {
    FileInfo tmp{std::move(src)};
    Swap(&tmp);
    return *this;
  }

  // Member swap for move assignment.
  void Swap(FileInfo *src) noexcept {
    std::swap(m_type, src->m_type);
    std::swap(m_name, src->m_name);
  }

  const char *name() const { return m_name; }
  file_info::OpenType type() const { return m_type; }
};

using FileInfoAllocator = Malloc_allocator<FileInfo>;
using FileInfoVector = std::vector<FileInfo, FileInfoAllocator>;
FileInfoVector *fivp = nullptr;
}  // namespace

namespace file_info {

/**
   Add FileInfo entry for file descriptor. Increments status variable
   for open files/streams.
   @relates file_info::RegisterFilename

   @param fd file descriptor
   @param file_name name of file
   @param type_of_file tag indicating how the fd was created
 */
void RegisterFilename(File fd, const char *file_name, OpenType type_of_file) {
  DBUG_ASSERT(fd > -1);
  FileInfoVector &fiv = *fivp;
  MUTEX_LOCK(g, &THR_LOCK_open);
  if (static_cast<size_t>(fd) >= fiv.size()) {
    fiv.resize(fd + 1);
  }
  CountFileOpen(fiv[fd].type(), type_of_file);
  fiv[fd] = {file_name, type_of_file};

  dbug("fileinfo", [&]() {
    std::cerr << "Registering (" << fd << ", '" << file_name << ")"
              << std::endl;
  });
}

/**
   Remove FileInfo entry for file descriptor. Decrements status
   variables for open files/streams.
   @relates file_info::UnregisterFilename

   @param fd file descriptor
 */
void UnregisterFilename(File fd) {
  FileInfoVector &fiv = *fivp;
  MUTEX_LOCK(g, &THR_LOCK_open);

  if (static_cast<size_t>(fd) >= fiv.size()) {
    dbug("fileinfo", [&]() {
      std::cerr << "Un-registering unknown fd:" << fd << "!" << std::endl;
    });
    return;
  }
  if (fiv[fd].type() == OpenType::UNOPEN) {
    dbug("fileinfo", [&]() {
      std::cerr << "Un-registering already UNOPEN fd:" << fd << std::endl;
    });
    return;
  }
  CountFileClose(fiv[fd].type());

  dbug("fileinfo", [&]() {
    std::cerr << "Un-registering (" << fd << ", '" << fiv[fd].name() << "')"
              << std::endl;
  });
  fiv[fd] = {};
}
}  // namespace file_info

/**
  Get filename of file.

  @param fd file descriptor
  @return file name in file_info object
*/
const char *my_filename(File fd) {
  DBUG_TRACE;
  const FileInfoVector &fiv = *fivp;
  MUTEX_LOCK(g, &THR_LOCK_open);
  if (fd < 0 || fd >= static_cast<int>(fiv.size())) {
    return "<fd out of range>";
  }
  const FileInfo &fi = fiv[fd];
  if (fi.type() == file_info::OpenType::UNOPEN) {
    return "<unopen fd>";
  }
  return fi.name();
}

/**
  Sets the OS limit on the number of open files (if supported).

  @param files Number of requested files

  @return  The actual new OS limit which may be both more or less than
           what was requested.
*/
uint my_set_max_open_files(uint files) {
  DBUG_TRACE;
  return SetOsLimitMaxOpenFiles(std::min(files + MY_FILE_MIN, OS_FILE_LIMIT));
}

/**
  Constructs static objects.
*/
void MyFileInit() {
  fivp =
      new FileInfoVector(Malloc_allocator<FileInfo>{key_memory_my_file_info});
}

/**
  Destroys static objects.
*/
void MyFileEnd() { delete fivp; }
