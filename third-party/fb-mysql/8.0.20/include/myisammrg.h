/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file include/myisammrg.h
  This file should be included when using merge isam functions.
*/

#ifndef _myisammrg_h
#define _myisammrg_h

#include <sys/types.h>

#include "my_base.h"
#include "my_inttypes.h"
#include "my_list.h"
#include "my_macros.h"
#include "myisam.h"
#include "mysql/psi/mysql_mutex.h"
#include "storage/myisam/queues.h"
#include "typelib.h"

#define MYRG_NAME_EXT ".MRG"

/* In which table to INSERT rows */
#define MERGE_INSERT_DISABLED 0
#define MERGE_INSERT_TO_FIRST 1
#define MERGE_INSERT_TO_LAST 2

extern TYPELIB merge_insert_method;

/* Param to/from myrg_info */

struct MYMERGE_INFO /* Struct from h_info */
{
  ulonglong records; /* Records in database */
  ulonglong deleted; /* Deleted records in database */
  ulonglong recpos;  /* Pos for last used record */
  ulonglong data_file_length;
  ulonglong dupp_key_pos; /* Offset of the Duplicate key in the merge table */
  uint reclength;         /* Recordlength */
  int errkey;             /* With key was dupplicated on err */
  uint options;           /* HA_OPTION_... used */
  ulong *rec_per_key;     /* for sql optimizing */
};

struct MYRG_TABLE {
  MI_INFO *table;
  ulonglong file_offset;
};

struct MYRG_INFO {
  MYRG_TABLE *open_tables, *current_table, *end_table, *last_used_table;
  ulonglong records; /* records in tables */
  ulonglong del;     /* Removed records */
  ulonglong data_file_length;
  ulong cache_size;
  uint merge_insert_method;
  uint tables, options, reclength, keys;
  /* If MERGE children attached to parent. See top comment in ha_myisammrg.cc */
  bool children_attached;
  LIST open_list;
  QUEUE by_key;
  ulong *rec_per_key_part; /* for sql optimizing */
  mysql_mutex_t mutex;
};

/* Prototypes for merge-functions */

extern int myrg_close(MYRG_INFO *file);
extern int myrg_delete(MYRG_INFO *file, const uchar *buff);
extern MYRG_INFO *myrg_open(const char *name, int mode, int wait_if_locked);
extern MYRG_INFO *myrg_parent_open(const char *parent_name,
                                   int (*callback)(void *, const char *),
                                   void *callback_param);
extern int myrg_attach_children(MYRG_INFO *m_info, int handle_locking,
                                MI_INFO *(*callback)(void *),
                                void *callback_param, bool *need_compat_check);
extern int myrg_detach_children(MYRG_INFO *m_info);
extern int myrg_panic(enum ha_panic_function function);
extern int myrg_rfirst(MYRG_INFO *file, uchar *buf, int inx);
extern int myrg_rlast(MYRG_INFO *file, uchar *buf, int inx);
extern int myrg_rnext(MYRG_INFO *file, uchar *buf, int inx);
extern int myrg_rprev(MYRG_INFO *file, uchar *buf, int inx);
extern int myrg_rnext_same(MYRG_INFO *file, uchar *buf);
extern int myrg_rkey(MYRG_INFO *info, uchar *buf, int inx, const uchar *key,
                     key_part_map keypart_map,
                     enum ha_rkey_function search_flag);
extern int myrg_rrnd(MYRG_INFO *file, uchar *buf, ulonglong pos);
extern int myrg_update(MYRG_INFO *file, const uchar *old, uchar *new_rec);
extern int myrg_write(MYRG_INFO *info, uchar *rec);
extern int myrg_status(MYRG_INFO *file, MYMERGE_INFO *x, int flag);
extern int myrg_lock_database(MYRG_INFO *file, int lock_type);
extern int myrg_create(const char *name, const char **table_names,
                       uint insert_method, bool fix_names);
extern int myrg_extra(MYRG_INFO *file, enum ha_extra_function function,
                      void *extra_arg);
extern int myrg_reset(MYRG_INFO *info);
extern ha_rows myrg_records_in_range(MYRG_INFO *info, int inx,
                                     key_range *min_key, key_range *max_key);
extern ha_rows myrg_records(MYRG_INFO *info);

extern ulonglong myrg_position(MYRG_INFO *info);

#endif /* _myisammrg_h */
