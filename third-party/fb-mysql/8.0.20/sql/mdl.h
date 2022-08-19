#ifndef MDL_H
#define MDL_H
/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <new>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "m_string.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_systime.h"  // Timout_type
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_mdl_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql_com.h"
#include "sql/sql_plist.h"
#include "sql_string.h"
#include "template_utils.h"

class MDL_context;
class MDL_lock;
class MDL_ticket;
class THD;
struct LF_PINS;
struct MDL_key;
struct MEM_ROOT;

/**
  @def ENTER_COND(C, M, S, O)
  Start a wait on a condition.
  @param C the condition to wait on
  @param M the associated mutex
  @param S the new stage to enter
  @param O the previous stage
  @sa EXIT_COND().
*/
#define ENTER_COND(C, M, S, O) \
  enter_cond(C, M, S, O, __func__, __FILE__, __LINE__)

/**
  @def EXIT_COND(S)
  End a wait on a condition
  @param S the new stage to enter
*/
#define EXIT_COND(S) exit_cond(S, __func__, __FILE__, __LINE__)

/**
   An interface to separate the MDL module from the THD, and the rest of the
   server code.
 */

class MDL_context_owner {
 public:
  virtual ~MDL_context_owner() {}

  /**
    Enter a condition wait.
    For @c enter_cond() / @c exit_cond() to work the mutex must be held before
    @c enter_cond(); this mutex must then be released before @c exit_cond().
    Usage must be: lock mutex; enter_cond(); your code; unlock mutex;
    exit_cond().
    @param cond the condition to wait on
    @param mutex the associated mutex
    @param [in] stage the stage to enter, or NULL
    @param [out] old_stage the previous stage, or NULL
    @param src_function function name of the caller
    @param src_file file name of the caller
    @param src_line line number of the caller
    @sa ENTER_COND(), THD::enter_cond()
    @sa EXIT_COND(), THD::exit_cond()
  */
  virtual void enter_cond(mysql_cond_t *cond, mysql_mutex_t *mutex,
                          const PSI_stage_info *stage,
                          PSI_stage_info *old_stage, const char *src_function,
                          const char *src_file, int src_line) = 0;

  /**
    End a wait on a condition
    @param [in] stage the new stage to enter
    @param src_function function name of the caller
    @param src_file file name of the caller
    @param src_line line number of the caller
    @sa ENTER_COND(), THD::enter_cond()
    @sa EXIT_COND(), THD::exit_cond()
  */
  virtual void exit_cond(const PSI_stage_info *stage, const char *src_function,
                         const char *src_file, int src_line) = 0;
  /**
     Has the owner thread been killed?
   */
  virtual int is_killed() const = 0;

  /**
    Does the owner still have connection to the client?
  */
  virtual bool is_connected() = 0;

  /**
     Within MDL subsystem this one is only used for DEBUG_SYNC.
     Do not use it to peek/poke into other parts of THD from MDL.
     However it is OK to use this method in callbacks provided
     by SQL-layer to MDL subsystem (since SQL-layer has full
     access to THD anyway).

     @warning For some derived classes implementation of this method
              can return nullptr. Calling side must be ready to handle
              this case.
   */
  virtual THD *get_thd() = 0;

  /**
     @see THD::notify_shared_lock()
   */
  virtual void notify_shared_lock(MDL_context_owner *in_use,
                                  bool needs_thr_lock_abort) = 0;

  virtual bool kill_shared_lock(MDL_context_owner *in_use) = 0;

  /**
    Notify/get permission from interested storage engines before acquiring
    exclusive lock for the key.

    The returned argument 'victimized' specify reason for lock
    not granted. If 'true', lock was refused in an attempt to
    resolve a possible MDL->GSL deadlock. Locking may then be retried.

    @return False if notification was successful and it is OK to acquire lock,
            True if one of SEs asks to abort lock acquisition.
  */
  virtual bool notify_hton_pre_acquire_exclusive(const MDL_key *mdl_key,
                                                 bool *victimized) = 0;
  /**
    Notify interested storage engines that we have just released exclusive
    lock for the key.
  */
  virtual void notify_hton_post_release_exclusive(const MDL_key *mdl_key) = 0;

  /**
    Get random seed specific to this THD to be used for initialization
    of PRNG for the MDL_context.
  */
  virtual uint get_rand_seed() const = 0;
};

/**
  Type of metadata lock request.

  @sa Comments for MDL_object_lock::can_grant_lock() and
      MDL_scoped_lock::can_grant_lock() for details.
*/

enum enum_mdl_type {
  /*
    An intention exclusive metadata lock. Used only for scoped locks.
    Owner of this type of lock can acquire upgradable exclusive locks on
    individual objects.
    This lock type is also used when doing lookups in the dictionary
    cache. When acquiring objects in a schema, we lock the schema with IX
    to prevent the schema from being deleted. This should conceptually
    be an IS lock, but it would have the same behavior as the current IX.
    Compatible with other IX locks, but is incompatible with scoped S and
    X locks.
  */
  MDL_INTENTION_EXCLUSIVE = 0,
  /*
    A shared metadata lock.
    To be used in cases when we are interested in object metadata only
    and there is no intention to access object data (e.g. for stored
    routines or during preparing prepared statements).
    We also mis-use this type of lock for open HANDLERs, since lock
    acquired by this statement has to be compatible with lock acquired
    by LOCK TABLES ... WRITE statement, i.e. SNRW (We can't get by by
    acquiring S lock at HANDLER ... OPEN time and upgrading it to SR
    lock for HANDLER ... READ as it doesn't solve problem with need
    to abort DML statements which wait on table level lock while having
    open HANDLER in the same connection).
    To avoid deadlock which may occur when SNRW lock is being upgraded to
    X lock for table on which there is an active S lock which is owned by
    thread which waits in its turn for table-level lock owned by thread
    performing upgrade we have to use thr_abort_locks_for_thread()
    facility in such situation.
    This problem does not arise for locks on stored routines as we don't
    use SNRW locks for them. It also does not arise when S locks are used
    during PREPARE calls as table-level locks are not acquired in this
    case.
  */
  MDL_SHARED,
  /*
    A high priority shared metadata lock.
    Used for cases when there is no intention to access object data (i.e.
    data in the table).
    "High priority" means that, unlike other shared locks, it is granted
    ignoring pending requests for exclusive locks. Intended for use in
    cases when we only need to access metadata and not data, e.g. when
    filling an INFORMATION_SCHEMA table.
    Since SH lock is compatible with SNRW lock, the connection that
    holds SH lock lock should not try to acquire any kind of table-level
    or row-level lock, as this can lead to a deadlock. Moreover, after
    acquiring SH lock, the connection should not wait for any other
    resource, as it might cause starvation for X locks and a potential
    deadlock during upgrade of SNW or SNRW to X lock (e.g. if the
    upgrading connection holds the resource that is being waited for).
  */
  MDL_SHARED_HIGH_PRIO,
  /*
    A shared metadata lock for cases when there is an intention to read data
    from table.
    A connection holding this kind of lock can read table metadata and read
    table data (after acquiring appropriate table and row-level locks).
    This means that one can only acquire TL_READ, TL_READ_NO_INSERT, and
    similar table-level locks on table if one holds SR MDL lock on it.
    To be used for tables in SELECTs, subqueries, and LOCK TABLE ...  READ
    statements.
  */
  MDL_SHARED_READ,
  /*
    A shared metadata lock for cases when there is an intention to modify
    (and not just read) data in the table.
    A connection holding SW lock can read table metadata and modify or read
    table data (after acquiring appropriate table and row-level locks).
    To be used for tables to be modified by INSERT, UPDATE, DELETE
    statements, but not LOCK TABLE ... WRITE or DDL). Also taken by
    SELECT ... FOR UPDATE.
  */
  MDL_SHARED_WRITE,
  /*
    A version of MDL_SHARED_WRITE lock which has lower priority than
    MDL_SHARED_READ_ONLY locks. Used by DML statements modifying
    tables and using the LOW_PRIORITY clause.
  */
  MDL_SHARED_WRITE_LOW_PRIO,
  /*
    An upgradable shared metadata lock which allows concurrent updates and
    reads of table data.
    A connection holding this kind of lock can read table metadata and read
    table data. It should not modify data as this lock is compatible with
    SRO locks.
    Can be upgraded to SNW, SNRW and X locks. Once SU lock is upgraded to X
    or SNRW lock data modification can happen freely.
    To be used for the first phase of ALTER TABLE.
  */
  MDL_SHARED_UPGRADABLE,
  /*
    A shared metadata lock for cases when we need to read data from table
    and block all concurrent modifications to it (for both data and metadata).
    Used by LOCK TABLES READ statement.
  */
  MDL_SHARED_READ_ONLY,
  /*
    An upgradable shared metadata lock which blocks all attempts to update
    table data, allowing reads.
    A connection holding this kind of lock can read table metadata and read
    table data.
    Can be upgraded to X metadata lock.
    Note, that since this type of lock is not compatible with SNRW or SW
    lock types, acquiring appropriate engine-level locks for reading
    (TL_READ* for MyISAM, shared row locks in InnoDB) should be
    contention-free.
    To be used for the first phase of ALTER TABLE, when copying data between
    tables, to allow concurrent SELECTs from the table, but not UPDATEs.
  */
  MDL_SHARED_NO_WRITE,
  /*
    An upgradable shared metadata lock which allows other connections
    to access table metadata, but not data.
    It blocks all attempts to read or update table data, while allowing
    INFORMATION_SCHEMA and SHOW queries.
    A connection holding this kind of lock can read table metadata modify and
    read table data.
    Can be upgraded to X metadata lock.
    To be used for LOCK TABLES WRITE statement.
    Not compatible with any other lock type except S and SH.
  */
  MDL_SHARED_NO_READ_WRITE,
  /*
    An exclusive metadata lock.
    A connection holding this lock can modify both table's metadata and data.
    No other type of metadata lock can be granted while this lock is held.
    To be used for CREATE/DROP/RENAME TABLE statements and for execution of
    certain phases of other DDL statements.
  */
  MDL_EXCLUSIVE,
  /* This should be the last !!! */
  MDL_TYPE_END
};

