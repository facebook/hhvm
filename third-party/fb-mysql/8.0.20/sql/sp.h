/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _SP_H_
#define _SP_H_

#include <stddef.h>
#include <sys/types.h>
#include <string>

#include "field_types.h"
#include "lex_string.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/udf_registration_types.h"
#include "sql/item.h"     // Item::Type
#include "sql/sp_head.h"  // Stored_program_creation_ctx
#include "sql/sql_lex.h"

class Object_creation_ctx;
class Query_arena;
class THD;
struct CHARSET_INFO;
struct LEX_USER;
struct MEM_ROOT;

namespace dd {
class Routine;
class Schema;
}  // namespace dd

class Field;
class Sroutine_hash_entry;
class String;
class sp_cache;
struct TABLE;
struct TABLE_LIST;

typedef ulonglong sql_mode_t;
template <typename T>
class SQL_I_List;

enum class enum_sp_type;

/* Tells what SP_DEFAULT_ACCESS should be mapped to */
#define SP_DEFAULT_ACCESS_MAPPING SP_CONTAINS_SQL

/* Tells what SP_IS_DEFAULT_SUID should be mapped to */
#define SP_DEFAULT_SUID_MAPPING SP_IS_SUID

/* Max length(LONGBLOB field type length) of stored routine body */
static const uint MYSQL_STORED_ROUTINE_BODY_LENGTH = 4294967295U;

/* Max length(TEXT field type length) of stored routine comment */
static const int MYSQL_STORED_ROUTINE_COMMENT_LENGTH = 65535;

enum enum_sp_return_code {
  SP_OK = 0,

  // Schema does not exists
  SP_NO_DB_ERROR,

  // Routine does not exists
  SP_DOES_NOT_EXISTS,

  // Routine already exists
  SP_ALREADY_EXISTS,

  // Create routine failed
  SP_STORE_FAILED,

  // Drop routine failed
  SP_DROP_FAILED,

  // Routine load failed
  SP_LOAD_FAILED,

  // Routine parse failed
  SP_PARSE_ERROR,

  // Internal errors
  SP_INTERNAL_ERROR
};

/*
  Fields in mysql.proc table in 5.7. This enum is used to read and
  update mysql.routines dictionary table during upgrade scenario.

  Note:  This enum should not be used for other purpose
         as it will be removed eventually.
*/
enum {
  MYSQL_PROC_FIELD_DB = 0,
  MYSQL_PROC_FIELD_NAME,
  MYSQL_PROC_MYSQL_TYPE,
  MYSQL_PROC_FIELD_SPECIFIC_NAME,
  MYSQL_PROC_FIELD_LANGUAGE,
  MYSQL_PROC_FIELD_ACCESS,
  MYSQL_PROC_FIELD_DETERMINISTIC,
  MYSQL_PROC_FIELD_SECURITY_TYPE,
  MYSQL_PROC_FIELD_PARAM_LIST,
  MYSQL_PROC_FIELD_RETURNS,
  MYSQL_PROC_FIELD_BODY,
  MYSQL_PROC_FIELD_DEFINER,
  MYSQL_PROC_FIELD_CREATED,
  MYSQL_PROC_FIELD_MODIFIED,
  MYSQL_PROC_FIELD_SQL_MODE,
  MYSQL_PROC_FIELD_COMMENT,
  MYSQL_PROC_FIELD_CHARACTER_SET_CLIENT,
  MYSQL_PROC_FIELD_COLLATION_CONNECTION,
  MYSQL_PROC_FIELD_DB_COLLATION,
  MYSQL_PROC_FIELD_BODY_UTF8,
  MYSQL_PROC_FIELD_COUNT
};

/*************************************************************************/

/**
  Stored_routine_creation_ctx -- creation context of stored routines
  (stored procedures and functions).
*/

class Stored_routine_creation_ctx : public Stored_program_creation_ctx {
 public:
  static Stored_routine_creation_ctx *create_routine_creation_ctx(
      const dd::Routine *routine);

  static Stored_routine_creation_ctx *load_from_db(THD *thd,
                                                   const sp_name *name,
                                                   TABLE *proc_tbl);

 public:
  Stored_program_creation_ctx *clone(MEM_ROOT *mem_root) override;

 protected:
  Object_creation_ctx *create_backup_ctx(THD *thd) const override;
  void delete_backup_ctx() override;

 private:
  explicit Stored_routine_creation_ctx(THD *thd)
      : Stored_program_creation_ctx(thd) {}

  Stored_routine_creation_ctx(const CHARSET_INFO *client_cs,
                              const CHARSET_INFO *connection_cl,
                              const CHARSET_INFO *db_cl)
      : Stored_program_creation_ctx(client_cs, connection_cl, db_cl) {}
};

/* Drop all routines in database 'db' */
enum_sp_return_code sp_drop_db_routines(THD *thd, const dd::Schema &schema);

