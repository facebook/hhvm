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

#include "sql/dd/impl/tables/collations.h"

#include <stddef.h>
#include <new>
#include <set>
#include <vector>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "sql/dd/cache/dictionary_client.h"     // dd::cache::Dictionary_...
#include "sql/dd/dd.h"                          // dd::create_object
#include "sql/dd/impl/cache/storage_adapter.h"  // Storage_adapter
#include "sql/dd/impl/raw/object_keys.h"        // Global_name_key
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/collation_impl.h"  // dd::Collation_impl
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/types/collation.h"
#include "sql/item_create.h"
#include "sql/sql_class.h"  // THD

namespace dd {
namespace tables {

const Collations &Collations::instance() {
  static Collations *s_instance = new Collations();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Collations::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Collations::Collations() {
  m_target_def.set_table_name("collations");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_CHARACTER_SET_ID, "FIELD_CHARACTER_SET_ID",
                         "character_set_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_IS_COMPILED, "FIELD_IS_COMPILED",
                         "is_compiled BOOL NOT NULL");
  m_target_def.add_field(FIELD_SORT_LENGTH, "FIELD_SORT_LENGTH",
                         "sort_length INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_PAD_ATTRIBUTE, "FIELD_PAD_ATTRIBUTE",
                         "pad_attribute ENUM('PAD SPACE', 'NO PAD') NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_NAME, "INDEX_UK_NAME", "UNIQUE KEY(name)");
  m_target_def.add_index(INDEX_K_CHARACTER_SET_ID, "INDEX_K_CHARACTER_SET_ID",
                         "KEY(character_set_id)");

  m_target_def.add_foreign_key(FK_CHARACTER_SET_ID, "FK_CHARCTER_SET_ID",
                               "FOREIGN KEY (character_set_id) REFERENCES "
                               "character_sets(id)");
}

///////////////////////////////////////////////////////////////////////////

// The table is populated when the server is started, unless it is
// started in read only mode.

bool Collations::populate(THD *thd) const {
  // Obtain a list of the previously stored collations.
  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  std::vector<const Collation *> prev_coll;
  if (thd->dd_client()->fetch_global_components(&prev_coll)) return true;

  std::set<Object_id> prev_coll_ids;
  for (const Collation *coll : prev_coll) prev_coll_ids.insert(coll->id());

  // We have an outer loop identifying the primary collations, i.e.,
  // the collations which are default for some character set. The character
  // set of these primary collations is available for use, and non-primary
  // collations referring to these character sets may therefore be
  // made available. This is the purpose of the inner loop, which is
  // executed when we have found a character set of a primary collation.
  // The inner loop will iterate over all collations, and for each available
  // collation referring to the newly identified character set, an entry
  // will be added to the dd.collations table.

  // A simpler solution would be to have a single loop, and to use the
  // CHARSET_INFO::primary_number for identifying the character set id
  // (relying on the fact that the character set ids are the same as the
  // id of the character set's default collation). However, the field
  // 'primary_number' is not assigned correctly, thus, we use the outer
  // loop to identify the primary collations for now.

  Collation_impl *new_collation = create_object<Collation_impl>();
  bool error = false;
  for (int internal_charset_id = 0;
       internal_charset_id < MY_ALL_CHARSETS_SIZE && !error;
       internal_charset_id++) {
    CHARSET_INFO *cs = all_charsets[internal_charset_id];
    if (cs && (cs->state & MY_CS_PRIMARY) && (cs->state & MY_CS_AVAILABLE) &&
        !(cs->state & MY_CS_HIDDEN)) {
      // We have identified a primary collation
      for (int internal_collation_id = 0;
           internal_collation_id < MY_ALL_CHARSETS_SIZE && !error;
           internal_collation_id++) {
        CHARSET_INFO *cl = all_charsets[internal_collation_id];
        if (cl && (cl->state & MY_CS_AVAILABLE) && my_charset_same(cs, cl)) {
          // Remove the id from the set of non-updated old ids.
          prev_coll_ids.erase(cl->number);

          // Preapre the new collation object.
          new_collation->set_id(cl->number);
          new_collation->set_name(cl->name);

          // The id of the primary collation is used as the character set id
          new_collation->set_charset_id(cs->number);
          new_collation->set_is_compiled((cl->state & MY_CS_COMPILED));
          new_collation->set_sort_length(cl->strxfrm_multiply);
          if (cl->pad_attribute == PAD_SPACE)
            new_collation->set_pad_attribute(Collation::PA_PAD_SPACE);
          else
            new_collation->set_pad_attribute(Collation::PA_NO_PAD);

          // If the collation exists, it will be updated; otherwise,
          // it will be inserted.
          error = cache::Storage_adapter::instance()->store(
              thd, static_cast<Collation *>(new_collation));
        }
      }
    }
  }
  delete new_collation;

  // The remaining ids in the prev_coll_ids set were not updated, and must
  // therefore be deleted from the DD since they are not supported anymore.
  for (std::set<Object_id>::const_iterator del_it = prev_coll_ids.begin();
       del_it != prev_coll_ids.end(); ++del_it) {
    const Collation *del_coll = nullptr;
    if (thd->dd_client()->acquire(*del_it, &del_coll)) return true;

    DBUG_ASSERT(del_coll);
    if (thd->dd_client()->drop(del_coll)) return true;
  }

  return error;
}

///////////////////////////////////////////////////////////////////////////

Collation *Collations::create_entity_object(const Raw_record &) const {
  return new (std::nothrow) Collation_impl();
}

///////////////////////////////////////////////////////////////////////////

bool Collations::update_object_key(Global_name_key *key,
                                   const String_type &collation_name) {
  key->update(FIELD_NAME, collation_name, name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
