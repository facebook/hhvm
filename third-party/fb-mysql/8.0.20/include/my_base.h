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
  @file include/my_base.h
  This file includes constants used by all storage engines.
*/

#ifndef _my_base_h
#define _my_base_h

#include <limits.h>
#include <sys/types.h>

#include "my_config.h"
#include "my_double2ulonglong.h"
#include "my_inttypes.h"
#include "my_macros.h"

/* The following is bits in the flag parameter to ha_open() */

#define HA_OPEN_ABORT_IF_LOCKED 0 /* default */
#define HA_OPEN_WAIT_IF_LOCKED 1
#define HA_OPEN_IGNORE_IF_LOCKED 2
#define HA_OPEN_TMP_TABLE 4       /* Table is a temp table */
#define HA_OPEN_DELAY_KEY_WRITE 8 /* Don't update index  */
#define HA_OPEN_ABORT_IF_CRASHED 16
#define HA_OPEN_FOR_REPAIR 32 /* open even if crashed */
#define HA_OPEN_FROM_SQL_LAYER 64
#define HA_OPEN_COPY 256 /* Open copy (for repair) */
/**
   Internal temp table, used for temporary results; one or more instance of it
   may be be created.
*/
#define HA_OPEN_INTERNAL_TABLE 512

/* The following is parameter to ha_rkey() how to use key */

/*
  We define a complete-field prefix of a key value as a prefix where
  the last included field in the prefix contains the full field, not
  just some bytes from the start of the field. A partial-field prefix
  is allowed to contain only a few first bytes from the last included
  field.

  Below HA_READ_KEY_EXACT, ..., HA_READ_BEFORE_KEY can take a
  complete-field prefix of a key value as the search
  key. HA_READ_PREFIX and HA_READ_PREFIX_LAST could also take a
  partial-field prefix, but currently (4.0.10) they are only used with
  complete-field prefixes. MySQL uses a padding trick to implement
  LIKE 'abc%' queries.

  NOTE that in InnoDB HA_READ_PREFIX_LAST will NOT work with a
  partial-field prefix because InnoDB currently strips spaces from the
  end of varchar fields!
*/

enum ha_rkey_function {
  HA_READ_KEY_EXACT,           /* Find first record else error */
  HA_READ_KEY_OR_NEXT,         /* Record or next record */
  HA_READ_KEY_OR_PREV,         /* Record or previous */
  HA_READ_AFTER_KEY,           /* Find next rec. after key-record */
  HA_READ_BEFORE_KEY,          /* Find next rec. before key-record */
  HA_READ_PREFIX,              /* Key which as same prefix */
  HA_READ_PREFIX_LAST,         /* Last key with the same prefix */
  HA_READ_PREFIX_LAST_OR_PREV, /* Last or prev key with the same prefix */
  HA_READ_MBR_CONTAIN,         /* Minimum Bounding Rectangle contains */
  HA_READ_MBR_INTERSECT,       /* Minimum Bounding Rectangle intersect */
  HA_READ_MBR_WITHIN,          /* Minimum Bounding Rectangle within */
  HA_READ_MBR_DISJOINT,        /* Minimum Bounding Rectangle disjoint */
  HA_READ_MBR_EQUAL,           /* Minimum Bounding Rectangle equal */
  HA_READ_INVALID = -1         /* Invalid enumeration value, always last. */
};

/* Key algorithm types */

enum ha_key_alg {
  /**
    Used for cases when key algorithm which is supported by SE can't be
    described by one of other classes from this enum (@sa Federated,
    PerfSchema SE, @sa dd::Index::IA_SE_SPECIFIC).

    @note Assigned as default value for key algorithm by parser, replaced by
          SEs default algorithm for keys in mysql_prepare_create_table().
  */
  HA_KEY_ALG_SE_SPECIFIC = 0,
  HA_KEY_ALG_BTREE = 1,   /* B-tree. */
  HA_KEY_ALG_RTREE = 2,   /* R-tree, for spatial searches */
  HA_KEY_ALG_HASH = 3,    /* HASH keys (HEAP, NDB). */
  HA_KEY_ALG_FULLTEXT = 4 /* FULLTEXT. */
};

/* Storage media types */

enum ha_storage_media {
  HA_SM_DEFAULT = 0, /* Not specified (engine default) */
  HA_SM_DISK = 1,    /* DISK storage */
  HA_SM_MEMORY = 2   /* MAIN MEMORY storage */
};

/* The following is parameter to ha_extra() */

/*
  We divide the HA_EXTRA entries into the following categories:

  1) Operations used by most handlers:
     HA_EXTRA_KEYREAD
     HA_EXTRA_NO_KEYREAD
     HA_EXTRA_FLUSH
     HA_EXTRA_PREPARE_FOR_RENAME
     HA_EXTRA_FORCE_REOPEN

  2) Operations used by some non-MyISAM handlers
     HA_EXTRA_KEYREAD_PRESERVE_FIELDS
     HA_EXTRA_IGNORE_DUP_KEY
     HA_EXTRA_NO_IGNORE_DUP_KEY

  3) Operations used only by MyISAM
     HA_EXTRA_NORMAL
     HA_EXTRA_QUICK
     HA_EXTRA_PREPARE_FOR_UPDATE
     HA_EXTRA_PREPARE_FOR_DROP
     HA_EXTRA_READCHECK
     HA_EXTRA_NO_READCHECK

  4) Operations only used by temporary tables for query processing
     HA_EXTRA_RESET_STATE
     HA_EXTRA_NO_ROWS

  5) Operations only used by MyISAM internally
     HA_EXTRA_NO_USER_CHANGE
     HA_EXTRA_WAIT_LOCK
     HA_EXTRA_WAIT_NOLOCK
     HA_EXTRA_NO_KEYS
     HA_EXTRA_KEYREAD_CHANGE_POS
     HA_EXTRA_REMEMBER_POS
     HA_EXTRA_RESTORE_POS
     HA_EXTRA_PRELOAD_BUFFER_SIZE
     HA_EXTRA_CHANGE_KEY_TO_DUP
     HA_EXTRA_CHANGE_KEY_TO_UNIQUE

  6) Operations only used by federated tables for query processing
     HA_EXTRA_INSERT_WITH_UPDATE

  7) Operations only used by NDB
     HA_EXTRA_DELETE_CANNOT_BATCH
     HA_EXTRA_UPDATE_CANNOT_BATCH

  8) Operations only used by MERGE
     HA_EXTRA_ADD_CHILDREN_LIST
     HA_EXTRA_ATTACH_CHILDREN
     HA_EXTRA_IS_ATTACHED_CHILDREN
     HA_EXTRA_DETACH_CHILDREN

  9) Operations only used by InnoDB
     HA_EXTRA_EXPORT

  10) Operations only used by partitioning
      HA_EXTRA_SECONDARY_SORT_ROWID

  The individual entries are described in more detail below.
*/

