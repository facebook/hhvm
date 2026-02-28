/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_THR_COND_BITS_H
#define COMPONENTS_SERVICES_THR_COND_BITS_H

/**
  @file
  MySQL condition variable implementation.

  native_cond_t
    Windows    - ConditionVariable
    Other OSes - pthread
*/

#include <stddef.h>
#include <sys/types.h>
#ifdef _WIN32
#include <time.h>
#include "my_systime.h"
#endif

#ifdef _WIN32
typedef CONDITION_VARIABLE native_cond_t;
#else
typedef pthread_cond_t native_cond_t;
#endif

#endif /* COMPONENTS_SERVICES_THR_COND_BITS_H */
