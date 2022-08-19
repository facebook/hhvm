/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file include/myisam.h
  This file should be included when using myisam functions.
*/

#ifndef _myisam_h
#define _myisam_h

#include "my_config.h"

#include <sys/types.h>
#include <time.h>

#include "keycache.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_check_opt.h"
#include "my_compare.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"

/*
  Limit max keys according to HA_MAX_POSSIBLE_KEY
*/

#if MAX_INDEXES > HA_MAX_POSSIBLE_KEY
#define MI_MAX_KEY HA_MAX_POSSIBLE_KEY /* Max allowed keys */
#else
#define MI_MAX_KEY MAX_INDEXES /* Max allowed keys */
#endif

#define MI_MAX_POSSIBLE_KEY_BUFF HA_MAX_POSSIBLE_KEY_BUFF
/*
  The following defines can be increased if necessary.
  But beware the dependency of MI_MAX_POSSIBLE_KEY_BUFF and MI_MAX_KEY_LENGTH.
*/
#define MI_MAX_KEY_LENGTH 1000 /* Max length in bytes */
#define MI_MAX_KEY_SEG 16      /* Max segments for key */

#define MI_MAX_KEY_BUFF (MI_MAX_KEY_LENGTH + MI_MAX_KEY_SEG * 6 + 8 + 8)
#define MI_MAX_MSG_BUF 1024 /* used in CHECK TABLE, REPAIR TABLE */
#define MI_NAME_IEXT ".MYI"
#define MI_NAME_DEXT ".MYD"

/* Possible values for myisam_block_size (must be power of 2) */
#define MI_KEY_BLOCK_LENGTH 1024U     /* default key block length */
#define MI_MIN_KEY_BLOCK_LENGTH 1024U /* Min key block length */
#define MI_MAX_KEY_BLOCK_LENGTH 16384U

/*
  In the following macros '_keyno_' is 0 .. keys-1.
  If there can be more keys than bits in the key_map, the highest bit
  is for all upper keys. They cannot be switched individually.
  This means that clearing of high keys is ignored, setting one high key
  sets all high keys.
*/
#define MI_KEYMAP_BITS (8 * SIZEOF_LONG_LONG)
#define MI_KEYMAP_HIGH_MASK (1ULL << (MI_KEYMAP_BITS - 1))
#define mi_get_mask_all_keys_active(_keys_) \
  (((_keys_) < MI_KEYMAP_BITS) ? ((1ULL << (_keys_)) - 1ULL) : (~0ULL))

#if MI_MAX_KEY > MI_KEYMAP_BITS

inline bool mi_is_key_active(uint64 _keymap_, unsigned int _keyno_) {
  return (((_keyno_) < MI_KEYMAP_BITS) ? ((_keymap_) & (1ULL << (_keyno_)))
                                       : ((_keymap_)&MI_KEYMAP_HIGH_MASK));
}

#define mi_set_key_active(_keymap_, _keyno_)                        \
  (_keymap_) |= (((_keyno_) < MI_KEYMAP_BITS) ? (1ULL << (_keyno_)) \
                                              : MI_KEYMAP_HIGH_MASK)
#define mi_clear_key_active(_keymap_, _keyno_)                         \
  (_keymap_) &= (((_keyno_) < MI_KEYMAP_BITS) ? (~(1ULL << (_keyno_))) \
                                              : (~(0ULL)) /*ignore*/)

#else

inline bool mi_is_key_active(uint64 _keymap_, unsigned int _keyno_) {
  return ((_keymap_) & (1ULL << (_keyno_)));
}
#define mi_set_key_active(_keymap_, _keyno_) (_keymap_) |= (1ULL << (_keyno_))
#define mi_clear_key_active(_keymap_, _keyno_) \
  (_keymap_) &= (~(1ULL << (_keyno_)))

#endif

inline bool mi_is_any_key_active(uint64 _keymap_) { return (_keymap_ != 0); }
#define mi_is_all_keys_active(_keymap_, _keys_) \
  ((_keymap_) == mi_get_mask_all_keys_active(_keys_))
#define mi_set_all_keys_active(_keymap_, _keys_) \
  (_keymap_) = mi_get_mask_all_keys_active(_keys_)
