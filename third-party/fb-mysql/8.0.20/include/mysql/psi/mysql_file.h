/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/mysql/psi/mysql_file.h
  Instrumentation helpers for mysys file io.
  This header file provides the necessary declarations
  to use the mysys file API with the performance schema instrumentation.
  In some compilers (SunStudio), 'static inline' functions, when declared
  but not used, are not optimized away (because they are unused) by default,
  so that including a static inline function from a header file does
  create unwanted dependencies, causing unresolved symbols at link time.
  Other compilers, like gcc, optimize these dependencies by default.

  Since the instrumented APIs declared here are wrapper on top
  of mysys file io APIs, including mysql/psi/mysql_file.h assumes that
  the dependency on my_sys already exists.
*/

#ifndef MYSQL_FILE_H
#define MYSQL_FILE_H

/* For strlen() */
#include <string.h>

#include "my_dbug.h"
/* For MY_STAT */
#include "my_dir.h"
/* For my_chsize */
#include "my_sys.h"
#include "mysql/psi/psi_file.h"
#include "mysql/service_mysql_alloc.h"
#include "pfs_file_provider.h"

#ifndef PSI_FILE_CALL
#define PSI_FILE_CALL(M) psi_file_service->M
#endif

/**
  @defgroup psi_api_file File Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_file_register(P1, P2, P3)
  File registration.
*/
#define mysql_file_register(P1, P2, P3) inline_mysql_file_register(P1, P2, P3)

/**
  @def mysql_file_fgets(P1, P2, F)
  Instrumented fgets.
  @c mysql_file_fgets is a replacement for @c fgets.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fgets(P1, P2, F) \
  inline_mysql_file_fgets(__FILE__, __LINE__, P1, P2, F)
#else
#define mysql_file_fgets(P1, P2, F) inline_mysql_file_fgets(P1, P2, F)
#endif

/**
  @def mysql_file_fgetc(F)
  Instrumented fgetc.
  @c mysql_file_fgetc is a replacement for @c fgetc.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fgetc(F) inline_mysql_file_fgetc(__FILE__, __LINE__, F)
#else
#define mysql_file_fgetc(F) inline_mysql_file_fgetc(F)
#endif

/**
  @def mysql_file_fputs(P1, F)
  Instrumented fputs.
  @c mysql_file_fputs is a replacement for @c fputs.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fputs(P1, F) \
  inline_mysql_file_fputs(__FILE__, __LINE__, P1, F)
#else
#define mysql_file_fputs(P1, F) inline_mysql_file_fputs(P1, F)
#endif

/**
  @def mysql_file_fputc(P1, F)
  Instrumented fputc.
  @c mysql_file_fputc is a replacement for @c fputc.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fputc(P1, F) \
  inline_mysql_file_fputc(__FILE__, __LINE__, P1, F)
#else
#define mysql_file_fputc(P1, F) inline_mysql_file_fputc(P1, F)
#endif

/**
  @def mysql_file_fprintf
  Instrumented fprintf.
  @c mysql_file_fprintf is a replacement for @c fprintf.
*/
#define mysql_file_fprintf inline_mysql_file_fprintf

/**
  @def mysql_file_vfprintf(F, P1, P2)
  Instrumented vfprintf.
  @c mysql_file_vfprintf is a replacement for @c vfprintf.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_vfprintf(F, P1, P2) \
  inline_mysql_file_vfprintf(__FILE__, __LINE__, F, P1, P2)
#else
#define mysql_file_vfprintf(F, P1, P2) inline_mysql_file_vfprintf(F, P1, P2)
#endif

/**
  @def mysql_file_fflush(F, P1, P2)
  Instrumented fflush.
  @c mysql_file_fflush is a replacement for @c fflush.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fflush(F) inline_mysql_file_fflush(__FILE__, __LINE__, F)
#else
#define mysql_file_fflush(F) inline_mysql_file_fflush(F)
#endif

/**
  @def mysql_file_feof(F)
  Instrumented feof.
  @c mysql_file_feof is a replacement for @c feof.
*/
#define mysql_file_feof(F) inline_mysql_file_feof(F)

