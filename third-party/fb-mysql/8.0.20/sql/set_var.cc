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

#include "sql/set_var.h"

#include <string.h>
#include <sys/types.h>
#include <cstdlib>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin_audit.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // SUPER_ACL, generate_password
#include "sql/auth/sql_security_ctx.h"
#include "sql/derror.h"  // ER_THD
#include "sql/enum_query_type.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/log.h"
#include "sql/mysqld.h"              // system_charset_info
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/persisted_variable.h"
#include "sql/protocol_classic.h"
#include "sql/session_tracker.h"
#include "sql/sql_audit.h"  // mysql_audit
#include "sql/sql_base.h"   // lock_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"        // is_supported_parser_charset
#include "sql/sql_select.h"       // free_underlaid_joins
#include "sql/sql_show.h"         // append_identifier
#include "sql/sys_vars_shared.h"  // PolyLock_mutex
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql_string.h"

using std::min;
using std::string;
using set_var_list_map = std::unordered_map<ulong, List<set_var_base>>;

static collation_unordered_map<string, sys_var *> *system_variable_hash;
static PolyLock_mutex PLock_global_system_variables(
    &LOCK_global_system_variables);
ulonglong system_variable_hash_version = 0;

collation_unordered_map<string, sys_var *> *get_system_variable_hash(void) {
  return system_variable_hash;
}

/** list of variables that shouldn't be persisted in all cases */
static collation_unordered_set<string> *never_persistable_vars;

/** The global per-user session variables instance */
static Per_user_session_variables *per_user_session_variables;

Per_user_session_variables *get_per_user_session_variables() {
  return per_user_session_variables;
}

/**
  Get source of a given system variable given its name and name length.
*/
bool get_sysvar_source(const char *name, uint length,
                       enum enum_variable_source *source) {
  DBUG_TRACE;

  bool ret = false;
  sys_var *sysvar = nullptr;

  mysql_rwlock_wrlock(&LOCK_system_variables_hash);

  /* system_variable_hash should have been initialized. */
  DBUG_ASSERT(get_system_variable_hash() != nullptr);
  std::string str(name, length);
  sysvar = find_or_nullptr(*get_system_variable_hash(), str);

  if (sysvar == nullptr) {
    ret = true;
  } else {
    *source = sysvar->get_source();
  }

  mysql_rwlock_unlock(&LOCK_system_variables_hash);
  return ret;
}

sys_var_chain all_sys_vars = {nullptr, nullptr};

int sys_var_init() {
  DBUG_TRACE;

  /* Must be already initialized. */
  DBUG_ASSERT(system_charset_info != nullptr);

  system_variable_hash = new collation_unordered_map<string, sys_var *>(
      system_charset_info, PSI_INSTRUMENT_ME);

  never_persistable_vars = new collation_unordered_set<string>(
      {PERSIST_ONLY_ADMIN_X509_SUBJECT, PERSISTED_GLOBALS_LOAD},
      system_charset_info, PSI_INSTRUMENT_ME);

  if (mysql_add_sys_var_chain(all_sys_vars.first)) goto error;

  per_user_session_variables = new Per_user_session_variables();

  return 0;

error:
  LogErr(ERROR_LEVEL, ER_FAILED_TO_INIT_SYS_VAR);
  return 1;
}

int sys_var_add_options(std::vector<my_option> *long_options, int parse_flags) {
  DBUG_TRACE;

  for (sys_var *var = all_sys_vars.first; var; var = var->next) {
    if (var->register_option(long_options, parse_flags)) goto error;
  }

  return 0;

error:
  LogErr(ERROR_LEVEL, ER_FAILED_TO_INIT_SYS_VAR);
  return 1;
}

void sys_var_end() {
  DBUG_TRACE;

  delete system_variable_hash;
  delete never_persistable_vars;
  system_variable_hash = nullptr;

  delete per_user_session_variables;
  per_user_session_variables = nullptr;

  for (sys_var *var = all_sys_vars.first; var; var = var->next) var->cleanup();
}

/**
  This function will check for necessary privileges needed to perform RESET
  PERSIST or SET PERSIST[_ONLY] operation.

  @param [in] thd                     Pointer to connection handle.
  @param [in] static_variable         describes if variable is static or dynamic

  @return 0 Success
  @return 1 Failure
*/
bool check_priv(THD *thd, bool static_variable) {
  Security_context *sctx = thd->security_context();
  /* for dynamic variables user needs SUPER_ACL or SYSTEM_VARIABLES_ADMIN */
  if (!static_variable) {
    if (!sctx->check_access(SUPER_ACL) &&
        !(sctx->has_global_grant(STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"))
              .first)) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
               "SUPER or SYSTEM_VARIABLES_ADMIN");
      return true;
    }
  } else {
    /*
     for static variables user needs both SYSTEM_VARIABLES_ADMIN and
     PERSIST_RO_VARIABLES_ADMIN
    */
    if (!(sctx->has_global_grant(STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"))
              .first &&
          sctx->has_global_grant(STRING_WITH_LEN("PERSIST_RO_VARIABLES_ADMIN"))
              .first)) {
      my_error(ER_PERSIST_ONLY_ACCESS_DENIED_ERROR, MYF(0),
               "SYSTEM_VARIABLES_ADMIN and PERSIST_RO_VARIABLES_ADMIN");
      return true;
    }
  }
  return false;
}

/**
  sys_var constructor

  @param chain     variables are linked into chain for mysql_add_sys_var_chain()
  @param name_arg  the name of the variable. Must be 0-terminated and exist
                   for the liftime of the sys_var object. @sa my_option::name
  @param comment   shown in mysqld --help, @sa my_option::comment
  @param flags_arg or'ed flag_enum values
  @param off       offset of the global variable value from the
                   &global_system_variables.
  @param getopt_id -1 for no command-line option, otherwise @sa my_option::id
  @param getopt_arg_type @sa my_option::arg_type
  @param show_val_type_arg what value_ptr() returns for sql_show.cc
  @param def_val   default value, @sa my_option::def_value
  @param lock      mutex or rw_lock that protects the global variable
                   *in addition* to LOCK_global_system_variables.
  @param binlog_status_arg @sa binlog_status_enum
  @param on_check_func a function to be called at the end of sys_var::check,
                   put your additional checks here
  @param on_update_func a function to be called at the end of sys_var::update,
                   any post-update activity should happen here
  @param substitute If non-NULL, this variable is deprecated and the
  string describes what one should use instead. If an empty string,
  the variable is deprecated but no replacement is offered.
  @param parse_flag either PARSE_EARLY or PARSE_NORMAL
*/
sys_var::sys_var(sys_var_chain *chain, const char *name_arg,
                 const char *comment, int flags_arg, ptrdiff_t off,
                 int getopt_id, enum get_opt_arg_type getopt_arg_type,
                 SHOW_TYPE show_val_type_arg, longlong def_val, PolyLock *lock,
                 enum binlog_status_enum binlog_status_arg,
                 on_check_function on_check_func,
                 on_update_function on_update_func, const char *substitute,
                 int parse_flag)
    : next(nullptr),
      binlog_status(binlog_status_arg),
      flags(flags_arg),
      m_parse_flag(parse_flag),
      show_val_type(show_val_type_arg),
      guard(lock),
      offset(off),
      on_check(on_check_func),
      on_update(on_update_func),
      deprecation_substitute(substitute),
      is_os_charset(false) {
  /*
    There is a limitation in handle_options() related to short options:
    - either all short options should be declared when parsing in multiple
    stages,
    - or none should be declared.
    Because a lot of short options are used in the normal parsing phase
    for mysqld, we enforce here that no short option is present
    in the first (PARSE_EARLY) stage.
    See handle_options() for details.
  */
  DBUG_ASSERT(parse_flag == PARSE_NORMAL || getopt_id <= 0 || getopt_id >= 255);

  name.str = name_arg;  // ER_NO_DEFAULT relies on 0-termination of name_arg
  name.length = strlen(name_arg);  // and so does this.
  DBUG_ASSERT(name.length <= NAME_CHAR_LEN);

  memset(&option, 0, sizeof(option));
  option.name = name_arg;
  option.id = getopt_id;
  option.comment = comment;
  option.arg_type = getopt_arg_type;
  option.value = (uchar **)global_var_ptr();
  option.def_value = def_val;

  /* set default values */
  source.m_source = enum_variable_source::COMPILED;

  timestamp = 0;
  user[0] = '\0';
  host[0] = '\0';

  memset(source.m_path_name, 0, FN_REFLEN);
  option.arg_source = &source;

  if (chain->last)
    chain->last->next = this;
  else
    chain->first = this;
  chain->last = this;
}

