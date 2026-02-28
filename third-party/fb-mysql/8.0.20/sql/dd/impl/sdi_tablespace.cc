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

#include <algorithm>

#include "my_dbug.h"                         // DBUG_PRINT
#include "my_inttypes.h"                     // uint32
#include "sql/dd/cache/dictionary_client.h"  // dd::Dictionary_client
#include "sql/dd/collection.h"               // dd::Collection
#include "sql/dd/dd_tablespace.h"            // dd::get_tablespace_name
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::bootstrap::SERVER_VERSION_80016
#include "sql/dd/impl/sdi.h"                      // dd::serialize
#include "sql/dd/impl/sdi_utils.h"                // sdi_utils::checked_return
#include "sql/dd/properties.h"                    // dd::Properties
#include "sql/dd/string_type.h"                   // dd::String_type
#include "sql/dd/tablespace_id_owner_visitor.h"  // dd::visit_tablespace_id_owner
#include "sql/dd/types/index.h"                  // dd::Index
#include "sql/dd/types/partition.h"              // dd::Partition
#include "sql/dd/types/partition_index.h"        // dd::Partition_index
#include "sql/dd/types/schema.h"                 // dd::Schema
#include "sql/dd/types/table.h"                  // dd::Table
#include "sql/dd/types/tablespace.h"             // dd::Tablespace
#include "sql/handler.h"                         // sdi_set
#include "sql/mdl.h"                             // MDL_key::TABLESPACE
#include "sql/sql_class.h"                       // THD
#include "template_utils.h"                      // ReturnValueOrError
/**
  @file
  @ingroup sdi

  Storage and retrieval of SDIs to/form tablespaces. This allows SDIs
  to be stored transactionally.
*/

using namespace dd::sdi_utils;

namespace {

using DC = dd::cache::Dictionary_client;

#ifndef DBUG_OFF
const char *ge_type(const dd::Table &) { return "TABLE"; }

const char *ge_type(const dd::Index &) { return "INDEX"; }

const char *ge_type(const dd::Partition &) { return "PARTITION"; }

const char *ge_type(const dd::Partition_index &) { return "PARTITION_INDEX"; }
#endif /* DBUG_OFF */

/**
   Traverses Table object with sub objects. Returns when the first
   valid (not INVALID_OBJECT_ID) tablespace id is found.
 */
dd::Object_id fetch_first_tablespace_id(const dd::Table &t) {
  DBUG_PRINT("ddsdi", ("fetch_first_tablespace_id(%s)", t.name().c_str()));
  dd::Object_id tspid = dd::INVALID_OBJECT_ID;
  visit_tablespace_id_owners(t, [&](auto &ge) {
    dd::Object_id tid = ge.tablespace_id();
    DBUG_PRINT("ddsdi", ("Checking %s '%s'", ge_type(ge), ge.name().c_str()));
    if (tid != dd::INVALID_OBJECT_ID) {
      DBUG_PRINT("ddsdi", ("Found id:%llu, source:%s", tid, ge_type(ge)));
      tspid = tid;
      return true;  // Returning true to terminate visitation
    }
    return false;
  });
  return tspid;
}

/**
   Traverses Table object with sub objects. Returns when the
   Tablespace object corresponding to the first valid (not
   INVALID_OBJECT_ID) tablespace id is found.
 */
ReturnValueOrError<const dd::Tablespace *> fetch_first_tablespace(
    THD *thd, const dd::Table &t) {
  dd::Object_id tsid = fetch_first_tablespace_id(t);
  if (tsid == dd::INVALID_OBJECT_ID) {
    return {nullptr, false};
  }
  // The tablespace object may not have MDL
  // Need to use acquire_uncached_uncommitted to get name for MDL
  dd::Tablespace *tblspc_ = nullptr;
  if (thd->dd_client()->acquire_uncached_uncommitted(tsid, &tblspc_)) {
    return {nullptr, true};
  }
  if (tblspc_ == nullptr) {
    // When dropping a table in an implicit tablespace, the
    // refrenced tablespace may already have been removed. This
    // is ok since this means that the sdis in the tablespace
    // have been removed also. Note that since tsids is only used
    // to check for duplicates, it makes sense to leave tsid
    // there and return false so that we can proceed.
    return {nullptr, false};
  }

  if (mdl_lock(thd, MDL_key::TABLESPACE, "", tblspc_->name(),
               MDL_INTENTION_EXCLUSIVE)) {
    return {nullptr, true};
  }

  // Re-acquire after getting lock to make sure it is still there...
  const dd::Tablespace *tblspc = nullptr;
  if (thd->dd_client()->acquire(tsid, &tblspc)) {
    return {nullptr, true};
  }
  return {tblspc, false};
}
}  // namespace