/**
  @def mysql_file_fstat(FN, S)
  Instrumented fstat.
  @c mysql_file_fstat is a replacement for @c my_fstat.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fstat(FN, S) \
  inline_mysql_file_fstat(__FILE__, __LINE__, FN, S)
#else
#define mysql_file_fstat(FN, S) inline_mysql_file_fstat(FN, S)
#endif

/**
  @def mysql_file_stat(K, FN, S, FL)
  Instrumented stat.
  @c mysql_file_stat is a replacement for @c my_stat.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_stat(K, FN, S, FL) \
  inline_mysql_file_stat(K, __FILE__, __LINE__, FN, S, FL)
#else
#define mysql_file_stat(K, FN, S, FL) inline_mysql_file_stat(FN, S, FL)
#endif

/**
  @def mysql_file_chsize(F, P1, P2, P3)
  Instrumented chsize.
  @c mysql_file_chsize is a replacement for @c my_chsize.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_chsize(F, P1, P2, P3) \
  inline_mysql_file_chsize(__FILE__, __LINE__, F, P1, P2, P3)
#else
#define mysql_file_chsize(F, P1, P2, P3) inline_mysql_file_chsize(F, P1, P2, P3)
#endif

/**
  @def mysql_file_fopen(K, N, F1, F2)
  Instrumented fopen.
  @c mysql_file_fopen is a replacement for @c my_fopen.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fopen(K, N, F1, F2) \
  inline_mysql_file_fopen(K, __FILE__, __LINE__, N, F1, F2)
#else
#define mysql_file_fopen(K, N, F1, F2) inline_mysql_file_fopen(N, F1, F2)
#endif

/**
  @def mysql_file_fclose(FD, FL)
  Instrumented fclose.
  @c mysql_file_fclose is a replacement for @c my_fclose.
  Without the instrumentation, this call will have the same behavior as the
  undocumented and possibly platform specific my_fclose(NULL, ...) behavior.
  With the instrumentation, mysql_fclose(NULL, ...) will safely return 0,
  which is an extension compared to my_fclose and is therefore compliant.
  mysql_fclose is on purpose *not* implementing
  @code DBUG_ASSERT(file != NULL) @endcode,
  since doing so could introduce regressions.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fclose(FD, FL) \
  inline_mysql_file_fclose(__FILE__, __LINE__, FD, FL)
#else
#define mysql_file_fclose(FD, FL) inline_mysql_file_fclose(FD, FL)
#endif

/**
  @def mysql_file_fread(FD, P1, P2, P3)
  Instrumented fread.
  @c mysql_file_fread is a replacement for @c my_fread.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fread(FD, P1, P2, P3) \
  inline_mysql_file_fread(__FILE__, __LINE__, FD, P1, P2, P3)
#else
#define mysql_file_fread(FD, P1, P2, P3) inline_mysql_file_fread(FD, P1, P2, P3)
#endif

/**
  @def mysql_file_fwrite(FD, P1, P2, P3)
  Instrumented fwrite.
  @c mysql_file_fwrite is a replacement for @c my_fwrite.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fwrite(FD, P1, P2, P3) \
  inline_mysql_file_fwrite(__FILE__, __LINE__, FD, P1, P2, P3)
#else
#define mysql_file_fwrite(FD, P1, P2, P3) \
  inline_mysql_file_fwrite(FD, P1, P2, P3)
#endif

/**
  @def mysql_file_fseek(FD, P, W)
  Instrumented fseek.
  @c mysql_file_fseek is a replacement for @c my_fseek.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_fseek(FD, P, W) \
  inline_mysql_file_fseek(__FILE__, __LINE__, FD, P, W)
#else
#define mysql_file_fseek(FD, P, W) inline_mysql_file_fseek(FD, P, W)
#endif

/**
  @def mysql_file_ftell(FD)
  Instrumented ftell.
  @c mysql_file_ftell is a replacement for @c my_ftell.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_ftell(FD) inline_mysql_file_ftell(__FILE__, __LINE__, FD)
#else
#define mysql_file_ftell(FD) inline_mysql_file_ftell(FD)
#endif

/**
  @def mysql_file_create(K, N, F1, F2, F3)
  Instrumented create.
  @c mysql_file_create is a replacement for @c my_create.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_create(K, N, F1, F2, F3) \
  inline_mysql_file_create(K, __FILE__, __LINE__, N, F1, F2, F3)
#else
#define mysql_file_create(K, N, F1, F2, F3) \
  inline_mysql_file_create(N, F1, F2, F3)
#endif

/**
  @def mysql_file_create_temp(K, T, D, P, M, U, F)
  Instrumented create_temp_file.
  @c mysql_file_create_temp is a replacement for @c create_temp_file.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_create_temp(K, T, D, P, M, U, F) \
  inline_mysql_file_create_temp(K, __FILE__, __LINE__, T, D, P, M, U, F)
#else
#define mysql_file_create_temp(K, T, D, P, M, U, F) \
  inline_mysql_file_create_temp(T, D, P, M, U, F)
#endif

/**
  @def mysql_file_open(K, N, F1, F2)
  Instrumented open.
  @c mysql_file_open is a replacement for @c my_open.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_open(K, N, F1, F2) \
  inline_mysql_file_open(K, __FILE__, __LINE__, N, F1, F2)
#else
#define mysql_file_open(K, N, F1, F2) inline_mysql_file_open(N, F1, F2)
#endif

/**
  @def mysql_file_close(FD, F)
  Instrumented close.
  @c mysql_file_close is a replacement for @c my_close.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_close(FD, F) \
  inline_mysql_file_close(__FILE__, __LINE__, FD, F)
#else
#define mysql_file_close(FD, F) inline_mysql_file_close(FD, F)
#endif

/**
  @def mysql_file_read(FD, B, S, F)
  Instrumented read.
  @c mysql_read is a replacement for @c my_read.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_read(FD, B, S, F) \
  inline_mysql_file_read(__FILE__, __LINE__, FD, B, S, F)
#else
#define mysql_file_read(FD, B, S, F) inline_mysql_file_read(FD, B, S, F)
#endif

/**
  @def mysql_file_write(FD, B, S, F)
  Instrumented write.
  @c mysql_file_write is a replacement for @c my_write.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_write(FD, B, S, F) \
  inline_mysql_file_write(__FILE__, __LINE__, FD, B, S, F)
#else
#define mysql_file_write(FD, B, S, F) inline_mysql_file_write(FD, B, S, F)
#endif

/**
  @def mysql_file_pread(FD, B, S, O, F)
  Instrumented pread.
  @c mysql_pread is a replacement for @c my_pread.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_pread(FD, B, S, O, F) \
  inline_mysql_file_pread(__FILE__, __LINE__, FD, B, S, O, F)
#else
#define mysql_file_pread(FD, B, S, O, F) inline_mysql_file_pread(FD, B, S, O, F)
#endif

/**
  @def mysql_file_pwrite(FD, B, S, O, F)
  Instrumented pwrite.
  @c mysql_file_pwrite is a replacement for @c my_pwrite.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_pwrite(FD, B, S, O, F) \
  inline_mysql_file_pwrite(__FILE__, __LINE__, FD, B, S, O, F)
#else
#define mysql_file_pwrite(FD, B, S, O, F) \
  inline_mysql_file_pwrite(FD, B, S, O, F)
#endif

/**
  @def mysql_file_seek(FD, P, W, F)
  Instrumented seek.
  @c mysql_file_seek is a replacement for @c my_seek.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_seek(FD, P, W, F) \
  inline_mysql_file_seek(__FILE__, __LINE__, FD, P, W, F)
#else
#define mysql_file_seek(FD, P, W, F) inline_mysql_file_seek(FD, P, W, F)
#endif

/**
  @def mysql_file_tell(FD, F)
  Instrumented tell.
  @c mysql_file_tell is a replacement for @c my_tell.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_tell(FD, F) inline_mysql_file_tell(__FILE__, __LINE__, FD, F)
#else
#define mysql_file_tell(FD, F) inline_mysql_file_tell(FD, F)
#endif

/**
  @def mysql_file_delete(K, P1, P2)
  Instrumented delete.
  @c mysql_file_delete is a replacement for @c my_delete.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_delete(K, P1, P2) \
  inline_mysql_file_delete(K, __FILE__, __LINE__, P1, P2)
#else
#define mysql_file_delete(K, P1, P2) inline_mysql_file_delete(P1, P2)
#endif

/**
  @def mysql_file_rename(K, P1, P2, P3)
  Instrumented rename.
  @c mysql_file_rename is a replacement for @c my_rename.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_rename(K, P1, P2, P3) \
  inline_mysql_file_rename(K, __FILE__, __LINE__, P1, P2, P3)
#else
#define mysql_file_rename(K, P1, P2, P3) inline_mysql_file_rename(P1, P2, P3)
#endif

/**
  @def mysql_file_create_with_symlink(K, P1, P2, P3, P4, P5)
  Instrumented create with symbolic link.
  @c mysql_file_create_with_symlink is a replacement
  for @c my_create_with_symlink.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_create_with_symlink(K, P1, P2, P3, P4, P5)                  \
  inline_mysql_file_create_with_symlink(K, __FILE__, __LINE__, P1, P2, P3, P4, \
                                        P5)
#else
#define mysql_file_create_with_symlink(K, P1, P2, P3, P4, P5) \
  inline_mysql_file_create_with_symlink(P1, P2, P3, P4, P5)
#endif

/**
  @def mysql_file_delete_with_symlink(K, P1, P2)
  Instrumented delete with symbolic link.
  @c mysql_file_delete_with_symlink is a replacement
  for @c my_delete_with_symlink.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_delete_with_symlink(K, P1, P2) \
  inline_mysql_file_delete_with_symlink(K, __FILE__, __LINE__, P1, P2)
#else
#define mysql_file_delete_with_symlink(K, P1, P2) \
  inline_mysql_file_delete_with_symlink(P1, P2)
#endif

/**
  @def mysql_file_rename_with_symlink(K, P1, P2, P3)
  Instrumented rename with symbolic link.
  @c mysql_file_rename_with_symlink is a replacement
  for @c my_rename_with_symlink.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_rename_with_symlink(K, P1, P2, P3) \
  inline_mysql_file_rename_with_symlink(K, __FILE__, __LINE__, P1, P2, P3)
#else
#define mysql_file_rename_with_symlink(K, P1, P2, P3) \
  inline_mysql_file_rename_with_symlink(P1, P2, P3)
#endif

/**
  @def mysql_file_sync(P1, P2)
  Instrumented file sync.
  @c mysql_file_sync is a replacement for @c my_sync.
*/
#ifdef HAVE_PSI_FILE_INTERFACE
#define mysql_file_sync(P1, P2) \
  inline_mysql_file_sync(__FILE__, __LINE__, P1, P2)