/**
   Acquires exclusive metadata lock on all stored routines in the
   given database.

   @param  thd     Thread handler
   @param  schema  Schema object

   @retval  false  Success
   @retval  true   Failure
 */
bool lock_db_routines(THD *thd, const dd::Schema &schema);

sp_head *sp_find_routine(THD *thd, enum_sp_type type, sp_name *name,
                         sp_cache **cp, bool cache_only);

sp_head *sp_setup_routine(THD *thd, enum_sp_type type, sp_name *name,
                          sp_cache **cp);

enum_sp_return_code sp_cache_routine(THD *thd, Sroutine_hash_entry *rt,
                                     bool lookup_only, sp_head **sp);

enum_sp_return_code sp_cache_routine(THD *thd, enum_sp_type type, sp_name *name,
                                     bool lookup_only, sp_head **sp);

bool sp_exist_routines(THD *thd, TABLE_LIST *procs, bool is_proc);

bool sp_show_create_routine(THD *thd, enum_sp_type type, sp_name *name);

enum_sp_return_code db_load_routine(
    THD *thd, enum_sp_type type, const char *sp_db, size_t sp_db_len,
    const char *sp_name, size_t sp_name_len, sp_head **sphp,
    sql_mode_t sql_mode, const char *params, const char *returns,
    const char *body, st_sp_chistics *chistics, const char *definer_user,
    const char *definer_host, longlong created, longlong modified,
    Stored_program_creation_ctx *creation_ctx);

bool sp_create_routine(THD *thd, sp_head *sp, const LEX_USER *definer);

bool sp_update_routine(THD *thd, enum_sp_type type, sp_name *name,
                       st_sp_chistics *chistics);

enum_sp_return_code sp_drop_routine(THD *thd, enum_sp_type type, sp_name *name);

/**
  Structure that represents element in the set of stored routines
  used by statement or routine.
*/

class Sroutine_hash_entry {
 public:
  /**
    Key identifying routine or other object added to the set.

    Key format: "@<1-byte entry type@>@<db name@>\0@<routine/object name@>\0".

    @note We use binary comparison for these keys as the @<db name@> component
          requires case-sensitive comparison on --lower-case-table-names=0
          systems. On systems where --lower-case-table-names > 0 database
          names which passed to functions working with this set are already
          lowercased. So binary comparison is equivalent to case-insensitive
          comparison for them.
          Routine names are case and accent insensitive. To achieve such
          comparison we normalize routine names by converting their characters
          to their sort weights (according to case and accent insensitive
          collation). In this case, the actual routine name is also stored in
          the member m_object_name.

    @note For Foreign Key objects, '@<db name@>\0@<object name@>\0' part of the
          key is compatible with keys used by MDL. So one can easily construct
          MDL_key from this key.
  */
  char *m_key;
  LEX_CSTRING m_object_name;
  uint16 m_key_length;
  uint16 m_db_length;

  enum entry_type {
    FUNCTION,
    PROCEDURE,
    TRIGGER,
    /**
      Parent table in a foreign key on which child table there was insert
      or update. We will lookup new values in parent, so need to acquire
      SR lock on it.
    */
    FK_TABLE_ROLE_PARENT_CHECK,
    /**
      Child table in a foreign key with RESTRICT/NO ACTION as corresponding
      rule and on which parent table there was delete or update.
      We will check if old parent key is referenced by child table,
      so need to acquire SR lock on it.
    */
    FK_TABLE_ROLE_CHILD_CHECK,
    /**
      Child table in a foreign key with CASCADE/SET NULL/SET DEFAULT as
      'on update' rule, on which parent there was update, or with SET NULL/
      SET DEFAULT as 'on delete' rule, on which parent there was delete.
      We might need to update rows in child table, so we need to acquire
      SW lock on it. We also need to take into account that child table
      might be parent for some other FKs, so such update needs
      to be handled recursively.
    */
    FK_TABLE_ROLE_CHILD_UPDATE,
    /**
      Child table in a foreign key with CASCADE as 'on delete' rule for
      which there was delete from the parent table.
      We might need to delete rows from the child table, so we need to
      acquire SW lock on it.
      We also need to take into account that child table might be parent
      for some other FKs, so such delete needs to be handled recursively
      (and even might result in updates).
    */
    FK_TABLE_ROLE_CHILD_DELETE
  };

  entry_type type() const { return (entry_type)m_key[0]; }
  const char *db() const { return (char *)m_key + 1; }
  size_t db_length() const { return m_db_length; }
  const char *name() const {
    return use_normalized_key() ? m_object_name.str
                                : (char *)m_key + 1 + m_db_length + 1;
  }
  size_t name_length() const {
    return use_normalized_key() ? m_object_name.length
                                : m_key_length - 1U - m_db_length - 1U - 1U;
  }
  bool use_normalized_key() const {
    return (type() == FUNCTION || type() == PROCEDURE || type() == TRIGGER);
  }

