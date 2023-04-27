/*
   Copyright (c) 2001, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_PSI_CONFIG_INCLUDED
#define MY_PSI_CONFIG_INCLUDED

/**
  @file include/my_psi_config.h
  Defines various enable/disable and HAVE_ macros related to the
  performance schema instrumentation system, without pulling in
  any system \#include files (which breaks the ABI checker).

*/

#include "my_config.h"

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
#define HAVE_PSI_INTERFACE
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

#ifdef HAVE_PSI_INTERFACE

/**
 @def DISABLE_PSI_MUTEX
 Compiling option to disable the mutex instrumentation.
 This option is mostly intended to be used during development,
 when doing special builds with only a subset of the performance schema
 instrumentation, for code analysis / profiling / performance tuning of a
 specific instrumentation alone.
 @sa DISABLE_PSI_COND
 @sa DISABLE_PSI_DATA_LOCK
 @sa DISABLE_PSI_ERROR
 @sa DISABLE_PSI_FILE
 @sa DISABLE_PSI_IDLE
 @sa DISABLE_PSI_MEMORY
 @sa DISABLE_PSI_METADATA
 @sa DISABLE_PSI_PS
 @sa DISABLE_PSI_RWLOCK
 @sa DISABLE_PSI_SOCKET
 @sa DISABLE_PSI_SP
 @sa DISABLE_PSI_STAGE
 @sa DISABLE_PSI_STATEMENT
 @sa DISABLE_PSI_STATEMENT_DIGEST
 @sa DISABLE_PSI_SYSTEM
 @sa DISABLE_PSI_TABLE
 @sa DISABLE_PSI_THREAD
 @sa DISABLE_PSI_TRANSACTION
 */

#ifndef DISABLE_PSI_MUTEX
#define HAVE_PSI_MUTEX_INTERFACE
#endif /* DISABLE_PSI_MUTEX */

/**
  @def DISABLE_PSI_RWLOCK
  Compiling option to disable the rwlock instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_RWLOCK
#define HAVE_PSI_RWLOCK_INTERFACE
#endif /* DISABLE_PSI_RWLOCK */

/**
  @def DISABLE_PSI_COND
  Compiling option to disable the cond instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_COND
#define HAVE_PSI_COND_INTERFACE
#endif /* DISABLE_PSI_COND */

/**
  @def DISABLE_PSI_FILE
  Compiling option to disable the file instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_FILE
#define HAVE_PSI_FILE_INTERFACE
#endif /* DISABLE_PSI_FILE */

/**
  @def DISABLE_PSI_THREAD
  Compiling option to disable the thread instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_THREAD
#define HAVE_PSI_THREAD_INTERFACE
#endif /* DISABLE_PSI_THREAD */

/**
  @def DISABLE_PSI_TABLE
  Compiling option to disable the table instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_TABLE
#define HAVE_PSI_TABLE_INTERFACE
#endif /* DISABLE_PSI_TABLE */

/**
  @def DISABLE_PSI_STAGE
  Compiling option to disable the stage instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_STAGE
#define HAVE_PSI_STAGE_INTERFACE
#endif /* DISABLE_PSI_STAGE */

/**
  @def DISABLE_PSI_STATEMENT
  Compiling option to disable the statement instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_STATEMENT
#define HAVE_PSI_STATEMENT_INTERFACE
#endif /* DISABLE_PSI_STATEMENT */

/**
  @def DISABLE_PSI_SP
  Compiling option to disable the stored program instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_SP
#define HAVE_PSI_SP_INTERFACE
#endif /* DISABLE_PSI_SP */

/**
  @def DISABLE_PSI_PS
  Compiling option to disable the prepared statement instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_STATEMENT
#ifndef DISABLE_PSI_PS
#define HAVE_PSI_PS_INTERFACE
#endif /* DISABLE_PSI_PS */
#endif /* DISABLE_PSI_STATEMENT */

/**
  @def DISABLE_PSI_STATEMENT_DIGEST
  Compiling option to disable the statement digest instrumentation.
*/

#ifndef DISABLE_PSI_STATEMENT
#ifndef DISABLE_PSI_STATEMENT_DIGEST
#define HAVE_PSI_STATEMENT_DIGEST_INTERFACE
#endif /* DISABLE_PSI_STATEMENT_DIGEST */
#endif /* DISABLE_PSI_STATEMENT */

/**
  @def DISABLE_PSI_TRANSACTION
  Compiling option to disable the transaction instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_TRANSACTION
#define HAVE_PSI_TRANSACTION_INTERFACE
#endif /* DISABLE_PSI_TRANSACTION */

/**
  @def DISABLE_PSI_SOCKET
  Compiling option to disable the statement instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_SOCKET
#define HAVE_PSI_SOCKET_INTERFACE
#endif /* DISABLE_PSI_SOCKET */

/**
  @def DISABLE_PSI_MEMORY
  Compiling option to disable the memory instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_MEMORY
#define HAVE_PSI_MEMORY_INTERFACE
#endif /* DISABLE_PSI_MEMORY */

/**
  @def DISABLE_PSI_ERROR
  Compiling option to disable the error instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_ERROR
#define HAVE_PSI_ERROR_INTERFACE
#endif /* DISABLE_PSI_ERROR */

/**
  @def DISABLE_PSI_IDLE
  Compiling option to disable the idle instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_IDLE
#define HAVE_PSI_IDLE_INTERFACE
#endif /* DISABLE_PSI_IDLE */

/**
  @def DISABLE_PSI_METADATA
  Compiling option to disable the metadata instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_METADATA
#define HAVE_PSI_METADATA_INTERFACE
#endif /* DISABLE_PSI_METADATA */

/**
  @def DISABLE_PSI_DATA_LOCK
  Compiling option to disable the data lock instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_DATA_LOCK
#define HAVE_PSI_DATA_LOCK_INTERFACE
#endif /* DISABLE_PSI_DATA_LOCK */

/**
  @def DISABLE_PSI_SYSTEM
  Compiling option to disable the system instrumentation.
  @sa DISABLE_PSI_MUTEX
*/

#ifndef DISABLE_PSI_SYSTEM
#define HAVE_PSI_SYSTEM_INTERFACE
#endif /* DISABLE_PSI_SYSTEM */

#endif /* HAVE_PSI_INTERFACE */

#endif  // MY_PSI_CONFIG_INCLUDED