#else
#define mysql_file_sync(P1, P2) inline_mysql_file_sync(P1, P2)
#endif

/**
  An instrumented FILE structure.
  @c MYSQL_FILE is a drop-in replacement for @c FILE.
  @sa mysql_file_open
*/
struct MYSQL_FILE {
  /** The real file. */
  FILE *m_file;
  /**
    The instrumentation hook.
    Note that this hook is not conditionally defined,
    for binary compatibility of the @c MYSQL_FILE interface.
  */
  struct PSI_file *m_psi;
};

static inline void inline_mysql_file_register(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *category, PSI_file_info *info, int count
#else
    const char *category MY_ATTRIBUTE((unused)),
    void *info MY_ATTRIBUTE((unused)), int count MY_ATTRIBUTE((unused))
#endif
) {
#ifdef HAVE_PSI_FILE_INTERFACE
  PSI_FILE_CALL(register_file)(category, info, count);
#endif
}

static inline char *inline_mysql_file_fgets(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    char *str, int size, MYSQL_FILE *file) {
  char *result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_READ);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)size, src_file, src_line);
    result = fgets(str, size, file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, result ? strlen(result) : 0);
    return result;
  }
#endif

  result = fgets(str, size, file->m_file);
  return result;
}

static inline int inline_mysql_file_fgetc(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_READ);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)1, src_file, src_line);
    result = fgetc(file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)1);
    return result;
  }
