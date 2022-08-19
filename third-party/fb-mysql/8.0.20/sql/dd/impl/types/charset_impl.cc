/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/charset_impl.h"

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/dd/impl/raw/object_keys.h"
#include "sql/dd/impl/raw/raw_record.h"         // Raw_record
#include "sql/dd/impl/tables/character_sets.h"  // Character_sets
#include "sql/dd/impl/transaction_impl.h"       // Open_dictionary_tables_ctx

using dd::tables::Character_sets;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Charset_impl implementation.
///////////////////////////////////////////////////////////////////////////

bool Charset_impl::validate() const {
  if (m_default_collation_id == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Collation ID is not set");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Charset_impl::restore_attributes(const Raw_record &r) {
  restore_id(r, Character_sets::FIELD_ID);
  restore_name(r, Character_sets::FIELD_NAME);

  m_mb_max_length = r.read_uint(Character_sets::FIELD_MB_MAX_LENGTH);
  m_comment = r.read_str(Character_sets::FIELD_COMMENT);

  m_default_collation_id =
      r.read_ref_id(Character_sets::FIELD_DEFAULT_COLLATION_ID);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Charset_impl::store_attributes(Raw_record *r) {
  return store_id(r, Character_sets::FIELD_ID) ||
         store_name(r, Character_sets::FIELD_NAME) ||
         r->store_ref_id(Character_sets::FIELD_DEFAULT_COLLATION_ID,
                         m_default_collation_id) ||
         r->store(Character_sets::FIELD_COMMENT, m_comment) ||
         r->store(Character_sets::FIELD_MB_MAX_LENGTH, m_mb_max_length);
}

///////////////////////////////////////////////////////////////////////////

bool Charset::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Charset::update_name_key(Name_key *key, const String_type &name) {
  return Character_sets::update_object_key(key, name);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Charset_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Charset_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Character_sets>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