#define mi_clear_all_keys_active(_keymap_) (_keymap_) = 0
#define mi_intersect_keys_active(_to_, _from_) (_to_) &= (_from_)
#define mi_is_any_intersect_keys_active(_keymap1_, _keys_, _keymap2_) \
  ((_keymap1_) & (_keymap2_)&mi_get_mask_all_keys_active(_keys_))
#define mi_copy_keys_active(_to_, _maxkeys_, _from_) \
  (_to_) = (mi_get_mask_all_keys_active(_maxkeys_) & (_from_))

/* Param to/from mi_status */

struct MI_ISAMINFO /* Struct from h_info */
{
  ha_rows records{0};           /* Records in database */
  ha_rows deleted{0};           /* Deleted records in database */
  my_off_t recpos{0};           /* Pos for last used record */
  my_off_t newrecpos{0};        /* Pos if we write new record */
  my_off_t dupp_key_pos{0};     /* Position to record with dupp key */
  my_off_t data_file_length{0}, /* Length of data file */
      max_data_file_length{0}, index_file_length{0}, max_index_file_length{0},
      delete_length{0};
  ulong reclength{0};      /* Recordlength */
  ulong mean_reclength{0}; /* Mean recordlength (if packed) */
  ulonglong auto_increment{0};
  ulonglong key_map{0}; /* Which keys are used */
  char *data_file_name{nullptr}, *index_file_name{nullptr};
  uint keys{0};          /* Number of keys in use */
  uint options{};        /* HA_OPTION_... used */
  int errkey{0},         /* With key was dupplicated on err */
      sortkey{0};        /* clustered by this key */
  File filenr{0};        /* (uniq) filenr for datafile */
  time_t create_time{0}; /* When table was created */
  time_t check_time{0};
  time_t update_time{0};
  uint reflength{0};
  ulong record_offset{0};
  ulong *rec_per_key{nullptr}; /* for sql optimizing */
};

struct MI_CREATE_INFO {
  const char *index_file_name, *data_file_name; /* If using symlinks */
  ha_rows max_rows;
  ha_rows reloc_rows;
  ulonglong auto_increment;
  ulonglong data_file_length;
  ulonglong key_file_length;
  uint old_options;
  uint16 language;
  bool with_auto_increment;
};

struct MI_INFO; /* For referense */
struct MYISAM_SHARE;
struct MI_INFO;
struct MI_KEY_PARAM;

struct MI_KEYDEF /* Key definition with open & info */
{
  MYISAM_SHARE *share; /* Pointer to base (set in mi_open) */
  uint16 keysegs;      /* Number of key-segment */
  uint16 flag;         /* NOSAME, PACK_USED */

  uint8 key_alg;                 /* BTREE, RTREE */
  uint16 block_length;           /* Length of keyblock (auto) */
  uint16 underflow_block_length; /* When to execute underflow */
  uint16 keylength;              /* Tot length of keyparts (auto) */
  uint16 minlength;              /* min length of (packed) key (auto) */
  uint16 maxlength;              /* max length of (packed) key (auto) */
  uint16 block_size_index;       /* block_size (auto) */
  uint32 version;                /* For concurrent read/write */
  uint32 ftkey_nr;               /* full-text index number */

  HA_KEYSEG *seg, *end;
  struct st_mysql_ftparser *parser; /* Fulltext [pre]parser */
  int (*bin_search)(MI_INFO *info, MI_KEYDEF *keyinfo, uchar *page, uchar *key,
                    uint key_len, uint comp_flag, uchar **ret_pos, uchar *buff,
                    bool *was_last_key);
  uint (*get_key)(MI_KEYDEF *keyinfo, uint nod_flag, uchar **page, uchar *key);
  int (*pack_key)(MI_KEYDEF *keyinfo, uint nod_flag, const uchar *next_key,
                  uchar *org_key, uchar *prev_key, const uchar *key,
                  MI_KEY_PARAM *s_temp);
  void (*store_key)(MI_KEYDEF *keyinfo, uchar *key_pos, MI_KEY_PARAM *s_temp);
  int (*ck_insert)(MI_INFO *inf, uint k_nr, uchar *k, uint klen);
  int (*ck_delete)(MI_INFO *inf, uint k_nr, uchar *k, uint klen);
};