bool sys_var::update(THD *thd, set_var *var) {
  enum_var_type type = var->type;
  if (type == OPT_GLOBAL || type == OPT_PERSIST || scope() == GLOBAL) {
    /*
      Yes, both locks need to be taken before an update, just as
      both are taken to get a value. If we'll take only 'guard' here,
      then value_ptr() for strings won't be safe in SHOW VARIABLES anymore,
      to make it safe we'll need value_ptr_unlock().
    */
    AutoWLock lock1(&PLock_global_system_variables);
    AutoWLock lock2(guard);
    return global_update(thd, var) ||
           (on_update && on_update(this, thd, OPT_GLOBAL));
  } else {
    /* Block reads from other threads. */
    mysql_mutex_lock(&thd->LOCK_thd_sysvar);

    bool ret = session_update(thd, var) ||
               (on_update && on_update(this, thd, OPT_SESSION));

    mysql_mutex_unlock(&thd->LOCK_thd_sysvar);

    /*
      Make sure we don't session-track variables that are not actually
      part of the session. tx_isolation and and tx_read_only for example
      exist as GLOBAL, SESSION, and one-shot ("for next transaction only").
    */
    if ((var->type == OPT_SESSION) || !is_trilevel()) {
      if ((!ret) && thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)
                        ->is_enabled())
        thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)
            ->mark_as_changed(thd, &(var->var->name));

      if ((!ret) &&
          thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
              ->is_enabled())
        thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
            ->mark_as_changed(thd, &var->var->name);
    }

    return ret;
  }
}

const uchar *sys_var::session_value_ptr(THD *, THD *target_thd, LEX_STRING *) {
  return session_var_ptr(target_thd);
}

const uchar *sys_var::global_value_ptr(THD *, LEX_STRING *) {
  return global_var_ptr();
}

uchar *sys_var::session_var_ptr(THD *thd) {
  return ((uchar *)&(thd->variables)) + offset;
}

uchar *sys_var::global_var_ptr() {
  return ((uchar *)&global_system_variables) + offset;
}

bool sys_var::check(THD *thd, set_var *var) {
  if ((var->value && do_check(thd, var)) ||
      (on_check && on_check(this, thd, var))) {
    if (!thd->is_error()) {
      char buff[STRING_BUFFER_USUAL_SIZE];
      String str(buff, sizeof(buff), system_charset_info), *res;

      if (!var->value) {
        str.set(STRING_WITH_LEN("DEFAULT"), &my_charset_latin1);
        res = &str;
      } else if (!(res = var->value->val_str(&str))) {
        str.set(STRING_WITH_LEN("NULL"), &my_charset_latin1);
        res = &str;
      }
      ErrConvString err(res);
      my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), name.str, err.ptr());
    }
    return true;
  }
  return false;
}

const uchar *sys_var::value_ptr(THD *running_thd, THD *target_thd,
                                enum_var_type type, LEX_STRING *base) {
  if (type == OPT_GLOBAL || type == OPT_PERSIST || scope() == GLOBAL) {
    mysql_mutex_assert_owner(&LOCK_global_system_variables);
    AutoRLock lock(guard);
    return global_value_ptr(running_thd, base);
  } else
    return session_value_ptr(running_thd, target_thd, base);
}

const uchar *sys_var::value_ptr(THD *thd, enum_var_type type,
                                LEX_STRING *base) {
  return value_ptr(thd, thd, type, base);
}

bool sys_var::set_default(THD *thd, set_var *var) {
  DBUG_TRACE;
  if (var->is_global_persist() || scope() == GLOBAL)
    global_save_default(thd, var);
  else
    session_save_default(thd, var);

  bool ret = check(thd, var) || update(thd, var);
  return ret;
}

void sys_var::set_user_host(THD *thd) {
  memset(user, 0, sizeof(user));
  DBUG_ASSERT(thd->security_context()->user().length < sizeof(user));
  /* set client user */
  if (thd->security_context()->user().length > 0)
    strncpy(user, thd->security_context()->user().str,
            thd->security_context()->user().length);
  memset(host, 0, sizeof(host));
  if (thd->security_context()->host().length > 0) {
    int host_len =
        min<size_t>(sizeof(host) - 1, thd->security_context()->host().length);
    strncpy(host, thd->security_context()->host().str, host_len);
  }
}

void sys_var::do_deprecated_warning(THD *thd) {
  if (deprecation_substitute != nullptr) {
    char buf1[NAME_CHAR_LEN + 3];
    strxnmov(buf1, sizeof(buf1) - 1, "@@", name.str, 0);

    /*
       if deprecation_substitute is an empty string,
       there is no replacement for the syntax
    */
    uint errmsg = deprecation_substitute[0] == '\0'
                      ? ER_DEPRECATE_MSG_NO_REPLACEMENT
                      : ER_DEPRECATE_MSG_WITH_REPLACEMENT;
    if (thd)
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WARN_DEPRECATED_SYNTAX,
          ER_THD_NONCONST(thd, errmsg), buf1, deprecation_substitute);
    else
      LogErr(WARNING_LEVEL, errmsg, buf1, deprecation_substitute);
  }
}

