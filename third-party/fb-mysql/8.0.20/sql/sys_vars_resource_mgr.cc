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

#include "sql/sys_vars_resource_mgr.h"

#include <unordered_map>
#include <utility>

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/psi_memory_key.h"
#include "template_utils.h"

/**
  Allocates memory for Sys_var_charptr session variable during session
  initialization.

  @param var         The variable.

  @return
  Success - false
  Failure - true
*/

bool Session_sysvar_resource_manager::init(char **var) {
  if (*var) {
    char *ptr = my_strdup(key_memory_THD_Session_sysvar_resource_manager, *var,
                          MYF(MY_WME));
    if (ptr == nullptr) return true; /* Error */
    m_sysvar_string_alloc_hash.emplace(var, unique_ptr_my_free<char>(ptr));

    /* Update the variable to point to the newly alloced copy. */
    *var = ptr;
  }
  return false;
}

/**
  Frees the old alloced memory, memdup()'s the given val to a new memory
  address & updated the session variable pointer.

  @param var         The variable.
  @param val         The new value.
  @param val_len     Length of the new value.

  @return
  Success - false
  Failure - true
*/

bool Session_sysvar_resource_manager::update(char **var, char *val,
                                             size_t val_len) {
  char *ptr = nullptr;

  // Memory allocation for the new value of the variable.
  if (val) {
    ptr = pointer_cast<char *>(
        my_memdup(PSI_NOT_INSTRUMENTED, val, val_len + 1, MYF(MY_WME)));
    if (ptr == nullptr) return true;
    ptr[val_len] = 0;
  }

  if (ptr == nullptr)
    m_sysvar_string_alloc_hash.erase(var);
  else
    m_sysvar_string_alloc_hash[var].reset(ptr);

  /*
    Update the variable to point to the newly alloced copy.

    If current value and the new value are both nullptr,
    this function effectively does nothing.
  */
  *var = ptr;
  return false;
}

void Session_sysvar_resource_manager::claim_memory_ownership() {
  /* Release Sys_var_charptr resources here. */
  for (const auto &key_and_value : m_sysvar_string_alloc_hash) {
    my_claim(key_and_value.second.get());
  }
}

/**
  @brief Frees the memory allocated for Sys_var_charptr session variables.
*/

void Session_sysvar_resource_manager::deinit() {
  m_sysvar_string_alloc_hash.clear();
}
