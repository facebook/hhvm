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

#include "sql/dd/impl/types/tablespace_impl.h"
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // DD_bootstrap_ctx

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "sql/dd/impl/object_key.h"
#include "sql/dd/impl/properties_impl.h"     // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"      // Raw_record
#include "sql/dd/impl/raw/raw_record_set.h"  // Raw_record_set
#include "sql/dd/impl/raw/raw_table.h"       // Raw_table
#include "sql/dd/impl/sdi_impl.h"            // sdi read/write functions
#include "sql/dd/impl/sdi_utils.h"           // sdi_utils::checked_return
#include "sql/dd/impl/tables/index_partitions.h"  // dd::tables::Index_partitions::FIELD_TABLESPACE_ID
#include "sql/dd/impl/tables/indexes.h"  // dd::tables::Indexes::FIELD_TABLESPACE_ID
#include "sql/dd/impl/tables/schemata.h"  // dd::tables::schemata::FIELD_SCHEMA_NAME
#include "sql/dd/impl/tables/table_partitions.h"  // dd::tables::Table_partitions::FIELD_TABLESPACE_ID
#include "sql/dd/impl/tables/tables.h"            // create_key_by_tablespace_id
#include "sql/dd/impl/tables/tablespace_files.h"  // Tablespace_files
#include "sql/dd/impl/tables/tablespaces.h"       // Tablespaces
#include "sql/dd/impl/transaction_impl.h"         // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/index_impl.h"         // Index_impl
#include "sql/dd/impl/types/partition_index_impl.h"  // Partition_index_impl
#include "sql/dd/impl/types/tablespace_file_impl.h"  // Tablespace_file_impl
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/partition.h"  // Partition
#include "sql/dd/types/tablespace_file.h"
#include "sql/dd/types/weak_object.h"

#include "sql/handler.h"    // handlerton
#include "sql/sql_class.h"  // THD
#include "sql/strfunc.h"    // casedn

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"  // ER_*

using dd::tables::Index_partitions;
using dd::tables::Indexes;
using dd::tables::Schemata;
using dd::tables::Table_partitions;
using dd::tables::Tables;
using dd::tables::Tablespace_files;
using dd::tables::Tablespaces;

using dd::sdi_utils::checked_return;