Item *sys_var::copy_value(THD *thd) {
  LEX_STRING str;
  const uchar *val_ptr = session_value_ptr(thd, thd, &str);
  switch (get_var_type()) {
    case GET_INT:
      return new Item_int(*pointer_cast<const int *>(val_ptr));
    case GET_UINT:
      return new Item_int(
          static_cast<ulonglong>(*pointer_cast<const uint *>(val_ptr)));
    case GET_LONG:
      return new Item_int(
          static_cast<longlong>(*pointer_cast<const long *>(val_ptr)));
    case GET_ULONG:
      return new Item_int(
          static_cast<ulonglong>(*pointer_cast<const ulong *>(val_ptr)));
    case GET_LL:
      return new Item_int(*pointer_cast<const longlong *>(val_ptr));
    case GET_ULL:
      return new Item_int(*pointer_cast<const ulonglong *>(val_ptr));
    case GET_BOOL:
      return new Item_int(*pointer_cast<const bool *>(val_ptr));
    case GET_ENUM:
    case GET_SET:
    case GET_FLAGSET:
    case GET_STR_ALLOC:
    case GET_STR:
    case GET_NO_ARG:
    case GET_PASSWORD: {
      const char *tmp_str_val = pointer_cast<const char *>(val_ptr);
      return new Item_string(tmp_str_val, strlen(tmp_str_val),
                             system_charset_info);
    }
    case GET_DOUBLE:
      return new Item_float(*pointer_cast<const double *>(val_ptr),
                            DECIMAL_NOT_SPECIFIED);
    default:
      DBUG_ASSERT(0);
  }
  return nullptr;
}

/**
  Throw warning (error in STRICT mode) if value for variable needed bounding.
  Plug-in interface also uses this.

  @param thd         thread handle
  @param name        variable's name
  @param fixed       did we have to correct the value? (throw warn/err if so)
  @param is_unsigned is value's type unsigned?
  @param v           variable's value

  @retval         true on error, false otherwise (warning or ok)
 */
bool throw_bounds_warning(THD *thd, const char *name, bool fixed,
                          bool is_unsigned, longlong v) {
  if (fixed) {
    char buf[22];

    if (is_unsigned)
      ullstr((ulonglong)v, buf);
    else
      llstr(v, buf);

    if (thd->variables.sql_mode & MODE_STRICT_ALL_TABLES) {
      my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), name, buf);
      return true;
    }
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_TRUNCATED_WRONG_VALUE,
                        ER_THD(thd, ER_TRUNCATED_WRONG_VALUE), name, buf);
  }
  return false;
}

bool throw_bounds_warning(THD *thd, const char *name, bool fixed, double v) {
  if (fixed) {
    char buf[64];

    my_gcvt(v, MY_GCVT_ARG_DOUBLE, static_cast<int>(sizeof(buf)) - 1, buf,
            nullptr);

    if (thd->variables.sql_mode & MODE_STRICT_ALL_TABLES) {
      my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), name, buf);
      return true;
    }
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_TRUNCATED_WRONG_VALUE,
                        ER_THD(thd, ER_TRUNCATED_WRONG_VALUE), name, buf);
  }
  return false;
}

const CHARSET_INFO *sys_var::charset(THD *thd) {
  return is_os_charset ? thd->variables.character_set_filesystem
                       : system_charset_info;
}

struct my_old_conv {
  const char *old_name;
  const char *new_name;
};

static my_old_conv old_conv[] = {{"cp1251_koi8", "cp1251"},
                                 {"cp1250_latin2", "cp1250"},
                                 {"kam_latin2", "keybcs2"},
                                 {"mac_latin2", "MacRoman"},
                                 {"macce_latin2", "MacCE"},
                                 {"pc2_latin2", "pclatin2"},
                                 {"vga_latin2", "pclatin1"},
                                 {"koi8_cp1251", "koi8r"},
                                 {"win1251ukr_koi8_ukr", "win1251ukr"},
                                 {"koi8_ukr_win1251ukr", "koi8u"},
                                 {nullptr, nullptr}};

const CHARSET_INFO *get_old_charset_by_name(const char *name) {
  my_old_conv *conv;

  for (conv = old_conv; conv->old_name; conv++) {
    if (!my_strcasecmp(&my_charset_latin1, name, conv->old_name))
      return get_charset_by_csname(conv->new_name, MY_CS_PRIMARY, MYF(0));
  }
  return nullptr;
}

/****************************************************************************
  Main handling of variables:
  - Initialisation
  - Searching during parsing
  - Update loop
****************************************************************************/

/**
  Add variables to the dynamic hash of system variables

  @param first       Pointer to first system variable to add

  @retval
    0           SUCCESS
  @retval
    otherwise   FAILURE
*/

int mysql_add_sys_var_chain(sys_var *first) {
  sys_var *var;

  /* A write lock should be held on LOCK_system_variables_hash */

  for (var = first; var; var = var->next) {
    /* this fails if there is a conflicting variable name. */
    if (!system_variable_hash->emplace(to_string(var->name), var).second) {
      LogErr(ERROR_LEVEL, ER_DUPLICATE_SYS_VAR, var->name.str);
      goto error;
    }
  }

  /* Update system_variable_hash version. */
  system_variable_hash_version++;
  return 0;

error:
  for (; first != var; first = first->next)
    system_variable_hash->erase(to_string(var->name));
  return 1;
}

/*
  Remove variables to the dynamic hash of system variables

  SYNOPSIS
    mysql_del_sys_var_chain()
    first       Pointer to first system variable to remove

  RETURN VALUES
    0           SUCCESS
    otherwise   FAILURE
*/

int mysql_del_sys_var_chain(sys_var *first) {
  int result = 0;

  /* A write lock should be held on LOCK_system_variables_hash */

  for (sys_var *var = first; var; var = var->next)
    result |= !system_variable_hash->erase(to_string(var->name));

  /* Update system_variable_hash version. */
  system_variable_hash_version++;

  return result;
}

/*
  Comparison function for std::sort.
  @param a  SHOW_VAR element
  @param b  SHOW_VAR element

  @retval
    True if a < b.
  @retval
    False if a >= b.
*/
static int show_cmp(const void *a, const void *b) {
  return strcmp(static_cast<const SHOW_VAR *>(a)->name,
                static_cast<const SHOW_VAR *>(b)->name);
}

/*
  Number of records in the system_variable_hash.
  Requires lock on LOCK_system_variables_hash.
*/
ulong get_system_variable_hash_records(void) {
  return (system_variable_hash->size());
}

/*
  Current version of the system_variable_hash.
  Requires lock on LOCK_system_variables_hash.
*/
ulonglong get_system_variable_hash_version(void) {
  return (system_variable_hash_version);
}