#endif

  result = fgetc(file->m_file);
  return result;
}

static inline int inline_mysql_file_fputs(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    const char *str, MYSQL_FILE *file) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    bytes = str ? strlen(str) : 0;
    PSI_FILE_CALL(start_file_wait)(locker, bytes, src_file, src_line);
    result = fputs(str, file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, bytes);
    return result;
  }
#endif

  result = fputs(str, file->m_file);
  return result;
}

static inline int inline_mysql_file_fputc(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    char c, MYSQL_FILE *file) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)1, src_file, src_line);
    result = fputc(c, file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)1);
    return result;
  }
#endif

  result = fputc(c, file->m_file);
  return result;
}

static inline int inline_mysql_file_fprintf(MYSQL_FILE *file,
                                            const char *format, ...)
    MY_ATTRIBUTE((format(printf, 2, 3)));

static inline int inline_mysql_file_fprintf(MYSQL_FILE *file,
                                            const char *format, ...) {
  /*
    TODO: figure out how to pass src_file and src_line from the caller.
  */
  int result;
  va_list args;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, __FILE__, __LINE__);
    va_start(args, format);
    result = vfprintf(file->m_file, format, args);
    va_end(args);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)result);
    return result;
  }
#endif

  va_start(args, format);
  result = vfprintf(file->m_file, format, args);
  va_end(args);
  return result;
}

