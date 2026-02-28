/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__TRIGGER_INCLUDED
#define DD__TRIGGER_INCLUDED

#include <time.h>

#ifdef _WIN32
#include <winsock2.h>  // timeval
#endif

#include "my_inttypes.h"
#include "sql/dd/sdi_fwd.h"              // dd::Sdi_wcontext
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

struct MDL_key;
struct CHARSET_INFO;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Trigger_impl;

namespace tables {
class Triggers;
}

///////////////////////////////////////////////////////////////////////////

/// Class representing a Trigger in DD framework.
class Trigger : virtual public Entity_object {
 public:
  typedef Trigger_impl Impl;
  typedef tables::Triggers DD_table;

 public:
  enum class enum_event_type { ET_INSERT = 1, ET_UPDATE, ET_DELETE };

  enum class enum_action_timing { AT_BEFORE = 1, AT_AFTER };

 public:
  virtual ~Trigger() {}

  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // table.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id table_id() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // Trigger type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_event_type event_type() const = 0;
  virtual void set_event_type(enum_event_type event_type) = 0;

  /////////////////////////////////////////////////////////////////////////
  // action_timing.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_action_timing action_timing() const = 0;
  virtual void set_action_timing(enum_action_timing type) = 0;

  /////////////////////////////////////////////////////////////////////////
  // action_order.
  /////////////////////////////////////////////////////////////////////////

  virtual uint action_order() const = 0;
  virtual void set_action_order(uint action_order) = 0;

  /////////////////////////////////////////////////////////////////////////
  // action_statement
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &action_statement() const = 0;
  virtual void set_action_statement(const String_type &action_statement) = 0;

  virtual const String_type &action_statement_utf8() const = 0;
  virtual void set_action_statement_utf8(
      const String_type &action_statement_utf8) = 0;

  /////////////////////////////////////////////////////////////////////////
  // created.
  /////////////////////////////////////////////////////////////////////////

  virtual timeval created() const = 0;
  virtual void set_created(timeval created) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last altered.
  /////////////////////////////////////////////////////////////////////////

  virtual timeval last_altered() const = 0;
  virtual void set_last_altered(timeval last_altered) = 0;

  /////////////////////////////////////////////////////////////////////////
  // sql_mode
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong sql_mode() const = 0;
  virtual void set_sql_mode(ulonglong sql_mode) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definer.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definer_user() const = 0;
  virtual const String_type &definer_host() const = 0;
  virtual void set_definer(const String_type &username,
                           const String_type &hostname) = 0;

  /////////////////////////////////////////////////////////////////////////
  // collations.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id client_collation_id() const = 0;
  virtual void set_client_collation_id(Object_id client_collation_id) = 0;

  virtual Object_id connection_collation_id() const = 0;
  virtual void set_connection_collation_id(
      Object_id connection_collation_id) = 0;

  virtual Object_id schema_collation_id() const = 0;
  virtual void set_schema_collation_id(Object_id schema_collation_id) = 0;

  static void create_mdl_key(const String_type &schema_name,
                             const String_type &name, MDL_key *key);
  static const CHARSET_INFO *name_collation();
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__TRIGGER_INCLUDED