/**
  Constructs an array of system variables for display to the user.

  @param show_var_array Prealloced_array of SHOW_VAR elements for display
  @param sort           If true, the system variables should be sorted
  @param query_scope    OPT_GLOBAL or OPT_SESSION for SHOW GLOBAL|SESSION
  VARIABLES
  @param strict         Use strict scope checking
  @retval               True on error, false otherwise
*/
bool enumerate_sys_vars(Show_var_array *show_var_array, bool sort,
                        enum enum_var_type query_scope, bool strict) {
  DBUG_ASSERT(show_var_array != nullptr);
  DBUG_ASSERT(query_scope == OPT_SESSION || query_scope == OPT_GLOBAL);
  int count = system_variable_hash->size();

  /* Resize array if necessary. */
  if (show_var_array->reserve(count + 1)) return true;

  if (show_var_array) {
    for (const auto &key_and_value : *system_variable_hash) {
      sys_var *sysvar = key_and_value.second;

      if (strict) {
        /*
          Strict scope match (5.7). Success if this is a:
            - global query and the variable scope is GLOBAL or SESSION, OR
            - session query and the variable scope is SESSION or ONLY_SESSION.
        */
        if (!sysvar->check_scope(query_scope)) continue;
      } else {
        /*
          Non-strict scope match (5.6). Success if this is a:
            - global query and the variable scope is GLOBAL or SESSION, OR
            - session query and the variable scope is GLOBAL, SESSION or
          ONLY_SESSION.
        */
        if (query_scope == OPT_GLOBAL && !sysvar->check_scope(query_scope))
          continue;
      }

      /* Don't show non-visible variables. */
      if (sysvar->not_visible()) continue;

      SHOW_VAR show_var;
      show_var.name = sysvar->name.str;
      show_var.value = (char *)sysvar;
      show_var.type = SHOW_SYS;
      show_var.scope = SHOW_SCOPE_UNDEF; /* not used for sys vars */
      show_var_array->push_back(show_var);
    }

    if (sort)
      std::qsort(show_var_array->begin(), show_var_array->size(),
                 show_var_array->element_size(), show_cmp);

    /* Make last element empty. */
    show_var_array->push_back(SHOW_VAR());
  }

  return false;
}

/**
  Find a user set-table variable.

  @param str       Name of system variable to find
  @param length    Length of variable.  zero means that we should use strlen()
                   on the variable

  @retval
    pointer     pointer to variable definitions
  @retval
    0           Unknown variable (error message is given)
*/

sys_var *intern_find_sys_var(const char *str, size_t length) {
  sys_var *var;

  /*
    This function is only called from the sql_plugin.cc.
    A lock on LOCK_system_variable_hash should be held
  */
  var = find_or_nullptr(*system_variable_hash,
                        string(str, length ? length : strlen(str)));

  /* Don't show non-visible variables. */
  if (var && var->not_visible()) return nullptr;

  return var;
}

/**
  Helper function to set variables.
  Use the thd -> list<var> map to iterate over all threads
  For each thread, for each variable in its list, perform the specified function
  @param THD            Current thread
  @param thd_var_map    Map of threads to list of variables to be updated
  @param func           Function to be executed for each variable
  @retval
    0   ok
  @retval
    1   ERROR
*/

static int process_set_variable_map(THD *thd, set_var_list_map &thd_var_map,
                                    int (*func)(THD *, set_var_base *)) {
  DBUG_ENTER("process_set_variable_map");
  int error = 0;

  for (auto var_iter : thd_var_map) {
    ulong thd_id = var_iter.first;
    THD *change_thd = thd;
    // Thread id of the current thread is 0
    if (thd_id) {
      Find_thd_with_id find_thd_with_id(thd_id);
      change_thd =
          Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
      if (!change_thd) {
        my_error(ER_NO_SUCH_THREAD, MYF(0), thd_id);
        DBUG_RETURN(1);
      }

      Security_context *sctx = thd->security_context();
      if (!sctx->check_access(SUPER_ACL) &&
          !sctx->has_global_grant(STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"))
               .first) {
        mysql_mutex_unlock(&change_thd->LOCK_thd_data);
        my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
                 "SUPER or SYSTEM_VARIABLES_ADMIN");
        DBUG_RETURN(1);
      }
    }

    List_iterator_fast<set_var_base> it(var_iter.second);
    set_var_base *var;
    while ((var = it++)) {
      if ((error = func(change_thd, var))) break;
    }

    if (thd_id) mysql_mutex_unlock(&change_thd->LOCK_thd_data);

    if (error) break;
  }

  DBUG_RETURN(error);
}

/**
  Execute update of all variables.

  First run a check of all variables that all updates will go ok.
  If yes, then execute all updates, returning an error if any one failed.

  This should ensure that in all normal cases none all or variables are
  updated.

  @param thd            Thread id
  @param var_list       List of variables to update
  @param opened         True means tables are open and this function will lock
                        them.

  @retval
    0   ok
  @retval
    1   ERROR, message sent (normally no variables was updated)
  @retval
    -1  ERROR, message not sent
*/

int sql_set_variables(THD *thd, List<set_var_base> *var_list, bool opened) {
  int error;
  List_iterator_fast<set_var_base> it(*var_list);
  DBUG_TRACE;

  LEX *lex = thd->lex;
  set_var_base *var;
  const ulong current_thd_id = thd->thread_id();
  /*
    This map stores a list of variables to be updated for each thread specified
    by the user, essentially grouping variabled by thread ID. Variable updates
    are processed per thread.
  */
  set_var_list_map thd_var_map;

  while ((var = it++)) {
    ulong thd_id_opt = var->thd_id;
    if (thd_id_opt == current_thd_id) thd_id_opt = 0;
    thd_var_map[thd_id_opt].push_back(var);
  }

  if ((error = process_set_variable_map(thd, thd_var_map,
                                        [](THD *each_thd, set_var_base *svar) {
                                          return svar->resolve(each_thd);
                                        })))
    goto err;

  if ((error = thd->is_error())) goto err;

  if (opened && lock_tables(thd, lex->query_tables, lex->table_count, 0)) {
    error = 1;
    goto err;
  }

  if ((error = process_set_variable_map(thd, thd_var_map,
                                        [](THD *each_thd, set_var_base *svar) {
                                          return svar->check(each_thd);
                                        })))
    goto err;

  if ((error = thd->is_error())) goto err;

  if ((error = process_set_variable_map(
           thd, thd_var_map, [](THD *each_thd, set_var_base *svar) {
             return svar->update(each_thd); /* Returns 0, -1 or 1 */
           })))
    goto err;

  if (!error) {
    /* At this point SET statement is considered a success. */
    Persisted_variables_cache *pv = nullptr;
    it.rewind();
    while ((var = it++)) {
      set_var *setvar = dynamic_cast<set_var *>(var);
      if (setvar &&
          (setvar->type == OPT_PERSIST || setvar->type == OPT_PERSIST_ONLY)) {
        pv = Persisted_variables_cache::get_instance();
        /* update in-memory copy of persistent options */
        if (pv->set_variable(thd, setvar)) return 1;
      }
    }
    /* flush all persistent options to a file */
    if (pv && pv->flush_to_file()) {
      my_error(ER_VARIABLE_NOT_PERSISTED, MYF(0));
      return 1;
    }
  }
err:
  free_underlaid_joins(thd, thd->lex->select_lex);
  return error;
}

/**
  This function is used to check if key management UDFs like
  keying_key_generate/store/remove should proceed or not. If global
  variable @@keyring_operations is OFF then above said udfs will fail.

  @return Operation status
    @retval 0 OK
    @retval 1 ERROR, keyring operations are not allowed

  @sa Sys_keyring_operations
*/
bool keyring_access_test() {
  bool keyring_operations;
  mysql_mutex_lock(&LOCK_keyring_operations);
  keyring_operations = !opt_keyring_operations;
  mysql_mutex_unlock(&LOCK_keyring_operations);
  return keyring_operations;
}

/*****************************************************************************
  Functions to handle SET mysql_internal_variable=const_expr
*****************************************************************************/