static inline int inline_mysql_file_vfprintf(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, const char *format, va_list args)
#ifdef HAVE_PSI_FILE_INTERFACE
    MY_ATTRIBUTE((format(printf, 4, 0)));
#else
    MY_ATTRIBUTE((format(printf, 2, 0)));
#endif

static inline int inline_mysql_file_vfprintf(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, const char *format, va_list args) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = vfprintf(file->m_file, format, args);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)result);
    return result;
  }
#endif

  result = vfprintf(file->m_file, format, args);
  return result;
}

static inline int inline_mysql_file_fflush(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_FLUSH);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = fflush(file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = fflush(file->m_file);
  return result;
}

static inline int inline_mysql_file_feof(MYSQL_FILE *file) {
  /* Not instrumented, there is no wait involved */
  return feof(file->m_file);
}

static inline int inline_mysql_file_fstat(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    int filenr, MY_STAT *stat_area) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, filenr,
                                                            PSI_FILE_FSTAT);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_fstat(filenr, stat_area);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_fstat(filenr, stat_area);
  return result;
}

static inline MY_STAT *inline_mysql_file_stat(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *path, MY_STAT *stat_area, myf flags) {
  MY_STAT *result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_STAT, path, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_open_wait)(locker, src_file, src_line);
    result = my_stat(path, stat_area, flags);
    PSI_FILE_CALL(end_file_open_wait)(locker, result);
    return result;
  }
#endif

  result = my_stat(path, stat_area, flags);
  return result;
}

static inline int inline_mysql_file_chsize(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, my_off_t newlength, int filler, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_CHSIZE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)
    (locker, (size_t)newlength, src_file, src_line);
    result = my_chsize(file, newlength, filler, flags);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)newlength);
    return result;
  }
#endif

  result = my_chsize(file, newlength, filler, flags);
  return result;
}

static inline MYSQL_FILE *inline_mysql_file_fopen(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *filename, int flags, myf myFlags) {
  MYSQL_FILE *that;
  that = (MYSQL_FILE *)my_malloc(PSI_NOT_INSTRUMENTED, sizeof(MYSQL_FILE),
                                 MYF(MY_WME));
  if (likely(that != nullptr)) {
#ifdef HAVE_PSI_FILE_INTERFACE
    struct PSI_file_locker *locker;
    PSI_file_locker_state state;
    locker = PSI_FILE_CALL(get_thread_file_name_locker)(
        &state, key, PSI_FILE_STREAM_OPEN, filename, that);
    if (likely(locker != nullptr)) {
      PSI_FILE_CALL(start_file_open_wait)
      (locker, src_file, src_line);
      that->m_file = my_fopen(filename, flags, myFlags);
      that->m_psi = PSI_FILE_CALL(end_file_open_wait)(locker, that->m_file);
      if (unlikely(that->m_file == nullptr)) {
        my_free(that);
        return nullptr;
      }
      return that;
    }
#endif

    that->m_psi = nullptr;
    that->m_file = my_fopen(filename, flags, myFlags);
    if (unlikely(that->m_file == nullptr)) {
      my_free(that);
      return nullptr;
    }
  }
  return that;
}

static inline int inline_mysql_file_fclose(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, myf flags) {
  int result = 0;
  if (likely(file != nullptr)) {
#ifdef HAVE_PSI_FILE_INTERFACE
    struct PSI_file_locker *locker;
    PSI_file_locker_state state;
    locker = PSI_FILE_CALL(get_thread_file_stream_locker)(
        &state, file->m_psi, PSI_FILE_STREAM_CLOSE);
    if (likely(locker != nullptr)) {
      PSI_FILE_CALL(start_file_close_wait)(locker, src_file, src_line);
      result = my_fclose(file->m_file, flags);
      PSI_FILE_CALL(end_file_close_wait)(locker, result);
      my_free(file);
      return result;
    }
#endif

    result = my_fclose(file->m_file, flags);
    my_free(file);
  }
  return result;
}

