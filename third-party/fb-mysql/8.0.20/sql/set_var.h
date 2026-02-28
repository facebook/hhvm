#ifndef SET_VAR_INCLUDED
#define SET_VAR_INCLUDED
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

/**
  @file
  "public" interface to sys_var - server configuration variables.
*/

#include "my_config.h"

#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_getopt.h"    // get_opt_arg_type
#include "my_hostname.h"  // HOSTNAME_LENGTH
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_systime.h"  // my_micro_time()
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/components/services/system_variable_source_type.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/status_var.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"           // Item_result
#include "prealloced_array.h"    // Prealloced_array
#include "sql/sql_const.h"       // SHOW_COMP_OPTION
#include "sql/sql_plugin_ref.h"  // plugin_ref
#include "typelib.h"             // TYPELIB

class Item;
class Item_func_set_user_var;
class PolyLock;
class String;
class THD;
class Time_zone;
class set_var;
class sys_var;
class sys_var_pluginvar;
struct LEX_USER;
template <class Key, class Value>
class collation_unordered_map;

typedef ulonglong sql_mode_t;
typedef enum enum_mysql_show_type SHOW_TYPE;
typedef enum enum_mysql_show_scope SHOW_SCOPE;
template <class T>
class List;

extern TYPELIB bool_typelib;

/* Number of system variable elements to preallocate. */
#define SHOW_VAR_PREALLOC 200
typedef Prealloced_array<SHOW_VAR, SHOW_VAR_PREALLOC> Show_var_array;

struct sys_var_chain {
  sys_var *first;
  sys_var *last;
};

int mysql_add_sys_var_chain(sys_var *chain);
int mysql_del_sys_var_chain(sys_var *chain);

enum enum_var_type : int {
  OPT_DEFAULT = 0,
  OPT_SESSION,
  OPT_GLOBAL,
  OPT_PERSIST,
  OPT_PERSIST_ONLY
};

/**
  A class representing one system variable - that is something
  that can be accessed as @@global.variable_name or @@session.variable_name,
  visible in SHOW xxx VARIABLES and in INFORMATION_SCHEMA.xxx_VARIABLES,
  optionally it can be assigned to, optionally it can have a command-line
  counterpart with the same name.
*/
class sys_var {
 public:
  sys_var *next;
  LEX_CSTRING name;
  ulong thd_id;
  enum flag_enum {
    GLOBAL = 0x0001,
    SESSION = 0x0002,
    ONLY_SESSION = 0x0004,
    SCOPE_MASK = 0x03FF,  // 1023
    READONLY = 0x0400,    // 1024
    ALLOCATED = 0x0800,   // 2048
    INVISIBLE = 0x1000,   // 4096
    TRI_LEVEL = 0x2000,   // 8192 - default is neither GLOBAL nor SESSION
    NOTPERSIST = 0x4000,
    HINT_UPDATEABLE = 0x8000,  // Variable is updateable using SET_VAR hint
    /**
     There can be some variables which needs to be set before plugin is loaded.
     ex: binlog_checksum needs to be set before GR plugin is loaded.
     Also, there are some variables which needs to be set before some server
     internal component initialization.
     ex: binlog_encryption needs to be set before binary and relay log
     files generation.
    */

    PERSIST_AS_READ_ONLY = 0x10000
  };
  static const int PARSE_EARLY = 1;
  static const int PARSE_NORMAL = 2;
  /**
    Enumeration type to indicate for a system variable whether
    it will be written to the binlog or not.
  */
  enum binlog_status_enum {
    VARIABLE_NOT_IN_BINLOG,
    SESSION_VARIABLE_IN_BINLOG
  } binlog_status;

 protected:
  typedef bool (*on_check_function)(sys_var *self, THD *thd, set_var *var);
  typedef bool (*on_update_function)(sys_var *self, THD *thd,
                                     enum_var_type type);

  int flags;                      ///< or'ed flag_enum values
  int m_parse_flag;               ///< either PARSE_EARLY or PARSE_NORMAL.
  const SHOW_TYPE show_val_type;  ///< what value_ptr() returns for sql_show.cc
  my_option option;               ///< min, max, default values are stored here
  PolyLock *guard;                ///< *second* lock that protects the variable
  ptrdiff_t offset;  ///< offset to the value from global_system_variables
  on_check_function on_check;
  on_update_function on_update;
  const char *const deprecation_substitute;
  bool is_os_charset;  ///< true if the value is in character_set_filesystem
  struct get_opt_arg_source source;
  char user[USERNAME_CHAR_LENGTH + 1]; /* which user  has set this variable */
  char host[HOSTNAME_LENGTH + 1];      /* host on which this variable is set */
  ulonglong timestamp; /* represents when this variable was set */