set_var::set_var(enum_var_type type_arg, sys_var *var_arg,
                 LEX_CSTRING base_name_arg, Item *value_arg)
    : var(var_arg), type(type_arg), base(base_name_arg) {
  /*
    If the set value is a field, change it to a string to allow things like
    SET table_type=MYISAM;
  */
  if (value_arg && value_arg->type() == Item::FIELD_ITEM) {
    Item_field *item = (Item_field *)value_arg;
    if (item->field_name) {
      if (!(value = new Item_string(item->field_name, strlen(item->field_name),
                                    system_charset_info)))  // names are utf8
        value = value_arg; /* Give error message later */
    } else {
      /* Both Item_field and Item_insert_value will return the type as
         Item::FIELD_ITEM. If the item->field_name is NULL, we assume the
         object to be Item_insert_value. */
      value = value_arg;
    }
  } else
    value = value_arg;
}

/**
  global X509 subject name to require from the client session
  to allow SET PERSIST[_ONLY] on sys_var::NOTPERSIST variables

  @sa set_var::resolve
*/
char *sys_var_persist_only_admin_x509_subject = nullptr;

/**
  Checks if a THD can set non-persist variables

  Requires that:
  * the session uses SSL
  * the peer has presented a valid certificate
  * the certificate has a certain subject name

  The format checked is deliberately kept the same as the
  other SSL system and status variables representing names.
  Hence X509_NAME_oneline is used.

  @retval true the THD can set NON_PERSIST variables
  @retval false usual restrictions apply
  @param thd the THD handle
  @param var the variable to be set
  @param setvar_type  the operation to check against.

  @sa sys_variables_admin_dn
*/
static bool can_persist_non_persistent_var(THD *thd, sys_var *var,
                                           enum_var_type setvar_type) {
  SSL *ssl = nullptr;
  X509 *cert = nullptr;
  char *ptr = nullptr;
  bool result = false;

  /* Bail off if no subject is set */
  if (likely(!sys_var_persist_only_admin_x509_subject ||
             !sys_var_persist_only_admin_x509_subject[0]))
    return false;

  /* Can't persist read only variables without command line support */
  if (unlikely(setvar_type == OPT_PERSIST_ONLY &&
               !var->is_settable_at_command_line() &&
               (var->is_readonly() || var->is_persist_readonly())))
    return false;

  /* do not allow setting the controlling variables */
  if (never_persistable_vars->find(var->name.str) !=
      never_persistable_vars->end())
    return false;

  ssl = thd->get_ssl();
  if (!ssl) return false;

  cert = SSL_get_peer_certificate(ssl);
  if (!cert) goto done;

  ptr = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
  if (!ptr) goto done;

  result = !strcmp(sys_var_persist_only_admin_x509_subject, ptr);
done:
  if (ptr) OPENSSL_free(ptr);
  if (cert) X509_free(cert);
  return result;
}

/**
Resolve the variable assignment

@param thd Thread handler

@return status code
@retval -1 Failure
@retval 0 Success
*/

int set_var::resolve(THD *thd) {
  DBUG_TRACE;
  var->do_deprecated_warning(thd);
  if (var->is_readonly()) {
    if (type != OPT_PERSIST_ONLY) {
      my_error(ER_INCORRECT_GLOBAL_LOCAL_VAR, MYF(0), var->name.str,
               "read only");
      return -1;
    }
    if (type == OPT_PERSIST_ONLY && var->is_non_persistent() &&
        !can_persist_non_persistent_var(thd, var, type)) {
      my_error(ER_INCORRECT_GLOBAL_LOCAL_VAR, MYF(0), var->name.str,
               "non persistent read only");
      return -1;
    }
  }
  if (!var->check_scope(type)) {
    int err = (is_global_persist()) ? ER_LOCAL_VARIABLE : ER_GLOBAL_VARIABLE;
    my_error(err, MYF(0), var->name.str);
    return -1;
  }
  if (type == OPT_GLOBAL || type == OPT_PERSIST) {
    /* Either the user has SUPER_ACL or she has SYSTEM_VARIABLES_ADMIN */
    if (check_priv(thd, false)) return 1;
  }
  if (type == OPT_PERSIST_ONLY) {
    if (check_priv(thd, true)) return 1;
  }

  /* check if read/write non-persistent variables can be persisted */
  if ((type == OPT_PERSIST || type == OPT_PERSIST_ONLY) &&
      var->is_non_persistent() &&
      !can_persist_non_persistent_var(thd, var, type)) {
    my_error(ER_INCORRECT_GLOBAL_LOCAL_VAR, MYF(0), var->name.str,
             "non persistent");
    return -1;
  }

  /* value is a NULL pointer if we are using SET ... = DEFAULT */
  if (!value) return 0;

  if ((!value->fixed && value->fix_fields(thd, &value)) || value->check_cols(1))
    return -1;

  return 0;
}

/**
  Verify that the supplied value is correct.

  @param thd Thread handler

  @return status code
   @retval -1 Failure
   @retval 0 Success
*/

int set_var::check(THD *thd) {
  DBUG_TRACE;

  /* value is a NULL pointer if we are using SET ... = DEFAULT */
  if (!value) return 0;

  if (var->check_update_type(value->result_type())) {
    my_error(ER_WRONG_TYPE_FOR_VAR, MYF(0), var->name.str);
    return -1;
  }
  int ret = (type != OPT_PERSIST_ONLY && var->check(thd, this)) ? -1 : 0;

  if (!ret && (is_global_persist())) {
    ret = mysql_audit_notify(thd, AUDIT_EVENT(MYSQL_AUDIT_GLOBAL_VARIABLE_SET),
                             var->name.str, value->item_name.ptr(),
                             value->item_name.length());
  }

  return ret;
}

