/*
   Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef DD_ROUTINE_INCLUDED
#define DD_ROUTINE_INCLUDED

#include "sql/table.h"

class THD;
class sp_head;
struct st_sp_chistics;

struct LEX_USER;

namespace dd {
class Routine;
class Schema;

/**
  Prepares dd:Routine object from sp_head and updates DD tables
  accordingly.

  @param[in]  thd      Thread handle.
  @param[in]  schema   Schema to create the routine in.
  @param[in]  sp       Stored routine object to store.
  @param[in]  definer  Stored routine definer.

  @retval false      ON SUCCESS
  @retval true       ON FAILURE
*/

bool create_routine(THD *thd, const Schema &schema, sp_head *sp,
                    const LEX_USER *definer);

/**
  Alters routine characteristics in the DD table.

  @param[in]  thd       Thread handle.
  @param[in]  routine   Procedure or Function to alter.
  @param[in]  chistics  New values of stored routine attributes to write.

  @retval false      ON SUCCESS
  @retval true       ON FAILURE
*/

bool alter_routine(THD *thd, Routine *routine, st_sp_chistics *chistics);
}  // namespace dd
#endif  // DD_ROUTINE_INCLUDED