static inline size_t inline_mysql_file_fread(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, uchar *buffer, size_t count, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_read;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_READ);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_fread(file->m_file, buffer, count, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_read = (result == 0) ? count : 0;
    } else {
      bytes_read = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_read);
    return result;
  }
#endif

  result = my_fread(file->m_file, buffer, count, flags);
  return result;
}

static inline size_t inline_mysql_file_fwrite(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, const uchar *buffer, size_t count, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_written;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_fwrite(file->m_file, buffer, count, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_written = (result == 0) ? count : 0;
    } else {
      bytes_written = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_written);
    return result;
  }
#endif

  result = my_fwrite(file->m_file, buffer, count, flags);
  return result;
}

static inline my_off_t inline_mysql_file_fseek(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file, my_off_t pos, int whence) {
  my_off_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_SEEK);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_fseek(file->m_file, pos, whence);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_fseek(file->m_file, pos, whence);
  return result;
}

static inline my_off_t inline_mysql_file_ftell(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_FILE *file) {
  my_off_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_stream_locker)(&state, file->m_psi,
                                                        PSI_FILE_TELL);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_ftell(file->m_file);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_ftell(file->m_file);
  return result;
}

static inline File inline_mysql_file_create(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *filename, int create_flags, int access_flags, myf myFlags) {
  File file;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_CREATE, filename, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_open_wait)(locker, src_file, src_line);
    file = my_create(filename, create_flags, access_flags, myFlags);
    PSI_FILE_CALL(end_file_open_wait_and_bind_to_descriptor)(locker, file);
    return file;
  }
#endif

  file = my_create(filename, create_flags, access_flags, myFlags);
  return file;
}

static inline File inline_mysql_file_create_temp(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    char *to, const char *dir, const char *pfx, int mode,
    UnlinkOrKeepFile unlink_or_keep, myf myFlags) {
  File file;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_CREATE, nullptr, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_open_wait)(locker, src_file, src_line);
    /* The file name is generated by create_temp_file(). */
    file = create_temp_file(to, dir, pfx, mode, unlink_or_keep, myFlags);
    PSI_FILE_CALL(end_temp_file_open_wait_and_bind_to_descriptor)
    (locker, file, (const char *)to);
    return file;
  }
#endif

  file = create_temp_file(to, dir, pfx, mode, unlink_or_keep, myFlags);
  return file;
}

static inline File inline_mysql_file_open(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *filename, int flags, myf myFlags) {
  File file;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_OPEN, filename, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_open_wait)(locker, src_file, src_line);
    file = my_open(filename, flags, myFlags);
    PSI_FILE_CALL(end_file_open_wait_and_bind_to_descriptor)(locker, file);
    return file;
  }
#endif

  file = my_open(filename, flags, myFlags);
  return file;
}

static inline int inline_mysql_file_close(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_CLOSE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_close_wait)(locker, src_file, src_line);
    result = my_close(file, flags);
    PSI_FILE_CALL(end_file_close_wait)(locker, result);
    return result;
  }
#endif

  result = my_close(file, flags);
  return result;
}

static inline size_t inline_mysql_file_read(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, uchar *buffer, size_t count, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_read;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_READ);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_read(file, buffer, count, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_read = (result == 0) ? count : 0;
    } else {
      bytes_read = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_read);
    return result;
  }
#endif

  result = my_read(file, buffer, count, flags);
  return result;
}

static inline size_t inline_mysql_file_write(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, const uchar *buffer, size_t count, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_written;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_write(file, buffer, count, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_written = (result == 0) ? count : 0;
    } else {
      bytes_written = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_written);
    return result;
  }
#endif

  result = my_write(file, buffer, count, flags);
  return result;
}

static inline size_t inline_mysql_file_pread(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, uchar *buffer, size_t count, my_off_t offset, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_read;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_READ);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_pread(file, buffer, count, offset, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_read = (result == 0) ? count : 0;
    } else {
      bytes_read = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_read);
    return result;
  }