/** Duration of metadata lock. */

enum enum_mdl_duration {
  /**
    Locks with statement duration are automatically released at the end
    of statement or transaction.
  */
  MDL_STATEMENT = 0,
  /**
    Locks with transaction duration are automatically released at the end
    of transaction.
  */
  MDL_TRANSACTION,
  /**
    Locks with explicit duration survive the end of statement and transaction.
    They have to be released explicitly by calling MDL_context::release_lock().
  */
  MDL_EXPLICIT,
  /* This should be the last ! */
  MDL_DURATION_END
};

/** Maximal length of key for metadata locking subsystem. */
#define MAX_MDLKEY_LENGTH (1 + NAME_LEN + 1 + NAME_LEN + 1)

/**
  Metadata lock object key.

  A lock is requested or granted based on a fully qualified name and type.
  E.g. They key for a table consists of @<0 (=table)@>+@<database@>+@<table
  name@>. Elsewhere in the comments this triple will be referred to simply as
  "key" or "name".
*/

struct MDL_key {
 public:
#ifdef HAVE_PSI_INTERFACE
  static void init_psi_keys();
#endif

  /**
    Object namespaces.
    Sic: when adding a new member to this enum make sure to
    update m_namespace_to_wait_state_name array in mdl.cc!

    Different types of objects exist in different namespaces
     - GLOBAL is used for the global read lock.
     - BACKUP_LOCK is to block any operations that could cause
       inconsistent backup. Such operations are most DDL statements,
       and some administrative statements.
     - TABLESPACE is for tablespaces.
     - SCHEMA is for schemas (aka databases).
     - TABLE is for tables and views.
     - FUNCTION is for stored functions.
     - PROCEDURE is for stored procedures.
     - TRIGGER is for triggers.
     - EVENT is for event scheduler events.
     - COMMIT is for enabling the global read lock to block commits.
     - USER_LEVEL_LOCK is for user-level locks.
     - LOCKING_SERVICE is for the name plugin RW-lock service
     - SRID is for spatial reference systems
     - ACL_CACHE is for ACL caches
     - COLUMN_STATISTICS is for column statistics, such as histograms
     - RESOURCE_GROUPS is for resource groups.
     - FOREIGN_KEY is for foreign key names.
     - CHECK_CONSTRAINT is for check constraint names.
    Note that requests waiting for user-level locks get special
    treatment - waiting is aborted if connection to client is lost.
  */
  enum enum_mdl_namespace {
    GLOBAL = 0,
    BACKUP_LOCK,
    TABLESPACE,
    SCHEMA,
    TABLE,
    FUNCTION,
    PROCEDURE,
    TRIGGER,
    EVENT,
    COMMIT,
    USER_LEVEL_LOCK,
    LOCKING_SERVICE,
    SRID,
    ACL_CACHE,
    COLUMN_STATISTICS,
    RESOURCE_GROUPS,
    FOREIGN_KEY,
    CHECK_CONSTRAINT,
    /* This should be the last ! */
    NAMESPACE_END
  };

  const uchar *ptr() const { return pointer_cast<const uchar *>(m_ptr); }
  uint length() const { return m_length; }

  const char *db_name() const { return m_ptr + 1; }
  uint db_name_length() const { return m_db_name_length; }

  const char *name() const {
    return (use_normalized_object_name() ? m_ptr + m_length
                                         : m_ptr + m_db_name_length + 2);
  }
  uint name_length() const { return m_object_name_length; }

  const char *col_name() const {
    DBUG_ASSERT(!use_normalized_object_name());

    if (m_db_name_length + m_object_name_length + 3 < m_length) {
      /* A column name was stored in the key buffer. */
      return m_ptr + m_db_name_length + m_object_name_length + 3;
    }

    /* No column name stored. */
    return nullptr;
  }

  uint col_name_length() const {
    DBUG_ASSERT(!use_normalized_object_name());

    if (m_db_name_length + m_object_name_length + 3 < m_length) {
      /* A column name was stored in the key buffer. */
      return m_length - m_db_name_length - m_object_name_length - 4;
    }

    /* No column name stored. */
    return 0;
  }

  enum_mdl_namespace mdl_namespace() const {
    return (enum_mdl_namespace)(m_ptr[0]);
  }

