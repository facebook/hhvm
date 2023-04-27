/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/* Defines to make different thread packages compatible */

#ifndef THREAD_TYPE_INCLUDED
#define THREAD_TYPE_INCLUDED

/**
  @file include/mysql/thread_type.h
*/

/* Flags for the THD::system_thread variable */
enum enum_thread_type {
  NON_SYSTEM_THREAD = 0,
  SYSTEM_THREAD_SLAVE_IO = 1,
  SYSTEM_THREAD_SLAVE_SQL = 2,
  SYSTEM_THREAD_NDBCLUSTER_BINLOG = 4,
  SYSTEM_THREAD_EVENT_SCHEDULER = 8,
  SYSTEM_THREAD_EVENT_WORKER = 16,
  SYSTEM_THREAD_INFO_REPOSITORY = 32,
  SYSTEM_THREAD_SLAVE_WORKER = 64,
  SYSTEM_THREAD_COMPRESS_GTID_TABLE = 128,
  SYSTEM_THREAD_BACKGROUND = 256,
  SYSTEM_THREAD_DD_INITIALIZE = 512,
  SYSTEM_THREAD_DD_RESTART = 1024,
  SYSTEM_THREAD_SERVER_INITIALIZE = 2048,
  SYSTEM_THREAD_INIT_FILE = 4096,
  SYSTEM_THREAD_SERVER_UPGRADE = 8192
};

#endif /* THREAD_TYPE_INCLUDED */
