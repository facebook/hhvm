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

#include "sql/dd/impl/types/entity_object_table_impl.h"

#include <stddef.h>
#include <memory>

#include "my_dbug.h"
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/types/entity_object.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Open_dictionary_tables_ctx;
class Raw_record;

/**
  @brief
    Prepare a DD object by given raw record.

  @param otx - Context with information about open tables.
  @param record - raw record from which the object is to be prepare.
  @param o (OUT) - The object that is prepared.

  @return true - on failure and error is reported.
  @return false - on success.
*/
bool Entity_object_table_impl::restore_object_from_record(
    Open_dictionary_tables_ctx *otx, const Raw_record &record,
    Entity_object **o) const {
  DBUG_TRACE;

  // Create object instance.

  std::unique_ptr<Entity_object> obj(this->create_entity_object(record));

  /*
    Restore object attributes from the found record.
    Validate if the object state is correct.
    Restore collections within this object.
  */
  if (obj->impl()->restore_attributes(record) ||
      obj->impl()->restore_children(otx) || obj->impl()->validate()) {
    *o = nullptr;
    return true;
  }

  *o = obj.release();

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