  /**
    Construct a metadata lock key from a triplet (mdl_namespace,
    database and name).

    @remark The key for a table is @<mdl_namespace@>+@<database name@>+@<table
    name@>

    @param  mdl_namespace Id of namespace of object to be locked
    @param  db            Name of database to which the object belongs
    @param  name          Name of of the object
  */
  void mdl_key_init(enum_mdl_namespace mdl_namespace, const char *db,
                    const char *name) {
    m_ptr[0] = (char)mdl_namespace;

    DBUG_ASSERT(!use_normalized_object_name());

    /*
      It is responsibility of caller to ensure that db and object names
      are not longer than NAME_LEN. Still we play safe and try to avoid
      buffer overruns.

      Implicit tablespace names in InnoDB may be longer than NAME_LEN.
      We will lock based on the first NAME_LEN characters.

      TODO: The patch acquires metadata locks on the NAME_LEN
            first bytest of the tablespace names. For long names,
            the consequence of locking on this prefix is
            that locking a single implicit tablespace might end up
            effectively lock all implicit tablespaces in the same
            schema. A possible fix is to lock on a prefix of length
            NAME_LEN * 2, since this is the real buffer size of
            the metadata lock key. Dependencies from the PFS
            implementation, possibly relying on the key format,
            must be investigated first, though.
    */
    DBUG_ASSERT(strlen(db) <= NAME_LEN);
    DBUG_ASSERT((mdl_namespace == TABLESPACE) || (strlen(name) <= NAME_LEN));
    m_db_name_length =
        static_cast<uint16>(strmake(m_ptr + 1, db, NAME_LEN) - m_ptr - 1);
    m_object_name_length = static_cast<uint16>(
        strmake(m_ptr + m_db_name_length + 2, name, NAME_LEN) - m_ptr -
        m_db_name_length - 2);
    m_length = m_db_name_length + m_object_name_length + 3;
  }

  /**
    Construct a metadata lock key from a quadruplet (mdl_namespace,
    database, table and column name).

    @remark The key for a column is
      @<mdl_namespace@>+@<database name@>+@<table name@>+@<column name@>

    @param  mdl_namespace Id of namespace of object to be locked
    @param  db            Name of database to which the object belongs
    @param  name          Name of of the object
    @param  column_name   Name of of the column
  */
  void mdl_key_init(enum_mdl_namespace mdl_namespace, const char *db,
                    const char *name, const char *column_name) {
    m_ptr[0] = (char)mdl_namespace;
    char *start;
    char *end;

    DBUG_ASSERT(!use_normalized_object_name());

    DBUG_ASSERT(strlen(db) <= NAME_LEN);
    start = m_ptr + 1;
    end = strmake(start, db, NAME_LEN);
    m_db_name_length = static_cast<uint16>(end - start);

    DBUG_ASSERT(strlen(name) <= NAME_LEN);
    start = end + 1;
    end = strmake(start, name, NAME_LEN);
    m_object_name_length = static_cast<uint16>(end - start);

    size_t col_len = strlen(column_name);
    DBUG_ASSERT(col_len <= NAME_LEN);
    start = end + 1;
    size_t remaining =
        MAX_MDLKEY_LENGTH - m_db_name_length - m_object_name_length - 3;
    uint16 extra_length = 0;

    /*
      In theory:
      - schema name is up to NAME_LEN characters
      - object name is up to NAME_LEN characters
      - column name is up to NAME_LEN characters
      - NAME_LEN is 64 characters
      - 1 character is up to 3 bytes (UTF8MB3),
        and when moving to UTF8MB4, up to 4 bytes.
      - Storing a SCHEMA + OBJECT MDL key
        can take up to 387 bytes
      - Storing a SCHEMA + OBJECT + COLUMN MDL key
        can take up to 580 bytes.

      In practice:
      - full storage is allocated for SCHEMA + OBJECT only,
        storage for COLUMN is **NOT** reserved.
      - SCHEMA and OBJECT names are typically shorter,
        and are not using systematically multi-bytes characters
        for each character, so that less space is required.
      - MDL keys that are not COLUMN_STATISTICS
        are stored in full, without truncation.

      For the COLUMN_STATISTICS name space:
      - either the full SCHEMA + OBJECT + COLUMN key fits
        within 387 bytes, in which case the fully qualified
        column name is stored,
        leading to MDL locks per column (as intended)
      - or the SCHEMA and OBJECT names are very long,
        so that not enough room is left to store a column name,
        in which case the MDL key is truncated to be
        COLUMN_STATISTICS + SCHEMA + NAME.
        In this case, MDL locks for columns col_X and col_Y
        in table LONG_FOO.LONG_BAR will both share the same
        key LONG_FOO.LONG_BAR, in effect providing a lock
        granularity not per column but per table.
        This is a degraded mode of operation,
        which serializes MDL access to columns
        (for tables with a very long fully qualified name),
        to reduce the memory footprint for all MDL access.

      To be revised if the MDL key buffer is allocated dynamically
      instead.
    */

    static_assert(MAX_MDLKEY_LENGTH == 387, "UTF8MB3");

    /*
      Check if there is room to store the whole column name.
      This code is not trying to store truncated column names,
      to avoid cutting column_name in the middle of a
      multi-byte character.
    */
    if (remaining >= col_len + 1) {
      end = strmake(start, column_name, remaining);
      extra_length = static_cast<uint16>(end - start) + 1;  // With \0
    }
    m_length = m_db_name_length + m_object_name_length + 3 + extra_length;
    DBUG_ASSERT(m_length <= MAX_MDLKEY_LENGTH);
  }

  /**
    Construct a metadata lock key from a quadruplet (mdl_namespace, database,
    normalized object name buffer and the object name).

    @remark The key for a routine/event/resource group/trigger is
      @<mdl_namespace@>+@<database name@>+@<normalized object name@>
      additionaly @<object name@> is stored in the same buffer for information
      purpose if buffer has sufficent space.

    Routine, Event and Resource group names are case sensitive and accent
    sensitive. So normalized object name is used to form a MDL_key.

    With the UTF8MB3 charset space reserved for the db name/object name is
    64 * 3  bytes. utf8_general_ci collation is used for the Routine, Event and
    Resource group names. With this collation, the normalized object name uses
    just 2 bytes for each character (max length = 64 * 2 bytes). MDL_key has
    still some space to store the object names. If there is a sufficient space
    for the object name in the MDL_key then it is stored in the MDL_key (similar
    to the column names in the MDL_key). Actual object name is used by the PFS.
    Not listing actual object name from the PFS should be OK when there is no
    space to store it (instead of increasing the MDL_key size). Object name is
    not used in the key comparisons. So only (mdl_namespace + strlen(db) + 1 +
    normalized_name_len + 1) value is stored in the m_length member.

    @param  mdl_namespace       Id of namespace of object to be locked.
    @param  db                  Name of database to which the object belongs.
    @param  normalized_name     Normalized name of the object.
    @param  normalized_name_len Length of the normalized object name.
    @param  name                Name of the object.
  */
  void mdl_key_init(enum_mdl_namespace mdl_namespace, const char *db,
                    const char *normalized_name, size_t normalized_name_len,
                    const char *name) {
    m_ptr[0] = (char)mdl_namespace;

    /*
      FUNCTION, PROCEDURE, EVENT and RESOURCE_GROUPS names are case and accent
      insensitive. For other objects key should not be formed from this method.
    */
    DBUG_ASSERT(use_normalized_object_name());

    DBUG_ASSERT(strlen(db) <= NAME_LEN && strlen(name) <= NAME_LEN &&
                normalized_name_len <= NAME_CHAR_LEN * 2);

    // Database name.
    m_db_name_length =
        static_cast<uint16>(strmake(m_ptr + 1, db, NAME_LEN) - m_ptr - 1);

    // Normalized object name.
    m_length = static_cast<uint16>(m_db_name_length + normalized_name_len + 3);
    memcpy(m_ptr + m_db_name_length + 2, normalized_name, normalized_name_len);
    *(m_ptr + m_length - 1) = 0;

    /*
      Copy name of the object if there is a sufficient space to store the name
      in the MDL key. This code is not trying to store truncated object names,
      to avoid cutting object_name in the middle of a multi-byte character.
    */
    if (strlen(name) < static_cast<size_t>(MAX_MDLKEY_LENGTH - m_length)) {
      m_object_name_length = static_cast<uint16>(
          (strmake(m_ptr + m_length, name, MAX_MDLKEY_LENGTH - m_length - 1) -
           m_ptr - m_length));
    } else {
      m_object_name_length = 0;
      *(m_ptr + m_length) = 0;
    }

    DBUG_ASSERT(m_length + m_object_name_length < MAX_MDLKEY_LENGTH);
  }