#define MI_UNIQUE_HASH_LENGTH 4

struct MI_UNIQUEDEF /* Segment definition of unique */
{
  uint16 keysegs; /* Number of key-segment */
  uchar key;      /* Mapped to which key */
  uint8 null_are_equal;
  HA_KEYSEG *seg, *end;
};

struct MI_DECODE_TREE /* Decode huff-table */
{
  uint16 *table;
  uint quick_table_bits;
  uchar *intervalls;
};

struct MI_BIT_BUFF;

/*
  Note that null markers should always be first in a row !
  When creating a column, one should only specify:
  type, length, null_bit and null_pos
*/

struct MI_COLUMNDEF /* column information */
{
  int16 type;      /* en_fieldtype */
  uint16 length;   /* length of field */
  uint32 offset;   /* Offset to position in row */
  uint8 null_bit;  /* If column may be 0 */
  uint16 null_pos; /* position for null marker */

  void (*unpack)(MI_COLUMNDEF *rec, MI_BIT_BUFF *buff, uchar *start,
                 uchar *end);
  enum en_fieldtype base_type;
  uint space_length_bits, pack_type;
  MI_DECODE_TREE *huff_tree;
};

extern const char *myisam_log_filename; /* Name of logfile */
extern ulong myisam_block_size;
extern ulong myisam_concurrent_insert;
extern bool myisam_flush, myisam_delay_key_write, myisam_single_user;
extern my_off_t myisam_max_temp_length;
extern ulong myisam_data_pointer_size;

/* usually used to check if a symlink points into the mysql data home */
/* which is normally forbidden                                        */
extern int (*myisam_test_invalid_symlink)(const char *filename);
extern ulonglong myisam_mmap_size, myisam_mmap_used;
extern mysql_mutex_t THR_LOCK_myisam_mmap;

/* Prototypes for myisam-functions */

extern int mi_close_share(MI_INFO *file, bool *closed_share);
#define mi_close(file) mi_close_share(file, NULL)
extern int mi_delete(MI_INFO *file, const uchar *buff);
extern MI_INFO *mi_open_share(const char *name, MYISAM_SHARE *old_share,
                              int mode, uint wait_if_locked);
#define mi_open(name, mode, wait_if_locked) \
  mi_open_share(name, NULL, mode, wait_if_locked)
extern int mi_panic(enum ha_panic_function function);
extern int mi_rfirst(MI_INFO *file, uchar *buf, int inx);
extern int mi_rkey(MI_INFO *info, uchar *buf, int inx, const uchar *key,
                   key_part_map keypart_map, enum ha_rkey_function search_flag);
extern int mi_rlast(MI_INFO *file, uchar *buf, int inx);
extern int mi_rnext(MI_INFO *file, uchar *buf, int inx);
extern int mi_rnext_same(MI_INFO *info, uchar *buf);
extern int mi_rprev(MI_INFO *file, uchar *buf, int inx);
extern int mi_rrnd(MI_INFO *file, uchar *buf, my_off_t pos);
extern int mi_scan_init(MI_INFO *file);
extern int mi_scan(MI_INFO *file, uchar *buf);
extern int mi_rsame(MI_INFO *file, uchar *record, int inx);
extern int mi_rsame_with_pos(MI_INFO *file, uchar *record, int inx,
                             my_off_t pos);
extern int mi_update(MI_INFO *file, const uchar *old, uchar *new_record);
extern int mi_write(MI_INFO *file, uchar *buff);
extern my_off_t mi_position(MI_INFO *file);
extern int mi_status(MI_INFO *info, MI_ISAMINFO *x, uint flag);
extern int mi_lock_database(MI_INFO *file, int lock_type);
extern int mi_create(const char *name, uint keys, MI_KEYDEF *keydef,
                     uint columns, MI_COLUMNDEF *columndef, uint uniques,
                     MI_UNIQUEDEF *uniquedef, MI_CREATE_INFO *create_info,
                     uint flags);
