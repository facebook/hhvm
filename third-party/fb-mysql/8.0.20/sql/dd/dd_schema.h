/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SCHEMA_INCLUDED
#define DD_SCHEMA_INCLUDED

#include <stddef.h>

#include "sql/mdl.h"  // enum_mdl_duration

class MDL_ticket;
class THD;

struct CHARSET_INFO;

namespace dd {

/**
  Check if given schema exists.

  @param         thd         Thread context.
  @param         schema_name Schema to check for.
  @param  [out]  exists      true if schema exists, else false.
  @return        false if success, true if error.
*/
bool schema_exists(THD *thd, const char *schema_name, bool *exists);

/** Create a schema record into dd.schemata. */
bool create_schema(THD *thd, const char *schema_name,
                   const CHARSET_INFO *charset_info,
                   const bool default_encryption, const char *db_metadata);

/**
  Acquire MDL on schema name.
  @param thd         Thread context.
  @param schema_name Schema to check for.
  @param duration    Duration type for MDL
  @param ticket      Where to store ticket pointer
  (default: nullptr, no ticket pointer will be stored)
  @return        false if success, true if error.
*/
bool mdl_lock_schema(THD *thd, const char *schema_name,
                     enum_mdl_duration duration, MDL_ticket **ticket = nullptr);

/**
  RAII based class to acquire and release schema meta data locks.

  When an instance of this class is created, and 'ensure_lock()' is called,
  it will acquire an IX lock on the submitted schema name, unless we already
  have one. When the instance goes out of scope or is deleted, the ticket
  registered will be released.

  @note It is vital that the order of releasing and unlocking the schema
        is correct. The Schema_MDL_locker must always be declared *before*
        the corresponding Auto_releaser to make sure that the schema locker
        is deleted *after* the auto releaser. Otherwise, there will be
        situations where we have the schema object referenced locally, but
        without a meta data lock. This may, in turn, violate asserts in the
        shared cache, and open up for improper usage.

  @todo TODO: Re-design this for a more complete long term solution of
        this problem. The current solution will mean that e.g. deadlock
        errors are propagated even when autocommit == 1.
*/

class Schema_MDL_locker {
 private:
  THD *m_thd;            // Thread context.
  MDL_ticket *m_ticket;  // MDL ticket.

 public:
  Schema_MDL_locker(THD *thd) : m_thd(thd), m_ticket(nullptr) {}

  /**
    Make sure we have an IX meta data lock on the schema name.

    If the circumstances indicate that we need a meta data lock, and
    we do not already have one, then an IX meta data lock is acquired.

    @param  schema_name   The name of the schema.

    @retval true    Failed to ensure that the schema name is locked.
            false   It is ensured that we have at least an IX lock on
                    the schema name.
  */

  bool ensure_locked(const char *schema_name);

  /**
    Release the MDL ticket, if any, when the instance of this
    class leaves scope or is deleted.
  */

  ~Schema_MDL_locker();
};

}  // namespace dd
#endif  // DD_SCHEMA_INCLUDED