#endif

  result = my_pread(file, buffer, count, offset, flags);
  return result;
}

static inline size_t inline_mysql_file_pwrite(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, const uchar *buffer, size_t count, my_off_t offset, myf flags) {
  size_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  size_t bytes_written;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_WRITE);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, count, src_file, src_line);
    result = my_pwrite(file, buffer, count, offset, flags);
    if (flags & (MY_NABP | MY_FNABP)) {
      bytes_written = (result == 0) ? count : 0;
    } else {
      bytes_written = (result != MY_FILE_ERROR) ? result : 0;
    }
    PSI_FILE_CALL(end_file_wait)(locker, bytes_written);
    return result;
  }
#endif

  result = my_pwrite(file, buffer, count, offset, flags);
  return result;
}

static inline my_off_t inline_mysql_file_seek(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, my_off_t pos, int whence, myf flags) {
  my_off_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_SEEK);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_seek(file, pos, whence, flags);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_seek(file, pos, whence, flags);
  return result;
}

static inline my_off_t inline_mysql_file_tell(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File file, myf flags) {
  my_off_t result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_TELL);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_tell(file, flags);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_tell(file, flags);
  return result;
}

static inline int inline_mysql_file_delete(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *name, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_DELETE, name, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_close_wait)(locker, src_file, src_line);
    result = my_delete(name, flags);
    PSI_FILE_CALL(end_file_close_wait)(locker, result);
    return result;
  }
#endif

  result = my_delete(name, flags);
  return result;
}

static inline int inline_mysql_file_rename(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *from, const char *to, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_RENAME, from, &locker);
  if (likely(locker != NULL)) {
    PSI_FILE_CALL(start_file_rename_wait)
    (locker, (size_t)0, from, to, src_file, src_line);
    result = my_rename(from, to, flags);
    PSI_FILE_CALL(end_file_rename_wait)(locker, from, to, result);
    return result;
  }
#endif

  result = my_rename(from, to, flags);
  return result;
}

static inline File inline_mysql_file_create_with_symlink(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *linkname, const char *filename, int create_flags,
    int access_flags, myf flags) {
  File file;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_CREATE, filename, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_open_wait)(locker, src_file, src_line);
    file = my_create_with_symlink(linkname, filename, create_flags,
                                  access_flags, flags);
    PSI_FILE_CALL(end_file_open_wait_and_bind_to_descriptor)(locker, file);
    return file;
  }
#endif

  file = my_create_with_symlink(linkname, filename, create_flags, access_flags,
                                flags);
  return file;
}

static inline int inline_mysql_file_delete_with_symlink(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *name, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_DELETE, name, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_close_wait)(locker, src_file, src_line);
    result = my_delete_with_symlink(name, flags);
    PSI_FILE_CALL(end_file_close_wait)(locker, result);
    return result;
  }
#endif

  result = my_delete_with_symlink(name, flags);
  return result;
}

static inline int inline_mysql_file_rename_with_symlink(
#ifdef HAVE_PSI_FILE_INTERFACE
    PSI_file_key key, const char *src_file, uint src_line,
#endif
    const char *from, const char *to, myf flags) {
  int result;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, key, PSI_FILE_RENAME, from, &locker);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_rename_wait)
    (locker, (size_t)0, from, to, src_file, src_line);
    result = my_rename_with_symlink(from, to, flags);
    PSI_FILE_CALL(end_file_rename_wait)(locker, from, to, result);
    return result;
  }
#endif

  result = my_rename_with_symlink(from, to, flags);
  return result;
}

static inline int inline_mysql_file_sync(
#ifdef HAVE_PSI_FILE_INTERFACE
    const char *src_file, uint src_line,
#endif
    File fd, myf flags) {
  int result = 0;
#ifdef HAVE_PSI_FILE_INTERFACE
  struct PSI_file_locker *locker;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, fd,
                                                            PSI_FILE_SYNC);
  if (likely(locker != nullptr)) {
    PSI_FILE_CALL(start_file_wait)(locker, (size_t)0, src_file, src_line);
    result = my_sync(fd, flags);
    PSI_FILE_CALL(end_file_wait)(locker, (size_t)0);
    return result;
  }
#endif

  result = my_sync(fd, flags);
  return result;
}

/** @} (end of group psi_api_file) */

#endif
