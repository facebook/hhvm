/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BINARY_LOG_INCLUDED
#define BINARY_LOG_INCLUDED

/*
  This file is a convenience header file meant for the users, for inclusion
  of other related header files. Please refrain from adding any definitions
  to this header.
*/
#include "binlog_event.h"
#include "control_events.h"
#include "field_types.h"
#include "load_data_events.h"
#include "rows_event.h"
#include "statement_events.h"

#endif /* BINARY_LOG_INCLUDED */
