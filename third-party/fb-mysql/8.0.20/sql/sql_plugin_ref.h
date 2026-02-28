/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_PLUGIN_REF_INCLUDED
#define SQL_PLUGIN_REF_INCLUDED

#include "lex_string.h"
#include "my_alloc.h"
#include "mysql/mysql_lex_string.h"
#include "prealloced_array.h"

class sys_var;
struct st_mysql_plugin;
struct st_plugin_dl;

enum enum_plugin_load_option {
  PLUGIN_OFF,
  PLUGIN_ON,
  PLUGIN_FORCE,
  PLUGIN_FORCE_PLUS_PERMANENT
};

/* A handle of a plugin */

struct st_plugin_int {
  LEX_CSTRING name{nullptr, 0};
  st_mysql_plugin *plugin{nullptr};
  st_plugin_dl *plugin_dl{nullptr};
  uint state{0};
  uint ref_count{0};             /* number of threads using the plugin */
  void *data{nullptr};           /* plugin type specific, e.g. handlerton */
  MEM_ROOT mem_root;             /* memory for dynamic plugin structures */
  sys_var *system_vars{nullptr}; /* server variables for this plugin */
  enum_plugin_load_option load_option{
      PLUGIN_OFF}; /* OFF, ON, FORCE, F+PERMANENT */
};

/*
  See intern_plugin_lock() for the explanation for the
  conditionally defined plugin_ref type
*/

#ifdef DBUG_OFF
typedef struct st_plugin_int *plugin_ref;

inline st_mysql_plugin *plugin_decl(st_plugin_int *ref) { return ref->plugin; }
inline st_plugin_dl *plugin_dlib(st_plugin_int *ref) { return ref->plugin_dl; }
template <typename T>
inline T plugin_data(st_plugin_int *ref) {
  return static_cast<T>(ref->data);
}
inline LEX_CSTRING *plugin_name(st_plugin_int *ref) { return &(ref->name); }
inline uint plugin_state(st_plugin_int *ref) { return ref->state; }
inline enum_plugin_load_option plugin_load_option(st_plugin_int *ref) {
  return ref->load_option;
}
inline bool plugin_equals(st_plugin_int *ref1, st_plugin_int *ref2) {
  return ref1 == ref2;
}

#else

typedef struct st_plugin_int **plugin_ref;

inline st_mysql_plugin *plugin_decl(st_plugin_int **ref) {
  return ref[0]->plugin;
}
inline st_plugin_dl *plugin_dlib(st_plugin_int **ref) {
  return ref[0]->plugin_dl;
}
template <typename T>
inline T plugin_data(st_plugin_int **ref) {
  return static_cast<T>(ref[0]->data);
}
inline LEX_CSTRING *plugin_name(st_plugin_int **ref) { return &(ref[0]->name); }
inline uint plugin_state(st_plugin_int **ref) { return ref[0]->state; }
inline enum_plugin_load_option plugin_load_option(st_plugin_int **ref) {
  return ref[0]->load_option;
}
inline bool plugin_equals(st_plugin_int **ref1, st_plugin_int **ref2) {
  return ref1 && ref2 && (ref1[0] == ref2[0]);
}
#endif

/**
  @class Plugin_array

  @brief Plugin array helper class.
*/
class Plugin_array : public Prealloced_array<plugin_ref, 2> {
 public:
  /**
    Class construction.

    @param psi_key PSI key.
  */
  explicit Plugin_array(PSI_memory_key psi_key)
      : Prealloced_array<plugin_ref, 2>(psi_key) {}

  /**
    Check, whether the plugin specified by the plugin argument has been
    already added into the array.

    @param plugin Plugin to check.

    @retval true  Plugin has been already added.
    @retval false There is no plugin in the array.
  */
  bool exists(plugin_ref plugin) {
    Plugin_array::iterator i;

    for (i = begin(); i != end(); ++i)
      if (plugin_equals(*i, plugin)) return true;

    return false;
  }
};

#endif  // SQL_PLUGIN_REF_INCLUDED