/**
  Check variable, but without assigning value (used by PS).

  @param thd            thread handler

  @retval
    0   ok
  @retval
    1   ERROR, message sent (normally no variables was updated)
  @retval
    -1   ERROR, message not sent
*/
int set_var::light_check(THD *thd) {
  if (!var->check_scope(type)) {
    int err = (is_global_persist()) ? ER_LOCAL_VARIABLE : ER_GLOBAL_VARIABLE;
    my_error(err, MYF(0), var->name.str);
    return -1;
  }
  Security_context *sctx = thd->security_context();
  if ((type == OPT_GLOBAL || type == OPT_PERSIST) &&
      !(sctx->check_access(SUPER_ACL) ||
        sctx->has_global_grant(STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"))
            .first))
    return 1;

  if ((type == OPT_PERSIST_ONLY) &&
      !(sctx->has_global_grant(STRING_WITH_LEN("PERSIST_RO_VARIABLES_ADMIN"))
            .first &&
        sctx->has_global_grant(STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"))
            .first))
    return 1;

  if (value && ((!value->fixed && value->fix_fields(thd, &value)) ||
                value->check_cols(1)))
    return -1;
  return 0;
}

/**
  Update variable source, user, host and timestamp values.
*/

void set_var::update_source_user_host_timestamp(THD *thd) {
  var->set_source(enum_variable_source::DYNAMIC);
  var->set_source_name(EMPTY_CSTR.str);
  var->set_user_host(thd);
  var->set_timestamp();
}

/**
  Update variable

  @param   thd    thread handler
  @returns 0|1    ok or ERROR

  @note ERROR can be only due to abnormal operations involving
  the server's execution evironment such as
  out of memory, hard disk failure or the computer blows up.
  Consider set_var::check() method if there is a need to return
  an error due to logics.
*/
int set_var::update(THD *thd) {
  int ret = 0;
  char before_value[SHOW_VAR_FUNC_BUFF_SIZE + 1];
  size_t before_value_len = 0;
  char after_value[SHOW_VAR_FUNC_BUFF_SIZE + 1];
  size_t after_value_len = 0;
  SHOW_VAR show_var;
  show_var.name = var->name.str;
  show_var.value = (char *)var;
  show_var.type = SHOW_SYS;
  show_var.scope = SHOW_SCOPE_UNDEF;
  const bool should_log = opt_log_global_var_changes && (type == OPT_GLOBAL);

  if (should_log) {
    AutoWLock lock(&PLock_global_system_variables);
    const char *variable_value =
        get_one_variable(thd, &show_var, OPT_GLOBAL, show_var.type, nullptr,
                         nullptr, before_value, &before_value_len);
    before_value_len = min(before_value_len, (size_t)SHOW_VAR_FUNC_BUFF_SIZE);
    if (variable_value != before_value) {
      memcpy(before_value, variable_value, before_value_len);
    }
    before_value[before_value_len] = '\0';
  }

  /* for persist only syntax do not update the value */
  if (type != OPT_PERSIST_ONLY) {
    if (value)
      ret = (int)var->update(thd, this);
    else
      ret = (int)var->set_default(thd, this);
  }
  /*
   For PERSIST_ONLY syntax we dont change the value of the variable
   for the current session, thus we should not change variables
   source/timestamp/user/host.
  */
  if (ret == 0 && type != OPT_PERSIST_ONLY) {
    update_source_user_host_timestamp(thd);
  }

  if (should_log) {
    AutoWLock lock(&PLock_global_system_variables);
    const char *variable_value =
        get_one_variable(thd, &show_var, OPT_GLOBAL, show_var.type, nullptr,
                         nullptr, after_value, &after_value_len);
    after_value_len = min(after_value_len, (size_t)SHOW_VAR_FUNC_BUFF_SIZE);
    if (variable_value != after_value) {
      memcpy(after_value, variable_value, after_value_len);
    }
    after_value[after_value_len] = '\0';

    LogErr(WARNING_LEVEL, ER_LOG_GLOBAL_VAR_CHANGE, var->name.str, before_value,
           after_value, thd->security_context()->user().str,
           thd->security_context()->host().str);
  }
  return ret;
}

void set_var::print_short(const THD *thd, String *str) {
  str->append(var->name.str, var->name.length);
  str->append(STRING_WITH_LEN("="));
  if (value)
    value->print(thd, str, QT_ORDINARY);
  else
    str->append(STRING_WITH_LEN("DEFAULT"));
}

/**
  Self-print assignment

  @param thd Thread handle
  @param str String buffer to append the partial assignment to.
*/
void set_var::print(const THD *thd, String *str) {
  switch (type) {
    case OPT_PERSIST:
      str->append("PERSIST ");
      break;
    case OPT_PERSIST_ONLY:
      str->append("PERSIST_ONLY ");
      break;
    case OPT_GLOBAL:
      str->append("GLOBAL ");
      break;
    default:
      str->append("SESSION ");
  }
  if (base.length) {
    str->append(base.str, base.length);
    str->append(STRING_WITH_LEN("."));
  }
  print_short(thd, str);
}

/*****************************************************************************
  Functions to handle SET @user_variable=const_expr
*****************************************************************************/

int set_var_user::resolve(THD *thd) {
  /*
    Item_func_set_user_var can't substitute something else on its place =>
    0 can be passed as last argument (reference on item)
  */
  return user_var_item->fix_fields(thd, nullptr) ? -1 : 0;
}

int set_var_user::check(THD *) {
  /*
    Item_func_set_user_var can't substitute something else on its place =>
    0 can be passed as last argument (reference on item)
  */
  return user_var_item->check(false) ? -1 : 0;
}

/**
  Check variable, but without assigning value (used by PS).

  @param thd            thread handler

  @retval
    0   ok
  @retval
    1   ERROR, message sent (normally no variables was updated)
  @retval
    -1   ERROR, message not sent
*/
int set_var_user::light_check(THD *thd) {
  /*
    Item_func_set_user_var can't substitute something else on its place =>
    0 can be passed as last argument (reference on item)
  */
  return (user_var_item->fix_fields(thd, (Item **)nullptr));
}

int set_var_user::update(THD *thd) {
  if (user_var_item->update()) {
    /* Give an error if it's not given already */
    my_error(ER_SET_CONSTANTS_ONLY, MYF(0));
    return -1;
  }
  if (thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
          ->is_enabled())
    thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
        ->mark_as_changed(thd, nullptr);
  return 0;
}

void set_var_user::print(const THD *thd, String *str) {
  user_var_item->print_assignment(thd, str, QT_ORDINARY);
}

/*****************************************************************************
  Functions to handle SET PASSWORD
*****************************************************************************/

set_var_password::set_var_password(LEX_USER *user_arg, char *password_arg,
                                   char *current_password_arg,
                                   bool retain_current, bool gen_pass)
    : user(user_arg),
      password(password_arg),
      current_password(current_password_arg),
      retain_current_password(retain_current),
      generate_password(gen_pass) {
  if (current_password != nullptr) {
    user_arg->uses_replace_clause = true;
    user_arg->current_auth.str = current_password_arg;
    user_arg->current_auth.length = strlen(current_password_arg);
  }
  user_arg->retain_current_password = retain_current_password;
}

set_var_password::~set_var_password() {
  // We copied the generated password buffer to circumvent
  // the password nullification code in change_password()
  if (generate_password) my_free(password);
}

/**
  Check the validity of the SET PASSWORD request

  @param  thd  The current thread
  @return      status code
  @retval 0    failure
  @retval 1    success
*/
int set_var_password::check(THD *thd) {
  /* Returns 1 as the function sends error to client */
  return check_change_password(thd, user->host.str, user->user.str,
                               retain_current_password)
             ? 1
             : 0;
}

int set_var_password::update(THD *thd) {
  if (generate_password) {
    thd->m_disable_password_validation = true;
    std::string generated_password;
    generate_random_password(&generated_password,
                             thd->variables.generated_random_password_length);
    /*
      We need to copy the password buffer here because it will be set to \0
      later by change_password() and since we're generated a random password
      we need to retain it until it can be sent to the client.
      Because set_var_password never will get its destructor called we also
      need to move the string allocated memory to the THD mem root.
    */
    password = thd->mem_strdup(generated_password.c_str());
    str_generated_password = thd->mem_strdup(generated_password.c_str());
  }
  /* Returns 1 as the function sends error to client */
  auto res = change_password(thd, user, password, current_password,
                             retain_current_password)
                 ? 1
                 : 0;
  return res;
}

void set_var_password::print(const THD *thd, String *str) {
  if (user->user.str != nullptr && user->user.length > 0) {
    str->append(STRING_WITH_LEN("PASSWORD FOR "));
    append_identifier(thd, str, user->user.str, user->user.length);
    if (user->host.str != nullptr && user->host.length > 0) {
      str->append(STRING_WITH_LEN("@"));
      append_identifier(thd, str, user->host.str, user->host.length);
    }
    str->append(STRING_WITH_LEN("="));
  } else
    str->append(STRING_WITH_LEN("PASSWORD FOR CURRENT_USER()="));
  str->append(STRING_WITH_LEN("<secret>"));
  if (user->uses_replace_clause) {
    str->append(STRING_WITH_LEN(" REPLACE <secret>"));
  }
  if (user->retain_current_password) {
    str->append(STRING_WITH_LEN(" RETAIN CURRENT PASSWORD"));
  }
}

/*****************************************************************************
  Functions to handle SET NAMES and SET CHARACTER SET
*****************************************************************************/

int set_var_collation_client::check(THD *) {
  /* Currently, UCS-2 cannot be used as a client character set */
  if (!is_supported_parser_charset(character_set_client)) {
    my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), "character_set_client",
             character_set_client->csname);
    return 1;
  }
  return 0;
}

