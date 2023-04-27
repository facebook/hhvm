#ifndef DD_SP_INCLUDED
#define DD_SP_INCLUDED
/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/string_type.h"
#include "sql/dd/types/routine.h"  // dd::Routine

class THD;
struct st_sp_chistics;

/**
  Method to prepare sp_chistics object using the dd::Routine object read
  from the Data Dictionary.

  @param[in]  routine     Routine object read from the Data Dictionary.
  @param[out] sp_chistics st_sp_chistics type's object to be prepared from the
                          routine param.
*/

void prepare_sp_chistics_from_dd_routine(const dd::Routine *routine,
                                         st_sp_chistics *sp_chistics);

/**
  Helper method to prepare stored routine type in string format using the
  Routine object read from the Data Dictionary.

  @param[in]   thd              Thread handle.
  @param[in]   routine          dd::Routine type object read from the Data
                                Dictionary.
  @param[out]  return_type_str  Stored routine return type in string format.
*/

void prepare_return_type_string_from_dd_routine(
    THD *thd, const dd::Routine *routine, dd::String_type *return_type_str);

/**
  Method to prepare stored routine's parameter string using the Routine
  object read from the Data Dictionary.

  @param[in]   thd              Thread handle.
  @param[in]   routine          dd::Routine type object read from the Data
                                Dictionary.
  @param[out]  params_str       String prepared from the all the parameters of
                                stored routine.
*/

void prepare_params_string_from_dd_routine(THD *thd, const dd::Routine *routine,
                                           dd::String_type *params_str);

/**
  Method to check whether routine object is of stored function type
  or not.

  @param[in]  routine     Routine object read from the Data Dictionary.
*/

inline bool is_dd_routine_type_function(const dd::Routine *routine) {
  return (routine->type() == dd::Routine::RT_FUNCTION);
}
#endif  // DD_SP_INCLUDED
