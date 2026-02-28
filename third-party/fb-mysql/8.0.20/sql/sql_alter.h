/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_ALTER_TABLE_H
#define SQL_ALTER_TABLE_H

#include <assert.h>
#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_sqlcommand.h"
#include "mysql/psi/psi_base.h"
#include "nullable.h"
#include "sql/dd/types/column.h"
#include "sql/gis/srid.h"
#include "sql/mdl.h"                   // MDL_request
#include "sql/mem_root_array.h"        // Mem_root_array
#include "sql/sql_check_constraint.h"  // Sql_check_constraint_spec_list
#include "sql/sql_cmd.h"               // Sql_cmd
#include "sql/sql_cmd_ddl_table.h"     // Sql_cmd_ddl_table
#include "sql/sql_list.h"              // List
#include "sql/thr_malloc.h"

class Create_field;
class FOREIGN_KEY;
class Value_generator;
class Item;
class Key_spec;
class String;
class THD;
struct TABLE_LIST;

enum enum_field_types : int;

using Mysql::Nullable;

/**
  Class representing DROP COLUMN, DROP KEY, DROP FOREIGN KEY, DROP CHECK
  CONSTRAINT and DROP CONSTRAINT clauses in ALTER TABLE statement.
*/

class Alter_drop {
 public:
  enum drop_type { KEY, COLUMN, FOREIGN_KEY, CHECK_CONSTRAINT, ANY_CONSTRAINT };
  const char *name;
  drop_type type;

  Alter_drop(drop_type par_type, const char *par_name)
      : name(par_name), type(par_type) {
    DBUG_ASSERT(par_name != nullptr);
  }
};

/**
  Class representing SET DEFAULT, DROP DEFAULT and RENAME
  COLUMN clause in ALTER TABLE statement.
*/

class Alter_column {
 public:
  /// The column name being altered.
  const char *name;

  /// The default value supplied.
  Item *def;

  /// The expression to be used to generated the default value.
  Value_generator *m_default_val_expr;

  /// The new colum name.
  const char *m_new_name;

  enum class Type { SET_DEFAULT, DROP_DEFAULT, RENAME_COLUMN };

 public:
  /// Type of change requested in ALTER TABLE.
  inline Type change_type() const { return m_type; }

  /// Constructor used when altering the field's default value with a literal
  /// constant or when dropping a field's default value.
  Alter_column(const char *par_name, Item *literal)
      : name(par_name),
        def(literal),
        m_default_val_expr(nullptr),
        m_new_name(nullptr) {
    if (def)
      m_type = Type::SET_DEFAULT;
    else
      m_type = Type::DROP_DEFAULT;
  }

  /// Constructor used when setting a field's DEFAULT value to an expression.
  Alter_column(const char *par_name, Value_generator *gen_def)
      : name(par_name),
        def(nullptr),
        m_default_val_expr(gen_def),
        m_new_name(nullptr),
        m_type(Type::SET_DEFAULT) {}

  /// Constructor used while renaming field name.
  Alter_column(const char *old_name, const char *new_name)
      : name(old_name),
        def(nullptr),
        m_new_name(new_name),
        m_type(Type::RENAME_COLUMN) {}

 private:
  Type m_type;
};

/// An ALTER INDEX operation that changes the visibility of an index.
class Alter_index_visibility {
 public:
  Alter_index_visibility(const char *name, bool is_visible)
      : m_name(name), m_is_visible(is_visible) {
    assert(name != nullptr);
  }

  const char *name() const { return m_name; }

  /// The visibility after the operation is performed.
  bool is_visible() const { return m_is_visible; }

 private:
  const char *m_name;
  bool m_is_visible;
};

/**
  Class which instances represent RENAME INDEX clauses in
  ALTER TABLE statement.
*/

class Alter_rename_key {
 public:
  const char *old_name;
  const char *new_name;

  Alter_rename_key(const char *old_name_arg, const char *new_name_arg)
      : old_name(old_name_arg), new_name(new_name_arg) {}
};

/**
  Class representing ALTER CHECK and ALTER CONSTRAINT clauses in ALTER TABLE
  statement.
*/

