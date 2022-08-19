#ifndef COMP_CREATOR_INCLUDED
#define COMP_CREATOR_INCLUDED

/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

// Declaration of chooser_compare_func_creator, in a separate file
// to make sure that parser_yystype.h does not need to depend on
// item_subselect.h.

class Comp_creator;

/**
  Convenience typedef for a function that returns factories for Item comparators
  (ie., returns Comp_creator).

  @retval nullptr In case of semantic errors.
*/
using chooser_compare_func_creator = Comp_creator *(*)(bool invert);

#endif  // COMP_CREATOR_INCLUDED