 public:
  sys_var(sys_var_chain *chain, const char *name_arg, const char *comment,
          int flag_args, ptrdiff_t off, int getopt_id,
          enum get_opt_arg_type getopt_arg_type, SHOW_TYPE show_val_type_arg,
          longlong def_val, PolyLock *lock,
          enum binlog_status_enum binlog_status_arg,
          on_check_function on_check_func, on_update_function on_update_func,
          const char *substitute, int parse_flag);

  virtual ~sys_var() {}

  /**
    All the cleanup procedures should be performed here
  */
  virtual void cleanup() {}
  /**
    downcast for sys_var_pluginvar. Returns this if it's an instance
    of sys_var_pluginvar, and 0 otherwise.
  */
  virtual sys_var_pluginvar *cast_pluginvar() { return nullptr; }

  bool check(THD *thd, set_var *var);
  const uchar *value_ptr(THD *running_thd, THD *target_thd, enum_var_type type,
                         LEX_STRING *base);
  const uchar *value_ptr(THD *thd, enum_var_type type, LEX_STRING *base);
  virtual void update_default(longlong new_def_value) {
    option.def_value = new_def_value;
  }
  longlong get_default() { return option.def_value; }
  virtual longlong get_min_value() { return option.min_value; }
  virtual ulonglong get_max_value() { return option.max_value; }
  /**
    Returns variable type.

    @return variable type
  */
  virtual ulong get_var_type() { return (option.var_type & GET_TYPE_MASK); }
  virtual void set_arg_source(get_opt_arg_source *) {}
  virtual void set_is_plugin(bool) {}
  enum_variable_source get_source() { return source.m_source; }
  const char *get_source_name() { return source.m_path_name; }
  void set_source(enum_variable_source src) {
    option.arg_source->m_source = src;
  }
  bool set_source_name(const char *path) {
    return set_and_truncate(option.arg_source->m_path_name, path,
                            sizeof(option.arg_source->m_path_name));
  }
  bool set_user(const char *usr) {
    return set_and_truncate(user, usr, sizeof(user));
  }
  const char *get_user() { return user; }
  const char *get_host() { return host; }
  bool set_host(const char *hst) {
    return set_and_truncate(host, hst, sizeof(host));
  }
  ulonglong get_timestamp() const { return timestamp; }
  void set_user_host(THD *thd);
  my_option *get_option() { return &option; }
  void set_timestamp() { timestamp = my_micro_time(); }
  void set_timestamp(ulonglong ts) { timestamp = ts; }
  void clear_user_host_timestamp() {
    user[0] = '\0';
    host[0] = '\0';
    timestamp = 0;
  }
  virtual bool is_non_persistent() { return flags & NOTPERSIST; }

  /**
     Update the system variable with the default value from either
     session or global scope.  The default value is stored in the
     'var' argument. Return false when successful.
  */
  bool set_default(THD *thd, set_var *var);
  bool update(THD *thd, set_var *var);

  /**
    This function converts value stored in save_result to string. This
    function must ba called after calling save_default() as save_default() will
    store default value to save_result.
  */
  virtual void saved_value_to_string(THD *thd, set_var *var, char *def_val) = 0;

  SHOW_TYPE show_type() { return show_val_type; }
  int scope() const { return flags & SCOPE_MASK; }
  const CHARSET_INFO *charset(THD *thd);
  bool is_readonly() const { return flags & READONLY; }
  bool not_visible() const { return flags & INVISIBLE; }
  bool is_trilevel() const { return flags & TRI_LEVEL; }
  bool is_persist_readonly() const { return flags & PERSIST_AS_READ_ONLY; }
  /**
    Check if the variable can be set using SET_VAR hint.

    @return true if the variable can be set using SET_VAR hint,
            false otherwise.
  */
  bool is_hint_updateable() const { return flags & HINT_UPDATEABLE; }
  /**
    the following is only true for keycache variables,
    that support the syntax @@keycache_name.variable_name
  */
  bool is_struct() { return option.var_type & GET_ASK_ADDR; }
  bool is_written_to_binlog(enum_var_type type) {
    return type != OPT_GLOBAL && binlog_status == SESSION_VARIABLE_IN_BINLOG;
  }
  virtual bool check_update_type(Item_result type) = 0;