  /**
    Construct a metadata lock key from namespace and partial key, which
    contains info about object database and name.

    @remark The partial key must be "<database>\0<name>\0".

    @param  mdl_namespace   Id of namespace of object to be locked
    @param  part_key        Partial key.
    @param  part_key_length Partial key length
    @param  db_length       Database name length.
  */
  void mdl_key_init(enum_mdl_namespace mdl_namespace, const char *part_key,
                    size_t part_key_length, size_t db_length) {
    /*
      Key suffix provided should be in compatible format and
      its components should adhere to length restrictions.
    */
    DBUG_ASSERT(strlen(part_key) == db_length);
    DBUG_ASSERT(db_length + 1 + strlen(part_key + db_length + 1) + 1 ==
                part_key_length);
    DBUG_ASSERT(db_length <= NAME_LEN);
    DBUG_ASSERT(part_key_length <= NAME_LEN + 1 + NAME_LEN + 1);

    m_ptr[0] = (char)mdl_namespace;
    /*
      Partial key of objects with normalized object name can not be used to
      initialize MDL key.
    */
    DBUG_ASSERT(!use_normalized_object_name());

    memcpy(m_ptr + 1, part_key, part_key_length);
    m_length = static_cast<uint16>(part_key_length + 1);
    m_db_name_length = static_cast<uint16>(db_length);
    m_object_name_length = m_length - m_db_name_length - 3;
  }
  void mdl_key_init(const MDL_key *rhs) {
    uint16 copy_length = rhs->use_normalized_object_name()
                             ? rhs->m_length + rhs->m_object_name_length + 1
                             : rhs->m_length;
    memcpy(m_ptr, rhs->m_ptr, copy_length);
    m_length = rhs->m_length;
    m_db_name_length = rhs->m_db_name_length;
    m_object_name_length = rhs->m_object_name_length;
  }
  void reset() {
    m_ptr[0] = NAMESPACE_END;
    m_db_name_length = 0;
    m_object_name_length = 0;
    m_length = 0;
  }
  bool is_equal(const MDL_key *rhs) const {
    return (m_length == rhs->m_length &&
            memcmp(m_ptr, rhs->m_ptr, m_length) == 0);
  }
  /**
    Compare two MDL keys lexicographically.
  */
  int cmp(const MDL_key *rhs) const {
    /*
      For the keys with the normalized names, there is a possibility of getting
      '\0' in its middle. So only key content comparison would yield incorrect
      result. Hence comparing key length too when keys are equal.
      For other keys, key buffer is always '\0'-terminated. Since key character
      set is utf-8, we can safely assume that no character starts with a zero
      byte.
    */
    int res = memcmp(m_ptr, rhs->m_ptr, std::min(m_length, rhs->m_length));
    if (res == 0) res = m_length - rhs->m_length;
    return res;
  }

  MDL_key(const MDL_key &rhs) { mdl_key_init(&rhs); }

  MDL_key &operator=(const MDL_key &rhs) {
    mdl_key_init(&rhs);
    return *this;
  }

  MDL_key(enum_mdl_namespace namespace_arg, const char *db_arg,
          const char *name_arg) {
    mdl_key_init(namespace_arg, db_arg, name_arg);
  }
  MDL_key() {} /* To use when part of MDL_request. */

  /**
    Get thread state name to be used in case when we have to
    wait on resource identified by key.
  */
  const PSI_stage_info *get_wait_state_name() const noexcept {
    return &m_namespace_to_wait_state_name[(int)mdl_namespace()].stage_info;
  }

  const char *get_namespace_name() const noexcept {
    return m_namespace_to_wait_state_name[(int)mdl_namespace()].namespace_name;
  }

 private:
  /**
    Check if normalized object name should be used.

    @return true if normlized object name should be used, false
    otherwise.
  */
  bool use_normalized_object_name() const {
    return (mdl_namespace() == FUNCTION || mdl_namespace() == PROCEDURE ||
            mdl_namespace() == EVENT || mdl_namespace() == RESOURCE_GROUPS ||
            mdl_namespace() == TRIGGER);
  }

 private:
  uint16 m_length{0};
  uint16 m_db_name_length{0};
  uint16 m_object_name_length{0};
  char m_ptr[MAX_MDLKEY_LENGTH]{0};
  struct PSI_stage_info_with_name {
    PSI_stage_info stage_info;
    const char *namespace_name;
  };
  static PSI_stage_info_with_name m_namespace_to_wait_state_name[NAMESPACE_END];
};

/**
  A pending metadata lock request.

  A lock request and a granted metadata lock are represented by
  different classes because they have different allocation
  sites and hence different lifetimes. The allocation of lock requests is
  controlled from outside of the MDL subsystem, while allocation of granted
  locks (tickets) is controlled within the MDL subsystem.
*/

class MDL_request {
 public:
  /** Type of metadata lock. */
  enum_mdl_type type{MDL_INTENTION_EXCLUSIVE};
  /** Duration for requested lock. */
  enum_mdl_duration duration{MDL_STATEMENT};

  /**
    Pointers for participating in the list of lock requests for this context.
  */
  MDL_request *next_in_list{nullptr};
  MDL_request **prev_in_list{nullptr};
  /**
    Pointer to the lock ticket object for this lock request.
    Valid only if this lock request is satisfied.
  */
  MDL_ticket *ticket{nullptr};

  /** A lock is requested based on a fully qualified name and type. */
  MDL_key key;

  const char *m_src_file{nullptr};
  uint m_src_line{0};

 public:
  static void *operator new(size_t size, MEM_ROOT *mem_root,
                            const std::nothrow_t &arg MY_ATTRIBUTE((unused)) =
                                std::nothrow) noexcept {
    return mem_root->Alloc(size);
  }

  static void operator delete(void *, MEM_ROOT *,
                              const std::nothrow_t &)noexcept {}

  void init_with_source(MDL_key::enum_mdl_namespace namespace_arg,
                        const char *db_arg, const char *name_arg,
                        enum_mdl_type mdl_type_arg,
                        enum_mdl_duration mdl_duration_arg,
                        const char *src_file, uint src_line);
  void init_by_key_with_source(const MDL_key *key_arg,
                               enum_mdl_type mdl_type_arg,
                               enum_mdl_duration mdl_duration_arg,
                               const char *src_file, uint src_line);
  void init_by_part_key_with_source(MDL_key::enum_mdl_namespace namespace_arg,
                                    const char *part_key_arg,
                                    size_t part_key_length_arg,
                                    size_t db_length_arg,
                                    enum_mdl_type mdl_type_arg,
                                    enum_mdl_duration mdl_duration_arg,
                                    const char *src_file, uint src_line);
  /** Set type of lock request. Can be only applied to pending locks. */
  inline void set_type(enum_mdl_type type_arg) {
    DBUG_ASSERT(ticket == nullptr);
    type = type_arg;
  }

  /**
    Is this a request for a lock which allow data to be updated?

    @note This method returns true for MDL_SHARED_UPGRADABLE type of
          lock. Even though this type of lock doesn't allow updates
          it will always be upgraded to one that does.
  */
  bool is_write_lock_request() const {
    return (type >= MDL_SHARED_WRITE && type != MDL_SHARED_READ_ONLY);
  }

