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
  @file mysys/my_static.cc
  Static variables for mysys library. All defined here for easy making of
  a shared library.
*/

#include "mysys/my_static.h"

#include "my_config.h"

#include <stdarg.h>
#include <stddef.h>

#include "my_compiler.h"
#include "my_loglevel.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/psi/psi_stage.h"
#include "mysys/mysys_priv.h"  // IWYU pragma: keep

/* get memory in hunks */
constexpr uint ONCE_ALLOC_INIT = 4096 - MALLOC_OVERHEAD;

PSI_memory_key key_memory_charset_file;
PSI_memory_key key_memory_charset_loader;
PSI_memory_key key_memory_lf_node;
PSI_memory_key key_memory_lf_dynarray;
PSI_memory_key key_memory_lf_slist;
PSI_memory_key key_memory_LIST;
PSI_memory_key key_memory_IO_CACHE;
PSI_memory_key key_memory_KEY_CACHE;
PSI_memory_key key_memory_SAFE_HASH_ENTRY;
PSI_memory_key key_memory_MY_BITMAP_bitmap;
PSI_memory_key key_memory_my_compress_alloc;
PSI_memory_key key_memory_my_err_head;
PSI_memory_key key_memory_my_file_info;
PSI_memory_key key_memory_max_alloca;
PSI_memory_key key_memory_MY_DIR;
PSI_memory_key key_memory_MY_TMPDIR_full_list;
PSI_memory_key key_memory_DYNAMIC_STRING;
PSI_memory_key key_memory_TREE;

PSI_thread_key key_thread_timer_notifier;

#ifdef _WIN32
PSI_memory_key key_memory_win_SECURITY_ATTRIBUTES;
PSI_memory_key key_memory_win_PACL;
PSI_memory_key key_memory_win_IP_ADAPTER_ADDRESSES;
PSI_memory_key key_memory_win_handle_info;
#endif /* _WIN32 */

/* from my_init */
char *home_dir = nullptr;
const char *my_progname = nullptr;
char curr_dir[FN_REFLEN] = {0}, home_dir_buff[FN_REFLEN] = {0};

ulong my_tmp_file_created = 0;

ulong my_stream_opened = 0;
ulong my_file_opened = 0;
ulong my_file_total_opened = 0;

namespace file_info {
/**
   Increment status variables.
   @relates file_info::CountFileOpen

   @param pt previous file_type (only relevant when assigning an fd to a stream
   in my_fdopen):
   @param ct current file type (to differentiate betweeen streams and files).
 */
void CountFileOpen(OpenType pt, OpenType ct) {
  mysql_mutex_assert_owner(&THR_LOCK_open);
  DBUG_ASSERT(my_file_opened + my_stream_opened == my_file_total_opened);
  DBUG_ASSERT(pt == OpenType::UNOPEN || ct == OpenType::STREAM_BY_FDOPEN);
  switch (ct) {
    case OpenType::UNOPEN:
      DBUG_ASSERT(false);
      return;

    case OpenType::STREAM_BY_FDOPEN:
      if (pt != OpenType::UNOPEN) {
        // If fd was opened through mysys, we have already counted
        // it in my_file_opened_. Since we will now increment
        // my_file_stream_opened_ for it, we decrement my_file_opened_
        // so that it is not counted twice.
        DBUG_ASSERT(pt != OpenType::STREAM_BY_FOPEN &&
                    pt != OpenType::STREAM_BY_FDOPEN);
        --my_file_opened;
        ++my_stream_opened;
        DBUG_ASSERT(my_file_opened + my_stream_opened == my_file_total_opened);
        return;
      }
      // Fallthrough
    case OpenType::STREAM_BY_FOPEN:
      ++my_stream_opened;
      break;

    default:
      ++my_file_opened;
  }
  ++my_file_total_opened;
  DBUG_ASSERT(my_file_opened + my_stream_opened == my_file_total_opened);
}

/**
   Decrement status variables.
   @relates file_info::CountFileClose

   @param ft file type (to differentiate betweeen streams and files).
 */
void CountFileClose(OpenType ft) {
  mysql_mutex_assert_owner(&THR_LOCK_open);
  DBUG_ASSERT(my_file_opened + my_stream_opened == my_file_total_opened);
  switch (ft) {
    case OpenType::UNOPEN:
      return;
    case OpenType::STREAM_BY_FOPEN:
    case OpenType::STREAM_BY_FDOPEN:
      --my_stream_opened;
      break;
    default:
      --my_file_opened;
  };
  --my_file_total_opened;
  DBUG_ASSERT(my_file_opened + my_stream_opened == my_file_total_opened);
}
}  // namespace file_info