  /**
    Return true for success if:
      Global query and variable scope is GLOBAL or SESSION, or
      Session query and variable scope is SESSION or ONLY_SESSION.
  */
  bool check_scope(enum_var_type query_type) {
    switch (query_type) {
      case OPT_PERSIST:
      case OPT_PERSIST_ONLY:
      case OPT_GLOBAL:
        return scope() & (GLOBAL | SESSION);
      case OPT_SESSION:
        return scope() & (SESSION | ONLY_SESSION);
      case OPT_DEFAULT:
        return scope() & (SESSION | ONLY_SESSION);
    }
    return false;
  }
  bool is_global_persist(enum_var_type type) {
    return (type == OPT_GLOBAL || type == OPT_PERSIST ||
            type == OPT_PERSIST_ONLY);
  }

  /**
    Return true if settable at the command line
  */
  bool is_settable_at_command_line() { return option.id != -1; }

  bool register_option(std::vector<my_option> *array, int parse_flags) {
    return is_settable_at_command_line() && (m_parse_flag & parse_flags) &&
           (array->push_back(option), false);
  }
  void do_deprecated_warning(THD *thd);
  /**
    Create item from system variable value.

    @param  thd  pointer to THD object

    @return pointer to Item object or NULL if it's
            impossible to obtain the value.
  */
  Item *copy_value(THD *thd);

  void save_default(THD *thd, set_var *var) { global_save_default(thd, var); }

 private:
  inline static bool set_and_truncate(char *dst, const char *string,
                                      size_t sizeof_dst) {
    size_t string_length = strlen(string), length;
    length = std::min(sizeof_dst - 1, string_length);
    memcpy(dst, string, length);
    dst[length] = 0;
    return length < string_length;  // truncated
  }
  virtual bool do_check(THD *thd, set_var *var) = 0;
  /**
    save the session default value of the variable in var
  */
  virtual void session_save_default(THD *thd, set_var *var) = 0;
  /**
    save the global default value of the variable in var
  */
  virtual void global_save_default(THD *thd, set_var *var) = 0;
  virtual bool session_update(THD *thd, set_var *var) = 0;
  virtual bool global_update(THD *thd, set_var *var) = 0;

 protected:
  /**
    A pointer to a value of the variable for SHOW.
    It must be of show_val_type type (bool for SHOW_BOOL, int for SHOW_INT,
    longlong for SHOW_LONGLONG, etc).
  */
  virtual const uchar *session_value_ptr(THD *running_thd, THD *target_thd,
                                         LEX_STRING *base);
  virtual const uchar *global_value_ptr(THD *thd, LEX_STRING *base);

  /**
    A pointer to a storage area of the variable, to the raw data.
    Typically it's the same as session_value_ptr(), but it's different,
    for example, for ENUM, that is printed as a string, but stored as a number.
  */
  uchar *session_var_ptr(THD *thd);

  uchar *global_var_ptr();
};

/****************************************************************************
  Classes for parsing of the SET command
****************************************************************************/

/**
  A base class for everything that can be set with SET command.
  It's similar to Items, an instance of this is created by the parser
  for every assigmnent in SET (or elsewhere, e.g. in SELECT).
*/
class set_var_base {
 public:
  set_var_base() {}
  virtual ~set_var_base() {}
  virtual int resolve(THD *thd) = 0;  ///< Check privileges & fix_fields
  virtual int check(THD *thd) = 0;    ///< Evaluate the expression
  virtual int update(THD *thd) = 0;   ///< Set the value
  virtual void print(const THD *thd, String *str) = 0;  ///< To self-print

  /**
    @returns whether this variable is @@@@optimizer_trace.
  */
  virtual bool is_var_optimizer_trace() const { return false; }

  /**
    Used only by prepared statements to resolve and check. No locking of tables
    between the two phases.
  */
  virtual int light_check(THD *thd) { return (resolve(thd) || check(thd)); }

  ulong thd_id = 0;
};

