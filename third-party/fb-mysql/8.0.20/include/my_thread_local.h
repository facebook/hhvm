/* Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_THREAD_LOCAL_INCLUDED
#define MY_THREAD_LOCAL_INCLUDED

/**
  @file include/my_thread_local.h
*/

#include "my_inttypes.h"
#include "my_macros.h"

typedef uint32 my_thread_id;

/**
  Retrieve the MySQL thread-local storage variant of errno.
*/
int my_errno();

/**
  Set the MySQL thread-local storage variant of errno.
*/
void set_my_errno(int my_errno);

#ifdef _WIN32
/*
  thr_winerr is used for returning the original OS error-code in Windows,
  my_osmaperr() returns EINVAL for all unknown Windows errors, hence we
  preserve the original Windows Error code in thr_winerr.
*/
int thr_winerr();

void set_thr_winerr(int winerr);

#endif

#ifndef DBUG_OFF
/* Return pointer to DBUG for holding current state */
struct CODE_STATE;
CODE_STATE **my_thread_var_dbug();

my_thread_id my_thread_var_id();

void set_my_thread_var_id(my_thread_id id);

#endif

#endif  // MY_THREAD_LOCAL_INCLUDED