extern int mi_delete_table(const char *name);
extern int mi_rename(const char *from, const char *to);
extern int mi_extra(MI_INFO *file, enum ha_extra_function function,
                    void *extra_arg);
extern int mi_reset(MI_INFO *file);
extern ha_rows mi_records_in_range(MI_INFO *info, int inx, key_range *min_key,
                                   key_range *max_key);
extern int mi_log(int activate_log);
extern int mi_is_changed(MI_INFO *info);
extern int mi_delete_all_rows(MI_INFO *info);
extern ulong _mi_calc_blob_length(uint length, const uchar *pos);
extern uint mi_get_pointer_length(ulonglong file_length, uint def);

#define MEMMAP_EXTRA_MARGIN 7 /* Write this as a suffix for mmap file */
/* this is used to pass to mysql_myisamchk_table */

#define MYISAMCHK_REPAIR 1 /* equivalent to myisamchk -r */
#define MYISAMCHK_VERIFY 2 /* Verify, run repair if failure */

/*
  Flags used by myisamchk.c or/and ha_myisam.cc that are NOT passed
  to mi_check.c follows:
*/

#define TT_USEFRM 1
#define TT_FOR_UPGRADE 2

#define O_NEW_INDEX 1 /* Bits set in out_flag */
#define O_NEW_DATA 2
#define O_DATA_LOST 4

/* these struct is used by my_check to tell it what to do */

struct SORT_KEY_BLOCKS /* Used when sorting */
{
  uchar *buff, *end_pos;
  uchar lastkey[MI_MAX_POSSIBLE_KEY_BUFF];
  uint last_length;
  int inited;
};

/*
  MyISAM supports several statistics collection methods. Currently statistics
  collection method is not stored in MyISAM file and has to be specified for
  each table analyze/repair operation in  MI_CHECK::stats_method.
*/

typedef enum {
  /* Treat NULLs as inequal when collecting statistics (default for 4.1/5.0) */
  MI_STATS_METHOD_NULLS_NOT_EQUAL,
  /* Treat NULLs as equal when collecting statistics (like 4.0 did) */
  MI_STATS_METHOD_NULLS_EQUAL,
  /* Ignore NULLs - count only tuples without NULLs in the index components */
  MI_STATS_METHOD_IGNORE_NULLS
} enum_mi_stats_method;

struct MI_CHECK {
  ulonglong auto_increment_value{0};
  ulonglong max_data_file_length{0};
  ulonglong keys_in_use{~(ulonglong)0};
  ulonglong max_record_length{LLONG_MAX};
  ulonglong sort_buffer_length{0};
  my_off_t search_after_block{HA_OFFSET_ERROR};
  my_off_t new_file_pos{0}, key_file_blocks{0};
  my_off_t keydata, totaldata{0}, key_blocks{0}, start_check_pos{0};
  ha_rows total_records{0}, total_deleted{0};
  ha_checksum record_checksum{0}, glob_crc{0};
  ulonglong use_buffers{0};
  ulong read_buffer_length{0}, write_buffer_length{0}, sort_key_blocks{0};
  uint out_flag{0}, warning_printed{0}, error_printed{0}, verbose{0};
  uint opt_sort_key{0}, total_files{0}, max_level{0};
  uint testflag{0}, key_cache_block_size{KEY_CACHE_BLOCK_SIZE};
  uint16 language{0};
  bool using_global_keycache{false}, opt_follow_links{true};
  bool retry_repair{false}, force_sort{false};
  char temp_filename[FN_REFLEN]{0}, *isam_file_name{nullptr};
  MY_TMPDIR *tmpdir{nullptr};
  int tmpfile_createflag{0};
  myf myf_rw{MY_NABP | MY_WME | MY_WAIT_IF_FULL};
  IO_CACHE read_cache;

  /*
    The next two are used to collect statistics, see update_key_parts for
    description.
  */
  ulonglong unique_count[MI_MAX_KEY_SEG + 1]{0};
  ulonglong notnull_count[MI_MAX_KEY_SEG + 1]{0};