class Alter_constraint_enforcement {
 public:
  enum class Type { CHECK_CONSTRAINT, ANY_CONSTRAINT };
  const char *name;
  Type type;
  bool is_enforced;

  Alter_constraint_enforcement(Type par_type, const char *par_name,
                               bool par_is_enforced)
      : name(par_name), type(par_type), is_enforced(par_is_enforced) {
    DBUG_ASSERT(par_name != nullptr);
  }
};

/**
  Data describing the table being created by CREATE TABLE or
  altered by ALTER TABLE.
*/

class Alter_info {
 public:
  /*
    These flags are set by the parser and describes the type of
    operation(s) specified by the ALTER TABLE statement.

    They do *not* describe the type operation(s) to be executed
    by the storage engine. For example, we don't yet know the
    type of index to be added/dropped.
  */

  enum Alter_info_flag : ulonglong {
    /// Set for ADD [COLUMN]
    ALTER_ADD_COLUMN = 1ULL << 0,

    /// Set for DROP [COLUMN]
    ALTER_DROP_COLUMN = 1ULL << 1,

    /// Set for CHANGE [COLUMN] | MODIFY [CHANGE]
    /// Set by mysql_recreate_table()
    ALTER_CHANGE_COLUMN = 1ULL << 2,

    /// Set for ADD INDEX | ADD KEY | ADD PRIMARY KEY | ADD UNIQUE KEY |
    ///         ADD UNIQUE INDEX | ALTER ADD [COLUMN]
    ALTER_ADD_INDEX = 1ULL << 3,

    /// Set for DROP PRIMARY KEY | DROP FOREIGN KEY | DROP KEY | DROP INDEX
    ALTER_DROP_INDEX = 1ULL << 4,

    /// Set for RENAME [TO]
    ALTER_RENAME = 1ULL << 5,

    /// Set for ORDER BY
    ALTER_ORDER = 1ULL << 6,

    /// Set for table_options
    ALTER_OPTIONS = 1ULL << 7,

    /// Set for ALTER [COLUMN] ... SET DEFAULT ... | DROP DEFAULT
    ALTER_CHANGE_COLUMN_DEFAULT = 1ULL << 8,

    /// Set for DISABLE KEYS | ENABLE KEYS
    ALTER_KEYS_ONOFF = 1ULL << 9,

    /// Set for FORCE
    /// Set for ENGINE(same engine)
    /// Set by mysql_recreate_table()
    ALTER_RECREATE = 1ULL << 10,

    /// Set for ADD PARTITION
    ALTER_ADD_PARTITION = 1ULL << 11,

    /// Set for DROP PARTITION
    ALTER_DROP_PARTITION = 1ULL << 12,

    /// Set for COALESCE PARTITION
    ALTER_COALESCE_PARTITION = 1ULL << 13,

    /// Set for REORGANIZE PARTITION ... INTO
    ALTER_REORGANIZE_PARTITION = 1ULL << 14,

    /// Set for partition_options
    ALTER_PARTITION = 1ULL << 15,

    /// Set for LOAD INDEX INTO CACHE ... PARTITION
    /// Set for CACHE INDEX ... PARTITION
    ALTER_ADMIN_PARTITION = 1ULL << 16,

    /// Set for REORGANIZE PARTITION
    ALTER_TABLE_REORG = 1ULL << 17,

    /// Set for REBUILD PARTITION
    ALTER_REBUILD_PARTITION = 1ULL << 18,

    /// Set for partitioning operations specifying ALL keyword
    ALTER_ALL_PARTITION = 1ULL << 19,

    /// Set for REMOVE PARTITIONING
    ALTER_REMOVE_PARTITIONING = 1ULL << 20,

    /// Set for ADD FOREIGN KEY
    ADD_FOREIGN_KEY = 1ULL << 21,

    /// Set for DROP FOREIGN KEY
    DROP_FOREIGN_KEY = 1ULL << 22,

    /// Set for EXCHANGE PARITION
    ALTER_EXCHANGE_PARTITION = 1ULL << 23,

