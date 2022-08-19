/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _sql_comp_common_h
#define _sql_comp_common_h

#include <string.h>
#include <sys/types.h>
#include <new>
#include <string>

#include "lex_string.h"
#include "map_helpers.h"
#include "memory_debugging.h"
#include "my_compiler.h"
#include "my_getopt.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "sql/set_var.h"

class Item;
class THD;
struct MEM_ROOT;
struct SYS_VAR;
struct TYPELIB;
struct st_plugin_int;

/*
  Below define's are used for internal purpose(i.e used by plugin and
  component infrastuctures, These are not supposed to be used in other files.
*/
#define OPTION_SET_LIMITS(type, options, opt) \
  options->var_type = type;                   \
  options->def_value = (opt)->def_val;        \
  options->min_value = (opt)->min_val;        \
  options->max_value = (opt)->max_val;        \
  options->block_size = (long)(opt)->blk_sz

#define OPTION_SET_LIMITS_DOUBLE(options, opt)                            \
  options->var_type = GET_DOUBLE;                                         \
  options->def_value = (longlong)getopt_double2ulonglong((opt)->def_val); \
  options->min_value = (longlong)getopt_double2ulonglong((opt)->min_val); \
  options->max_value = getopt_double2ulonglong((opt)->max_val);           \
  options->block_size = (long)(opt)->blk_sz;

/****************************************************************************
  Internal type declarations for variables support
****************************************************************************/

#undef MYSQL_SYSVAR_NAME
#define MYSQL_SYSVAR_NAME(name) name
#define PLUGIN_VAR_TYPEMASK 0x007f
#define PLUGIN_VAR_WITH_SIGN_TYPEMASK 0x00ff

#define EXTRA_OPTIONS 3 /* options for: 'foo', 'plugin-foo' and NULL */

/*
  Below typedef's are used for internal purpose(i.e used by plugin and
  component infrastuctures, These are not supposed to be used in other files.
*/
typedef DECLARE_MYSQL_SYSVAR_BASIC(sysvar_bool_t, bool);
typedef DECLARE_MYSQL_THDVAR_BASIC(thdvar_bool_t, bool);
typedef DECLARE_MYSQL_SYSVAR_BASIC(sysvar_str_t, char *);
typedef DECLARE_MYSQL_THDVAR_BASIC(thdvar_str_t, char *);

typedef DECLARE_MYSQL_SYSVAR_TYPELIB(sysvar_enum_t, unsigned long);
typedef DECLARE_MYSQL_THDVAR_TYPELIB(thdvar_enum_t, unsigned long);
typedef DECLARE_MYSQL_SYSVAR_TYPELIB(sysvar_set_t, ulonglong);
typedef DECLARE_MYSQL_THDVAR_TYPELIB(thdvar_set_t, ulonglong);

typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_int_t, int);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_long_t, long);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_longlong_t, longlong);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_uint_t, uint);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_ulong_t, ulong);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_ulonglong_t, ulonglong);
typedef DECLARE_MYSQL_SYSVAR_SIMPLE(sysvar_double_t, double);

typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_int_t, int);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_long_t, long);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_longlong_t, longlong);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_uint_t, uint);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_ulong_t, ulong);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_ulonglong_t, ulonglong);
typedef DECLARE_MYSQL_THDVAR_SIMPLE(thdvar_double_t, double);

/****************************************************************************
  default variable data check and update functions
****************************************************************************/
int check_func_bool(THD *, SYS_VAR *, void *save, st_mysql_value *value);

int check_func_int(THD *thd, SYS_VAR *var, void *save, st_mysql_value *value);

int check_func_long(THD *thd, SYS_VAR *var, void *save, st_mysql_value *value);

int check_func_longlong(THD *thd, SYS_VAR *var, void *save,
                        st_mysql_value *value);

int check_func_str(THD *thd, SYS_VAR *, void *save, st_mysql_value *value);

int check_func_enum(THD *, SYS_VAR *var, void *save, st_mysql_value *value);

int check_func_set(THD *, SYS_VAR *var, void *save, st_mysql_value *value);

int check_func_double(THD *thd, SYS_VAR *var, void *save,
                      st_mysql_value *value);

void update_func_bool(THD *, SYS_VAR *, void *tgt, const void *save);

void update_func_int(THD *, SYS_VAR *, void *tgt, const void *save);

void update_func_long(THD *, SYS_VAR *, void *tgt, const void *save);

void update_func_longlong(THD *, SYS_VAR *, void *tgt, const void *save);

void update_func_str(THD *, SYS_VAR *, void *tgt, const void *save);

void update_func_double(THD *, SYS_VAR *, void *tgt, const void *save);

SHOW_TYPE pluginvar_show_type(SYS_VAR *plugin_var);

int item_value_type(st_mysql_value *value);
const char *item_val_str(st_mysql_value *value, char *buffer, int *length);
int item_val_int(st_mysql_value *value, long long *buf);
int item_is_unsigned(st_mysql_value *value);
int item_val_real(st_mysql_value *value, double *buf);

void plugin_opt_set_limits(struct my_option *, const SYS_VAR *);

uchar *intern_sys_var_ptr(THD *thd, int offset, bool global_lock);

bool plugin_var_memalloc_global_update(THD *thd, SYS_VAR *var, char **dest,
                                       const char *value);
bool plugin_var_memalloc_session_update(THD *thd, SYS_VAR *var, char **dest,
                                        const char *value);

/*
  stored in bookmark_hash, this structure is never removed from the
  hash and is used to mark a single offset for a thd local variable
  even if plugins have been uninstalled and reinstalled, repeatedly.
  This structure is allocated from plugin_mem_root.

  The key format is as follows:
    1 byte         - variable type code
    name_len bytes - variable name
    '\0'           - end of key
*/
struct st_bookmark {
  size_t name_len;
  int offset;
  uint version;
  char key[1];
};

