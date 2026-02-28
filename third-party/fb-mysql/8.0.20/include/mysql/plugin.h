/* Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_plugin_h
#define _my_plugin_h

/**
  @file include/mysql/plugin.h
*/

#ifndef MYSQL_ABI_CHECK
#include <stddef.h>

#include "mysql_version.h" /* MYSQL_VERSION_ID */
#ifndef MYSQL_PLUGIN_STRUCT_DEFS_ONLY
#ifdef __cplusplus
#include "sql/sql_plugin.h"  // plugin_thdvar_safe_update
#endif
#endif
#endif

#include "status_var.h"

/*
  On Windows, exports from DLL need to be declared.
  Also, plugin needs to be declared as extern "C" because MSVC
  unlike other compilers, uses C++ mangling for variables not only
  for functions.
*/
#if defined(_MSC_VER)
#if defined(MYSQL_DYNAMIC_PLUGIN)
#ifdef __cplusplus
#define MYSQL_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define MYSQL_PLUGIN_EXPORT __declspec(dllexport)
#endif
#else /* MYSQL_DYNAMIC_PLUGIN */
#ifdef __cplusplus
#define MYSQL_PLUGIN_EXPORT extern "C"
#else
#define MYSQL_PLUGIN_EXPORT
#endif
#endif /*MYSQL_DYNAMIC_PLUGIN */
#else  /*_MSC_VER */
#define MYSQL_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
class THD;
class Item;
#define MYSQL_THD THD *
#else
#define MYSQL_THD void *
#endif

typedef void *MYSQL_PLUGIN;

#ifndef MYSQL_ABI_CHECK
#ifndef MYSQL_PLUGIN_STRUCT_DEFS_ONLY
#include <mysql/services.h>
#endif
#endif

#define MYSQL_XIDDATASIZE 128
/**
  MYSQL_XID is binary compatible with the XID structure as
  in the X/Open CAE Specification, Distributed Transaction Processing:
  The XA Specification, X/Open Company Ltd., 1991.
  http://www.opengroup.org/bookstore/catalog/c193.htm

  @see XID in sql/handler.h
*/
struct MYSQL_XID {
  long formatID;
  long gtrid_length;
  long bqual_length;
  char data[MYSQL_XIDDATASIZE]; /* Not \0-terminated */
};

/*************************************************************************
  Plugin API. Common for all plugin types.
*/

#define MYSQL_PLUGIN_INTERFACE_VERSION 0x010A

/*
  The allowable types of plugins
*/
#define MYSQL_UDF_PLUGIN 0                /* User-defined function        */
#define MYSQL_STORAGE_ENGINE_PLUGIN 1     /* Storage Engine               */
#define MYSQL_FTPARSER_PLUGIN 2           /* Full-text parser plugin      */
#define MYSQL_DAEMON_PLUGIN 3             /* The daemon/raw plugin type */
#define MYSQL_INFORMATION_SCHEMA_PLUGIN 4 /* The I_S plugin type */
#define MYSQL_AUDIT_PLUGIN 5              /* The Audit plugin type        */
#define MYSQL_REPLICATION_PLUGIN 6        /* The replication plugin type */
#define MYSQL_AUTHENTICATION_PLUGIN 7     /* The authentication plugin type */
#define MYSQL_VALIDATE_PASSWORD_PLUGIN 8  /* validate password plugin type */
#define MYSQL_GROUP_REPLICATION_PLUGIN 9  /* The Group Replication plugin */
#define MYSQL_KEYRING_PLUGIN 10           /* The Keyring plugin type   */
#define MYSQL_CLONE_PLUGIN 11             /* The Clone plugin type   */
#define MYSQL_MAX_PLUGIN_TYPE_NUM 12      /* The number of plugin types   */

/* We use the following strings to define licenses for plugins */
#define PLUGIN_LICENSE_PROPRIETARY 0
#define PLUGIN_LICENSE_GPL 1
#define PLUGIN_LICENSE_BSD 2

#define PLUGIN_LICENSE_PROPRIETARY_STRING "PROPRIETARY"
#define PLUGIN_LICENSE_GPL_STRING "GPL"
#define PLUGIN_LICENSE_BSD_STRING "BSD"

#define PLUGIN_AUTHOR_ORACLE "Oracle Corporation"