enum ha_extra_function {
  /*
    Optimize for space (def).
    Only used in MyISAM to reset quick mode, not implemented by any other
    handler. Quick mode is also reset in MyISAM by handler::reset().

    It is called after completing a successful DELETE query if the QUICK
    option is set.
  */
  HA_EXTRA_NORMAL = 0,
  /*
    Optimize for speed.
    When the user does DELETE QUICK FROM table where-clause; this extra
    option is called before the delete query is performed and
    HA_EXTRA_NORMAL is called after the delete query is completed.
    Temporary tables used internally in MySQL always set this option

    The meaning of quick mode is that when deleting in a B-tree no merging
    of leafs is performed. This is a common method and many large DBMS's
    actually only support this quick mode since it is very difficult to
    merge leaves in a tree used by many threads concurrently.
  */
  HA_EXTRA_QUICK = 1,
  HA_EXTRA_NOT_USED = 2,
  /*
    Only one call to HA_EXTRA_NO_READCHECK from ha_open where it says that
    this is not needed in SQL. The reason for this call is that MyISAM sets
    the READ_CHECK_USED in the open call so the call is needed for MyISAM
    to reset this feature.
    The idea with this parameter was to inform of doing/not doing a read
    check before applying an update. Since SQL always performs a read before
    applying the update No Read Check is needed in MyISAM as well.

    This is a cut from Docs/myisam.txt
     Sometimes you might want to force an update without checking whether
     another user has changed the record since you last read it. This is
     somewhat dangerous, so it should ideally not be used. That can be
     accomplished by wrapping the mi_update() call in two calls to mi_extra(),
     using the flags HA_EXTRA_NO_READCHECK and HA_EXTRA_READCHECK.
  */
  HA_EXTRA_NO_READCHECK = 5,
  HA_EXTRA_READCHECK = 6,
  /*
    These parameters are used to provide an optimisation hint to the handler.
    If HA_EXTRA_KEYREAD is set it is enough to read the index fields, for
    many handlers this means that the index-only scans can be used and it
    is not necessary to use the real records to satisfy this part of the
    query. Index-only scans is a very important optimisation for disk-based
    indexes. For main-memory indexes most indexes contain a reference to the
    record and thus KEYREAD only says that it is enough to read key fields.
    HA_EXTRA_NO_KEYREAD disables this for the handler, also, handler::reset()
    will disable this option.
    The handler will set HA_KEYREAD_ONLY in its table flags to indicate this
    feature is supported.
  */
  HA_EXTRA_KEYREAD = 7,
  HA_EXTRA_NO_KEYREAD = 8,
  /*
    No user is allowed to write. Only used by MyISAM, never called.
    Simulates lock_type as locked.
  */
  HA_EXTRA_NO_USER_CHANGE = 9,
  /*
    Only used by MyISAM, called from MyISAM handler but never from server
    code on top of the handler.
    Sets lock_wait on/off:
      Wait until file is avalable (def).
      If file is locked, return quickly.
  */
  HA_EXTRA_WAIT_LOCK = 12,
  HA_EXTRA_NO_WAIT_LOCK = 13,
  /*
    Remove all update of keys. Only used by MyISAM, only used internally in
    MyISAM handler, never called from server level.
  */
  HA_EXTRA_NO_KEYS = 16,
  HA_EXTRA_KEYREAD_CHANGE_POS = 17, /* Keyread, but change pos */
                                    /* xxxxchk -r must be used */
  HA_EXTRA_REMEMBER_POS = 18,       /* Remember pos for next/prev */
  HA_EXTRA_RESTORE_POS = 19,
  /*
    Only used by MyISAM and Archive, called when altering table,
    closing tables to enforce a reopen of the table files.
  */
  HA_EXTRA_FORCE_REOPEN = 21,
  /*
    Indication to flush tables to disk, is supposed to be used to
    ensure disk based tables are flushed at end of query execution.
    Currently is never used.
  */
  HA_EXTRA_FLUSH,
  /*
    Don't insert rows indication to HEAP and MyISAM, only used by temporary
    tables used in query processing.
  */
  HA_EXTRA_NO_ROWS,
  /*
    This option is used by most handlers and it resets the handler state
    to the same state as after an open call.

    It is called from the reset method in the handler interface. There are
    three instances where this is called.

    1) After completing a INSERT ... SELECT ... query the handler for the
       table inserted into is reset
    2) It is called from close_thread_table which in turn is called from
       close_thread_tables except in the case where the tables are locked
       in which case ha_commit_stmt is called instead.
       It is only called from here if refresh_version hasn't changed and the
       table is not an old table when calling close_thread_table.
       close_thread_tables is called from many places as a general clean up
       function after completing a query.
    3) It is called when deleting the QUICK_RANGE_SELECT object if the
       QUICK_RANGE_SELECT object had its own handler object. It is called
       immediatley before close of this local handler object.

    If there is a READ CACHE it is reinit'ed. A cache is reinit'ed to
    restart reading or to change type of cache between READ CACHE and
    WRITE CACHE.

    This extra function is always called immediately before calling
    delete_all_rows on the handler for temporary tables.
    There are cases however when HA_EXTRA_RESET_STATE isn't called in
    a similar case for a temporary table in sql_union.cc.

    The only case that delete_all_rows is called on non-temporary tables
    is in sql_delete.cc when DELETE FROM table; is called by a user.
    In this case no special extra calls are performed before or after this
    call.
  */
  HA_EXTRA_RESET_STATE,
  /*
    Informs the handler that we will not stop the transaction if we get
    duplicate key errors during insert/upate.
    Always called in pair, triggered by INSERT IGNORE and other similar
    SQL constructs.
    Not used by MyISAM.
  */
  HA_EXTRA_IGNORE_DUP_KEY,
  HA_EXTRA_NO_IGNORE_DUP_KEY,
  /*
    Only used by MyISAM, called in preparation for a DROP TABLE.
    It's used mostly by Windows that cannot handle dropping an open file.
    On other platforms it has the same effect as HA_EXTRA_FORCE_REOPEN.
  */
  HA_EXTRA_PREPARE_FOR_DROP,
  /*
    Remove read cache if problems.
    This is called as part of a multi-table update. When the table to be
    updated is also scanned then this informs MyISAM handler to drop any
    caches if dynamic records are used (fixed size records do not care
    about this call).
  */
  HA_EXTRA_PREPARE_FOR_UPDATE,
  HA_EXTRA_PRELOAD_BUFFER_SIZE, /* Set buffer size for preloading */
  /*
    On-the-fly switching between unique and non-unique key inserting.
  */
  HA_EXTRA_CHANGE_KEY_TO_UNIQUE,
  HA_EXTRA_CHANGE_KEY_TO_DUP,
  /*
    When using HA_EXTRA_KEYREAD, overwrite only key member fields and keep
    other fields intact. When this is off (by default) InnoDB will use memcpy
    to overwrite entire row.
    This is a strictly InnoDB feature that is more or less undocumented.
  */
  HA_EXTRA_KEYREAD_PRESERVE_FIELDS,
  /*
    Ignore if the a tuple is not found, continue processing the
    transaction and ignore that 'row'.  Needed for idempotency
    handling on the slave

    Currently only used by NDB storage engine.
  */
  HA_EXTRA_IGNORE_NO_KEY,
  HA_EXTRA_NO_IGNORE_NO_KEY,
  /*
    Mark the table as a log table. For some handlers (e.g. CSV) this results
    in a special locking for the table.
  */
  HA_EXTRA_MARK_AS_LOG_TABLE,
  /*
    Informs handler that write_row() which tries to insert new row into the
    table and encounters some already existing row with same primary/unique
    key can replace old row with new row instead of reporting error (basically
    it informs handler that we do REPLACE instead of simple INSERT).
    Off by default.
  */
  HA_EXTRA_WRITE_CAN_REPLACE,
  HA_EXTRA_WRITE_CANNOT_REPLACE,
  /*
    Inform handler that delete_row()/update_row() cannot batch deletes/updates
    and should perform them immediately. This may be needed when table has
    AFTER DELETE/UPDATE triggers which access to subject table.
    These flags are reset by the handler::reset() call.
  */
  HA_EXTRA_DELETE_CANNOT_BATCH,
  HA_EXTRA_UPDATE_CANNOT_BATCH,
  /*
    Inform handler that an "INSERT...ON DUPLICATE KEY UPDATE" will be
    executed. This condition is unset by HA_EXTRA_NO_IGNORE_DUP_KEY.
  */
  HA_EXTRA_INSERT_WITH_UPDATE,
  /*
    Informs the handler we are about to attempt a rename of the table.
    For handlers that have share open files (MyISAM key-file and
    Archive writer) they must close the files before rename is possible
    on Windows.
  */
  HA_EXTRA_PREPARE_FOR_RENAME,
  /*
    Special actions for MERGE tables.
  */
  HA_EXTRA_ADD_CHILDREN_LIST,
  HA_EXTRA_ATTACH_CHILDREN,
  HA_EXTRA_IS_ATTACHED_CHILDREN,
  HA_EXTRA_DETACH_CHILDREN,
  /*
    Prepare table for export
    (e.g. quiesce the table and write table metadata).
  */
  HA_EXTRA_EXPORT,
  /** Do secondary sort by handler::ref (rowid) after key sort. */
  HA_EXTRA_SECONDARY_SORT_ROWID,
  /*
    Skip Serializable isolation level on Views on DD tables.
    This will make reads on DD Views non blocking */
  HA_EXTRA_SKIP_SERIALIZABLE_DD_VIEW,
  /* Begin of insertion into intermediate table during copy alter operation. */
  HA_EXTRA_BEGIN_ALTER_COPY,
  /* Insertion is done in intermediate table during copy alter operation. */
  HA_EXTRA_END_ALTER_COPY,
  /* Do not use auto-increment locking. */
  HA_EXTRA_NO_AUTOINC_LOCKING,
  /*
    Allocate and use unique filter to weed out duplicates.
    Used with multi-valued index.
  */
  HA_EXTRA_ENABLE_UNIQUE_RECORD_FILTER,
  /* Disable and free unique record filter. */
  HA_EXTRA_DISABLE_UNIQUE_RECORD_FILTER
};