int set_var_collation_client::update(THD *thd) {
  thd->variables.character_set_client = character_set_client;
  thd->variables.character_set_results = character_set_results;
  thd->variables.collation_connection = collation_connection;
  thd->update_charset();

  /* Mark client collation variables as changed */
  if (thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)->is_enabled()) {
    LEX_CSTRING cs_client = {"character_set_client",
                             sizeof("character_set_client") - 1};
    thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)
        ->mark_as_changed(thd, &cs_client);
    LEX_CSTRING cs_results = {"character_set_results",
                              sizeof("character_set_results") - 1};
    thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)
        ->mark_as_changed(thd, &cs_results);
    LEX_CSTRING cs_connection = {"character_set_connection",
                                 sizeof("character_set_connection") - 1};
    thd->session_tracker.get_tracker(SESSION_SYSVARS_TRACKER)
        ->mark_as_changed(thd, &cs_connection);
  }
  if (thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
          ->is_enabled())
    thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
        ->mark_as_changed(thd, nullptr);
  thd->protocol_text->init(thd);
  thd->protocol_binary->init(thd);
  return 0;
}

void set_var_collation_client::print(const THD *, String *str) {
  str->append((set_cs_flags & SET_CS_NAMES) ? "NAMES " : "CHARACTER SET ");
  if (set_cs_flags & SET_CS_DEFAULT)
    str->append("DEFAULT");
  else {
    str->append("'");
    str->append(character_set_client->csname);
    str->append("'");
    if (set_cs_flags & SET_CS_COLLATE) {
      str->append(" COLLATE '");
      str->append(collation_connection->name);
      str->append("'");
    }
  }
}

Per_user_session_variables::Per_user_session_variables() {
  mysql_rwlock_register(
      "sql", key_rwlock_LOCK_per_user_session_var_info,
      array_elements(key_rwlock_LOCK_per_user_session_var_info));
  mysql_rwlock_init(key_rwlock_LOCK_per_user_session_var,
                    &LOCK_per_user_session_var);
}

Per_user_session_variables::~Per_user_session_variables() {
  mysql_rwlock_destroy(&LOCK_per_user_session_var);
}

#define CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only) \
  do {                                                  \
    if (err) return false;                              \
    if (validate_only) return true;                     \
  } while (0)
