#ifndef SQL_CONST_FOLDING_INCLUDED
#define SQL_CONST_FOLDING_INCLUDED

/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/sql_const_folding.h

  Interface to SQL expression constant folding
*/

#include "sql/item.h"  // Item, Item::cond_result
class THD;

/**
  Fold boolean condition {=, <>, >, >=, <, <=, <=>} involving constants and
  fields (or references to fields), possibly determining if the condition is
  always true or false based on type range of the field, or possibly simplifying
  >=, <= to = if constant lies on the range border.

  Also fold such conditions if they are present as arguments of other functions
  in the same way, except there we replace the arguments with manifest FALSE or
  field <> field to exclude NULLs if the field is nullable, cf.
  ignore_unknown().

  The constants are always converted to constants of the type of the field, no
  widening of the field type in therefore necessary after this folding for
  comparison to occur. When converting constants to decimal (when the field is a
  decimal), the constant will receive the same number of fractional decimals as
  the field. If decimal constant fraction truncation occurs, the comparison
  {GT,GE,LT,LE} logic is adjusted to preserve semantics without widening the
  field's type.

  @param      thd        session state
  @param      cond       the condition to handle
  @param[out] retcond    condition after const removal
  @param[out] cond_value resulting value of the condition
     - COND_OK:    condition must be evaluated (e.g int == 3)
     - COND_TRUE:  always true                 (e.g signed tinyint < 128)
     - COND_FALSE: always false                (e.g unsigned tinyint < 0)

  @param      manifest_result
                         For IS NOT NULL on a not nullable item, if true,
                         return item TRUE (1), else remove condition and return
                         COND_TRUE. For cmp items, this is determined by
                         Item_bool_func2::ignore_unknown.
  @returns false if success, true if error
*/
bool fold_condition(THD *thd, Item *cond, Item **retcond,
                    Item::cond_result *cond_value,
                    bool manifest_result = false);

#endif /* SQL_CONST_FOLDING_INCLUDED */