st_bookmark *find_bookmark(const char *plugin, const char *name, int flags);

/*
  skeleton of a plugin variable - portion of structure common to all.
*/
struct SYS_VAR {
  MYSQL_PLUGIN_VAR_HEADER;
};

inline void convert_underscore_to_dash(char *str, size_t len) {
  for (char *p = str; p <= str + len; p++)
    if (*p == '_') *p = '-';
}

/*
  sys_var class for access to all plugin variables visible to the user
*/
class sys_var_pluginvar : public sys_var {
  static bool on_check_pluginvar(sys_var *self, THD *, set_var *var);

 public:
  bool is_plugin;
  st_plugin_int *plugin;
  SYS_VAR *plugin_var;
  /**
    variable name from whatever is hard-coded in the plugin source
    and doesn't have pluginname- prefix is replaced by an allocated name
    with a plugin prefix. When plugin is uninstalled we need to restore the
    pointer to point to the hard-coded value, because plugin may be
    installed/uninstalled many times without reloading the shared object.
  */
  const char *orig_pluginvar_name;

  static void *operator new(size_t size, MEM_ROOT *mem_root,
                            const std::nothrow_t &arg MY_ATTRIBUTE((unused)) =
                                std::nothrow) noexcept {
    return mem_root->Alloc(size);
  }

  static void *operator new(size_t size) {
    return my_malloc(PSI_NOT_INSTRUMENTED, size, MYF(0));
  }

  static void operator delete(void *ptr_arg MY_ATTRIBUTE((unused)),
                              size_t size MY_ATTRIBUTE((unused))) {
    TRASH(ptr_arg, size);
  }

  static void operator delete(
      void *, MEM_ROOT *, const std::nothrow_t &)noexcept { /* never called */
  }

  static void operator delete(void *ptr) { my_free(ptr); }

  sys_var_pluginvar(sys_var_chain *chain, const char *name_arg,
                    SYS_VAR *plugin_var_arg)
      : sys_var(
            chain, name_arg, plugin_var_arg->comment,
            (plugin_var_arg->flags & PLUGIN_VAR_THDLOCAL ? SESSION : GLOBAL) |
                (plugin_var_arg->flags & PLUGIN_VAR_READONLY ? READONLY : 0) |
                (plugin_var_arg->flags & PLUGIN_VAR_INVISIBLE ? INVISIBLE : 0) |
                (plugin_var_arg->flags & PLUGIN_VAR_PERSIST_AS_READ_ONLY
                     ? PERSIST_AS_READ_ONLY
                     : 0),
            0, (plugin_var_arg->flags & PLUGIN_VAR_NOCMDOPT) ? -1 : 0,
            (plugin_var_arg->flags & PLUGIN_VAR_NOCMDARG
                 ? NO_ARG
                 : (plugin_var_arg->flags & PLUGIN_VAR_OPCMDARG
                        ? OPT_ARG
                        : (plugin_var_arg->flags & PLUGIN_VAR_RQCMDARG
                               ? REQUIRED_ARG
                               : REQUIRED_ARG))),
            pluginvar_show_type(plugin_var_arg), 0, nullptr,
            VARIABLE_NOT_IN_BINLOG,
            (plugin_var_arg->flags & PLUGIN_VAR_NODEFAULT) ? on_check_pluginvar
                                                           : nullptr,
            nullptr, nullptr, PARSE_NORMAL),
        plugin_var(plugin_var_arg),
        orig_pluginvar_name(plugin_var_arg->name) {
    plugin_var->name = name_arg;
    is_plugin = true;
  }
  sys_var_pluginvar *cast_pluginvar() { return this; }
  bool check_update_type(Item_result type);
  SHOW_TYPE show_type();
  uchar *real_value_ptr(THD *thd, enum_var_type type);
  TYPELIB *plugin_var_typelib(void);
  uchar *do_value_ptr(THD *running_thd, THD *target_thd, enum_var_type type,
                      LEX_STRING *base);
  uchar *do_value_ptr(THD *thd, enum_var_type type, LEX_STRING *base) {
    return do_value_ptr(thd, thd, type, base);
  }
  const uchar *session_value_ptr(THD *running_thd, THD *target_thd,
                                 LEX_STRING *base) {
    return do_value_ptr(running_thd, target_thd, OPT_SESSION, base);
  }
  const uchar *global_value_ptr(THD *thd, LEX_STRING *base) {
    return do_value_ptr(thd, OPT_GLOBAL, base);
  }
  bool do_check(THD *thd, set_var *var);
  virtual void session_save_default(THD *, set_var *) {}
  virtual void saved_value_to_string(THD *thd, set_var *var, char *def_val);
  virtual void global_save_default(THD *, set_var *) {}
  bool session_update(THD *thd, set_var *var);
  bool global_update(THD *thd, set_var *var);
  longlong get_min_value();
  ulonglong get_max_value();
  void set_arg_source(get_opt_arg_source *src) {
    strcpy(source.m_path_name, src->m_path_name);
    source.m_source = src->m_source;
  }
  bool is_non_persistent() {
    return (plugin_var->flags & PLUGIN_VAR_NOPERSIST);
  }
  void set_is_plugin(bool val) { is_plugin = val; }
};

/*
  hidden part of opaque value passed to variable check functions.
  Used to provide a object-like structure to non C++ consumers.
*/
struct st_item_value_holder : public st_mysql_value {
  Item *item;
};

// Defined in sql_plugin.cc, but declared here since sql_plugin.h has so
// many users and would like not to include "map_helpers.h".
malloc_unordered_map<std::string, st_bookmark *> *get_bookmark_hash();

#endif