/* Compatible option, to be deleted in 6.0 */
#define HA_EXTRA_PREPARE_FOR_DELETE HA_EXTRA_PREPARE_FOR_DROP

/* The following is parameter to ha_panic() */

enum ha_panic_function {
  HA_PANIC_CLOSE, /* Close all databases */
  HA_PANIC_WRITE, /* Unlock and write status */
  HA_PANIC_READ   /* Lock and read keyinfo */
};

/* The following is parameter to ha_create(); keytypes */

enum ha_base_keytype {
  HA_KEYTYPE_END = 0,
  HA_KEYTYPE_TEXT = 1,   /* Key is sorted as letters */
  HA_KEYTYPE_BINARY = 2, /* Key is sorted as unsigned chars */
  HA_KEYTYPE_SHORT_INT = 3,
  HA_KEYTYPE_LONG_INT = 4,
  HA_KEYTYPE_FLOAT = 5,
  HA_KEYTYPE_DOUBLE = 6,
  HA_KEYTYPE_NUM = 7, /* Not packed num with pre-space */
  HA_KEYTYPE_USHORT_INT = 8,
  HA_KEYTYPE_ULONG_INT = 9,
  HA_KEYTYPE_LONGLONG = 10,
  HA_KEYTYPE_ULONGLONG = 11,
  HA_KEYTYPE_INT24 = 12,
  HA_KEYTYPE_UINT24 = 13,
  HA_KEYTYPE_INT8 = 14,
  /* Varchar (0-255 bytes) with length packed with 1 byte */
  HA_KEYTYPE_VARTEXT1 = 15,   /* Key is sorted as letters */
  HA_KEYTYPE_VARBINARY1 = 16, /* Key is sorted as unsigned chars */
  /* Varchar (0-65535 bytes) with length packed with 2 bytes */
  HA_KEYTYPE_VARTEXT2 = 17,   /* Key is sorted as letters */
  HA_KEYTYPE_VARBINARY2 = 18, /* Key is sorted as unsigned chars */
  HA_KEYTYPE_BIT = 19
};