    /// Set by Sql_cmd_alter_table_truncate_partition::execute()
    ALTER_TRUNCATE_PARTITION = 1ULL << 24,

    /// Set for ADD [COLUMN] FIRST | AFTER
    ALTER_COLUMN_ORDER = 1ULL << 25,

    /// Set for RENAME INDEX
    ALTER_RENAME_INDEX = 1ULL << 26,

    /// Set for discarding the tablespace
    ALTER_DISCARD_TABLESPACE = 1ULL << 27,

    /// Set for importing the tablespace
    ALTER_IMPORT_TABLESPACE = 1ULL << 28,

    /// Means that the visibility of an index is changed.
    ALTER_INDEX_VISIBILITY = 1ULL << 29,

    /// Set for SECONDARY LOAD
    ALTER_SECONDARY_LOAD = 1ULL << 30,

    /// Set for SECONDARY UNLOAD
    ALTER_SECONDARY_UNLOAD = 1ULL << 31,

    /// Set for add check constraint.
    ADD_CHECK_CONSTRAINT = 1ULL << 32,

    /// Set for drop check constraint.
    DROP_CHECK_CONSTRAINT = 1ULL << 33,

    /// Set for check constraint enforce.
    ENFORCE_CHECK_CONSTRAINT = 1ULL << 34,

    /// Set for check constraint suspend.
    SUSPEND_CHECK_CONSTRAINT = 1ULL << 35,

    /// Set for DROP CONSTRAINT.
    DROP_ANY_CONSTRAINT = 1ULL << 36,

    /// Set for ALTER CONSTRAINT symbol ENFORCED.
    ENFORCE_ANY_CONSTRAINT = 1ULL << 37,

    /// Set for ALTER CONSTRAINT symbol NOT ENFORCED.
    SUSPEND_ANY_CONSTRAINT = 1ULL << 38,
  };

  enum enum_enable_or_disable { LEAVE_AS_IS, ENABLE, DISABLE };

  /**
     The different values of the ALGORITHM clause.
     Describes which algorithm to use when altering the table.
  */
  enum enum_alter_table_algorithm {
    // In-place if supported, copy otherwise.
    ALTER_TABLE_ALGORITHM_DEFAULT,

    // In-place if supported, error otherwise.
    ALTER_TABLE_ALGORITHM_INPLACE,

    // Instant if supported, error otherwise.
    ALTER_TABLE_ALGORITHM_INSTANT,

    // Copy if supported, error otherwise.
    ALTER_TABLE_ALGORITHM_COPY
  };

  /**
     The different values of the LOCK clause.
     Describes the level of concurrency during ALTER TABLE.
  */
  enum enum_alter_table_lock {
    // Maximum supported level of concurency for the given operation.
    ALTER_TABLE_LOCK_DEFAULT,

    // Allow concurrent reads & writes. If not supported, give erorr.
    ALTER_TABLE_LOCK_NONE,

    // Allow concurrent reads only. If not supported, give error.
    ALTER_TABLE_LOCK_SHARED,

    // Block reads and writes.
    ALTER_TABLE_LOCK_EXCLUSIVE
  };

  /**
    Status of validation clause in ALTER TABLE statement. Used during
    partitions and GC alterations.
  */
  enum enum_with_validation {
    /**
      Default value, used when it's not specified in the statement.
      Means WITH VALIDATION for partitions alterations and WITHOUT VALIDATION
      for altering virtual GC.
    */
    ALTER_VALIDATION_DEFAULT,
    ALTER_WITH_VALIDATION,
    ALTER_WITHOUT_VALIDATION
  };

  /**
     Columns, keys and constraints to be dropped.
  */
  Mem_root_array<const Alter_drop *> drop_list;
  // Columns for ALTER_COLUMN_CHANGE_DEFAULT.
  Mem_root_array<const Alter_column *> alter_list;
  // List of keys, used by both CREATE and ALTER TABLE.

  Mem_root_array<Key_spec *> key_list;
  // Keys to be renamed.
  Mem_root_array<const Alter_rename_key *> alter_rename_key_list;