int my_umask = 0664, my_umask_dir = 0777;

/* from mf_reccache.c */
ulong my_default_record_cache_size = RECORD_CACHE_SIZE;

/* from my_malloc */
USED_MEM *my_once_root_block = nullptr; /* pointer to first block */
uint my_once_extra = ONCE_ALLOC_INIT;   /* Memory to alloc / block */

/* from errors.c */
void (*error_handler_hook)(uint error, const char *str,
                           myf MyFlags) = my_message_stderr;
void (*fatal_error_handler_hook)(uint error, const char *str,
                                 myf MyFlags) = my_message_stderr;
void (*local_message_hook)(enum loglevel ll, uint ecode,
                           va_list args) = my_message_local_stderr;

static void enter_cond_dummy(void *a MY_ATTRIBUTE((unused)),
                             mysql_cond_t *b MY_ATTRIBUTE((unused)),
                             mysql_mutex_t *c MY_ATTRIBUTE((unused)),
                             const PSI_stage_info *d MY_ATTRIBUTE((unused)),
                             PSI_stage_info *e MY_ATTRIBUTE((unused)),
                             const char *f MY_ATTRIBUTE((unused)),
                             const char *g MY_ATTRIBUTE((unused)),
                             int h MY_ATTRIBUTE((unused))) {}

static void exit_cond_dummy(void *a MY_ATTRIBUTE((unused)),
                            const PSI_stage_info *b MY_ATTRIBUTE((unused)),
                            const char *c MY_ATTRIBUTE((unused)),
                            const char *d MY_ATTRIBUTE((unused)),
                            int e MY_ATTRIBUTE((unused))) {}

static void enter_stage_dummy(void *a MY_ATTRIBUTE((unused)),
                              const PSI_stage_info *b MY_ATTRIBUTE((unused)),
                              PSI_stage_info *c MY_ATTRIBUTE((unused)),
                              const char *d MY_ATTRIBUTE((unused)),
                              const char *e MY_ATTRIBUTE((unused)),
                              int f MY_ATTRIBUTE((unused))) {}

static void set_waiting_for_disk_space_dummy(void *a MY_ATTRIBUTE((unused)),
                                             bool b MY_ATTRIBUTE((unused))) {}

static int is_killed_dummy(const void *a MY_ATTRIBUTE((unused))) { return 0; }

/*
  Initialize these hooks to dummy implementations. The real server
  implementations will be set during server startup by
  init_server_components().
*/
void (*enter_cond_hook)(void *, mysql_cond_t *, mysql_mutex_t *,
                        const PSI_stage_info *, PSI_stage_info *, const char *,
                        const char *, int) = enter_cond_dummy;

void (*exit_cond_hook)(void *, const PSI_stage_info *, const char *,
                       const char *, int) = exit_cond_dummy;

void (*enter_stage_hook)(void *, const PSI_stage_info *, PSI_stage_info *,
                         const char *, const char *, int) = enter_stage_dummy;

void (*set_waiting_for_disk_space_hook)(void *, bool) =
    set_waiting_for_disk_space_dummy;

int (*is_killed_hook)(const void *) = is_killed_dummy;

#if defined(ENABLED_DEBUG_SYNC)
/**
   Global pointer to be set if callback function is defined
   (e.g. in mysqld). See sql/debug_sync.cc.
 */
DebugSyncCallbackFp debug_sync_C_callback_ptr;
#endif /* defined(ENABLED_DEBUG_SYNC) */

/* How to disable options */
bool my_disable_locking = false;
bool my_enable_symlinks = false;