#define HA_MAX_KEYTYPE 31 /* Must be log2-1 */

/*
  Flags for KEY::flags bitmap.

  Also used for similar bitmaps in storage engines (HP_KEYDEF::flag,
  MI_KEYDEF::flag, ...).
*/

/** Do not allow duplicate records. */
#define HA_NOSAME 1
/** Pack string key to previous key (optimization supported by MyISAM). */
#define HA_PACK_KEY (1 << 1)
/*
  Bits in KEY::flags, MI/HP_KEYDEF::flag which are automatically
  calculated based on other flags/members in these structures
  (often from info about key parts).
*/
/** Some key part packs space. Internal to MyISAM. */
#define HA_SPACE_PACK_USED (1 << 2)
/** Some key part has variable length. Internal to MyISAM and Heap engines. */
#define HA_VAR_LENGTH_KEY (1 << 3)
/**
  Auto-increment key.

  @note Not used by SQL-layer/ for KEY::flags. Only set by MyISAM and
        Heap SEs in MI/HP_KEYDEF::flag.
*/
#define HA_AUTO_KEY (1 << 4)
/** Packing of all keys to previous key (optimization supported by MyISAM). */
#define HA_BINARY_PACK_KEY (1 << 5)
/** Some key part is nullable. */
#define HA_NULL_PART_KEY (1 << 6)
/** Full-text key. */
#define HA_FULLTEXT (1 << 7)
/**
  Flag in MI_KEYDEF::flag which marks MyISAM's "uniques".

  @note Internal to MyISAM. Current server doesn't use this feature.
*/
#define HA_UNIQUE_CHECK (1 << 8)
/** Internal bit used when sorting records. Internal to MyISAM. */
#define HA_SORT_ALLOWS_SAME (1 << 9)
/** Spatial key. */
#define HA_SPATIAL (1 << 10)
/**
  NULLs in key are compared as equal.

  @note Used only for internal temporary tables created by optimizer.
*/
#define HA_NULL_ARE_EQUAL (1 << 11)
/** Key has comment. */
#define HA_USES_COMMENT (1 << 12)
/** Key was automatically created to support Foreign Key constraint. */
#define HA_GENERATED_KEY (1 << 13)

/* The combination of the above can be used for key type comparison. */
#define HA_KEYFLAG_MASK                                                       \
  (HA_NOSAME | HA_PACK_KEY | HA_AUTO_KEY | HA_BINARY_PACK_KEY | HA_FULLTEXT | \
   HA_UNIQUE_CHECK | HA_SPATIAL | HA_NULL_ARE_EQUAL | HA_GENERATED_KEY)

/** Fulltext index uses [pre]parser */
#define HA_USES_PARSER (1 << 14)
/** Key uses KEY_BLOCK_SIZE option. */
#define HA_USES_BLOCK_SIZE (1 << 15)
/**
  Key contains partial segments.

  @note This flag is internal to SQL-layer by design. It is not supposed to
        be used to storage engines. It is intended to pass information into
        internal static sort_keys(KEY *, KEY *) function.

  This flag can be calculated -- it's based on key lengths comparison.
*/
#define HA_KEY_HAS_PART_KEY_SEG (1 << 16)
/**
  Key was renamed (or is result of renaming a key).

  This is another flag internal to SQL-layer.
  Used by in-place ALTER TABLE implementation.

  @note This flag can be set for keys which have other changes than
        simple renaming as well. So from the point of view of storage
        engine such key might have to be dropped and re-created with
        new definition.
*/
#define HA_KEY_RENAMED (1 << 17)
/** Set if a key is on any virtual generated columns */
#define HA_VIRTUAL_GEN_KEY (1 << 18)
/** Multi-valued key */
#define HA_MULTI_VALUED_KEY (1 << 19)

/* These flags can be added to key-seg-flag */

#define HA_SPACE_PACK 1   /* Pack space in key-seg */
#define HA_PART_KEY_SEG 4 /* Used by MySQL for part-key-cols */
#define HA_VAR_LENGTH_PART 8
#define HA_NULL_PART 16
#define HA_BLOB_PART 32
#define HA_SWAP_KEY 64
#define HA_REVERSE_SORT 128 /* Sort key in reverse order */
#define HA_NO_SORT 256      /* do not bother sorting on this keyseg */
/*
  End space in unique/varchar are considered equal. (Like 'a' and 'a ')
  Only needed for internal temporary tables.
*/
#define HA_END_SPACE_ARE_EQUAL 512
#define HA_BIT_PART 1024

/*
  Flags for HA_CREATE_INFO::table_options and TABLE_SHARE::db_create_options
  TABLE_SHARE::db_options_in_use bitmaps.

  @note These bitmaps are used for storing information about some table
        option values/attributes.
  @note HA_CREATE_INFO::table_options and TABLE_SHARE::db_create_options
        are basically the same bitmap at the time of table creation and
        at the time of table opening/usage correspondingly.
  @note TABLE_SHARE::db_options_in_use is normally copy of db_create_options
        but can be overriden by SE. E.g. MyISAM does this at handler::open()
        and hander::info() time.

  Also used for similar bitmaps in MyISAM (MYISAM_SHARE::options,
  MI_ISAMINFO::options).
*/