/*
  Macros for beginning and ending plugin declarations.  Between
  mysql_declare_plugin and mysql_declare_plugin_end there should
  be a st_mysql_plugin struct for each plugin to be declared.
*/

#ifndef MYSQL_DYNAMIC_PLUGIN
#define __MYSQL_DECLARE_PLUGIN(NAME, VERSION, PSIZE, DECLS)         \
  MYSQL_PLUGIN_EXPORT int VERSION = MYSQL_PLUGIN_INTERFACE_VERSION; \
  MYSQL_PLUGIN_EXPORT int PSIZE = sizeof(struct st_mysql_plugin);   \
  MYSQL_PLUGIN_EXPORT struct st_mysql_plugin DECLS[] = {
#else
#define __MYSQL_DECLARE_PLUGIN(NAME, VERSION, PSIZE, DECLS)  \
  MYSQL_PLUGIN_EXPORT int _mysql_plugin_interface_version_ = \
      MYSQL_PLUGIN_INTERFACE_VERSION;                        \
  MYSQL_PLUGIN_EXPORT int _mysql_sizeof_struct_st_plugin_ =  \
      sizeof(struct st_mysql_plugin);                        \
  MYSQL_PLUGIN_EXPORT struct st_mysql_plugin _mysql_plugin_declarations_[] = {
#endif

#define mysql_declare_plugin(NAME)                                        \
  __MYSQL_DECLARE_PLUGIN(NAME, builtin_##NAME##_plugin_interface_version, \
                         builtin_##NAME##_sizeof_struct_st_plugin,        \
                         builtin_##NAME##_plugin)

#define mysql_declare_plugin_end                 \
  , { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } \
  }

/*
  Constants for plugin flags.
 */

#define PLUGIN_OPT_NO_INSTALL 1UL   /* Not dynamically loadable */
#define PLUGIN_OPT_NO_UNINSTALL 2UL /* Not dynamically unloadable */
#define PLUGIN_OPT_ALLOW_EARLY 4UL  /* allow --early-plugin-load */

/*
  declarations for server variables and command line options
*/

#define PLUGIN_VAR_BOOL 0x0001
#define PLUGIN_VAR_INT 0x0002
#define PLUGIN_VAR_LONG 0x0003
#define PLUGIN_VAR_LONGLONG 0x0004
#define PLUGIN_VAR_STR 0x0005
#define PLUGIN_VAR_ENUM 0x0006
#define PLUGIN_VAR_SET 0x0007
#define PLUGIN_VAR_DOUBLE 0x0008
#define PLUGIN_VAR_UNSIGNED 0x0080
#define PLUGIN_VAR_THDLOCAL 0x0100 /* Variable is per-connection */
#define PLUGIN_VAR_READONLY 0x0200 /* Server variable is read only */
#define PLUGIN_VAR_NOSYSVAR 0x0400 /* Configurable only by cmd-line */

/**
  plugin variable CAN'T be used through command line at all
  neither "--option", nor "--option=value" will work
  @note you should probably set a default variable value if you use this flag
*/
#define PLUGIN_VAR_NOCMDOPT 0x0800

/**
  plugin variable *value* CAN'T be set via command line
  you can invoke it with "--option" only, but "--option=value" will not work
  @note you should probably set a default variable value if you use this flag
*/
#define PLUGIN_VAR_NOCMDARG 0x1000

/**
  plugin variable CAN'T be used through command line without a value
  "--option=value" must be used, only "--option" won't work
*/
#define PLUGIN_VAR_RQCMDARG 0x0000

/**
  plugin variable can be set via command line, both with or without value
  either "--option=value", or "--option" will work
  @note you should probably set a default variable value if you use this flag
*/
#define PLUGIN_VAR_OPCMDARG 0x2000
#define PLUGIN_VAR_NODEFAULT 0x4000 /* SET DEFAULT is prohibited */
#define PLUGIN_VAR_MEMALLOC 0x8000  /* String needs memory allocated */
#define PLUGIN_VAR_NOPERSIST                \
  0x10000 /* SET PERSIST_ONLY is prohibited \
             for read only variables */

/**
  There can be some variables which needs to be set before plugin is loaded but
  not after plugin is loaded. ex: GR specific variables. Below flag must be set
  for these kind of variables.
*/
#define PLUGIN_VAR_PERSIST_AS_READ_ONLY 0x20000
#define PLUGIN_VAR_INVISIBLE 0x40000 /* Variable should not be shown */

struct SYS_VAR;
struct st_mysql_value;

/*
  SYNOPSIS
    (*mysql_var_check_func)()
      thd               thread handle
      var               dynamic variable being altered
      save              pointer to temporary storage
      value             user provided value
  RETURN
    0   user provided value is OK and the update func may be called.
    any other value indicates error.

  This function should parse the user provided value and store in the
  provided temporary storage any data as required by the update func.
  There is sufficient space in the temporary storage to store a double.
  Note that the update func may not be called if any other error occurs
  so any memory allocated should be thread-local so that it may be freed
  automatically at the end of the statement.
*/

typedef int (*mysql_var_check_func)(MYSQL_THD thd, SYS_VAR *var, void *save,
                                    struct st_mysql_value *value);

/*
  SYNOPSIS
    (*mysql_var_update_func)()
      thd               thread handle
      var               dynamic variable being altered
      var_ptr           pointer to dynamic variable
      save              pointer to temporary storage
   RETURN
     NONE

   This function should use the validated value stored in the temporary store
   and persist it in the provided pointer to the dynamic variable.
   For example, strings may require memory to be allocated.
*/
typedef void (*mysql_var_update_func)(MYSQL_THD thd, SYS_VAR *var,
                                      void *var_ptr, const void *save);

/* the following declarations are for internal use only */

#define PLUGIN_VAR_MASK                                                \
  (PLUGIN_VAR_READONLY | PLUGIN_VAR_NOSYSVAR | PLUGIN_VAR_NOCMDOPT |   \
   PLUGIN_VAR_NOCMDARG | PLUGIN_VAR_OPCMDARG | PLUGIN_VAR_RQCMDARG |   \
   PLUGIN_VAR_MEMALLOC | PLUGIN_VAR_NODEFAULT | PLUGIN_VAR_NOPERSIST | \
   PLUGIN_VAR_PERSIST_AS_READ_ONLY | PLUGIN_VAR_INVISIBLE)

#define MYSQL_PLUGIN_VAR_HEADER \
  int flags;                    \
  const char *name;             \
  const char *comment;          \
  mysql_var_check_func check;   \
  mysql_var_update_func update

#define MYSQL_SYSVAR_NAME(name) mysql_sysvar_##name
#define MYSQL_SYSVAR(name) ((SYS_VAR *)&(MYSQL_SYSVAR_NAME(name)))

/*
  for global variables, the value pointer is the first
  element after the header, the default value is the second.
  for thread variables, the value offset is the first
  element after the header, the default value is the second.
*/

#define DECLARE_MYSQL_SYSVAR_BASIC(name, type) \
  struct {                                     \
    MYSQL_PLUGIN_VAR_HEADER;                   \
    type *value;                               \
    const type def_val;                        \
  } MYSQL_SYSVAR_NAME(name)

#define DECLARE_MYSQL_SYSVAR_SIMPLE(name, type) \
  struct {                                      \
    MYSQL_PLUGIN_VAR_HEADER;                    \
    type *value;                                \
    type def_val;                               \
    type min_val;                               \
    type max_val;                               \
    type blk_sz;                                \
  } MYSQL_SYSVAR_NAME(name)

#define DECLARE_MYSQL_SYSVAR_TYPELIB(name, type) \
  struct {                                       \
    MYSQL_PLUGIN_VAR_HEADER;                     \
    type *value;                                 \
    type def_val;                                \
    TYPELIB *typelib;                            \
  } MYSQL_SYSVAR_NAME(name)

#define DECLARE_THDVAR_FUNC(type) type *(*resolve)(MYSQL_THD thd, int offset)

#define DECLARE_MYSQL_THDVAR_BASIC(name, type) \
  struct {                                     \
    MYSQL_PLUGIN_VAR_HEADER;                   \
    int offset;                                \
    const type def_val;                        \
    DECLARE_THDVAR_FUNC(type);                 \
  } MYSQL_SYSVAR_NAME(name)

#define DECLARE_MYSQL_THDVAR_SIMPLE(name, type) \
  struct {                                      \
    MYSQL_PLUGIN_VAR_HEADER;                    \
    int offset;                                 \
    type def_val;                               \
    type min_val;                               \
    type max_val;                               \
    type blk_sz;                                \
    DECLARE_THDVAR_FUNC(type);                  \
  } MYSQL_SYSVAR_NAME(name)

#define DECLARE_MYSQL_THDVAR_TYPELIB(name, type) \
  struct {                                       \
    MYSQL_PLUGIN_VAR_HEADER;                     \
    int offset;                                  \
    type def_val;                                \
    DECLARE_THDVAR_FUNC(type);                   \
    TYPELIB *typelib;                            \
  } MYSQL_SYSVAR_NAME(name)

/*
  the following declarations are for use by plugin implementors
*/

#define MYSQL_SYSVAR_BOOL(name, varname, opt, comment, check, update, def) \
  DECLARE_MYSQL_SYSVAR_BASIC(name, bool) = {                               \
      PLUGIN_VAR_BOOL | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      &varname,                                                            \
      def}

#define MYSQL_SYSVAR_STR(name, varname, opt, comment, check, update, def) \
  DECLARE_MYSQL_SYSVAR_BASIC(name, char *) = {                            \
      PLUGIN_VAR_STR | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                              \
      comment,                                                            \
      check,                                                              \
      update,                                                             \
      &varname,                                                           \
      def}

#define MYSQL_SYSVAR_INT(name, varname, opt, comment, check, update, def, min, \
                         max, blk)                                             \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, int) = {                                   \
      PLUGIN_VAR_INT | ((opt)&PLUGIN_VAR_MASK),                                \
      #name,                                                                   \
      comment,                                                                 \
      check,                                                                   \
      update,                                                                  \
      &varname,                                                                \
      def,                                                                     \
      min,                                                                     \
      max,                                                                     \
      blk}