  /** Is this a request for a strong, DDL/LOCK TABLES-type, of lock? */
  bool is_ddl_or_lock_tables_lock_request() const {
    return type >= MDL_SHARED_UPGRADABLE;
  }

  /**
    This constructor exists for two reasons:

    - TABLE_LIST objects are sometimes default-constructed. We plan to remove
      this as there is no practical reason, the call to the default
      constructor is always followed by either a call to TABLE_LIST::operator=
      or memberwise assignments.

    - In some legacy cases TABLE_LIST objects are copy-assigned without
      intention to copy the TABLE_LIST::mdl_request member. In this cases they
      are overwritten with an uninitialized MDL_request object. The cases are:

      - Sql_cmd_handler_open::execute()
      - mysql_execute_command()
      - SELECT_LEX_UNIT::prepare()
      - fill_defined_view_parts()

      No new cases are expected.  In all other cases, so far only
      Locked_tables_list::rename_locked_table(), a move assignment is actually
      what is intended.
  */
  MDL_request() {}

  MDL_request(const MDL_request &rhs)
      : type(rhs.type), duration(rhs.duration), ticket(nullptr), key(rhs.key) {}

  MDL_request(MDL_request &&) = default;

  MDL_request &operator=(MDL_request &&) = default;
};

#define MDL_REQUEST_INIT(R, P1, P2, P3, P4, P5) \
  (*R).init_with_source(P1, P2, P3, P4, P5, __FILE__, __LINE__)

#define MDL_REQUEST_INIT_BY_KEY(R, P1, P2, P3) \
  (*R).init_by_key_with_source(P1, P2, P3, __FILE__, __LINE__)

#define MDL_REQUEST_INIT_BY_PART_KEY(R, P1, P2, P3, P4, P5, P6) \
  (*R).init_by_part_key_with_source(P1, P2, P3, P4, P5, P6, __FILE__, __LINE__)

/**
  An abstract class for inspection of a connected
  subgraph of the wait-for graph.
*/

class MDL_wait_for_graph_visitor {
 public:
  virtual bool enter_node(MDL_context *node) = 0;
  virtual void leave_node(MDL_context *node) = 0;

  virtual bool inspect_edge(MDL_context *dest) = 0;
  virtual ~MDL_wait_for_graph_visitor();
  MDL_wait_for_graph_visitor() : m_lock_open_count(0) {}

 public:
  /**
   XXX, hack: During deadlock search, we may need to
   inspect TABLE_SHAREs and acquire LOCK_open. Since
   LOCK_open is not a recursive mutex, count here how many
   times we "took" it (but only take and release once).
   Not using a native recursive mutex or rwlock in 5.5 for
   LOCK_open since it has significant performance impacts.
  */
  uint m_lock_open_count;
};

/**
  Abstract class representing an edge in the waiters graph
  to be traversed by deadlock detection algorithm.
*/

class MDL_wait_for_subgraph {
 public:
  virtual ~MDL_wait_for_subgraph();

  /**
    Accept a wait-for graph visitor to inspect the node
    this edge is leading to.
  */
  virtual bool accept_visitor(MDL_wait_for_graph_visitor *gvisitor) = 0;

  static const uint DEADLOCK_WEIGHT_DML = 0;
  static const uint DEADLOCK_WEIGHT_ULL = 50;
  static const uint DEADLOCK_WEIGHT_DDL = 100;

  /* A helper used to determine which lock request should be aborted. */
  virtual uint get_deadlock_weight() const = 0;
};

/**
  A granted metadata lock.

  @warning MDL_ticket members are private to the MDL subsystem.

  @note Multiple shared locks on a same object are represented by a
        single ticket. The same does not apply for other lock types.

  @note There are two groups of MDL_ticket members:
        - "Externally accessible". These members can be accessed from
          threads/contexts different than ticket owner in cases when
          ticket participates in some list of granted or waiting tickets
          for a lock. Therefore one should change these members before
          including then to waiting/granted lists or while holding lock
          protecting those lists.
        - "Context private". Such members are private to thread/context
          owning this ticket. I.e. they should not be accessed from other
          threads/contexts.
*/

class MDL_ticket : public MDL_wait_for_subgraph {
 public:
  /**
    Pointers for participating in the list of lock requests for this context.
    Context private.
  */
  MDL_ticket *next_in_context;
  MDL_ticket **prev_in_context;

  /**
    Pointers for participating in the list of satisfied/pending requests
    for the lock. Externally accessible.
  */
  MDL_ticket *next_in_lock;
  MDL_ticket **prev_in_lock;

 public:
  bool has_pending_conflicting_lock() const;

  MDL_context *get_ctx() const { return m_ctx; }
  bool is_upgradable_or_exclusive() const {
    return m_type == MDL_SHARED_UPGRADABLE || m_type == MDL_SHARED_NO_WRITE ||
           m_type == MDL_SHARED_NO_READ_WRITE || m_type == MDL_EXCLUSIVE;
  }
  enum_mdl_type get_type() const { return m_type; }
  MDL_lock *get_lock() const { return m_lock; }
  const MDL_key *get_key() const;
  void downgrade_lock(enum_mdl_type type);

  bool has_stronger_or_equal_type(enum_mdl_type type) const;

  bool is_incompatible_when_granted(enum_mdl_type type) const;
  bool is_incompatible_when_waiting(enum_mdl_type type) const;

  /** Implement MDL_wait_for_subgraph interface. */
  virtual bool accept_visitor(MDL_wait_for_graph_visitor *dvisitor);
  virtual uint get_deadlock_weight() const;

#ifndef DBUG_OFF
  enum_mdl_duration get_duration() const { return m_duration; }
  void set_duration(enum_mdl_duration dur) { m_duration = dur; }
#endif

 public:
  /**
    Status of lock request represented by the ticket as reflected in P_S.
  */
  enum enum_psi_status {
    PENDING = 0,
    GRANTED,
    PRE_ACQUIRE_NOTIFY,
    POST_RELEASE_NOTIFY
  };

 private:
  friend class MDL_context;

  MDL_ticket(MDL_context *ctx_arg, enum_mdl_type type_arg
#ifndef DBUG_OFF
             ,
             enum_mdl_duration duration_arg
#endif
             )
      : m_type(type_arg),
#ifndef DBUG_OFF
        m_duration(duration_arg),
#endif
        m_ctx(ctx_arg),
        m_lock(nullptr),
        m_is_fast_path(false),
        m_hton_notified(false),
        m_psi(nullptr) {
  }

  virtual ~MDL_ticket() { DBUG_ASSERT(m_psi == nullptr); }

  static MDL_ticket *create(MDL_context *ctx_arg, enum_mdl_type type_arg
#ifndef DBUG_OFF
                            ,
                            enum_mdl_duration duration_arg
#endif
  );
  static void destroy(MDL_ticket *ticket);

 private:
  /** Type of metadata lock. Externally accessible. */
  enum enum_mdl_type m_type;
#ifndef DBUG_OFF
  /**
    Duration of lock represented by this ticket.
    Context private. Debug-only.
  */
  enum_mdl_duration m_duration;
#endif
  /**
    Context of the owner of the metadata lock ticket. Externally accessible.
  */
  MDL_context *m_ctx;

  /**
    Pointer to the lock object for this lock ticket. Externally accessible.
  */
  MDL_lock *m_lock;

  /**
    Indicates that ticket corresponds to lock acquired using "fast path"
    algorithm. Particularly this means that it was not included into
    MDL_lock::m_granted bitmap/list and instead is accounted for by
    MDL_lock::m_fast_path_locks_granted_counter
  */
  bool m_is_fast_path;