/**
  set_var_base descendant for assignments to the system variables.
*/
class set_var : public set_var_base {
 public:
  sys_var *var;  ///< system variable to be updated
  Item *value;   ///< the expression that provides the new value of the variable
  enum_var_type type;
  union  ///< temp storage to hold a value between sys_var::check and ::update
  {
    ulonglong ulonglong_value;  ///< for all integer, set, enum sysvars
    double double_value;        ///< for Sys_var_double
    plugin_ref plugin;          ///< for Sys_var_plugin
    Time_zone *time_zone;       ///< for Sys_var_tz
    LEX_STRING string_value;    ///< for Sys_var_charptr and others
    const void *ptr;            ///< for Sys_var_struct
  } save_result;
  LEX_CSTRING
  base; /**< for structured variables, like keycache_name.variable_name */

  set_var(enum_var_type type_arg, sys_var *var_arg, LEX_CSTRING base_name_arg,
          Item *value_arg);

  int resolve(THD *thd);
  int check(THD *thd);
  int update(THD *thd);
  void update_source_user_host_timestamp(THD *thd);
  int light_check(THD *thd);
  /**
    Print variable in short form.

    @param thd Thread handle.
    @param str String buffer to append the partial assignment to.
  */
  void print_short(const THD *thd, String *str);
  void print(const THD *, String *str); /* To self-print */
  bool is_global_persist() {
    return (type == OPT_GLOBAL || type == OPT_PERSIST ||
            type == OPT_PERSIST_ONLY);
  }
  virtual bool is_var_optimizer_trace() const {
    extern sys_var *Sys_optimizer_trace_ptr;
    return var == Sys_optimizer_trace_ptr;
  }
};

/* User variables like @my_own_variable */
class set_var_user : public set_var_base {
  Item_func_set_user_var *user_var_item;

 public:
  set_var_user(Item_func_set_user_var *item) : user_var_item(item) {}
  int resolve(THD *thd);
  int check(THD *thd);
  int update(THD *thd);
  int light_check(THD *thd);
  void print(const THD *thd, String *str); /* To self-print */
};

class set_var_password : public set_var_base {
  LEX_USER *user;
  char *password;
  const char *current_password;
  bool retain_current_password;
  bool generate_password;
  char *str_generated_password;

 public:
  set_var_password(LEX_USER *user_arg, char *password_arg,
                   char *current_password_arg, bool retain_current,
                   bool generate_password);

  const LEX_USER *get_user(void) { return user; }
  bool has_generated_password(void) { return generate_password; }
  const char *get_generated_password(void) { return str_generated_password; }
  int resolve(THD *) { return 0; }
  int check(THD *thd);
  int update(THD *thd);
  void print(const THD *thd, String *str); /* To self-print */
  virtual ~set_var_password();
};

/* For SET NAMES and SET CHARACTER SET */

class set_var_collation_client : public set_var_base {
  int set_cs_flags;
  const CHARSET_INFO *character_set_client;
  const CHARSET_INFO *character_set_results;
  const CHARSET_INFO *collation_connection;

 public:
  enum set_cs_flags_enum {
    SET_CS_NAMES = 1,
    SET_CS_DEFAULT = 2,
    SET_CS_COLLATE = 4
  };
  set_var_collation_client(int set_cs_flags_arg,
                           const CHARSET_INFO *client_coll_arg,
                           const CHARSET_INFO *connection_coll_arg,
                           const CHARSET_INFO *result_coll_arg)
      : set_cs_flags(set_cs_flags_arg),
        character_set_client(client_coll_arg),
        character_set_results(result_coll_arg),
        collation_connection(connection_coll_arg) {}
  int resolve(THD *) { return 0; }
  int check(THD *thd);
  int update(THD *thd);
  void print(const THD *thd, String *str); /* To self-print */
};

/**
   Per-user session variables
*/

class Per_user_session_variables {
  /**
    A session variable item
    First  : the variable name
    Second : the value
  */
  using Session_var = std::pair<std::string, std::string>;

  /**
    The session variables for a user
    Key   : the session variable name
    Value : the default value of this session variable
  */
  using Session_vars = std::unordered_map<std::string, std::string>;
  using Session_vars_it = Session_vars::iterator;
  using Session_vars_sp = std::shared_ptr<Session_vars>;
  /**
    The session variables for users
    Key   : the user name
    Value : the collection of session variables of this user
  */
  using User_session_vars = std::unordered_map<std::string, Session_vars_sp>;
  using User_session_vars_it = User_session_vars::iterator;

  /**
    Global hash table for per-user session variables
  */
  using User_session_vars_sp = std::shared_ptr<User_session_vars>;