#define MYSQL_SYSVAR_UINT(name, varname, opt, comment, check, update, def, \
                          min, max, blk)                                   \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, unsigned int) = {                      \
      PLUGIN_VAR_INT | PLUGIN_VAR_UNSIGNED | ((opt)&PLUGIN_VAR_MASK),      \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      &varname,                                                            \
      def,                                                                 \
      min,                                                                 \
      max,                                                                 \
      blk}

#define MYSQL_SYSVAR_LONG(name, varname, opt, comment, check, update, def, \
                          min, max, blk)                                   \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, long) = {                              \
      PLUGIN_VAR_LONG | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      &varname,                                                            \
      def,                                                                 \
      min,                                                                 \
      max,                                                                 \
      blk}

#define MYSQL_SYSVAR_ULONG(name, varname, opt, comment, check, update, def, \
                           min, max, blk)                                   \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, unsigned long) = {                      \
      PLUGIN_VAR_LONG | PLUGIN_VAR_UNSIGNED | ((opt)&PLUGIN_VAR_MASK),      \
      #name,                                                                \
      comment,                                                              \
      check,                                                                \
      update,                                                               \
      &varname,                                                             \
      def,                                                                  \
      min,                                                                  \
      max,                                                                  \
      blk}

#define MYSQL_SYSVAR_LONGLONG(name, varname, opt, comment, check, update, def, \
                              min, max, blk)                                   \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, long long) = {                             \
      PLUGIN_VAR_LONGLONG | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                                   \
      comment,                                                                 \
      check,                                                                   \
      update,                                                                  \
      &varname,                                                                \
      def,                                                                     \
      min,                                                                     \
      max,                                                                     \
      blk}

