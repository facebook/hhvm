#ifndef SYS_VARS_RESOURCE_MGR_INCLUDED
#define SYS_VARS_RESOURCE_MGR_INCLUDED

/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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
  Session_sysvar_resource_manager
  -------------------------------
  When a session (THD) gets initialized, it receives a shallow copy of all
  global system variables.
  thd->variables= global_system_variables; (see plugin_thdvar_init())

  In case of Sys_var_charptr variables, we need to maintain a separate copy for
  each session though so that global and session variables can be altered
  independently.

  This class is responsible for alloc|dealloc-ating memory for Sys_var_charptr
  variables for every session. It works in three steps :

  (1) init :
        Creates a copy (memdup()) of global Sys_var_charptr system variable for
        the respective session variable (passed as a  parameter) & inserts it
        into sysvar_string_alloc_hash (containing the alloced address) to infer
        that memory has been allocated for the session. init() is called during
        the initialization of session system variables. (plugin_thdvar_init())
  (2) update :
        When the session variable is updated, the old memory is freed and new
        memory is allocated to hold the new value. The corresponding member in
        sysvar_string_alloc_hash is also updated to hold the new alloced memory
        address. (Sys_var_charptr::session_update())
  (3) deinit :
        Its a one-shot operation to free all the session Sys_var_charptr system
        variables. It basically traverses down the sysvar_string_alloc_hash
        hash and calls free() for all the addresses that it holds.

  Note, there should always be at most one node per Sys_var_charptr session
  system variable.

*/

#include <stddef.h>
#include <memory>

#include "map_helpers.h"
#include "sql/psi_memory_key.h"

class Session_sysvar_resource_manager {
 private:
  // The value always contains the string that the key points to.
  malloc_unordered_map<char **, unique_ptr_my_free<char>>
      m_sysvar_string_alloc_hash{
          key_memory_THD_Session_sysvar_resource_manager};

 public:
  /**
    Allocates memory for Sys_var_charptr session variable during session
    initialization.
  */
  bool init(char **var);

  /**
    Frees the old alloced memory, memdup()'s the given val to a new memory
    address & updated the session variable pointer.
  */
  bool update(char **var, char *val, size_t val_len);

  void claim_memory_ownership();

  /**
    Frees the memory allocated for Sys_var_charptr session variables.
  */
  void deinit();
};

#endif /* SYS_VARS_RESOURCE_MGR_INCLUDED */
