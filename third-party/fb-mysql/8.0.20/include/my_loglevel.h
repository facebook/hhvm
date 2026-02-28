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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MY_LOGLEVEL_H
#define MY_LOGLEVEL_H

/**
  @file include/my_loglevel.h
  Definition of the global "loglevel" enumeration.
*/

enum loglevel {
  SYSTEM_LEVEL = 0,
  ERROR_LEVEL = 1,
  WARNING_LEVEL = 2,
  INFORMATION_LEVEL = 3,
  DEBUG_LEVEL = 4
};

#endif  // MY_LOGLEVEL_H