#define MYSQL_SYSVAR_ULONGLONG(name, varname, opt, comment, check, update, \
                               def, min, max, blk)                         \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, unsigned long long) = {                \
      PLUGIN_VAR_LONGLONG | PLUGIN_VAR_UNSIGNED | ((opt)&PLUGIN_VAR_MASK), \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      &varname,                                                            \
      def,                                                                 \
      min,                                                                 \
      max,                                                                 \
      blk}

#define MYSQL_SYSVAR_ENUM(name, varname, opt, comment, check, update, def, \
                          typelib)                                         \
  DECLARE_MYSQL_SYSVAR_TYPELIB(name, unsigned long) = {                    \
      PLUGIN_VAR_ENUM | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      &varname,                                                            \
      def,                                                                 \
      typelib}

#define MYSQL_SYSVAR_SET(name, varname, opt, comment, check, update, def, \
                         typelib)                                         \
  DECLARE_MYSQL_SYSVAR_TYPELIB(name, unsigned long long) = {              \
      PLUGIN_VAR_SET | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                              \
      comment,                                                            \
      check,                                                              \
      update,                                                             \
      &varname,                                                           \
      def,                                                                \
      typelib}

#define MYSQL_SYSVAR_DOUBLE(name, varname, opt, comment, check, update, def, \
                            min, max, blk)                                   \
  DECLARE_MYSQL_SYSVAR_SIMPLE(name, double) = {                              \
      PLUGIN_VAR_DOUBLE | ((opt)&PLUGIN_VAR_MASK),                           \
      #name,                                                                 \
      comment,                                                               \
      check,                                                                 \
      update,                                                                \
      &varname,                                                              \
      def,                                                                   \
      min,                                                                   \
      max,                                                                   \
      blk}

