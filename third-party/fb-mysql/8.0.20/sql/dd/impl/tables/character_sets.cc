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

#include "sql/dd/impl/tables/character_sets.h"

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
#include "sql/dd/impl/types/charset_impl.h"    // dd::Charset_impl
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/types/charset.h"
#include "sql/item_create.h"
#include "sql/sql_class.h"  // THD

namespace dd {
namespace tables {

const Character_sets &Character_sets::instance() {
  static Character_sets *s_instance = new Character_sets();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Character_sets::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Character_sets::Character_sets() {
  m_target_def.set_table_name("character_sets");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_DEFAULT_COLLATION_ID,
                         "FIELD_DEFAULT_COLLATION_ID",
                         "default_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_COMMENT, "FIELD_COMMENT",
                         "comment VARCHAR(2048)"
                         " COLLATE utf8_general_ci NOT NULL");
  m_target_def.add_field(FIELD_MB_MAX_LENGTH, "FIELD_MB_MAX_LENGTH",
                         "mb_max_length INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_NAME, "INDEX_UK_NAME", "UNIQUE KEY(name)");
  m_target_def.add_index(INDEX_UK_DEFAULT_COLLATION_ID,
                         "INDEX_UK_DEFAULT_COLLATION_ID",
                         "UNIQUE KEY(default_collation_id)");

  m_target_def.add_foreign_key(FK_DEFAULT_COLLATION_ID,
                               "FK_DEFAULT_COLLATION_ID",
                               "FOREIGN KEY (default_collation_id) "
                               "REFERENCES collations(id)");
}

///////////////////////////////////////////////////////////////////////////

// The table is populated when the server is started, unless it is
// started in read only mode.

bool Character_sets::populate(THD *thd) const {
  // Obtain a list of the previously stored charsets.
  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  std::vector<const Charset *> prev_cset;
  if (thd->dd_client()->fetch_global_components(&prev_cset)) return true;

  std::set<Object_id> prev_cset_ids;
  for (const Charset *cset : prev_cset) prev_cset_ids.insert(cset->id());

  // We have an outer loop identifying the primary collations, i.e.,
  // the collations which are default for some character set. Then,
  // the character set of each primary collation is stored in an entry
  // in the dd.character_sets table. This means that if there are
  // collations referring to a character set which has no default
  // collation, we will not have an entry for this character set in
  // the dd.character_sets table. This also means that a given character
  // set can have only one primary collation, since character set identity
  // is given by the character set name, and we have a unique key on the
  // character set name in the dd.character_sets table. Populating the
  // dd.collations table follows a similar pattern, but has an additional
  // inner loop adding the actual collations referring to the character
  // sets. Each character set is stored with the id (primary key) of its
  // corresponding primary collation as the id (primary key).

  Charset_impl *new_charset = create_object<Charset_impl>();
  bool error = false;
  for (int internal_charset_id = 0;
       internal_charset_id < MY_ALL_CHARSETS_SIZE && !error;
       internal_charset_id++) {
    CHARSET_INFO *cs = all_charsets[internal_charset_id];
    if (cs && (cs->state & MY_CS_PRIMARY) && (cs->state & MY_CS_AVAILABLE) &&
        !(cs->state & MY_CS_HIDDEN)) {
      // Remove the id from the set of non-updated old ids.
      prev_cset_ids.erase(cs->number);

      // The character set is stored on the same id as its primary collation
      new_charset->set_id(cs->number);
      new_charset->set_name(cs->csname);
      new_charset->set_default_collation_id(cs->number);
      new_charset->set_mb_max_length(cs->mbmaxlen);
      new_charset->set_comment(cs->comment ? cs->comment : "");

      // If the charset exists, it will be updated; otherwise,
      // it will be inserted.
      error = cache::Storage_adapter::instance()->store(
          thd, static_cast<Charset *>(new_charset));
    }
  }
  delete new_charset;

  // The remaining ids in the prev_cset_ids set were not updated, and must
  // therefore be deleted from the DD since they are not supported anymore.
  for (std::set<Object_id>::const_iterator del_it = prev_cset_ids.begin();
       del_it != prev_cset_ids.end(); ++del_it) {
    const Charset *del_cset = nullptr;
    if (thd->dd_client()->acquire(*del_it, &del_cset)) return true;

    DBUG_ASSERT(del_cset);
    if (thd->dd_client()->drop(del_cset)) return true;
  }

  return error;
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Charset *Character_sets::create_entity_object(const Raw_record &) const {
  return new (std::nothrow) Charset_impl();
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

bool Character_sets::update_object_key(Global_name_key *key,
                                       const String_type &charset_name) {
  key->update(FIELD_NAME, charset_name, name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
