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

#include "sql/dd/impl/types/partition_index_impl.h"

#include <set>
#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                         // ER_*
#include "sql/dd/impl/raw/raw_record.h"           // Raw_record
#include "sql/dd/impl/sdi_impl.h"                 // sdi read/write functions
#include "sql/dd/impl/tables/index_partitions.h"  // Index_partitions
#include "sql/dd/impl/transaction_impl.h"         // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/index_impl.h"         // Index_impl
#include "sql/dd/impl/types/partition_impl.h"     // Partition_impl
#include "sql/dd/impl/types/table_impl.h"         // Table_impl
#include "sql/dd/string_type.h"                   // dd::String_type
#include "sql/dd/types/index.h"
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"

namespace dd {
class Entity_object_impl;
}  // namespace dd

using dd::tables::Index_partitions;

namespace dd {

class Object_key;
class Partition;
class Sdi_rcontext;
class Sdi_wcontext;

///////////////////////////////////////////////////////////////////////////
// Partition_index_impl implementation.
///////////////////////////////////////////////////////////////////////////

Partition_index_impl::Partition_index_impl()
    : m_options(),
      m_se_private_data(),
      m_partition(nullptr),
      m_index(nullptr),
      m_tablespace_id(INVALID_OBJECT_ID) {}

Partition_index_impl::Partition_index_impl(Partition_impl *partition,
                                           Index *index)
    : m_options(),
      m_se_private_data(),
      m_partition(partition),
      m_index(index),
      m_tablespace_id(INVALID_OBJECT_ID) {}

///////////////////////////////////////////////////////////////////////////

const Partition &Partition_index_impl::partition() const {
  return *m_partition;
}

Partition &Partition_index_impl::partition() { return *m_partition; }

///////////////////////////////////////////////////////////////////////////

const Index &Partition_index_impl::index() const { return *m_index; }

Index &Partition_index_impl::index() { return *m_index; }

///////////////////////////////////////////////////////////////////////////

bool Partition_index_impl::validate() const {
  if (!m_partition) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No partition object associated with this element.");
    return true;
  }

  if (!m_index) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No index object associated with this element.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_index_impl::restore_attributes(const Raw_record &r) {
  // Must resolve ambiguity by static cast.
  if (check_parent_consistency(
          static_cast<Entity_object_impl *>(m_partition),
          r.read_ref_id(Index_partitions::FIELD_PARTITION_ID)))
    return true;

  m_index = m_partition->table_impl().get_index(
      r.read_ref_id(Index_partitions::FIELD_INDEX_ID));

  m_tablespace_id = r.read_ref_id(Index_partitions::FIELD_TABLESPACE_ID);

  set_options(r.read_str(Index_partitions::FIELD_OPTIONS, ""));

  set_se_private_data(r.read_str(Index_partitions::FIELD_SE_PRIVATE_DATA, ""));

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Partition_index_impl::store_attributes(Raw_record *r) {
  return r->store(Index_partitions::FIELD_PARTITION_ID, m_partition->id()) ||
         r->store(Index_partitions::FIELD_INDEX_ID, m_index->id()) ||
         r->store(Index_partitions::FIELD_OPTIONS, m_options) ||
         r->store(Index_partitions::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store_ref_id(Index_partitions::FIELD_TABLESPACE_ID,
                         m_tablespace_id);
}

///////////////////////////////////////////////////////////////////////////
static_assert(
    Index_partitions::FIELD_TABLESPACE_ID == 4,
    "Index_partitions definition has changed, review (de)ser memfuns!");
void Partition_index_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  write_properties(w, m_options, STRING_WITH_LEN("options"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  write_opx_reference(w, m_index, STRING_WITH_LEN("index_opx"));

  serialize_tablespace_ref(wctx, w, m_tablespace_id,
                           STRING_WITH_LEN("tablespace_ref"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Partition_index_impl::deserialize(Sdi_rcontext *rctx,
                                       const RJ_Value &val) {
  read_properties(&m_options, val, "options");
  read_properties(&m_se_private_data, val, "se_private_data");
  read_opx_reference(rctx, &m_index, val, "index_opx");

  return deserialize_tablespace_ref(rctx, &m_tablespace_id, val,
                                    "tablespace_ref");
}

///////////////////////////////////////////////////////////////////////////

void Partition_index_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "PARTITION INDEX OBJECT: { "
     << "m_partition: {OID: " << m_partition->id() << "}; "
     << "m_index: {OID: " << m_index->id() << "}; "
     << "m_options " << m_options.raw_string() << "; "
     << "m_se_private_data " << m_se_private_data.raw_string() << "; "
     << "m_tablespace {OID: " << m_tablespace_id << "}";

  ss << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Object_key *Partition_index_impl::create_primary_key() const {
  return Index_partitions::create_primary_key(m_partition->id(), m_index->id());
}

bool Partition_index_impl::has_new_primary_key() const {
  /*
    Ideally, we should also check if index has newly generated ID.
    Unfortunately, we don't have Index_impl available here and it is
    hard to make it available.
    Since at the moment we can't have old partition object but new
    index objects the below check works OK.
    Also note that it is OK to be pessimistic and treat new key as an
    existing key. In theory, we simply get a bit higher probability of
    deadlock between two concurrent DDL as result. However in practice
    such deadlocks are impossible, since they also require two concurrent
    DDL updating metadata for the same existing partition which is not
    supported anyway.
  */
  return m_partition->has_new_primary_key();
}

///////////////////////////////////////////////////////////////////////////

Partition_index_impl *Partition_index_impl::clone(
    const Partition_index_impl &other, Partition_impl *partition) {
  Index *dstix = (*partition->table_impl()
                       .indexes())[other.m_index->ordinal_position() - 1];
  DBUG_ASSERT(dstix->ordinal_position() == other.m_index->ordinal_position() &&
              dstix->name() == other.m_index->name());
  return new Partition_index_impl(other, partition, dstix);
}

Partition_index_impl::Partition_index_impl(const Partition_index_impl &src,
                                           Partition_impl *parent, Index *index)
    : Weak_object(src),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_partition(parent),
      m_index(index),
      m_tablespace_id(src.m_tablespace_id) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &Partition_index_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Partition_index_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Index_partitions>();
}

///////////////////////////////////////////////////////////////////////////
}  // namespace dd