#define MYSQL_THDVAR_BOOL(name, opt, comment, check, update, def)      \
  DECLARE_MYSQL_THDVAR_BASIC(name, bool) = {                           \
      PLUGIN_VAR_BOOL | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK), \
      #name,                                                           \
      comment,                                                         \
      check,                                                           \
      update,                                                          \
      -1,                                                              \
      def,                                                             \
      NULL}

#define MYSQL_THDVAR_STR(name, opt, comment, check, update, def)      \
  DECLARE_MYSQL_THDVAR_BASIC(name, char *) = {                        \
      PLUGIN_VAR_STR | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK), \
      #name,                                                          \
      comment,                                                        \
      check,                                                          \
      update,                                                         \
      -1,                                                             \
      def,                                                            \
      NULL}

#define MYSQL_THDVAR_INT(name, opt, comment, check, update, def, min, max, \
                         blk)                                              \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, int) = {                               \
      PLUGIN_VAR_INT | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK),      \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      -1,                                                                  \
      def,                                                                 \
      min,                                                                 \
      max,                                                                 \
      blk,                                                                 \
      NULL}

#define MYSQL_THDVAR_UINT(name, opt, comment, check, update, def, min, max, \
                          blk)                                              \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, unsigned int) = {                       \
      PLUGIN_VAR_INT | PLUGIN_VAR_THDLOCAL | PLUGIN_VAR_UNSIGNED |          \
          ((opt)&PLUGIN_VAR_MASK),                                          \
      #name,                                                                \
      comment,                                                              \
      check,                                                                \
      update,                                                               \
      -1,                                                                   \
      def,                                                                  \
      min,                                                                  \
      max,                                                                  \
      blk,                                                                  \
      NULL}

#define MYSQL_THDVAR_LONG(name, opt, comment, check, update, def, min, max, \
                          blk)                                              \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, long) = {                               \
      PLUGIN_VAR_LONG | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK),      \
      #name,                                                                \
      comment,                                                              \
      check,                                                                \
      update,                                                               \
      -1,                                                                   \
      def,                                                                  \
      min,                                                                  \
      max,                                                                  \
      blk,                                                                  \
      NULL}

#define MYSQL_THDVAR_ULONG(name, opt, comment, check, update, def, min, max, \
                           blk)                                              \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, unsigned long) = {                       \
      PLUGIN_VAR_LONG | PLUGIN_VAR_THDLOCAL | PLUGIN_VAR_UNSIGNED |          \
          ((opt)&PLUGIN_VAR_MASK),                                           \
      #name,                                                                 \
      comment,                                                               \
      check,                                                                 \
      update,                                                                \
      -1,                                                                    \
      def,                                                                   \
      min,                                                                   \
      max,                                                                   \
      blk,                                                                   \
      NULL}

#define MYSQL_THDVAR_LONGLONG(name, opt, comment, check, update, def, min, \
                              max, blk)                                    \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, long long) = {                         \
      PLUGIN_VAR_LONGLONG | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK), \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      -1,                                                                  \
      def,                                                                 \
      min,                                                                 \
      max,                                                                 \
      blk,                                                                 \
      NULL}