  ha_checksum key_crc[HA_MAX_POSSIBLE_KEY]{0};
  ulong rec_per_key_part[MI_MAX_KEY_SEG * HA_MAX_POSSIBLE_KEY]{0};
  void *thd{nullptr};
  const char *db_name{nullptr}, *table_name{nullptr};
  const char *op_name{nullptr};
  enum_mi_stats_method stats_method{MI_STATS_METHOD_NULLS_NOT_EQUAL};
  mysql_mutex_t print_msg_mutex;
  bool need_print_msg_lock{false};
};

struct SORT_FT_BUF {
  uchar *buf, *end;
  int count;
  uchar lastkey[MI_MAX_KEY_BUFF];
};

struct SORT_INFO {
  my_off_t filelength{0}, dupp{0}, buff_length{0};
  ha_rows max_records{0};
  uint current_key{0}, total_keys{0};
  myf myf_rw{0};
  data_file_type new_data_file_type{STATIC_RECORD};
  MI_INFO *info{nullptr};
  MI_CHECK *param{nullptr};
  uchar *buff{nullptr};
  SORT_KEY_BLOCKS *key_block{nullptr}, *key_block_end{nullptr};
  SORT_FT_BUF *ft_buf{nullptr};
  /* sync things */
  uint got_error{0}, threads_running{0};
  mysql_mutex_t mutex;
  mysql_cond_t cond;
};

/* functions in mi_check */
void myisamchk_init(MI_CHECK *param);
int chk_status(MI_CHECK *param, MI_INFO *info);
int chk_del(MI_CHECK *param, MI_INFO *info, uint test_flag);
int chk_size(MI_CHECK *param, MI_INFO *info);
int chk_key(MI_CHECK *param, MI_INFO *info);
int chk_data_link(MI_CHECK *param, MI_INFO *info, int extend);
int mi_repair(MI_CHECK *param, MI_INFO *info, char *name, int rep_quick,
              bool no_copy_stat);
int mi_sort_index(MI_CHECK *param, MI_INFO *info, char *name,
                  bool no_copy_stat);
int mi_repair_by_sort(MI_CHECK *param, MI_INFO *info, const char *name,
                      int rep_quick, bool no_copy_stat);
int mi_repair_parallel(MI_CHECK *param, MI_INFO *info, const char *name,
                       int rep_quick, bool no_copy_stat);
int change_to_newfile(const char *filename, const char *old_ext,
                      const char *new_ext, myf myflags);
int lock_file(MI_CHECK *param, File file, int lock_type, const char *filetype,
              const char *filename);
void lock_memory(MI_CHECK *param);
void update_auto_increment_key(MI_CHECK *param, MI_INFO *info, bool repair);
int update_state_info(MI_CHECK *param, MI_INFO *info, uint update);
void update_key_parts(MI_KEYDEF *keyinfo, ulong *rec_per_key_part,
                      ulonglong *unique, ulonglong *notnull, ulonglong records);
int filecopy(MI_CHECK *param, File to, File from, my_off_t start,
             my_off_t length, const char *type);
int movepoint(MI_INFO *info, uchar *record, my_off_t oldpos, my_off_t newpos,
              uint prot_key);
int write_data_suffix(SORT_INFO *sort_info, bool fix_datafile);
int test_if_almost_full(MI_INFO *info);
int recreate_table(MI_CHECK *param, MI_INFO **org_info, char *filename);
void mi_disable_non_unique_index(MI_INFO *info, ha_rows rows);
bool mi_test_if_sort_rep(MI_INFO *info, ha_rows rows, ulonglong key_map,
                         bool force);

int mi_init_bulk_insert(MI_INFO *info, ulong cache_size, ha_rows rows);
void mi_flush_bulk_insert(MI_INFO *info, uint inx);
void mi_end_bulk_insert(MI_INFO *info);
int mi_assign_to_key_cache(MI_INFO *info, ulonglong key_map,
                           KEY_CACHE *key_cache);
void mi_change_key_cache(KEY_CACHE *old_key_cache, KEY_CACHE *new_key_cache);
int mi_preload(MI_INFO *info, ulonglong key_map, bool ignore_leaves);

extern st_keycache_thread_var main_thread_keycache_var;
st_keycache_thread_var *keycache_thread_var();
#endif
