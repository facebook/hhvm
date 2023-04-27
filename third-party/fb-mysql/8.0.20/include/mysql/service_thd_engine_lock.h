/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_SERVICE_THD_EGINE_LOCK_INCLUDED
#define MYSQL_SERVICE_THD_EGINE_LOCK_INCLUDED

/**
  @file include/mysql/service_thd_engine_lock.h
  This service provides functions for storage engines to report
  lock related activities.

  SYNOPSIS
  thd_row_lock_wait() - call it just when the engine find a transaction should
  wait another transaction to realease a row lock thd                   The
  session which is waiting  for the row lock to release. thd_wait_for
  The session which is holding the row lock.
*/

class THD;

void thd_report_row_lock_wait(THD *self, THD *wait_for);

#endif