#define MYSQL_THDVAR_ULONGLONG(name, opt, comment, check, update, def, min, \
                               max, blk)                                    \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, unsigned long long) = {                 \
      PLUGIN_VAR_LONGLONG | PLUGIN_VAR_THDLOCAL | PLUGIN_VAR_UNSIGNED |     \
          ((opt)&PLUGIN_VAR_MASK),                                          \
      #name,                                                                \
      comment,                                                              \
      check,                                                                \
      update,                                                               \
      -1,                                                                   \
      def,                                                                  \
      min,                                                                  \
      max,                                                                  \
      blk,                                                                  \
      NULL}

#define MYSQL_THDVAR_ENUM(name, opt, comment, check, update, def, typelib) \
  DECLARE_MYSQL_THDVAR_TYPELIB(name, unsigned long) = {                    \
      PLUGIN_VAR_ENUM | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK),     \
      #name,                                                               \
      comment,                                                             \
      check,                                                               \
      update,                                                              \
      -1,                                                                  \
      def,                                                                 \
      NULL,                                                                \
      typelib}

#define MYSQL_THDVAR_SET(name, opt, comment, check, update, def, typelib) \
  DECLARE_MYSQL_THDVAR_TYPELIB(name, unsigned long long) = {              \
      PLUGIN_VAR_SET | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK),     \
      #name,                                                              \
      comment,                                                            \
      check,                                                              \
      update,                                                             \
      -1,                                                                 \
      def,                                                                \
      NULL,                                                               \
      typelib}

#define MYSQL_THDVAR_DOUBLE(name, opt, comment, check, update, def, min, max, \
                            blk)                                              \
  DECLARE_MYSQL_THDVAR_SIMPLE(name, double) = {                               \
      PLUGIN_VAR_DOUBLE | PLUGIN_VAR_THDLOCAL | ((opt)&PLUGIN_VAR_MASK),      \
      #name,                                                                  \
      comment,                                                                \
      check,                                                                  \
      update,                                                                 \
      -1,                                                                     \
      def,                                                                    \
      min,                                                                    \
      max,                                                                    \
      blk,                                                                    \
      NULL}

/* accessor macros */

#define SYSVAR(name) (*(MYSQL_SYSVAR_NAME(name).value))

/* when thd == null, result points to global value */
#define THDVAR(thd, name) \
  (*(MYSQL_SYSVAR_NAME(name).resolve(thd, MYSQL_SYSVAR_NAME(name).offset)))

#define THDVAR_SET(thd, name, value)                 \
  plugin_thdvar_safe_update(thd, MYSQL_SYSVAR(name), \
                            (char **)&THDVAR(thd, name), (const char *)value);

/*
  Plugin description structure.
*/

struct st_mysql_plugin {
  int type;           /* the plugin type (a MYSQL_XXX_PLUGIN value)   */
  void *info;         /* pointer to type-specific plugin descriptor   */
  const char *name;   /* plugin name                                  */
  const char *author; /* plugin author (for I_S.PLUGINS)              */
  const char *descr;  /* general descriptive text (for I_S.PLUGINS)   */
  int license;        /* the plugin license (PLUGIN_LICENSE_XXX)      */
  /** Function to invoke when plugin is loaded. */
  int (*init)(MYSQL_PLUGIN);
  /** Function to invoke when plugin is uninstalled. */
  int (*check_uninstall)(MYSQL_PLUGIN);
  /** Function to invoke when plugin is unloaded. */
  int (*deinit)(MYSQL_PLUGIN);
  unsigned int version; /* plugin version (for I_S.PLUGINS)             */
  SHOW_VAR *status_vars;
  SYS_VAR **system_vars;
  void *__reserved1;   /* reserved for dependency checking             */
  unsigned long flags; /* flags for plugin */
};

/*************************************************************************
  API for Full-text parser plugin. (MYSQL_FTPARSER_PLUGIN)
*/
#define MYSQL_FTPARSER_INTERFACE_VERSION 0x0101

/*************************************************************************
  API for Query Rewrite plugin. (MYSQL_QUERY_REWRITE_PLUGIN)
*/

#define MYSQL_REWRITE_PRE_PARSE_INTERFACE_VERSION 0x0010
#define MYSQL_REWRITE_POST_PARSE_INTERFACE_VERSION 0x0010

