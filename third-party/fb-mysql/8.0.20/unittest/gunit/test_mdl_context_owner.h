/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TEST_MDL_CONTEXT_OWNER_INCLUDED
#define TEST_MDL_CONTEXT_OWNER_INCLUDED

#include "sql/mdl.h"

class Test_MDL_context_owner : public MDL_context_owner {
 public:
  Test_MDL_context_owner() {}
  virtual void enter_cond(mysql_cond_t *, mysql_mutex_t *,
                          const PSI_stage_info *, PSI_stage_info *,
                          const char *, const char *, int) {}

  virtual void exit_cond(const PSI_stage_info *, const char *, const char *,
                         int) {}

  virtual int is_killed() const final { return 0; }
  virtual bool is_connected() { return true; }
  virtual THD *get_thd() { return nullptr; }

  virtual bool notify_hton_pre_acquire_exclusive(const MDL_key *, bool *) {
    return false;
  }
  virtual void notify_hton_post_release_exclusive(const MDL_key *) {}

  virtual uint get_rand_seed() const { return 0; }

  virtual bool kill_shared_lock(MDL_context_owner *) { return false; }
};

#endif  // TEST_MDL_CONTEXT_OWNER_INCLUDED