  const char *part_mdl_key() {
    DBUG_ASSERT(!use_normalized_key());
    return (char *)m_key + 1;
  }
  size_t part_mdl_key_length() {
    DBUG_ASSERT(!use_normalized_key());
    return m_key_length - 1U;
  }

  /**
    Next element in list linking all routines in set. See also comments
    for LEX::sroutine/sroutine_list and sp_head::m_sroutines.
  */
  Sroutine_hash_entry *next;
  /**
    Uppermost view which directly or indirectly uses this routine.
    0 if routine is not used in view. Note that it also can be 0 if
    statement uses routine both via view and directly.
  */
  TABLE_LIST *belong_to_view;
  /**
    This is for prepared statement validation purposes.
    A statement looks up and pre-loads all its stored functions
    at prepare. Later on, if a function is gone from the cache,
    execute may fail. Similarly, tables involved in referential
    constraints are also prelocked.
    Remember the version of the cached item at prepare to be able to
    invalidate the prepared statement at execute if it
    changes.
  */
  int64 m_cache_version;
};

/*
  Enum to indicate SP name normalization required when constructing a key
  in sp_add_used_routine method.
*/
enum class Sp_name_normalize_type {
  LEAVE_AS_IS = 0,              // No normalization needed.
  LOWERCASE_NAME,               // Lower case SP name.
  UNACCENT_AND_LOWERCASE_NAME,  // Lower case SP name and remove accent.
};

/*
  Procedures for handling sets of stored routines used by statement or routine.
*/
bool sp_add_used_routine(Query_tables_list *prelocking_ctx, Query_arena *arena,
                         Sroutine_hash_entry::entry_type type, const char *db,
                         size_t db_length, const char *name, size_t name_length,
                         bool lowercase_db,
                         Sp_name_normalize_type name_normalize_type,
                         bool own_routine, TABLE_LIST *belong_to_view);

/**
  Convenience wrapper around sp_add_used_routine() for most common case -
  stored procedure or function which are explicitly used by the statement.
*/

inline bool sp_add_own_used_routine(Query_tables_list *prelocking_ctx,
                                    Query_arena *arena,
                                    Sroutine_hash_entry::entry_type type,
                                    sp_name *sp_name) {
  DBUG_ASSERT(type == Sroutine_hash_entry::FUNCTION ||
              type == Sroutine_hash_entry::PROCEDURE);

  return sp_add_used_routine(
      prelocking_ctx, arena, type, sp_name->m_db.str, sp_name->m_db.length,
      sp_name->m_name.str, sp_name->m_name.length, false,
      Sp_name_normalize_type::UNACCENT_AND_LOWERCASE_NAME, true, nullptr);
}

void sp_remove_not_own_routines(Query_tables_list *prelocking_ctx);
void sp_update_stmt_used_routines(
    THD *thd, Query_tables_list *prelocking_ctx,
    malloc_unordered_map<std::string, Sroutine_hash_entry *> *src,
    TABLE_LIST *belong_to_view);
void sp_update_stmt_used_routines(THD *thd, Query_tables_list *prelocking_ctx,
                                  SQL_I_List<Sroutine_hash_entry> *src,
                                  TABLE_LIST *belong_to_view);

const uchar *sp_sroutine_key(const uchar *ptr, size_t *plen);

sp_head *sp_load_for_information_schema(THD *thd, LEX_CSTRING db_name,
                                        const dd::Routine *routine,
                                        bool *free_sp_head);

bool load_charset(MEM_ROOT *mem_root, Field *field, const CHARSET_INFO *dflt_cs,
                  const CHARSET_INFO **cs);

bool load_collation(MEM_ROOT *mem_root, Field *field,
                    const CHARSET_INFO *dflt_cl, const CHARSET_INFO **cl);

///////////////////////////////////////////////////////////////////////////

sp_head *sp_start_parsing(THD *thd, enum_sp_type sp_type, sp_name *sp_name);

void sp_finish_parsing(THD *thd);

///////////////////////////////////////////////////////////////////////////

Item_result sp_map_result_type(enum enum_field_types type);
Item::Type sp_map_item_type(enum enum_field_types type);
uint sp_get_flags_for_command(LEX *lex);

bool sp_check_name(LEX_STRING *ident);

TABLE_LIST *sp_add_to_query_tables(THD *thd, LEX *lex, const char *db,
                                   const char *name);

Item *sp_prepare_func_item(THD *thd, Item **it_addr);

bool sp_eval_expr(THD *thd, Field *result_field, Item **expr_item_ptr);

String *sp_get_item_value(THD *thd, Item *item, String *str);

///////////////////////////////////////////////////////////////////////////

#endif /* _SP_H_ */