  /// Indexes whose visibilities are to be changed.
  Mem_root_array<const Alter_index_visibility *> alter_index_visibility_list;

  /// List of check constraints whose enforcement state is changed.
  Mem_root_array<const Alter_constraint_enforcement *>
      alter_constraint_enforcement_list;

  /// Check constraints specification for CREATE and ALTER TABLE operations.
  Sql_check_constraint_spec_list check_constraint_spec_list;

  // List of columns, used by both CREATE and ALTER TABLE.
  List<Create_field> create_list;
  // Type of ALTER TABLE operation.
  ulonglong flags;
  // Enable or disable keys.
  enum_enable_or_disable keys_onoff;
  // List of partitions.
  List<String> partition_names;
  // Number of partitions.
  uint num_parts;
  // Type of ALTER TABLE algorithm.
  enum_alter_table_algorithm requested_algorithm;
  // Type of ALTER TABLE lock.
  enum_alter_table_lock requested_lock;
  /*
    Whether VALIDATION is asked for an operation. Used during virtual GC and
    partitions alterations.
  */
  enum_with_validation with_validation;

  /// "new_db" (if any) or "db" (if any) or default database from
  /// ALTER TABLE [db.]table [ RENAME [TO|AS|=] [new_db.]new_table ]
  LEX_CSTRING new_db_name;

  /// New table name in the
  /// \code
  /// RENAME [TO] <table_name>
  /// \endcode
  /// clause or NULL_STR
  LEX_CSTRING new_table_name;

  explicit Alter_info(MEM_ROOT *mem_root)
      : drop_list(mem_root),
        alter_list(mem_root),
        key_list(mem_root),
        alter_rename_key_list(mem_root),
        alter_index_visibility_list(mem_root),
        alter_constraint_enforcement_list(mem_root),
        check_constraint_spec_list(mem_root),
        flags(0),
        keys_onoff(LEAVE_AS_IS),
        num_parts(0),
        requested_algorithm(ALTER_TABLE_ALGORITHM_DEFAULT),
        requested_lock(ALTER_TABLE_LOCK_DEFAULT),
        with_validation(ALTER_VALIDATION_DEFAULT),
        new_db_name(LEX_CSTRING{nullptr, 0}),
        new_table_name(LEX_CSTRING{nullptr, 0}) {}

  /**
    Construct a copy of this object to be used for mysql_alter_table
    and mysql_create_table.

    Historically, these two functions modify their Alter_info
    arguments. This behaviour breaks re-execution of prepared
    statements and stored procedures and is compensated by always
    supplying a copy of Alter_info to these functions.

    @param  rhs       Alter_info to make copy of
    @param  mem_root  Mem_root for new Alter_info

    @note You need to use check the error in THD for out
    of memory condition after calling this function.
  */
  Alter_info(const Alter_info &rhs, MEM_ROOT *mem_root);

  bool add_field(THD *thd, const LEX_STRING *field_name,
                 enum enum_field_types type, const char *length,
                 const char *decimal, uint type_modifier, Item *default_value,
                 Item *on_update_value, LEX_CSTRING *comment,
                 const char *change, List<String> *interval_list,
                 const CHARSET_INFO *cs, bool has_explicit_collation,
                 uint uint_geom_type, Value_generator *gcol_info,
                 Value_generator *default_val_expr, const char *opt_after,
                 Nullable<gis::srid_t> srid,
                 Sql_check_constraint_spec_list *check_cons_list,
                 dd::Column::enum_hidden_type hidden, bool is_array = false);

 private:
  Alter_info &operator=(const Alter_info &rhs);  // not implemented
  Alter_info(const Alter_info &rhs);             // not implemented
};

/** Runtime context for ALTER TABLE. */
class Alter_table_ctx {
 public:
  Alter_table_ctx();

  Alter_table_ctx(THD *thd, TABLE_LIST *table_list, uint tables_opened_arg,
                  const char *new_db_arg, const char *new_name_arg);

  ~Alter_table_ctx();

  /**
     @return true if the table is moved to another database, false otherwise.
  */
  bool is_database_changed() const { return (new_db != db); }