namespace dd {
namespace sdi_tablespace {
bool store_tbl_sdi(THD *thd, handlerton *hton, const dd::Sdi_type &sdi,
                   const dd::Table &table,
                   const dd::Schema &schema MY_ATTRIBUTE((unused))) {
  auto res = fetch_first_tablespace(thd, table);
  if (res.error) {
    return true;
  }
  if (res.value == nullptr) {
    return false;
  }
  DBUG_PRINT("ddsdi", ("store_sdi_with_schema[](Schema" ENTITY_FMT
                       ", Table" ENTITY_FMT ")",
                       ENTITY_VAL(schema), ENTITY_VAL(table)));
  sdi_key_t key = {SDI_TYPE_TABLE, table.id()};
  if (hton->sdi_set(hton, *res.value, &table, &key, sdi.c_str(),
                    sdi.length())) {
    return checked_return(true);
  }
  DBUG_PRINT("ddsdi", ("Successful return from hton->sdi_set() with "
                       "sdi=\n%s\nStored in Tablespace (" ENTITY_FMT ")",
                       sdi.c_str(), ENTITY_VAL(*res.value)));
  return false;
}

bool store_tsp_sdi(handlerton *hton, const Sdi_type &sdi,
                   const Tablespace &tblspc) {
  DBUG_PRINT("ddsdi", ("store_tsp_sdi(" ENTITY_FMT ")", ENTITY_VAL(tblspc)));
  sdi_key_t key = {SDI_TYPE_TABLESPACE, tblspc.id()};
  if (hton->sdi_set(hton, tblspc, nullptr, &key, sdi.c_str(), sdi.length())) {
    return checked_return(true);
  }
  return false;
}

bool drop_tbl_sdi(THD *thd, const handlerton &hton, const Table &table,
                  const Schema &schema MY_ATTRIBUTE((unused))) {
  DBUG_PRINT("ddsdi",
             ("store_tbl_sdi(Schema" ENTITY_FMT ", Table" ENTITY_FMT ")",
              ENTITY_VAL(schema), ENTITY_VAL(table)));

  auto res = fetch_first_tablespace(thd, table);
  if (res.error) {
    return true;
  }
  if (res.value == nullptr) {
    return false;
  }

  sdi_key_t key = {SDI_TYPE_TABLE, table.id()};

  if (table.subpartition_type() == Table::ST_NONE ||
      table.last_checked_for_upgrade_version_id() >
          dd::bootstrap::SERVER_VERSION_80016) {
    return checked_return(hton.sdi_delete(*res.value, &table, &key));
  }

  // Sub-partitioned tables from older versions which have not yet
  // been checked for upgrade may not have an SDI record in the SDI
  // index in the tablespace, so we need to install an error handler which
  // catches the resulting error.
  bool error_suppressed = false;
  if (sdi_utils::handle_errors(
          thd,
          [&](uint errnum, const char *,
              Sql_condition::enum_severity_level *level, const char *) {
            if (errnum == ER_SDI_OPERATION_FAILED_MISSING_RECORD) {
              (*level) = Sql_condition::SL_WARNING;
              error_suppressed = true;
            }
            return false;
          },
          [&]() { return hton.sdi_delete(*res.value, &table, &key); })) {
    return checked_return(!error_suppressed);
  }
  return false;
}
}  // namespace sdi_tablespace
}  // namespace dd