/*************************************************************************
  API for Storage Engine plugin. (MYSQL_DAEMON_PLUGIN)
*/

/* handlertons of different MySQL releases are incompatible */
#define MYSQL_DAEMON_INTERFACE_VERSION (MYSQL_VERSION_ID << 8)

/*
  Here we define only the descriptor structure, that is referred from
  st_mysql_plugin.
*/

struct st_mysql_daemon {
  int interface_version;
};

/*************************************************************************
  API for I_S plugin. (MYSQL_INFORMATION_SCHEMA_PLUGIN)
*/

/* handlertons of different MySQL releases are incompatible */
#define MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION (MYSQL_VERSION_ID << 8)

/*
  Here we define only the descriptor structure, that is referred from
  st_mysql_plugin.
*/

struct st_mysql_information_schema {
  int interface_version;
};

/*************************************************************************
  API for Storage Engine plugin. (MYSQL_STORAGE_ENGINE_PLUGIN)
*/

/* handlertons of different MySQL releases are incompatible */
#define MYSQL_HANDLERTON_INTERFACE_VERSION (MYSQL_VERSION_ID << 8)

/*
  The real API is in the sql/handler.h
  Here we define only the descriptor structure, that is referred from
  st_mysql_plugin.
*/

struct st_mysql_storage_engine {
  int interface_version;
};

struct handlerton;

/*
  API for Replication plugin. (MYSQL_REPLICATION_PLUGIN)
*/
#define MYSQL_REPLICATION_INTERFACE_VERSION 0x0400

/**
   Replication plugin descriptor
*/
struct Mysql_replication {
  int interface_version;
};

/*************************************************************************
  st_mysql_value struct for reading values from mysqld.
  Used by server variables framework to parse user-provided values.
  Will be used for arguments when implementing UDFs.

  Note that val_str() returns a string in temporary memory
  that will be freed at the end of statement. Copy the string
  if you need it to persist.
*/

#define MYSQL_VALUE_TYPE_STRING 0
#define MYSQL_VALUE_TYPE_REAL 1
#define MYSQL_VALUE_TYPE_INT 2

struct st_mysql_value {
  int (*value_type)(struct st_mysql_value *);
  const char *(*val_str)(struct st_mysql_value *, char *buffer, int *length);
  int (*val_real)(struct st_mysql_value *, double *realbuf);
  int (*val_int)(struct st_mysql_value *, long long *intbuf);
  int (*is_unsigned)(struct st_mysql_value *);
};

struct snapshot_info_st;

/*************************************************************************
  Miscellaneous functions for plugin implementors
*/

#define thd_proc_info(thd, msg) \
  set_thd_proc_info(thd, msg, __func__, __FILE__, __LINE__)