/**
  Indicates that storage engine needs to use packed row format.
  Set for tables with ROW_FORMAT=DYNAMIC clause, for tables with BLOB fields,
  and for tables with VARCHAR columns without ROW_FORMAT=FIXED.

  This flag is respected by MyISAM only (it might also decide to use this
  optimization for its own reasons). InnoDB relies on HA_CREATE_INFO::row_type
  directly instead.
*/
#define HA_OPTION_PACK_RECORD 1
/**
  PACK_KEYS=1 option was specified.

  PACK_KEYS=# option specifies whether key packing - optimization supported
  by MyISAM, should be used.
  * PACK_KEYS=1 means all keys should be packed,
  * PACK_KEYS=0 (denoted by @sa HA_OPTION_NO_PACK_KEYS flag) means that key
    packing should not be used at all.
  * Not using this option or using PACK_KEYS=DEFAULT clause (denoted by
    absence of both HA_OPTION_PACK_KEYS and HA_OPTION_NO_PACK_KEYS flags)
    means that key packing will be used for long string columns.
*/
#define HA_OPTION_PACK_KEYS 2
/**
  Flag indicating that table is compressed. Used by MyISAM storage engine to
  tell SQL-layer that tables is compressed. Not set or stored by SQL-layer,

  MyISAM doesn't respect ROW_FORMAT=COMPRESSED clause and doesn't allow direct
  creation of compressed tables. Existing tables can be compressed by external
  tool. This tool marks such tables with HA_OPTION_COMPRESS_RECORD flag in
  MYISAM_SHARE::options, MI_ISAMINFO::options. Then storage engine sets this
  flag in TABLE_SHARE::db_options_in_use to let SQL-layer know about the fact.
  It is never set in HA_CREATE_INFO::table_options or
  TABLE_SHARE::db_create_options.
*/
#define HA_OPTION_COMPRESS_RECORD 4
/**
  Unused. Formerly HA_OPTION_LONG_BLOB_PTR - new ISAM format/portable
  BLOB pointers.
*/
#define HA_OPTION_UNUSED1 8
/**
  Storage engine (MyISAM) internal flag for marking temporary tables.

  Used in MYISAM_SHARE::options, MI_ISAMINFO::options, not used by SQL-layer,
  in HA_CREATE_INFO::table_options or TABLE_SHARE::db_create_options.
*/
#define HA_OPTION_TMP_TABLE 16
/**
  CHECKSUM=1 option was specified.

  This option enables live checksumming for MyISAM tables. Not used by InnoDB.
  @sa HA_OPTION_NO_CHECKSUM.
*/
#define HA_OPTION_CHECKSUM 32
/**
  DELAY_KEY_WRITE=1 option was specified. This option enables MyISAM
  optimization which postpones key updates until table is closed/expelled from
  the table cache.

  @sa HA_OPTION_NO_DELAY_KEY_WRITE.
*/
#define HA_OPTION_DELAY_KEY_WRITE 64
/**
  PACK_KEYS=0 option was specified.

  @sa HA_OPTION_PACK_KEYS for further description.
  @note Unlike HA_OPTION_PACK_KEYS this flag is SQL-layer only.
*/
#define HA_OPTION_NO_PACK_KEYS 128
/**
  Flag specific to table creation and HA_CREATE_INFO::table_options.
  Indicates that storage engine instead of creating new table, needs
  to discover existing one (which metadata was discovered from SE
  earlier).
  Not used in TABLE_SHARE::db_create_options/db_options_in_use.
*/
#define HA_OPTION_CREATE_FROM_ENGINE 256
/**
  Storage engine (MyISAM) internal flag for marking tables which
  rely on SQL-layer because they have keys using fulltext parser plugin.

  Used in MYISAM_SHARE::options, MI_ISAMINFO::options, not used by SQL-layer,
  in HA_CREATE_INFO::table_options or TABLE_SHARE::db_create_options.
*/
#define HA_OPTION_RELIES_ON_SQL_LAYER 512
/** Unused. Formerly HA_OPTION_NULL_FIELDS - reserved for Maria SE. */
#define HA_OPTION_UNUSED2 1024
/** Unused. Formerly HA_OPTION_PAGE_CHECKSUM - reserved for Maria SE. */
#define HA_OPTION_UNUSED3 2048
/** STATS_PERSISTENT=1 has been specified in the SQL command (either CREATE
or ALTER TABLE). Table and index statistics that are collected by the
storage engine and used by the optimizer for query optimization will be
stored on disk and will not change after a server restart. */
#define HA_OPTION_STATS_PERSISTENT 4096
/** STATS_PERSISTENT=0 has been specified in CREATE/ALTER TABLE. Statistics
for the table will be wiped away on server shutdown and new ones recalculated
after the server is started again. If none of HA_OPTION_STATS_PERSISTENT or
HA_OPTION_NO_STATS_PERSISTENT is set, this means that the setting is not
explicitly set at table level and the corresponding table will use whatever
is the global server default. */
#define HA_OPTION_NO_STATS_PERSISTENT 8192
/**
  MyISAM internal flag used by myisamchk external tool.

  Not used by SQL-layer/in HA_CREATE_INFO::table_options and
  TABLE_SHARE::db_create_options.
*/
#define HA_OPTION_TEMP_COMPRESS_RECORD 16384
/**
  MyISAM internal flag which marks table as read-only.
  Set by myisampack external tool.

  Not used by SQL-layer/in HA_CREATE_INFO::table_options and
  TABLE_SHARE::db_create_options.
*/
#define HA_OPTION_READ_ONLY_DATA 32768
/**
  CHECKSUM=0 option was specified. Only used by SQL-layer, in
  HA_CREATE_INFO::table_options. Not persisted in data-dictionary.
*/
#define HA_OPTION_NO_CHECKSUM (1L << 17)
/**
  DELAY_KEY_WRITE=0 option was specified. Only used by SQL-layer, in
  HA_CREATE_INFO::table_options. Not persisted in data-dictionary.
*/
#define HA_OPTION_NO_DELAY_KEY_WRITE (1L << 18)