  /**
    Indicates that ticket corresponds to lock request which required
    storage engine notification during its acquisition and requires
    storage engine notification after its release.
  */
  bool m_hton_notified;

  PSI_metadata_lock *m_psi;

 private:
  MDL_ticket(const MDL_ticket &);            /* not implemented */
  MDL_ticket &operator=(const MDL_ticket &); /* not implemented */
};

/**
  Keep track of MDL_ticket for different durations. Maintains a
  hash-based secondary index into the linked lists, to speed up access
  by MDL_key.
 */
class MDL_ticket_store {
 public:
  /**
    Utility struct for representing a ticket pointer and its duration.
   */
  struct MDL_ticket_handle {
    enum_mdl_duration m_dur = MDL_DURATION_END;
    MDL_ticket *m_ticket = nullptr;

    MDL_ticket_handle() = default;
    MDL_ticket_handle(MDL_ticket *t, enum_mdl_duration d)
        : m_dur{d}, m_ticket{t} {}
  };

 private:
  using Ticket_p_list =
      I_P_List<MDL_ticket,
               I_P_List_adapter<MDL_ticket, &MDL_ticket::next_in_context,
                                &MDL_ticket::prev_in_context>>;

  struct Duration {
    Ticket_p_list m_ticket_list;
    /**
      m_mat_front tracks what was the front of m_ticket_list, the last
      time MDL_context::materialize_fast_path_locks() was called. This
      just an optimization which allows
      MDL_context::materialize_fast_path_locks() only to consider the
      locks added since the last time it ran. Consequently, it can be
      assumed that every ticket after m_mat_front is materialized, but
      the converse is not necessarily true as new, already
      materialized, locks may have been added since the last time
      materialize_fast_path_locks() ran.
     */
    MDL_ticket *m_mat_front = nullptr;
  };

  Duration m_durations[MDL_DURATION_END];

  struct Hash {
    size_t operator()(const MDL_key *k) const;
  };

  struct Key_equal {
    bool operator()(const MDL_key *a, const MDL_key *b) const {
      return a->is_equal(b);
    }
  };

  using Ticket_map = std::unordered_multimap<const MDL_key *, MDL_ticket_handle,
                                             Hash, Key_equal>;

  /**
    If the number of tickets in the ticket store (in all durations) is equal
    to, or exceeds this constant the hash index (in the form of an
    unordered_multi_map) will be maintained and used for lookups.

    The value 256 is chosen as it has worked well in benchmarks.
  */
  const size_t THRESHOLD = 256;

  /**
    Initial number of buckets in the hash index. THRESHOLD is chosen
    to get a fill-factor of 50% when reaching the threshold value.
   */
  const size_t INITIAL_BUCKET_COUNT = THRESHOLD * 2;
  size_t m_count = 0;

  std::unique_ptr<Ticket_map> m_map;

  MDL_ticket_handle find_in_lists(const MDL_request &req) const;
  MDL_ticket_handle find_in_hash(const MDL_request &req) const;

 public:
  /**
    Public alias.
  */
  using List_iterator = Ticket_p_list::Iterator;

  /**
    Constructs store. The hash index is initially empty. Filled on demand.
  */
  MDL_ticket_store()
      :  // Comment in to test threshold values in unit test micro benchmark
         // THRESHOLD{read_from_env("TS_THRESHOLD", 500)},
        m_map{nullptr} {}

  /**
    Calls the closure provided as argument for each of the MDL_tickets
    in the given duration.
    @param dur duration list to iterate over
    @param clos closure to invoke for each ticket in the list
   */
  template <typename CLOS>
  void for_each_ticket_in_duration_list(enum_mdl_duration dur, CLOS &&clos) {
    List_iterator it(m_durations[dur].m_ticket_list);
    for (MDL_ticket *t = it++; t != nullptr; t = it++) {
      clos(t, dur);
    }
  }

  /**
    Calls the closure provided as argument for each of the MDL_tickets
    in the store.
    @param clos closure to invoke for each ticket in the store
   */
  template <typename CLOS>
  void for_each_ticket_in_ticket_lists(CLOS &&clos) {
    for_each_ticket_in_duration_list(MDL_STATEMENT, std::forward<CLOS>(clos));
    for_each_ticket_in_duration_list(MDL_TRANSACTION, std::forward<CLOS>(clos));
    for_each_ticket_in_duration_list(MDL_EXPLICIT, std::forward<CLOS>(clos));
  }

  /**
    Predicate for the emptiness of the store.
    @return true if there are no tickets in the store
   */
  bool is_empty() const;

  /**
    Predicate for the emptiness of a given duration list.
    @param di the duration to check
    @return true if there are no tickets with the given duration
  */
  bool is_empty(int di) const;

  /**
    Return the first MDL_ticket for the given duration.

    @param di duration to get first ticket for

    @return first ticket in the given duration or nullptr if no such
    tickets exist
   */
  MDL_ticket *front(int di);

  /**
    Push a ticket onto the list for a given duration.
    @param dur duration list to push into
    @param ticket to push
  */
  void push_front(enum_mdl_duration dur, MDL_ticket *ticket);

  /**
    Remove a ticket from a duration list. Note that since the
    underlying list is an intrusive linked list there is no guarantee
    that the ticket is actually in the duration list. It will be
    removed from which ever list it is in.
   */
  void remove(enum_mdl_duration dur, MDL_ticket *ticket);

  /**
    Return a P-list iterator to the given duration.
    @param di duration list index
    @return P-list iterator to tickets with given duration
   */
  List_iterator list_iterator(int di) const {
    return List_iterator{m_durations[di].m_ticket_list};
  }

  /**
    Move all tickets to the explicit duration list.
   */
  void move_all_to_explicit_duration();

  /**
    Move all tickets to the transaction duration list.
   */
  void move_explicit_to_transaction_duration();

  /**
    Look up a ticket based on its MDL_key.
    @param req request to locate ticket for
    @return MDL_ticket_handle with ticket pointer and found duration
            (or nullptr and MDL_DURATION_END if not found
   */
  MDL_ticket_handle find(const MDL_request &req) const;

  /**
    Mark boundary for tickets with fast_path=false, so that later
    calls to materialize_fast_path_locks() do not have to traverse the
    whole set of tickets.
   */
  void set_materialized();

  /**
    Return the first ticket for which materialize_fast_path_locks
    already has been called for the given duration.

    @param di duration list index
    @return first materialized ticket for the given duration
   */
  MDL_ticket *materialized_front(int di);
};

/**
  Savepoint for MDL context.

  Doesn't include metadata locks with explicit duration as
  they are not released during rollback to savepoint.
*/

class MDL_savepoint {
 public:
  MDL_savepoint() {}

 private:
  MDL_savepoint(MDL_ticket *stmt_ticket, MDL_ticket *trans_ticket)
      : m_stmt_ticket(stmt_ticket), m_trans_ticket(trans_ticket) {}

  friend class MDL_context;

 private:
  /**
    Pointer to last lock with statement duration which was taken
    before creation of savepoint.
  */
  MDL_ticket *m_stmt_ticket;
  /**
    Pointer to last lock with transaction duration which was taken
    before creation of savepoint.
  */
  MDL_ticket *m_trans_ticket;
};

/**
  A reliable way to wait on an MDL lock.
*/

class MDL_wait {
 public:
  MDL_wait();
  ~MDL_wait();

  // WS_EMPTY since EMPTY conflicts with #define in system headers on some
  // platforms.
  enum enum_wait_status { WS_EMPTY = 0, GRANTED, VICTIM, TIMEOUT, KILLED };