  /**
     @return true if the table name is changed, false otherwise.
  */
  bool is_table_name_changed() const { return (new_name != table_name); }

  /**
     @return true if the table is renamed (i.e. its name or database changed),
             false otherwise.
  */
  bool is_table_renamed() const {
    return is_database_changed() || is_table_name_changed();
  }

  /**
     @return path to the original table.
  */
  const char *get_path() const {
    DBUG_ASSERT(!tmp_table);
    return path;
  }

  /**
     @return path to the temporary table created during ALTER TABLE.
  */
  const char *get_tmp_path() const { return tmp_path; }

 public:
  typedef uint error_if_not_empty_mask;
  static const error_if_not_empty_mask DATETIME_WITHOUT_DEFAULT = 1 << 0;
  static const error_if_not_empty_mask GEOMETRY_WITHOUT_DEFAULT = 1 << 1;

  Create_field *datetime_field;
  error_if_not_empty_mask error_if_not_empty;
  uint tables_opened;
  const char *db;
  const char *table_name;
  const char *alias;
  const char *new_db;
  const char *new_name;
  const char *new_alias;
  char tmp_name[80];

  /* Used to remember which foreign keys already existed in the table. */
  FOREIGN_KEY *fk_info;
  uint fk_count;
  /**
    Maximum number component used by generated foreign key names in the
    old version of table.
  */
  uint fk_max_generated_name_number;

  /**
    Metadata lock request on table's new name when this name or database
    are changed.
  */
  MDL_request target_mdl_request;
  /** Metadata lock request on table's new database if it is changed. */
  MDL_request target_db_mdl_request;

 private:
  char new_filename[FN_REFLEN + 1];
  char new_alias_buff[FN_REFLEN + 1];
  char path[FN_REFLEN + 1];
  char new_path[FN_REFLEN + 1];
  char tmp_path[FN_REFLEN + 1];

#ifndef DBUG_OFF
  /** Indicates that we are altering temporary table. Used only in asserts. */
  bool tmp_table;
#endif

  Alter_table_ctx &operator=(const Alter_table_ctx &rhs);  // not implemented
  Alter_table_ctx(const Alter_table_ctx &rhs);             // not implemented
};

/**
  Represents the common properties of the ALTER TABLE statements.
  @todo move Alter_info and other ALTER generic structures from Lex here.
*/
class Sql_cmd_common_alter_table : public Sql_cmd_ddl_table {
 public:
  using Sql_cmd_ddl_table::Sql_cmd_ddl_table;

  ~Sql_cmd_common_alter_table() override = 0;  // force abstract class

  enum_sql_command sql_command_code() const override final {
    return SQLCOM_ALTER_TABLE;
  }
};

inline Sql_cmd_common_alter_table::~Sql_cmd_common_alter_table() {}

/**
  Represents the generic ALTER TABLE statement.
  @todo move Alter_info and other ALTER specific structures from Lex here.
*/
class Sql_cmd_alter_table : public Sql_cmd_common_alter_table {
 public:
  using Sql_cmd_common_alter_table::Sql_cmd_common_alter_table;

  bool execute(THD *thd) override;
};

/**
  Represents ALTER TABLE IMPORT/DISCARD TABLESPACE statements.
*/
class Sql_cmd_discard_import_tablespace : public Sql_cmd_common_alter_table {
 public:
  using Sql_cmd_common_alter_table::Sql_cmd_common_alter_table;

  bool execute(THD *thd) override;

 private:
  bool mysql_discard_or_import_tablespace(THD *thd, TABLE_LIST *table_list);
};

/**
  Represents ALTER TABLE SECONDARY_LOAD/SECONDARY_UNLOAD statements.
*/
class Sql_cmd_secondary_load_unload final : public Sql_cmd_common_alter_table {
 public:
  // Inherit the constructors from the parent class.
  using Sql_cmd_common_alter_table::Sql_cmd_common_alter_table;

  bool execute(THD *thd) override;

 private:
  bool mysql_secondary_load_or_unload(THD *thd, TABLE_LIST *table_list);
};

#endif