/* Bits in flag to create() */

#define HA_DONT_TOUCH_DATA 1 /* Don't empty datafile (isamchk) */
#define HA_PACK_RECORD 2     /* Request packed record format */
#define HA_CREATE_TMP_TABLE 4
#define HA_CREATE_CHECKSUM 8
#define HA_CREATE_KEEP_FILES 16 /* don't overwrite .MYD and MYI */
#define HA_CREATE_PAGE_CHECKSUM 32
#define HA_CREATE_DELAY_KEY_WRITE 64
#define HA_CREATE_RELIES_ON_SQL_LAYER 128
#define HA_CREATE_INTERNAL_TABLE 256

/*
  The following flags (OR-ed) are passed to handler::info() method.
  The method copies misc handler information out of the storage engine
  to data structures accessible from MySQL

  Same flags are also passed down to mi_status, myrg_status, etc.
*/

/* this one is not used */
#define HA_STATUS_POS 1
/*
  assuming the table keeps shared actual copy of the 'info' and
  local, possibly outdated copy, the following flag means that
  it should not try to get the actual data (locking the shared structure)
  slightly outdated version will suffice
*/
#define HA_STATUS_NO_LOCK 2
/* update the time of the last modification (in handler::update_time) */
#define HA_STATUS_TIME 4
/*
  Recalculate loads of constant variables. MyISAM also sets things
  directly on the table share object.

  Check whether this should be fixed since handlers should not
  change things directly on the table object.

  Monty comment: This should NOT be changed!  It's the handlers
  responsibility to correct table->s->keys_xxxx information if keys
  have been disabled.

  The most important parameters set here is records per key on
  all indexes. block_size and primar key ref_length.

  For each index there is an array of rec_per_key.
  As an example if we have an index with three attributes a,b and c
  we will have an array of 3 rec_per_key.
  rec_per_key[0] is an estimate of number of records divided by
  number of unique values of the field a.
  rec_per_key[1] is an estimate of the number of records divided
  by the number of unique combinations of the fields a and b.
  rec_per_key[2] is an estimate of the number of records divided
  by the number of unique combinations of the fields a,b and c.

  Many handlers only set the value of rec_per_key when all fields
  are bound (rec_per_key[2] in the example above).

  If the handler doesn't support statistics, it should set all of the
  above to 0.

  update the 'constant' part of the info:
  handler::max_data_file_length, max_index_file_length, create_time
  sortkey, ref_length, block_size, data_file_name, index_file_name.
  handler::table->s->keys_in_use, keys_for_keyread, rec_per_key
*/
#define HA_STATUS_CONST 8
/*
  update the 'variable' part of the info:
  handler::records, deleted, data_file_length, index_file_length,
  check_time, mean_rec_length
*/
#define HA_STATUS_VARIABLE 16
/*
  This flag is used to get index number of the unique index that
  reported duplicate key.
  update handler::errkey and handler::dupp_ref
  see handler::get_dup_key()
*/
#define HA_STATUS_ERRKEY 32
/*
  update handler::auto_increment_value
*/
#define HA_STATUS_AUTO 64
/*
  Get also delete_length when HA_STATUS_VARIABLE is called. It's ok to set it
  also when only HA_STATUS_VARIABLE but it won't be used.
*/
#define HA_STATUS_VARIABLE_EXTRA 128