#ifdef __cplusplus
extern "C" {
#endif

int thd_in_lock_tables(const MYSQL_THD thd);
int thd_tablespace_op(const MYSQL_THD thd);
long long thd_test_options(const MYSQL_THD thd, long long test_options);
int thd_sql_command(const MYSQL_THD thd);
const char *set_thd_proc_info(MYSQL_THD thd, const char *info,
                              const char *calling_func,
                              const char *calling_file,
                              const unsigned int calling_line);
void **thd_ha_data(const MYSQL_THD thd, const struct handlerton *hton);
void thd_storage_lock_wait(MYSQL_THD thd, long long value);
int thd_tx_isolation(const MYSQL_THD thd);
int thd_tx_is_read_only(const MYSQL_THD thd);
MYSQL_THD thd_tx_arbitrate(MYSQL_THD requestor, MYSQL_THD holder);
int thd_tx_priority(const MYSQL_THD thd);
int thd_tx_is_dd_trx(const MYSQL_THD thd);
char *thd_security_context(MYSQL_THD thd, char *buffer, size_t length,
                           size_t max_query_len);
/* Increments the row counter, see THD::row_count */
void thd_inc_row_count(MYSQL_THD thd);
int thd_allow_batch(MYSQL_THD thd);

/**
  Mark transaction to rollback and mark error as fatal to a
  sub-statement if in sub statement mode.

  @param thd  user thread connection handle
  @param all  if all != 0, rollback the main transaction
*/

void thd_mark_transaction_to_rollback(MYSQL_THD thd, int all);

/**
  Create a temporary file.

  @details
  The temporary file is created in a location specified by the mysql
  server configuration (--tmpdir option).  The caller does not need to
  delete the file, it will be deleted automatically.

  @param prefix  prefix for temporary file name
  @retval -1    error
  @retval >= 0  a file handle that can be passed to dup or my_close
*/
int mysql_tmpfile(const char *prefix);

/**
  Check the killed state of a connection

  @details
  In MySQL support for the KILL statement is cooperative. The KILL
  statement only sets a "killed" flag. This function returns the value
  of that flag.  A thread should check it often, especially inside
  time-consuming loops, and gracefully abort the operation if it is
  non-zero.

  @param v_thd  user thread connection handle
  @retval 0  the connection is active
  @retval 1  the connection has been killed
*/
int thd_killed(const void *v_thd);

/**
  Set the killed status of the current statement.

  @param thd  user thread connection handle
*/
void thd_set_kill_status(const MYSQL_THD thd);

/**
  Get binary log position for latest written entry.

  @note The file variable will be set to a buffer holding the name of
  the file name currently, but this can change if a rotation
  occur. Copy the string if you want to retain it.

  @param thd Use thread connection handle
  @param file_var Pointer to variable that will hold the file name.
  @param pos_var Pointer to variable that will hold the file position.
  @param gtid_var Pointer to variable that will hold the GTID.
 */
void thd_binlog_pos(const MYSQL_THD thd, const char **file_var,
                    unsigned long long *pos_var, const char **gtid_var,
                    const char **max_gtid_var);

/**
  Return the thread id of a user thread

  @param thd  user thread connection handle
  @return  thread id
*/
unsigned long thd_get_thread_id(const MYSQL_THD thd);

/**
  Get the XID for this connection's transaction

  @param thd  user thread connection handle
  @param xid  location where identifier is stored
*/
void thd_get_xid(const MYSQL_THD thd, MYSQL_XID *xid);

/**
  Provide a handler data getter to simplify coding
*/
void *thd_get_ha_data(const MYSQL_THD thd, const struct handlerton *hton);

/**
  Provide a handler data setter to simplify coding

  @details
  Set ha_data pointer (storage engine per-connection information).

  To avoid unclean deactivation (uninstall) of storage engine plugin
  in the middle of transaction, additional storage engine plugin
  lock is acquired.

  If ha_data is not null and storage engine plugin was not locked
  by thd_set_ha_data() in this connection before, storage engine
  plugin gets locked.

  If ha_data is null and storage engine plugin was locked by
  thd_set_ha_data() in this connection before, storage engine
  plugin lock gets released.

  If handlerton::close_connection() didn't reset ha_data, server does
  it immediately after calling handlerton::close_connection().
*/
void thd_set_ha_data(MYSQL_THD thd, const struct handlerton *hton,
                     const void *ha_data);

/**
  Interface to remove the per thread openssl error queue.
  This function is a no-op when openssl is not used.
*/

void remove_ssl_err_thread_state();

/**
  Interface to get the number of VCPUs.
*/
unsigned int thd_get_num_vcpus();

/**
  Determine if the bin log is open.
*/
char mysql_bin_log_is_open(void);

/**
  Block MYSQL_BIN_LOG::ordered_commit

  @details
  Acquires the necessary mysql_bin_log locks in order to block commits so that
  START TRANSACTION WITH CONSISTENT INNODB SNAPSHOT can return the correct
  bin log filename and pos.
  @param  ss_info         Snapshot context containing binlog pos, executed gtid,
                          snapshot id etc.
*/
void mysql_bin_log_lock_commits(struct snapshot_info_st *ss_info);

/**
  Unblock MYSQL_BIN_LOG::ordered_commit

  @details
  Releases the mysql_bin_log locks which blocked commits so that
  START TRANSACTION WITH CONSISTENT INNODB SNAPSHOT can return the correct
  bin log filename and pos.
  @param  ss_info         Snapshot context containing binlog pos, executed gtid,
                          snapshot id etc. which was populated by
                          @mysql_bin_log_lock_commits(). Used for asserting that
                          nothing has been committed.
*/
void mysql_bin_log_unlock_commits(struct snapshot_info_st *ss_info);

#ifdef __cplusplus
}
#endif

#endif /* _my_plugin_h */
