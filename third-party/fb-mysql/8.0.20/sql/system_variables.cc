/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/system_variables.h"

/*
  Add all status variables to another status variable array

  SYNOPSIS
   add_to_status()
   to_var       add to this array
   from_var     from this array

  NOTES
    This function assumes that all variables are longlong/ulonglong.
    If this assumption will change, then we have to explictely add
    the other variables after the while loop
*/

void add_to_status(System_status_var *to_var, System_status_var *from_var) {
  int c;
  ulonglong *end = (ulonglong *)((uchar *)to_var +
                                 offsetof(System_status_var, LAST_STATUS_VAR) +
                                 sizeof(ulonglong));
  ulonglong *to = (ulonglong *)to_var, *from = (ulonglong *)from_var;

  while (to != end) *(to++) += *(from++);

  to_var->com_other += from_var->com_other;

  for (c = 0; c < SQLCOM_END; c++)
    to_var->com_stat[(uint)c] += from_var->com_stat[(uint)c];
}

/*
  Add the difference between two status variable arrays to another one.

  SYNOPSIS
    add_diff_to_status
    to_var       add to this array
    from_var     from this array
    dec_var      minus this array

  NOTE
    This function assumes that all variables are longlong/ulonglong.
*/

void add_diff_to_status(System_status_var *to_var, System_status_var *from_var,
                        System_status_var *dec_var) {
  int c;
  ulonglong *end = (ulonglong *)((uchar *)to_var +
                                 offsetof(System_status_var, LAST_STATUS_VAR) +
                                 sizeof(ulonglong));
  ulonglong *to = (ulonglong *)to_var, *from = (ulonglong *)from_var,
            *dec = (ulonglong *)dec_var;

  while (to != end) *(to++) += *(from++) - *(dec++);

  to_var->com_other += from_var->com_other - dec_var->com_other;

  for (c = 0; c < SQLCOM_END; c++)
    to_var->com_stat[(uint)c] +=
        from_var->com_stat[(uint)c] - dec_var->com_stat[(uint)c];
}

/*
  Reset a block of status variables.

  SYNOPSIS
    reset_system_status_vars
    status_vars    Struct of status variables to reset
*/
void reset_system_status_vars(System_status_var *status_vars) {
  memset(status_vars, 0, sizeof(*status_vars));
}