/*
  Errorcodes given by handler functions

  Do not add error numbers before HA_ERR_FIRST.
  If necessary to add lower numbers, change HA_ERR_FIRST accordingly.
*/
/** Copy of first error nr.*/
#define HA_ERR_FIRST 120
/** Didn't find key on read or update */
#define HA_ERR_KEY_NOT_FOUND 120
/** Dupplicate key on write */
#define HA_ERR_FOUND_DUPP_KEY 121
/** Internal error */
#define HA_ERR_INTERNAL_ERROR 122
/** Uppdate with is recoverable */
#define HA_ERR_RECORD_CHANGED 123
/** Wrong index given to function */
#define HA_ERR_WRONG_INDEX 124
/** Indexfile is crashed */
#define HA_ERR_CRASHED 126
/** Record-file is crashed */
#define HA_ERR_WRONG_IN_RECORD 127
/** Record-file is crashed */
#define HA_ERR_OUT_OF_MEM 128
/** not a MYI file - no signature */
#define HA_ERR_NOT_A_TABLE 130
/** Command not supported */
#define HA_ERR_WRONG_COMMAND 131
/** old database file */
#define HA_ERR_OLD_FILE 132
/** No record read in update() */
#define HA_ERR_NO_ACTIVE_RECORD 133
/** A record is not there */
#define HA_ERR_RECORD_DELETED 134
/** No more room in file */
#define HA_ERR_RECORD_FILE_FULL 135
/** No more room in file */
#define HA_ERR_INDEX_FILE_FULL 136
/** end in next/prev/first/last */
#define HA_ERR_END_OF_FILE 137
/** unsupported extension used */
#define HA_ERR_UNSUPPORTED 138
/** Too big row */
#define HA_ERR_TOO_BIG_ROW 139
/** Wrong create option */
#define HA_WRONG_CREATE_OPTION 140
/** Dupplicate unique on write */
#define HA_ERR_FOUND_DUPP_UNIQUE 141
/** Can't open charset */
#define HA_ERR_UNKNOWN_CHARSET 142
/** conflicting tables in MERGE */
#define HA_ERR_WRONG_MRG_TABLE_DEF 143
/** Last (automatic?) repair failed */
#define HA_ERR_CRASHED_ON_REPAIR 144
/** Table must be repaired */
#define HA_ERR_CRASHED_ON_USAGE 145
#define HA_ERR_LOCK_WAIT_TIMEOUT 146
#define HA_ERR_LOCK_TABLE_FULL 147
/** Updates not allowed */
#define HA_ERR_READ_ONLY_TRANSACTION 148
#define HA_ERR_LOCK_DEADLOCK 149
/** Cannot add a foreign key constr. */
#define HA_ERR_CANNOT_ADD_FOREIGN 150
/** Cannot add a child row */
#define HA_ERR_NO_REFERENCED_ROW 151
/** Cannot delete a parent row */
#define HA_ERR_ROW_IS_REFERENCED 152
/** No savepoint with that name */
#define HA_ERR_NO_SAVEPOINT 153
/** Non unique key block size */
#define HA_ERR_NON_UNIQUE_BLOCK_SIZE 154
/** The table does not exist in engine */
#define HA_ERR_NO_SUCH_TABLE 155
/** The table existed in storage engine */
#define HA_ERR_TABLE_EXIST 156
/** Could not connect to storage engine */
#define HA_ERR_NO_CONNECTION 157
/** NULLs are not supported in spatial index */
#define HA_ERR_NULL_IN_SPATIAL 158
/** The table changed in storage engine */
#define HA_ERR_TABLE_DEF_CHANGED 159
/** There's no partition in table for given value */
#define HA_ERR_NO_PARTITION_FOUND 160
/** Row-based binlogging of row failed */
#define HA_ERR_RBR_LOGGING_FAILED 161
/** Index needed in foreign key constraint */
#define HA_ERR_DROP_INDEX_FK 162
/** Upholding foreign key constraints would lead to a duplicate key error
in some other table. */
#define HA_ERR_FOREIGN_DUPLICATE_KEY 163
/** The table changed in storage engine */
#define HA_ERR_TABLE_NEEDS_UPGRADE 164
/** The table is not writable */
#define HA_ERR_TABLE_READONLY 165
/** Failed to get next autoinc value */
#define HA_ERR_AUTOINC_READ_FAILED 166
/** Failed to set row autoinc value */
#define HA_ERR_AUTOINC_ERANGE 167
/** Generic error */
#define HA_ERR_GENERIC 168
/** row not actually updated: new values same as the old values */
#define HA_ERR_RECORD_IS_THE_SAME 169
/** It is not possible to log this statement */
#define HA_ERR_LOGGING_IMPOSSIBLE 170
/** The event was corrupt, leading to illegal data being read */
#define HA_ERR_CORRUPT_EVENT 171
/** New file format */
#define HA_ERR_NEW_FILE 172
/** The event could not be processed no other hanlder error happened */
#define HA_ERR_ROWS_EVENT_APPLY 173
/** Error during initialization */
#define HA_ERR_INITIALIZATION 174
/** File too short */
#define HA_ERR_FILE_TOO_SHORT 175
/** Wrong CRC on page */
#define HA_ERR_WRONG_CRC 176
/** Too many active concurrent transactions */
#define HA_ERR_TOO_MANY_CONCURRENT_TRXS 177
/** There's no explicitly listed partition in table for the given value */
#define HA_ERR_NOT_IN_LOCK_PARTITIONS 178
/** Index column length exceeds limit */
#define HA_ERR_INDEX_COL_TOO_LONG 179
/** InnoDB index corrupted */
#define HA_ERR_INDEX_CORRUPT 180
/** Undo log record too big */
#define HA_ERR_UNDO_REC_TOO_BIG 181
/** Invalid InnoDB Doc ID */
#define HA_FTS_INVALID_DOCID 182
/** Table being used in foreign key check */
#define HA_ERR_TABLE_IN_FK_CHECK 183
/** The tablespace existed in storage engine */
#define HA_ERR_TABLESPACE_EXISTS 184
/** Table has too many columns */
#define HA_ERR_TOO_MANY_FIELDS 185
/** Row in wrong partition */
#define HA_ERR_ROW_IN_WRONG_PARTITION 186
/** InnoDB is in read only mode. */
#define HA_ERR_INNODB_READ_ONLY 187
/** FTS query exceeds result cache limit */
#define HA_ERR_FTS_EXCEED_RESULT_CACHE_LIMIT 188
/** Temporary file write failure */
#define HA_ERR_TEMP_FILE_WRITE_FAILURE 189
/** Innodb is in force recovery mode */
#define HA_ERR_INNODB_FORCED_RECOVERY 190
/** Too many words in a phrase */
#define HA_ERR_FTS_TOO_MANY_WORDS_IN_PHRASE 191
/** FK cascade depth exceeded */
#define HA_ERR_FK_DEPTH_EXCEEDED 192
/** Option Missing during Create */
#define HA_MISSING_CREATE_OPTION 193
/** Out of memory in storage engine */
#define HA_ERR_SE_OUT_OF_MEMORY 194
/** Table/Clustered index is corrupted. */
#define HA_ERR_TABLE_CORRUPT 195
/** The query was interrupted */
#define HA_ERR_QUERY_INTERRUPTED 196
/** Missing Tablespace */
#define HA_ERR_TABLESPACE_MISSING 197
/** Tablespace is not empty */
#define HA_ERR_TABLESPACE_IS_NOT_EMPTY 198
/** Invalid Filename */
#define HA_ERR_WRONG_FILE_NAME 199
/** Operation is not allowed */
#define HA_ERR_NOT_ALLOWED_COMMAND 200
/** Compute generated column value failed */
#define HA_ERR_COMPUTE_FAILED 201
/** Table's row format has changed in the storage engine.
Information in the data-dictionary needs to be updated. */
#define HA_ERR_ROW_FORMAT_CHANGED 202
/** Don't wait for record lock */
#define HA_ERR_NO_WAIT_LOCK 203
/** No more room in disk */
#define HA_ERR_DISK_FULL_NOWAIT 204
/** No session temporary space available */
#define HA_ERR_NO_SESSION_TEMP 205
/** Wrong or Invalid table name */
#define HA_ERR_WRONG_TABLE_NAME 206
/** Path is too long for the OS */
#define HA_ERR_TOO_LONG_PATH 207
/** Histogram sampling initialization failed */
#define HA_ERR_SAMPLING_INIT_FAILED 208
/** Write query exceeded cpu time limit */
#define HA_ERR_WRITE_CPU_LIMIT_EXCEEDED 209
/* Tmp disk usage exceeded max */
#define HA_ERR_MAX_TMP_DISK_USAGE_EXCEEDED 210
/** Copy of last error number */
#define HA_ERR_LAST 210