  /* The per-user session variable hash table */
  User_session_vars_sp per_user_session_vars;
  mysql_rwlock_t LOCK_per_user_session_var;
  PSI_rwlock_key key_rwlock_LOCK_per_user_session_var;
  PSI_rwlock_info key_rwlock_LOCK_per_user_session_var_info[1] = {
      {&key_rwlock_LOCK_per_user_session_var,
       "Per_user_session_variables::rwlock", PSI_FLAG_SINGLETON, 0,
       "rwlock for per-user session hash table"}};

 public:
  Per_user_session_variables();
  ~Per_user_session_variables();

 private:
  /**
    Set a session variable's value
    name  : the variable name
    value : the value
  */
  static bool set_val_do(sys_var *var, Item *item, THD *thd);
  static bool set_val(const std::string &name, const std::string &val,
                      THD *thd);
  /**
    Validate a session variable name and its value
    name  : the variable name
    value : the value
  */
  static bool validate_val(const std::string &name, const std::string &val);
  /**
    Validate and store a per user session variable segment
    users : user list have the same settings
    vars  : session variable list
  */
  static bool store(
      User_session_vars_sp &per_user_vars,
      const std::vector<std::string> &users,
      const std::vector<Per_user_session_variables::Session_var> &vars);
  /**
    Do the actual initialization
  */
  static bool init_do(User_session_vars_sp &per_user_vars,
                      const char *sys_var_str);

 public:
  /**
    Set per user session variables for a THD.
  */
  bool set_thd(THD *thd);
  /**
    Print all the values in the hash table into log file.
  */
  void print();
  /**
    Initialize the per user session variables.
    This is called by SET command.
  */
  bool init(const char *sys_var_str);
  /**
    Initialize the per user session variables.
    This is called during server starting time.
  */
  bool init();
};

/* optional things, have_* variables */
extern SHOW_COMP_OPTION have_profiling;

extern SHOW_COMP_OPTION have_symlink, have_dlopen;
extern SHOW_COMP_OPTION have_query_cache;
extern SHOW_COMP_OPTION have_geometry, have_rtree_keys;
extern SHOW_COMP_OPTION have_compress;
extern SHOW_COMP_OPTION have_statement_timeout;

/*
  Helper functions
*/
ulong get_system_variable_hash_records(void);
ulonglong get_system_variable_hash_version(void);
collation_unordered_map<std::string, sys_var *> *get_system_variable_hash(void);

Per_user_session_variables *get_per_user_session_variables(void);

extern bool get_sysvar_source(const char *name, uint length,
                              enum enum_variable_source *source);

bool enumerate_sys_vars(Show_var_array *show_var_array, bool sort,
                        enum enum_var_type type, bool strict);
void lock_plugin_mutex();
void unlock_plugin_mutex();
sys_var *find_sys_var(THD *thd, const char *str, size_t length = 0);
sys_var *find_sys_var_ex(THD *thd, const char *str, size_t length = 0,
                         bool throw_error = false, bool locked = false);
int sql_set_variables(THD *thd, List<set_var_base> *var_list, bool opened);
bool keyring_access_test();
bool fix_delay_key_write(sys_var *self, THD *thd, enum_var_type type);
bool set_gap_lock_exception_list(sys_var *, THD *, enum_var_type);

sql_mode_t expand_sql_mode(sql_mode_t sql_mode, THD *thd);
bool sql_mode_string_representation(THD *thd, sql_mode_t sql_mode,
                                    LEX_STRING *ls);
bool sql_mode_quoted_string_representation(THD *thd, sql_mode_t sql_mode,
                                           LEX_STRING *ls);
void update_parser_max_mem_size();

extern sys_var *Sys_autocommit_ptr;
extern sys_var *Sys_gtid_next_ptr;
extern sys_var *Sys_gtid_next_list_ptr;
extern sys_var *Sys_gtid_purged_ptr;

extern ulonglong system_variable_hash_version;

const CHARSET_INFO *get_old_charset_by_name(const char *old_name);

int sys_var_init();
int sys_var_add_options(std::vector<my_option> *long_options, int parse_flags);
void sys_var_end(void);

/* check needed privileges to perform SET PERSIST[_only] or RESET PERSIST */
bool check_priv(THD *thd, bool static_variable);

#define PERSIST_ONLY_ADMIN_X509_SUBJECT "persist_only_admin_x509_subject"
#define PERSISTED_GLOBALS_LOAD "persisted_globals_load"
extern char *sys_var_persist_only_admin_x509_subject;

#endif