  bool set_status(enum_wait_status result_arg);
  enum_wait_status get_status();
  void reset_status();
  enum_wait_status timed_wait(MDL_context_owner *owner,
                              struct timespec *abs_timeout, bool signal_timeout,
                              const PSI_stage_info *wait_state_name);

 private:
  /**
    Condvar which is used for waiting until this context's pending
    request can be satisfied or this thread has to perform actions
    to resolve a potential deadlock (we subscribe to such
    notification by adding a ticket corresponding to the request
    to an appropriate queue of waiters).
  */
  mysql_mutex_t m_LOCK_wait_status;
  mysql_cond_t m_COND_wait_status;
  enum_wait_status m_wait_status;
};

/**
  Base class to find out if the lock represented by a given ticket
  should be released. Users of release_locks() need to subclass
  this and specify an implementation of release(). Only for locks
  with explicit duration.
*/

class MDL_release_locks_visitor {
 public:
  virtual ~MDL_release_locks_visitor() {}
  /**
    Check if the given ticket represents a lock that should be released.

    @retval true if the lock should be released, false otherwise.
  */
  virtual bool release(MDL_ticket *ticket) = 0;
};

/**
  Abstract visitor class for inspecting MDL_context.
*/

class MDL_context_visitor {
 public:
  virtual ~MDL_context_visitor() {}
  virtual void visit_context(const MDL_context *ctx) = 0;
};

typedef I_P_List<MDL_request,
                 I_P_List_adapter<MDL_request, &MDL_request::next_in_list,
                                  &MDL_request::prev_in_list>,
                 I_P_List_counter>
    MDL_request_list;

typedef std::unordered_set<std::string> MDL_DB_Name_List;

/**
  Context of the owner of metadata locks. I.e. each server
  connection has such a context.
*/

class MDL_context {
 public:
  typedef I_P_List<MDL_ticket,
                   I_P_List_adapter<MDL_ticket, &MDL_ticket::next_in_context,
                                    &MDL_ticket::prev_in_context>>
      Ticket_list;

  typedef Ticket_list::Iterator Ticket_iterator;

  MDL_context();
  void destroy();

  bool try_acquire_lock(MDL_request *mdl_request);
  bool acquire_lock_nsec(MDL_request *mdl_request,
                         Timeout_type lock_wait_timeout_nsec);
  bool acquire_locks_nsec(MDL_request_list *requests,
                          Timeout_type lock_wait_timeout_nsec);
  bool upgrade_shared_lock_nsec(MDL_ticket *mdl_ticket, enum_mdl_type new_type,
                                Timeout_type lock_wait_timeout_nsec);

  bool clone_ticket(MDL_request *mdl_request);

  /**
    Create copy of all granted tickets of particular duration from given
    context to current context.
    Used by XA for preserving locks during client disconnect.

    @param ticket_owner  Owner of tickets to be cloned
    @param duration      MDL lock duration for that tickets are to be cloned

    @retval true   Out of memory or deadlock happened or
                   lock request was refused by storage engine.
    @retval false  Success.
  */

  bool clone_tickets(const MDL_context *ticket_owner,
                     enum_mdl_duration duration);

  void release_all_locks_for_name(MDL_ticket *ticket);
  void release_locks(MDL_release_locks_visitor *visitor);
  void release_lock(MDL_ticket *ticket);

  bool owns_equal_or_stronger_lock(const MDL_key *mdl_key,
                                   enum_mdl_type mdl_type);

  bool owns_equal_or_stronger_lock(MDL_key::enum_mdl_namespace mdl_namespace,
                                   const char *db, const char *name,
                                   enum_mdl_type mdl_type);

  bool find_lock_owner(const MDL_key *mdl_key, MDL_context_visitor *visitor);

  bool has_lock(const MDL_savepoint &mdl_savepoint, MDL_ticket *mdl_ticket);

  inline bool has_locks() const { return !m_ticket_store.is_empty(); }

  bool has_locks(MDL_key::enum_mdl_namespace mdl_namespace) const;

  bool has_locks_waited_for() const;

#ifndef DBUG_OFF
  bool has_locks(enum_mdl_duration duration) {
    return !m_ticket_store.is_empty(duration);
  }
#endif

  MDL_savepoint mdl_savepoint() {
    return MDL_savepoint(m_ticket_store.front(MDL_STATEMENT),
                         m_ticket_store.front(MDL_TRANSACTION));
  }

  void set_explicit_duration_for_all_locks();
  void set_transaction_duration_for_all_locks();
  void set_lock_duration(MDL_ticket *mdl_ticket, enum_mdl_duration duration);

  void release_statement_locks();
  void release_transactional_locks();
  void rollback_to_savepoint(const MDL_savepoint &mdl_savepoint);

  MDL_context_owner *get_owner() const { return m_owner; }

  /** @pre Only valid if we started waiting for lock. */
  inline uint get_deadlock_weight() const {
    return m_force_dml_deadlock_weight
               ? MDL_wait_for_subgraph::DEADLOCK_WEIGHT_DML
               : m_waiting_for->get_deadlock_weight();
  }

  void init(MDL_context_owner *arg) { m_owner = arg; }

  void set_needs_thr_lock_abort(bool needs_thr_lock_abort) {
    /*
      @note In theory, this member should be modified under protection
            of some lock since it can be accessed from different threads.
            In practice, this is not necessary as code which reads this
            value and so might miss the fact that value was changed will
            always re-try reading it after small timeout and therefore
            will see the new value eventually.
    */
    m_needs_thr_lock_abort = needs_thr_lock_abort;

    if (m_needs_thr_lock_abort) {
      /*
        For MDL_object_lock::notify_conflicting_locks() to work properly
        all context requiring thr_lock aborts should not have any "fast
        path" locks.
      */
      materialize_fast_path_locks();
    }
  }
  bool get_needs_thr_lock_abort() const { return m_needs_thr_lock_abort; }

  void get_locked_object_db_names(MDL_DB_Name_List &list);

  void set_force_dml_deadlock_weight(bool force_dml_deadlock_weight) {
    m_force_dml_deadlock_weight = force_dml_deadlock_weight;
  }

  /**
    Get pseudo random value in [0 .. 2^31-1] range.

    @note We use Linear Congruential Generator with venerable constant
          parameters for this.
          It is known to have problems with its lower bits are not being
          very random so probably is not good enough for generic use.
          However, we only use it to do random dives into MDL_lock objects
          hash when searching for unused objects to be freed, and for this
          purposes it is sufficient.
          We rely on values of "get_random() % 2^k" expression having "2^k"
          as a period to ensure that random dives eventually cover all hash
          (the former can be proven to be true). This also means that there
          is no bias towards any specific objects to be expelled (as hash
          values don't repeat), which is nice for performance.
  */
  uint get_random() {
    if (m_rand_state > INT_MAX32) {
      /*
        Perform lazy initialization of LCG. We can't initialize it at the
        point when MDL_context is created since THD represented through
        MDL_context_owner interface is not fully initialized at this point
        itself.
      */
      m_rand_state = m_owner->get_rand_seed() & INT_MAX32;
    }
    m_rand_state = (m_rand_state * 1103515245 + 12345) & INT_MAX32;
    return m_rand_state;
  }

  /**
    Within MDL subsystem this one is only used for DEBUG_SYNC.
    Do not use it to peek/poke into other parts of THD from MDL.
    @sa MDL_context_owner::get_thd().
  */
  THD *get_thd() const { return m_owner->get_thd(); }

 public:
  /**
    If our request for a lock is scheduled, or aborted by the deadlock
    detector, the result is recorded in this class.
  */
  MDL_wait m_wait;