/**
  Static method
   Set a session variable's value
  name  : the variable name
  value : the value
*/
bool Per_user_session_variables::set_val_do(sys_var *var, Item *item,
                                            THD *thd) {
  LEX_CSTRING tmp;
  set_var set_v(OPT_SESSION, var, tmp, item);

  /* validate and update */
  if (set_v.resolve(thd) || thd->is_error()) return false;
  if (set_v.check(thd) || thd->is_error()) return false;
  if (set_v.update(thd)) return false;
  return true;
}
/**
  Static method
*/
bool Per_user_session_variables::set_val(const std::string &name,
                                         const std::string &val, THD *thd) {
  sys_var *var = intern_find_sys_var(name.c_str(), name.length());
  DBUG_ASSERT(var);
  /* validate only if thd is NULL */
  bool validate_only = !thd;
  const my_option *opt = var->get_option();
  int err = 0;  // 0 means no error
  switch (opt->var_type & GET_TYPE_MASK) {
    case GET_ENUM: {
      bool valid = false;
      int type = find_type(val.c_str(), opt->typelib, FIND_TYPE_BASIC);
      if (type == 0) {
        /* Accept an integer representation of the enumerated item.*/
        char *endptr;
        ulong arg = strtoul(val.c_str(), &endptr, 10);
        if (!*endptr && arg < opt->typelib->count) valid = true;
      } else if (type > 0)
        valid = true;
      if (validate_only) return valid;
      Item_string item(val.c_str(), val.length(), thd->charset());
      return set_val_do(var, &item, thd);
    }
    case GET_BOOL: /* If argument differs from 0, enable option, else disable */
    {
      bool v = get_bool_argument(val.c_str(), (bool *)&err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_int item(v);
      return set_val_do(var, &item, thd);
    }
    case GET_INT: {
      int v = (int)getopt_ll(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_int item(v);
      return set_val_do(var, &item, thd);
    }
    case GET_UINT: {
      uint v = (uint)getopt_ull(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_uint item(v);
      return set_val_do(var, &item, thd);
    }
    case GET_LONG: {
      long v = (long)getopt_ll(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_int item((int32)v);
      return set_val_do(var, &item, thd);
    }
    case GET_ULONG: {
      /* Note that the name says the type is ulong
         but the actual internal type is long */
      long v = (long)getopt_ull(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_int item((int32)v);
      return set_val_do(var, &item, thd);
    }
    case GET_LL: {
      longlong v = (longlong)getopt_ll(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_int item(v);
      return set_val_do(var, &item, thd);
    }
    case GET_ULL: {
      ulonglong v = (ulonglong)getopt_ull(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_uint item(v);
      return set_val_do(var, &item, thd);
    }
    case GET_DOUBLE: {
      double v = getopt_double(val.c_str(), opt, &err);
      CHECK_ERR_AND_VALIDATE_ONLY(err, validate_only);
      Item_uint item(v);
      return set_val_do(var, &item, thd);
    }
    default:
      /* All other types are not supported */
      break;
  }
  return false;
}
/**
  Static method
   Validate a session variable name and its value
  name  : the variable name
  value : the value
*/
bool Per_user_session_variables::validate_val(const std::string &name,
                                              const std::string &val) {
  return set_val(name, val, NULL);
}
/**
  Static method
   Validate and store a per user session variable segment
  users : user list have the same settings
  vars  : session variable list
*/
bool Per_user_session_variables::store(User_session_vars_sp &per_user_vars,
                                       const std::vector<std::string> &users,
                                       const std::vector<Session_var> &vars) {
  if (users.empty() || vars.empty()) return false;
  Session_vars_sp sp_vars = std::make_shared<Session_vars>();
  for (auto it = vars.begin(); it != vars.end(); ++it) {
    const char *name = it->first.c_str();
    size_t name_len = it->first.size();
    const char *val = it->second.c_str();
    /* If the name is a valid var name */
    sys_var *var = intern_find_sys_var(name, name_len);
    if (!var) return false;
    /* If the value is valid */
    if (!validate_val(it->first, it->second)) return false;
    /* Insert the name value pair */
    std::pair<Session_vars_it, bool> ret =
        sp_vars->insert(std::make_pair(name, val));
    /* Duplicate entries are not allowed */
    if (!ret.second) return false;
  }
  DBUG_ASSERT(sp_vars->size() == vars.size());
  /* We don't validate user names so that per user session variables
    can be defined earlier and users can be added later */
  for (auto it = users.begin(); it != users.end(); ++it) {
    std::pair<User_session_vars_it, bool> ret =
        per_user_vars->insert(std::make_pair(*it, sp_vars));
    /* Duplicate entries are not allowed */
    if (!ret.second) return false;
  }
  return true;
}

/**
   Static method
*/
bool Per_user_session_variables::init_do(User_session_vars_sp &per_user_vars,
                                         const char *sys_var_str) {
  /* NULL and "" are valid values */
  if (!sys_var_str || !strlen(sys_var_str)) return true;
  std::vector<std::string> users;
  users.reserve(16);
  std::vector<Session_var> vars;
  vars.reserve(32);
  std::string key, val;
  char user_name_delimiter = per_user_session_var_user_name_delimiter_ptr[0];
  char delimiters[4] = {user_name_delimiter, '=', ',', '\0'};
  static const char *invalid_tokens = " \t\"\\/';";
  const char *p = sys_var_str;
  const char *prev = p;
  /* Parsing the string value, for example:
    --per-user-session-var-default-val="u1:u2:u3:k1=v1,k2=v2,k3=v3,u4:k4=v4"
 */
  while (*p) {
    /* Assume the user name delimiter is ':'. Get a token in the forms of
       "foo:", "bar=", "baz,", or the last one in the form of "tail" */
    while (*p && !strchr(delimiters, *p)) {
      /* Return immediately if invalid tokens are seen */
      if (strchr(invalid_tokens, *p++)) return false;
    }
    /* Consecutive delimiters */
    if (p == prev) return false;
    if (*p == user_name_delimiter) {
      /* Return if we have a key with no value */
      if (!key.empty()) return false;
      if (!vars.empty()) {
        /* Return false if we have key/values but no users, otherwise
           store the segment of per user session variables. */
        if (users.empty() || !store(per_user_vars, users, vars)) return false;
        /* Start a new segment */
        users.clear();
        vars.clear();
      }
      users.push_back(std::string(prev, p - prev));
    }
    /* Saw a key */
    else if (*p == '=') {
      /* Return if we have a key with no value */
      if (!key.empty()) return false;
      key = std::string(prev, p - prev);
    }
    /* Saw a val */
    else if (*p == ',' || *p == '\0') {
      /* Return false if we don't have a valid key,
         Or we have key/values but no users. */
      if (key.empty() || users.empty()) return false;
      val = std::string(prev, p - prev);
      vars.push_back(Session_var(key, val));
      if (*p == '\0') {
        /* Store the segment of per user session variables */
        if (!store(per_user_vars, users, vars)) return false;
        /* Done */
        return true;
      }
      key.clear();
    }
    /* Skip the delimiter and start a new token */
    prev = ++p;
  }
  /* Saw end of string at wrong stage */
  return false;
}

/**
   Set per user session variables for a THD
*/
bool Per_user_session_variables::set_thd(THD *thd) {
  bool ret = true;
  std::string err_msg;
  const char *user = thd->m_main_security_ctx.user().str;
  mysql_rwlock_rdlock(&LOCK_per_user_session_var);
  if (per_user_session_vars) {
    if (user) {
      User_session_vars_it iter = per_user_session_vars->find(user);
      if (iter != per_user_session_vars->end()) {
        /* Temporarily grant super privilege to this user if it doesn't
           have it because root privilege will be needed to set some
           session variables like binlog_format.
        */
        bool temp_super_acl = false;
        auto ctx = thd->security_context();
        if (!thd->security_context()->check_access(SUPER_ACL)) {
          ctx->set_master_access(ctx->master_access() | (SUPER_ACL));
          temp_super_acl = true;
        }

        Session_vars_sp vars_sp = iter->second;
        DBUG_ASSERT(vars_sp->size() > 0);
        for (Session_vars_it it = vars_sp->begin(); it != vars_sp->end();
             ++it) {
          if (!set_val(it->first, it->second, thd)) {
            /* Log the error and continue */
            ret = false;
            if (!err_msg.empty()) err_msg += ",";
            err_msg += it->first + "=" + it->second;
          }
        }
        /* Remove the temporary super ACL */
        if (temp_super_acl)
          ctx->set_master_access(ctx->master_access() & ~(SUPER_ACL));
      }
    } else {
      /* The user wasn't set. This should never happen. */
      DBUG_ASSERT(false);
      ret = false;
    }
  }
  mysql_rwlock_unlock(&LOCK_per_user_session_var);
  if (!ret) {
    /* Log the errors into server log */
    DBUG_ASSERT(!err_msg.empty());
    static const std::string s =
        "Failed to set per-user session variables for user ";
    err_msg = s + user + ":" + err_msg;
    fprintf(stderr, "[Warning] %s\n", err_msg.c_str());
  }
  return ret;
}

void Per_user_session_variables::print() {
  char name_delimiter = per_user_session_var_user_name_delimiter_ptr[0];
  mysql_rwlock_rdlock(&LOCK_per_user_session_var);
  if (per_user_session_vars) {
    for (auto it1 = per_user_session_vars->begin();
         it1 != per_user_session_vars->end(); ++it1) {
      std::string msg = it1->first + name_delimiter;
      for (auto it2 = it1->second->begin(); it2 != it1->second->end(); ++it2) {
        if (msg.back() != name_delimiter) msg += ",";
        msg += it2->first + "=" + it2->second;
      }
      fprintf(stderr, "[Per-user session variables] %s\n", msg.c_str());
    }
  }
  mysql_rwlock_unlock(&LOCK_per_user_session_var);
}

bool Per_user_session_variables::init(const char *sys_var_str) {
  /* Create a new hash table */
  User_session_vars_sp per_user_vars = std::make_shared<User_session_vars>();
  bool ret = false;
  if (init_do(per_user_vars, sys_var_str)) {
    /* Write-lock */
    mysql_rwlock_wrlock(&LOCK_per_user_session_var);
    per_user_session_vars.swap(per_user_vars);
    mysql_rwlock_unlock(&LOCK_per_user_session_var);
    ret = true;
  }
  /* Print all the values in the hash table into log file. */
  print();
  return ret;
}

bool Per_user_session_variables::init() {
  /* This is called during server starting time so it is safe
     to access the global pointer directly */
  if (!per_user_session_var_default_val_ptr ||
      !strlen(per_user_session_var_default_val_ptr)) {
    return true;
  }
  return init(per_user_session_var_default_val_ptr);
}