/* Number of different errors */
#define HA_ERR_ERRORS (HA_ERR_LAST - HA_ERR_FIRST + 1)

/* Other constants */

#define HA_NAMELEN 64          /* Max length of saved filename */
#define NO_SUCH_KEY (~(uint)0) /* used as a key no. */

typedef ulong key_part_map;
#define HA_WHOLE_KEY (~(key_part_map)0)

/* Intern constants in databases */

/* bits in _search */
#define SEARCH_FIND 1
#define SEARCH_NO_FIND 2
#define SEARCH_SAME 4
#define SEARCH_BIGGER 8
#define SEARCH_SMALLER 16
#define SEARCH_SAVE_BUFF 32
#define SEARCH_UPDATE 64
#define SEARCH_PREFIX 128
#define SEARCH_LAST 256
#define MBR_CONTAIN 512
#define MBR_INTERSECT 1024
#define MBR_WITHIN 2048
#define MBR_DISJOINT 4096
#define MBR_EQUAL 8192
#define MBR_DATA 16384
#define SEARCH_NULL_ARE_EQUAL 32768     /* NULL in keys are equal */
#define SEARCH_NULL_ARE_NOT_EQUAL 65536 /* NULL in keys are not equal */

/* bits in opt_flag */
#define QUICK_USED 1
#define READ_CACHE_USED 2
#define READ_CHECK_USED 4
#define KEY_READ_USED 8
#define WRITE_CACHE_USED 16
#define OPT_NO_ROWS 32

/* bits in update */
#define HA_STATE_CHANGED 1 /* Database has changed */
#define HA_STATE_AKTIV 2   /* Has a current record */
#define HA_STATE_WRITTEN 4 /* Record is written */
#define HA_STATE_DELETED 8
#define HA_STATE_NEXT_FOUND 16 /* Next found record (record before) */
#define HA_STATE_PREV_FOUND 32 /* Prev found record (record after) */
#define HA_STATE_NO_KEY 64     /* Last read didn't find record */
#define HA_STATE_KEY_CHANGED 128
#define HA_STATE_WRITE_AT_END 256 /* set in _ps_find_writepos */
#define HA_STATE_BUFF_SAVED 512   /* If current keybuff is info->buff */
#define HA_STATE_ROW_CHANGED 1024 /* To invalide ROW cache */
#define HA_STATE_EXTEND_BLOCK 2048

/* myisampack expects no more than 32 field types. */
enum en_fieldtype {
  FIELD_LAST = -1,
  FIELD_NORMAL,
  FIELD_SKIP_ENDSPACE,
  FIELD_SKIP_PRESPACE,
  FIELD_SKIP_ZERO,
  FIELD_BLOB,
  FIELD_CONSTANT,
  FIELD_INTERVALL,
  FIELD_ZERO,
  FIELD_VARCHAR,
  FIELD_CHECK,
  FIELD_enum_val_count
};

enum data_file_type {
  STATIC_RECORD,
  DYNAMIC_RECORD,
  COMPRESSED_RECORD,
  BLOCK_RECORD
};

/* For key ranges */

enum key_range_flags {
  NO_MIN_RANGE = 1 << 0,  ///< from -inf
  NO_MAX_RANGE = 1 << 1,  ///< to +inf
  /*  X < key, i.e. not including the left endpoint */
  NEAR_MIN = 1 << 2,
  /* X > key, i.e. not including the right endpoint */
  NEAR_MAX = 1 << 3,
  /*
    This flag means that index is a unique index, and the interval is
    equivalent to "AND(keypart_i = const_i)", where all of const_i are
    not NULLs.
  */
  UNIQUE_RANGE = 1 << 4,
  /*
    This flag means that the interval is equivalent to
    "AND(keypart_i = const_i)", where not all key parts may be used
    but all of const_i are not NULLs.
  */
  EQ_RANGE = 1 << 5,
  /*
    This flag has the same meaning as UNIQUE_RANGE, except that for at
    least one keypart the condition is "keypart IS NULL".
  */
  NULL_RANGE = 1 << 6,
  /**
    This flag means that the index is an rtree index, and the interval is
    specified using HA_READ_MBR_XXX defined in enum ha_rkey_function.
  */
  GEOM_FLAG = 1 << 7,
  /* Deprecated, currently used only by NDB at row retrieval */
  SKIP_RANGE = 1 << 8,
  /*
    Used to indicate that index dives can be skipped. This can happen when:
    a) Query satisfies JOIN_TAB::check_skip_records_in_range_qualification. OR
    b) Used together with EQ_RANGE to indicate that index statistics should be
       used instead of sampling the index.
  */
  SKIP_RECORDS_IN_RANGE = 1 << 9,
  /*
    Keypart is reverse-ordered (DESC) and ranges needs to be scanned
    backward. @see quick_range_seq_init, get_quick_keys.
  */
  DESC_FLAG = 1 << 10,
};

struct key_range {
  const uchar *key;
  uint length;
  key_part_map keypart_map;
  enum ha_rkey_function flag;
};

struct KEY_MULTI_RANGE {
  key_range start_key;
  key_range end_key;
  char *ptr;       /* Free to use by caller (ptr to row etc) */
  uint range_flag; /* key range flags see above */
};

/* For number of records */
#define rows2double(A) ulonglong2double(A)
typedef my_off_t ha_rows;

#define HA_POS_ERROR (~(ha_rows)0)
#define HA_OFFSET_ERROR (~(my_off_t)0)

#define MAX_FILE_SIZE LLONG_MAX

#define HA_VARCHAR_PACKLENGTH(field_length) ((field_length) < 256 ? 1 : 2)

#endif /* _my_base_h */