 private:
  /**
    Lists of all MDL tickets acquired by this connection.

    Lists of MDL tickets:
    ---------------------
    The entire set of locks acquired by a connection can be separated
    in three subsets according to their duration: locks released at
    the end of statement, at the end of transaction and locks are
    released explicitly.

    Statement and transactional locks are locks with automatic scope.
    They are accumulated in the course of a transaction, and released
    either at the end of uppermost statement (for statement locks) or
    on COMMIT, ROLLBACK or ROLLBACK TO SAVEPOINT (for transactional
    locks). They must not be (and never are) released manually,
    i.e. with release_lock() call.

    Tickets with explicit duration are taken for locks that span
    multiple transactions or savepoints.
    These are: HANDLER SQL locks (HANDLER SQL is
    transaction-agnostic), LOCK TABLES locks (you can COMMIT/etc
    under LOCK TABLES, and the locked tables stay locked), user level
    locks (GET_LOCK()/RELEASE_LOCK() functions) and
    locks implementing "global read lock".

    Statement/transactional locks are always prepended to the
    beginning of the appropriate list. In other words, they are
    stored in reverse temporal order. Thus, when we rollback to
    a savepoint, we start popping and releasing tickets from the
    front until we reach the last ticket acquired after the savepoint.

    Locks with explicit duration are not stored in any
    particular order, and among each other can be split into
    four sets:
    - LOCK TABLES locks
    - User-level locks
    - HANDLER locks
    - GLOBAL READ LOCK locks
  */
  MDL_ticket_store m_ticket_store;

  MDL_context_owner *m_owner;
  /**
    true -  if for this context we will break protocol and try to
            acquire table-level locks while having only S lock on
            some table.
            To avoid deadlocks which might occur during concurrent
            upgrade of SNRW lock on such object to X lock we have to
            abort waits for table-level locks for such connections.
    false - Otherwise.
  */
  bool m_needs_thr_lock_abort;

  /**
    Indicates that we need to use DEADLOCK_WEIGHT_DML deadlock
    weight for this context and ignore the deadlock weight provided
    by the MDL_wait_for_subgraph object which we are waiting for.

    @note Can be changed only when there is a guarantee that this
          MDL_context is not waiting for a metadata lock or table
          definition entry.
  */
  bool m_force_dml_deadlock_weight;

  /**
    Read-write lock protecting m_waiting_for member.

    @note The fact that this read-write lock prefers readers is
          important as deadlock detector won't work correctly
          otherwise. @sa Comment for MDL_lock::m_rwlock.
  */
  mysql_prlock_t m_LOCK_waiting_for;
  /**
    Tell the deadlock detector what metadata lock or table
    definition cache entry this session is waiting for.
    In principle, this is redundant, as information can be found
    by inspecting waiting queues, but we'd very much like it to be
    readily available to the wait-for graph iterator.
   */
  MDL_wait_for_subgraph *m_waiting_for;
  /**
    Thread's pins (a.k.a. hazard pointers) to be used by lock-free
    implementation of MDL_map::m_locks container. NULL if pins are
    not yet allocated from container's pinbox.
  */
  LF_PINS *m_pins;
  /**
    State for pseudo random numbers generator (PRNG) which output
    is used to perform random dives into MDL_lock objects hash
    when searching for unused objects to free.
  */
  uint m_rand_state;

  /**
    MDL_lock::object_lock_notify_conflicting_locks() checks THD of
    conflicting lock on nullptr value and doesn't call the virtual
    method MDL_context_owner::notify_shared_lock() in case condition
    satisfied. This field allows unit tests to work with THD set to nullptr.
  */
  bool m_ignore_owner_thd;

 private:
  MDL_ticket *find_ticket(MDL_request *mdl_req, enum_mdl_duration *duration);
  void release_locks_stored_before(enum_mdl_duration duration,
                                   MDL_ticket *sentinel);
  void release_lock(enum_mdl_duration duration, MDL_ticket *ticket);
  bool try_acquire_lock_impl(MDL_request *mdl_request, MDL_ticket **out_ticket);
  void materialize_fast_path_locks();
  inline bool fix_pins();

 public:
  void find_deadlock();

  bool visit_subgraph(MDL_wait_for_graph_visitor *dvisitor);

  /** Inform the deadlock detector there is an edge in the wait-for graph. */
  void will_wait_for(MDL_wait_for_subgraph *waiting_for_arg) {
    /*
      Before starting wait for any resource we need to materialize
      all "fast path" tickets belonging to this thread. Otherwise
      locks acquired which are represented by these tickets won't
      be present in wait-for graph and could cause missed deadlocks.

      It is OK for context which doesn't wait for any resource to
      have "fast path" tickets, as such context can't participate
      in any deadlock.
    */
    materialize_fast_path_locks();

    mysql_prlock_wrlock(&m_LOCK_waiting_for);
    m_waiting_for = waiting_for_arg;
    mysql_prlock_unlock(&m_LOCK_waiting_for);
  }

  /** Remove the wait-for edge from the graph after we're done waiting. */
  void done_waiting_for() {
    mysql_prlock_wrlock(&m_LOCK_waiting_for);
    m_waiting_for = nullptr;
    mysql_prlock_unlock(&m_LOCK_waiting_for);
  }
  void lock_deadlock_victim() { mysql_prlock_rdlock(&m_LOCK_waiting_for); }
  void unlock_deadlock_victim() { mysql_prlock_unlock(&m_LOCK_waiting_for); }
  void set_ignore_owner_thd(bool ignore_owner_thd) {
    m_ignore_owner_thd = ignore_owner_thd;
  }
  bool get_ignore_owner_thd() { return m_ignore_owner_thd; }

 private:
  MDL_context(const MDL_context &rhs);      /* not implemented */
  MDL_context &operator=(MDL_context &rhs); /* not implemented */
};

void mdl_init();
void mdl_destroy();

#ifndef DBUG_OFF
extern mysql_mutex_t LOCK_open;
#endif

/*
  Metadata locking subsystem tries not to grant more than
  max_write_lock_count high priority, strong locks successively,
  to avoid starving out weak, lower priority locks.
*/
extern ulong max_write_lock_count;

extern int32 mdl_locks_unused_locks_low_water;

/**
  Default value for threshold for number of unused MDL_lock objects after
  exceeding which we start considering freeing them. Only unit tests use
  different threshold value.
*/
const int32 MDL_LOCKS_UNUSED_LOCKS_LOW_WATER_DEFAULT = 1000;

/**
  Ratio of unused/total MDL_lock objects after exceeding which we
  start trying to free unused MDL_lock objects (assuming that
  mdl_locks_unused_locks_low_water threshold is passed as well).
  Note that this value should be high enough for our algorithm
  using random dives into hash to work well.
*/
const double MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO = 0.25;

int32 mdl_get_unused_locks_count();

/**
  Inspect if MDL_context is owned by any thread.
*/
class MDL_lock_is_owned_visitor : public MDL_context_visitor {
 public:
  MDL_lock_is_owned_visitor() : m_exists(false) {}

  /**
    Collects relevant information about the MDL lock owner.

    This function is only called by MDL_context::find_lock_owner() when
    searching for MDL lock owners to collect extra information about the
    owner. As we only need to know that the MDL lock is owned, setting
    m_exists to true is enough.
  */

  void visit_context(const MDL_context *ctx MY_ATTRIBUTE((unused))) override {
    m_exists = true;
  }

  /**
    Returns if an owner for the MDL lock being inspected exists.

    @return true when MDL lock is owned, false otherwise.
  */

  bool exists() const { return m_exists; }

 private:
  /* holds information about MDL being owned by any thread */
  bool m_exists;
};

String timeout_message(const char *command, const char *name1,
                       const char *name2);

#endif