namespace dd {

class Sdi_rcontext;
class Sdi_wcontext;

static const std::set<String_type> default_valid_option_keys = {"encryption"};

///////////////////////////////////////////////////////////////////////////
// Tablespace_impl implementation.
///////////////////////////////////////////////////////////////////////////

Tablespace_impl::Tablespace_impl()
    : m_options(default_valid_option_keys),
      m_se_private_data(),
      m_files() {} /* purecov: tested */

Tablespace_impl::~Tablespace_impl() {}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::validate() const {
  if (m_engine != "ndbcluster" && m_files.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No files associated with this tablespace.");
    return true;
  }

  if (m_engine.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Engine name is not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return m_files.restore_items(
      this, otx, otx->get_table<Tablespace_file>(),
      Tablespace_files::create_key_by_tablespace_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::store_children(Open_dictionary_tables_ctx *otx) {
  if (m_files.has_removed_items()) {
    if (m_files.drop_items(
            otx, otx->get_table<Tablespace_file>(),
            Tablespace_files::create_key_by_tablespace_id(this->id()))) {
      return true;
    }
    // Prevent store_items() below from also dropping removed files since
    // all are already dropped.
    m_files.clear_removed_items();
  }
  return m_files.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_files.drop_items(
      otx, otx->get_table<Tablespace_file>(),
      Tablespace_files::create_key_by_tablespace_id(this->id()));
}

/////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::restore_attributes(const Raw_record &r) {
  restore_id(r, Tablespaces::FIELD_ID);
  restore_name(r, Tablespaces::FIELD_NAME);

  m_comment = r.read_str(Tablespaces::FIELD_COMMENT);

  set_options(r.read_str(Tablespaces::FIELD_OPTIONS));

  set_se_private_data(r.read_str(Tablespaces::FIELD_SE_PRIVATE_DATA));

  m_engine = r.read_str(Tablespaces::FIELD_ENGINE);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::store_attributes(Raw_record *r) {
#ifndef DBUG_OFF
  if (my_strcasecmp(system_charset_info, "InnoDB", m_engine.c_str()) == 0) {
    /* Innodb can request for space rename during upgrade when options are not
    upgraded yet. */
    DBUG_ASSERT(m_options.exists("encryption") ||
                bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade());
  } else {
    DBUG_ASSERT(!m_options.exists("encryption"));
  }
#endif
  return store_id(r, Tablespaces::FIELD_ID) ||
         store_name(r, Tablespaces::FIELD_NAME) ||
         r->store(Tablespaces::FIELD_COMMENT, m_comment) ||
         r->store(Tablespaces::FIELD_OPTIONS, m_options) ||
         r->store(Tablespaces::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store(Tablespaces::FIELD_ENGINE, m_engine);
}

///////////////////////////////////////////////////////////////////////////

static_assert(Tablespaces::FIELD_ENGINE == 5,
              "Tablespaces definition has changed, review (de)ser memfuns!");
void Tablespace_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);
  write(w, m_comment, STRING_WITH_LEN("comment"));
  write_properties(w, m_options, STRING_WITH_LEN("options"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  write(w, m_engine, STRING_WITH_LEN("engine"));
  serialize_each(wctx, w, m_files, STRING_WITH_LEN("files"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);
  read(&m_comment, val, "comment");
  read_properties(&m_options, val, "options");
  read_properties(&m_se_private_data, val, "se_private_data");
  read(&m_engine, val, "engine");
  deserialize_each(
      rctx, [this]() { return add_file(); }, val, "files");
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace::update_name_key(Name_key *key, const String_type &name) {
  return Tablespaces::update_object_key(key, name);
}

///////////////////////////////////////////////////////////////////////////

// Check whether a tablespce is empty.
bool Tablespace_impl::is_empty(THD *thd, bool *empty) const {
  // Create the key based on the tablespace id.
  std::unique_ptr<Object_key> object_key(
      tables::Tables::create_key_by_tablespace_id(id()));

  // Start a read only transaction, read the set of tables.
  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<Abstract_table>();
  Raw_table *table = trx.otx.get_table<Abstract_table>();
  DBUG_ASSERT(table);

  std::unique_ptr<Raw_record_set> rs;
  if (trx.otx.open_tables() || table->open_record_set(object_key.get(), rs)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  DBUG_ASSERT(empty);
  *empty = (rs->current_record() == nullptr);

  return false;
}

///////////////////////////////////////////////////////////////////////////

void Tablespace_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "TABLESPACE OBJECT: { "
     << "id: {OID: " << id() << "}; "
     << "m_name: " << name() << "; "
     << "m_comment: " << m_comment << "; "
     << "m_options " << m_options.raw_string() << "; "
     << "m_se_private_data " << m_se_private_data.raw_string() << "; "
     << "m_engine: " << m_engine << "; "
     << "m_files: " << m_files.size() << " [ ";

  for (const Tablespace_file *f : files()) {
    String_type ob;
    f->debug_print(ob);
    ss << ob;
  }

  ss << "] }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Tablespace_file *Tablespace_impl::add_file() {
  Tablespace_file_impl *f = new (std::nothrow) Tablespace_file_impl(this);
  m_files.push_back(f);
  return f;
}

///////////////////////////////////////////////////////////////////////////

bool Tablespace_impl::remove_file(String_type data_file) {
  for (Tablespace_file *tsf : m_files) {
    if (tsf->filename() == data_file) {
      m_files.remove(dynamic_cast<Tablespace_file_impl *>(tsf));
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Tablespace_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Tablespace_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Tablespaces>();

  otx->register_tables<Tablespace_file>();
}

///////////////////////////////////////////////////////////////////////////

Tablespace_impl::Tablespace_impl(const Tablespace_impl &src)
    : Weak_object(src),
      Entity_object_impl(src),
      m_comment(src.m_comment),
      m_options(src.m_options),
      m_se_private_data(src.m_se_private_data),
      m_engine(src.m_engine),
      m_files() {
  m_files.deep_copy(src.m_files, this);
}

///////////////////////////////////////////////////////////////////////////

namespace {
/* purecov: begin inspected */
template <typename PT>
PT &ref(PT *tp) {
  DBUG_ASSERT(tp != nullptr);
  return *tp;
}
/* purecov: end */

template <typename SC, typename KT>
bool select_clos_where_key_matches(SC &&s, Raw_table *rtp, KT &&k) {
  Raw_table &rtbl = ref(rtp);

  std::unique_ptr<Raw_record_set> rs;
  if (rtbl.open_record_set(&k, rs)) {
    return dd::sdi_utils::checked_return(true);
  }

  Raw_record *r = rs->current_record();

  while (r) {
    if (s(r)) {
      return checked_return(true);
    }

    // Note r is passed by non-const ref here, and is advanced
    // to the next record, or set to nullptr
    if (rs->next(r)) {
      return checked_return(true);
    }
  }
  return false;
}

template <typename R, typename LESSF = std::less<typename R::value_type>,
          typename EQUALF = std::equal_to<typename R::value_type>>
typename R::iterator remove_duplicates(R *rp, LESSF &&lessf = LESSF(),
                                       EQUALF &&equalf = EQUALF()) {
  auto &r = ref(rp);
  std::sort(std::begin(r), std::end(r), lessf);
  return std::unique(std::begin(r), std::end(r), equalf);
}
}  // namespace

bool operator==(const Tablespace_table_ref &a, const Tablespace_table_ref &b) {
  return a.m_id == b.m_id;
}

bool operator<(const Tablespace_table_ref &a, const Tablespace_table_ref &b) {
  return a.m_id < b.m_id;
}

bool fetch_tablespace_table_refs(THD *thd, const Tablespace &tso,
                                 Tablespace_table_ref_vec *tblrefsp) {
  auto &tblrefs = ref(tblrefsp);

  // Start a read only transaction, read the set of tables.
  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<Partition_index>();
  trx.otx.register_tables<Partition>();
  trx.otx.register_tables<Index>();
  trx.otx.register_tables<Abstract_table>();
  trx.otx.register_tables<Schema>();

  if (trx.otx.open_tables()) {
    return checked_return(true);
  }

  std::vector<Object_id> partids;
  if (select_clos_where_key_matches(
          [&partids](Raw_record *r) {
            partids.push_back(
                r->read_ref_id(Index_partitions::FIELD_PARTITION_ID));
            return false;
          },
          trx.otx.get_table<Partition_index>(),
          Parent_id_range_key{Index_partitions::INDEX_K_TABLESPACE_ID,
                              Index_partitions::FIELD_TABLESPACE_ID,
                              tso.id()})) {
    return checked_return(true);
  }

  partids.resize(std::distance(partids.begin(), remove_duplicates(&partids)));
  std::vector<Object_id> tblids;
  for (auto &partid : partids) {
    if (select_clos_where_key_matches(
            [&tblids](Raw_record *r) {
              tblids.push_back(
                  r->read_ref_id(Table_partitions::FIELD_TABLE_ID));
              return false;
            },
            trx.otx.get_table<Partition>(),
            Parent_id_range_key{Table_partitions::INDEX_PK_ID,
                                Table_partitions::FIELD_ID, partid})) {
      return checked_return(true);
    }
  }

  if (select_clos_where_key_matches(
          [&tblids](Raw_record *r) {
            tblids.push_back(r->read_ref_id(Table_partitions::FIELD_TABLE_ID));
            return false;
          },
          trx.otx.get_table<Partition>(),
          Parent_id_range_key{Table_partitions::INDEX_K_TABLESPACE_ID,
                              Table_partitions::FIELD_TABLESPACE_ID,
                              tso.id()})) {
    return checked_return(true);
  }

  if (select_clos_where_key_matches(
          [&tblids](Raw_record *r) {
            tblids.push_back(r->read_ref_id(Indexes::FIELD_TABLE_ID));
            return false;
          },
          trx.otx.get_table<Index>(),
          Parent_id_range_key{Indexes::INDEX_K_TABLESPACE_ID,
                              Indexes::FIELD_TABLESPACE_ID, tso.id()})) {
    return checked_return(true);
  }

  tblids.resize(std::distance(tblids.begin(), remove_duplicates(&tblids)));
  for (auto &tid : tblids) {
    if (select_clos_where_key_matches(
            [&tblrefs, &tid](Raw_record *r) {
              long long int ht = r->read_int(Tables::FIELD_HIDDEN);
              if (ht == Abstract_table::HT_HIDDEN_SE) {
                // Skip tables owned by SE. SE must ensure proper locking.
                return false;
              }

              tblrefs.push_back({tid, r->read_str(Tables::FIELD_NAME),
                                 r->read_ref_id(Tables::FIELD_SCHEMA_ID)});
              return false;
            },
            trx.otx.get_table<Abstract_table>(),
            Parent_id_range_key{Tables::INDEX_PK_ID, Tables::FIELD_ID, tid})) {
      return checked_return(true);
    }
  }

  // Select tables with tableid == tso.id()
  if (select_clos_where_key_matches(
          [&tblrefs](Raw_record *r) {
            tblrefs.push_back({r->read_ref_id(Tables::FIELD_ID),
                               r->read_str(Tables::FIELD_NAME),
                               r->read_ref_id(Tables::FIELD_SCHEMA_ID)});
            return false;
          },
          trx.otx.get_table<Abstract_table>(),
          Parent_id_range_key{Tables::INDEX_K_TABLESPACE_ID,
                              Tables::FIELD_TABLESPACE_ID, tso.id()})) {
    return checked_return(true);
  }

  tblrefs.resize(std::distance(tblrefs.begin(), remove_duplicates(tblrefsp)));
  for (auto &tref : tblrefs) {
    if (select_clos_where_key_matches(
            [&tref](Raw_record *r) {
              tref.m_schema_name = r->read_str(Schemata::FIELD_NAME);
              tref.m_schema_encryption =
                  static_cast<enum_encryption_type>(
                      r->read_int(Schemata::FIELD_DEFAULT_ENCRYPTION)) ==
                          enum_encryption_type::ET_YES
                      ? true
                      : false;
              return false;
            },
            trx.otx.get_table<Schema>(),
            Parent_id_range_key{Schemata::INDEX_PK_ID, Schemata::FIELD_ID,
                                tref.m_schema_id})) {
      return checked_return(true);
    }
  }

  return false;
}

MDL_request *mdl_req(THD *thd, const Tablespace_table_ref &tref,
                     enum enum_mdl_type mdl_type) {
  MDL_request *r = new (thd->mem_root) MDL_request;

  if (lower_case_table_names == 2) {
    dd::String_type lc_schema_name = casedn(
        Object_table_definition_impl::fs_name_collation(), tref.m_schema_name);
    dd::String_type lc_name =
        casedn(Object_table_definition_impl::fs_name_collation(), tref.m_name);
    MDL_REQUEST_INIT(r, MDL_key::TABLE, lc_schema_name.c_str(), lc_name.c_str(),
                     mdl_type, MDL_TRANSACTION);
  } else {
    MDL_REQUEST_INIT(r, MDL_key::TABLE, tref.m_schema_name.c_str(),
                     tref.m_name.c_str(), mdl_type, MDL_TRANSACTION);
  }
  return r;
}

MDL_request *mdl_schema_req(THD *thd, const dd::String_type &schema_name) {
  MDL_request *r = new (thd->mem_root) MDL_request;

  if (lower_case_table_names == 2) {
    dd::String_type lc_schema_name =
        casedn(Object_table_definition_impl::fs_name_collation(), schema_name);
    MDL_REQUEST_INIT(r, MDL_key::SCHEMA, lc_schema_name.c_str(), "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  } else {
    MDL_REQUEST_INIT(r, MDL_key::SCHEMA, schema_name.c_str(), "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  }
  return r;
}

}  // namespace dd
