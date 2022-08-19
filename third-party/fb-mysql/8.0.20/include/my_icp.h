/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_icp_h
#define _my_icp_h

/**
  @file include/my_icp.h
*/

/**
  Values returned by index_cond_func_xxx functions.
*/

typedef enum icp_result {
  /** Index tuple doesn't satisfy the pushed index condition (the engine
  should discard the tuple and go to the next one) */
  ICP_NO_MATCH,

  /** Index tuple satisfies the pushed index condition (the engine should
  fetch and return the record) */
  ICP_MATCH,

  /** Index tuple is out of the range that we're scanning, e.g. if we're
  scanning "t.key BETWEEN 10 AND 20" and got a "t.key=21" tuple (the engine
  should stop scanning and return HA_ERR_END_OF_FILE right away). */
  ICP_OUT_OF_RANGE

} ICP_RESULT;

#endif /* _my_icp_h */
