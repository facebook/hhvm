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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/mysys_priv.h
*/

#ifndef MYSYS_PRIV_INCLUDED
#define MYSYS_PRIV_INCLUDED

#include <memory>  // std::unique_ptr

#include "my_inttypes.h"  // myf
#include "my_macros.h"

#include "my_psi_config.h"
#include "mysql/components/services/mysql_mutex_bits.h"  // for mysql_mutex_t
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_file_bits.h"    // for PSI_file_key
#include "mysql/components/services/psi_memory_bits.h"  // for PSI_memory_key
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"  // for PSI_rwlock_key
#include "mysql/components/services/psi_stage_bits.h"   // for PSI_stage_info
#include "mysql/components/services/psi_thread_bits.h"  // for PSI_thread_key
#include "mysql/psi/mysql_mutex.h"                      // for mysql_mutex_lock

extern PSI_mutex_key key_IO_CACHE_append_buffer_lock, key_IO_CACHE_SHARE_mutex,
    key_KEY_CACHE_cache_lock, key_THR_LOCK_charset, key_THR_LOCK_heap,
    key_THR_LOCK_lock, key_THR_LOCK_malloc, key_THR_LOCK_mutex,
    key_THR_LOCK_myisam, key_THR_LOCK_net, key_THR_LOCK_open,
    key_THR_LOCK_threads, key_TMPDIR_mutex, key_THR_LOCK_myisam_mmap;

extern PSI_rwlock_key key_SAFE_HASH_lock;

extern PSI_cond_key key_IO_CACHE_SHARE_cond, key_IO_CACHE_SHARE_cond_writer,
    key_THR_COND_threads;

extern PSI_stage_info stage_waiting_for_table_level_lock;

extern mysql_mutex_t THR_LOCK_malloc, THR_LOCK_open;
extern mysql_mutex_t THR_LOCK_net;
extern mysql_mutex_t THR_LOCK_charset;

#ifdef HAVE_LINUX_LARGE_PAGES
extern PSI_file_key key_file_proc_meminfo;
#endif /* HAVE_LINUX_LARGE_PAGES */
extern PSI_file_key key_file_charset;

/* These keys are always defined. */

extern PSI_memory_key key_memory_charset_file;
extern PSI_memory_key key_memory_charset_loader;
extern PSI_memory_key key_memory_lf_node;
extern PSI_memory_key key_memory_lf_dynarray;
extern PSI_memory_key key_memory_lf_slist;
extern PSI_memory_key key_memory_LIST;
extern PSI_memory_key key_memory_IO_CACHE;
extern PSI_memory_key key_memory_KEY_CACHE;
extern PSI_memory_key key_memory_SAFE_HASH_ENTRY;
extern PSI_memory_key key_memory_MY_TMPDIR_full_list;
extern PSI_memory_key key_memory_MY_BITMAP_bitmap;
extern PSI_memory_key key_memory_my_compress_alloc;
extern PSI_memory_key key_memory_my_err_head;
extern PSI_memory_key key_memory_my_file_info;
extern PSI_memory_key key_memory_MY_DIR;
extern PSI_memory_key key_memory_DYNAMIC_STRING;
extern PSI_memory_key key_memory_TREE;
extern PSI_memory_key key_memory_defaults;

#ifdef _WIN32
extern PSI_memory_key key_memory_win_SECURITY_ATTRIBUTES;
extern PSI_memory_key key_memory_win_PACL;
extern PSI_memory_key key_memory_win_IP_ADAPTER_ADDRESSES;
extern PSI_memory_key key_memory_win_handle_info;
#endif

extern PSI_thread_key key_thread_timer_notifier;

/*
  EDQUOT is used only in 3 C files only in mysys/. If it does not exist on
  system, we set it to some value which can never happen.
*/
#ifndef EDQUOT
#define EDQUOT (-1)
#endif

namespace mysys_priv {
template <class SYSC, class RET>
inline RET RetryOnEintr(SYSC &&sysc, RET err) {
  RET r;
  do {
    r = sysc();
  } while (r == err && errno == EINTR);
  return r;
}
}  // namespace mysys_priv

void my_error_unregister_all();

#ifdef _WIN32
#include <stdint.h>  // int64_t
#include <sys/stat.h>
// my_winfile.cc exports, should not be used outside mysys
File my_win_open(const char *path, int oflag);
int my_win_close(File fd);
int64_t my_win_pread(File fd, uchar *buffer, size_t count, int64_t offset);
int64_t my_win_pwrite(File fd, const uchar *buffer, size_t count,
                      int64_t offset);
int64_t my_win_lseek(File fd, int64_t pos, int whence);
int64_t my_win_write(File fd, const uchar *buffer, size_t count);
int my_win_chsize(File fd, int64_t newlength);
File my_win_fileno(FILE *file);
FILE *my_win_fopen(const char *filename, const char *mode);
FILE *my_win_fdopen(File Filedes, const char *mode);
File my_win_fclose(FILE *stream);
FILE *my_win_freopen(const char *path, const char *mode, FILE *stream);
int my_win_fstat(File fd, struct _stati64 *buf);
int my_win_stat(const char *path, struct _stati64 *buf);
int my_win_fsync(File fd);

void MyWinfileInit();
void MyWinfileEnd();

#endif /* _WIN32 */

namespace file_info {

/**
   How was this file opened (for debugging purposes).
   The important part is whether it is UNOPEN or not.
*/
enum class OpenType : char {
  UNOPEN = 0,
  FILE_BY_OPEN,
  FILE_BY_CREATE,
  STREAM_BY_FOPEN,
  STREAM_BY_FDOPEN,
  FILE_BY_MKSTEMP,
  FILE_BY_O_TMPFILE
};

void CountFileOpen(OpenType pt, OpenType ct);
void CountFileClose(OpenType ft);

void RegisterFilename(File fd, const char *FileName, OpenType type_of_file);
void UnregisterFilename(File fd);
}  // namespace file_info

void MyFileInit();
void MyFileEnd();

#endif /* MYSYS_PRIV_INCLUDED */
