/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file sql/histograms/histogram.cc
  Histogram base class (implementation).
*/

#include "sql/histograms/histogram.h"  // Histogram, Histogram_comparator

#include <sys/types.h>
#include <algorithm>
#include <map>
#include <memory>  // std::unique_ptr
#include <new>
#include <random>
#include <string>
#include <vector>

#include "field_types.h"  // enum_field_types
#include "lex_string.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"  // my_micro_time, get_charset
#include "my_systime.h"
#include "my_time.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "scope_guard.h"  // create_scope_guard
#include "sql/auth/auth_common.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/column_statistics.h"
#include "sql/dd/types/table.h"  // dd::Table
#include "sql/debug_sync.h"
#include "sql/field.h"  // Field
#include "sql/handler.h"
#include "sql/histograms/equi_height.h"  // Equi_height<T>
#include "sql/histograms/singleton.h"    // Singleton<T>
#include "sql/histograms/value_map.h"    // Value_map
#include "sql/item.h"
#include "sql/json_dom.h"  // Json_*
#include "sql/key.h"
#include "sql/mdl.h"  // MDL_request
#include "sql/my_decimal.h"
#include "sql/psi_memory_key.h"  // key_memory_histograms
#include "sql/sql_base.h"        // open_and_lock_tables,
#include "sql/sql_bitmap.h"
// close_thread_tables
#include "sql/sql_class.h"  // make_lex_string_root
#include "sql/sql_const.h"
#include "sql/strfunc.h"  // find_type2, find_set
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit_stmt, trans_rollback_stmt
#include "sql/tztime.h"       // my_tz_UTC
#include "sql_string.h"       // String
#include "template_utils.h"

struct TYPELIB;

namespace histograms {

/*
  This type represents a instrumented map of value maps, indexed by field
  number.
*/
using value_map_collection = std::map<
    uint16, std::unique_ptr<histograms::Value_map_base>, std::less<uint16>,
    Histogram_key_allocator<
        std::pair<const uint16, std::unique_ptr<histograms::Value_map_base>>>>;

void *Histogram_psi_key_alloc::operator()(size_t s) const {
  return my_malloc(key_memory_histograms, s, MYF(MY_WME | ME_FATALERROR));
}

/**
  Convert from enum_field_types to Value_map_type.

  @param field_type the field type
  @param is_unsigned whether the field type is unsigned or not. This is only
                     considered if the field type is LONGLONG

  @return A Value_map_type. May be INVALID if the Value_map does not support
          the field type.
*/
static Value_map_type field_type_to_value_map_type(
    const enum_field_types field_type, const bool is_unsigned) {
  switch (field_type) {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      return Value_map_type::DECIMAL;
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_BIT:
      return Value_map_type::INT;
    case MYSQL_TYPE_ENUM:
      return Value_map_type::ENUM;
    case MYSQL_TYPE_SET:
      return Value_map_type::SET;
    case MYSQL_TYPE_LONGLONG:
      return is_unsigned ? Value_map_type::UINT : Value_map_type::INT;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      return Value_map_type::DOUBLE;
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
      return Value_map_type::TIME;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_NEWDATE:
      return Value_map_type::DATE;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATETIME2:
      return Value_map_type::DATETIME;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VARCHAR:
      return Value_map_type::STRING;
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_NULL:
    default:
      return Value_map_type::INVALID;
  }

  // All cases should be handled, so this should not be hit.
  /* purecov: begin inspected */
  DBUG_ASSERT(false);
  return Value_map_type::INVALID;
  /* purecov: end */
}

/**
  Get the Value_map_type from a Field object.

  This effectively looks at the real_type() of a Field, and converts this to
  a Value_map_type

  @param field The field to convert from

  @return A Value_map_type. May be INVALID if the Value_map does not support
          the field type.
*/
static Value_map_type field_type_to_value_map_type(const Field *field) {
  bool is_unsigned = false;
  if (field->real_type() == MYSQL_TYPE_LONGLONG) {
    /*
      For most integer types, the Value_map_type will be INT (int64). This type
      will not cover the entire value range for the SQL data type UNSIGNED
      BIGINT, so we need to distinguish between SIGNED BIGINT and UNSIGNED
      BIGINT so that we can switch the Value_map_type to UINT (uint64).
    */
    const Field_num *field_num = down_cast<const Field_num *>(field);
    is_unsigned = field_num->unsigned_flag;
  }

  return field_type_to_value_map_type(field->real_type(), is_unsigned);
}

/**
  Lock a column statistic MDL key for writing (exclusive lock).

  @param thd thread handle
  @param mdl_key the MDL key to lock

  @return true on error, false on success
*/
static bool lock_for_write(THD *thd, const MDL_key &mdl_key) {
  DBUG_EXECUTE_IF("histogram_fail_during_lock_for_write", { return true; });

  MDL_request mdl_request;
  MDL_REQUEST_INIT_BY_KEY(&mdl_request, &mdl_key, MDL_EXCLUSIVE,
                          MDL_TRANSACTION);

  // If locking fails, an error has already been flagged.
  return thd->mdl_context.acquire_lock_nsec(
      &mdl_request, thd->variables.lock_wait_timeout_nsec);
}

Histogram::Histogram(MEM_ROOT *mem_root, const std::string &db_name,
                     const std::string &tbl_name, const std::string &col_name,
                     enum_histogram_type type, Value_map_type data_type)
    : m_null_values_fraction(INVALID_NULL_VALUES_FRACTION),
      m_charset(nullptr),
      m_num_buckets_specified(0),
      m_mem_root(mem_root),
      m_hist_type(type),
      m_data_type(data_type) {
  lex_string_strmake(m_mem_root, &m_database_name, db_name.c_str(),
                     db_name.length());

  lex_string_strmake(m_mem_root, &m_table_name, tbl_name.c_str(),
                     tbl_name.length());

  lex_string_strmake(m_mem_root, &m_column_name, col_name.c_str(),
                     col_name.length());
}

Histogram::Histogram(MEM_ROOT *mem_root, const Histogram &other)
    : m_sampling_rate(other.m_sampling_rate),
      m_null_values_fraction(other.m_null_values_fraction),
      m_charset(other.m_charset),
      m_num_buckets_specified(other.m_num_buckets_specified),
      m_mem_root(mem_root),
      m_hist_type(other.m_hist_type),
      m_data_type(other.m_data_type) {
  lex_string_strmake(m_mem_root, &m_database_name, other.m_database_name.str,
                     other.m_database_name.length);

  lex_string_strmake(m_mem_root, &m_table_name, other.m_table_name.str,
                     other.m_table_name.length);

  lex_string_strmake(m_mem_root, &m_column_name, other.m_column_name.str,
                     other.m_column_name.length);
}

bool Histogram::histogram_to_json(Json_object *json_object) const {
  // Get the current time in GMT timezone with microsecond accuray.
  timeval time_value;
  my_micro_time_to_timeval(my_micro_time(), &time_value);

  MYSQL_TIME current_time;
  my_tz_UTC->gmt_sec_to_TIME(&current_time, time_value);

  // last-updated
  const Json_datetime last_updated(current_time, MYSQL_TYPE_DATETIME);
  if (json_object->add_clone(last_updated_str(), &last_updated))
    return true; /* purecov: inspected */

  // histogram-type
  const Json_string histogram_type(histogram_type_to_str());
  if (json_object->add_clone(histogram_type_str(), &histogram_type))
    return true; /* purecov: inspected */

  // Sampling rate
  DBUG_ASSERT(get_sampling_rate() >= 0.0);
  DBUG_ASSERT(get_sampling_rate() <= 1.0);
  const Json_double sampling_rate(get_sampling_rate());
  if (json_object->add_clone(sampling_rate_str(), &sampling_rate))
    return true; /* purecov: inspected */

  // The number of buckets specified in the ANALYZE TABLE command
  const Json_int num_buckets_specified(get_num_buckets_specified());
  if (json_object->add_clone(numer_of_buckets_specified_str(),
                             &num_buckets_specified))
    return true; /* purecov: inspected */

  // Fraction of NULL values.
  DBUG_ASSERT(get_null_values_fraction() >= 0.0);
  DBUG_ASSERT(get_null_values_fraction() <= 1.0);
  const Json_double null_values(get_null_values_fraction());
  if (json_object->add_clone(null_values_str(), &null_values))
    return true; /* purecov: inspected */

  // charset-id
  const Json_uint charset_id(get_character_set()->number);
  if (json_object->add_clone(collation_id_str(), &charset_id))
    return true; /* purecov: inspected */
  return false;
}

double Histogram::get_null_values_fraction() const {
  if (m_null_values_fraction != INVALID_NULL_VALUES_FRACTION) {
    DBUG_ASSERT(m_null_values_fraction >= 0.0);
    DBUG_ASSERT(m_null_values_fraction <= 1.0);
  }

  return m_null_values_fraction;
}

template <class T>
Histogram *build_histogram(MEM_ROOT *mem_root, const Value_map<T> &value_map,
                           size_t num_buckets, const std::string &db_name,
                           const std::string &tbl_name,
                           const std::string &col_name) {
  Histogram *histogram = nullptr;

  /*
    If the number of buckets specified is greater or equal to the number
    of distinct values, we create a Singleton histogram. Otherwise we create
    an equi-height histogram.
  */
  if (num_buckets >= value_map.size()) {
    Singleton<T> *singleton = new (mem_root) Singleton<T>(
        mem_root, db_name, tbl_name, col_name, value_map.get_data_type());

    if (singleton == nullptr) return nullptr;

    if (singleton->build_histogram(value_map, num_buckets))
      return nullptr; /* purecov: inspected */

    histogram = singleton;
  } else {
    Equi_height<T> *equi_height = new (mem_root) Equi_height<T>(
        mem_root, db_name, tbl_name, col_name, value_map.get_data_type());

    if (equi_height == nullptr) return nullptr;

    if (equi_height->build_histogram(value_map, num_buckets))
      return nullptr; /* purecov: inspected */

    histogram = equi_height;
  }

  // We should not have a nullptr at this point.
  DBUG_ASSERT(histogram != nullptr);

  // Verify that the original number of buckets specified is set.
  DBUG_ASSERT(histogram->get_num_buckets_specified() == num_buckets);

  // Verify that we haven't created more buckets than requested.
  DBUG_ASSERT(histogram->get_num_buckets() <= num_buckets);

  // Ensure that the character set is set.
  DBUG_ASSERT(histogram->get_character_set() != nullptr);

  // Check that the fraction of NULL values has been set properly.
  DBUG_ASSERT(histogram->get_null_values_fraction() >= 0.0);
  DBUG_ASSERT(histogram->get_null_values_fraction() <= 1.0);

  return histogram;
}

Histogram *Histogram::json_to_histogram(MEM_ROOT *mem_root,
                                        const std::string &schema_name,
                                        const std::string &table_name,
                                        const std::string &column_name,
                                        const Json_object &json_object) {
  // Histogram type (equi-height or singleton).
  const Json_dom *histogram_type_dom =
      json_object.get(Histogram::histogram_type_str());
  if (histogram_type_dom == nullptr ||
      histogram_type_dom->json_type() != enum_json_type::J_STRING) {
    return nullptr; /* purecov: deadcode */
  }

  // Histogram data type
  const Json_dom *data_type_dom = json_object.get(Histogram::data_type_str());
  if (data_type_dom == nullptr ||
      data_type_dom->json_type() != enum_json_type::J_STRING) {
    return nullptr; /* purecov: deadcode */
  }

  const Json_string *histogram_type =
      down_cast<const Json_string *>(histogram_type_dom);
  const Json_string *data_type = down_cast<const Json_string *>(data_type_dom);

  Histogram *histogram = nullptr;
  if (histogram_type->value() == Histogram::equi_height_str()) {
    // Equi-height histogram
    if (data_type->value() == "double") {
      histogram = new (mem_root)
          Equi_height<double>(mem_root, schema_name, table_name, column_name,
                              Value_map_type::DOUBLE);
    } else if (data_type->value() == "int") {
      histogram = new (mem_root) Equi_height<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::INT);
    } else if (data_type->value() == "enum") {
      histogram = new (mem_root) Equi_height<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::ENUM);
    } else if (data_type->value() == "set") {
      histogram = new (mem_root) Equi_height<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::SET);
    } else if (data_type->value() == "uint") {
      histogram = new (mem_root) Equi_height<ulonglong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::UINT);
    } else if (data_type->value() == "string") {
      histogram = new (mem_root)
          Equi_height<String>(mem_root, schema_name, table_name, column_name,
                              Value_map_type::STRING);
    } else if (data_type->value() == "date") {
      histogram = new (mem_root) Equi_height<MYSQL_TIME>(
          mem_root, schema_name, table_name, column_name, Value_map_type::DATE);
    } else if (data_type->value() == "time") {
      histogram = new (mem_root) Equi_height<MYSQL_TIME>(
          mem_root, schema_name, table_name, column_name, Value_map_type::TIME);
    } else if (data_type->value() == "datetime") {
      histogram = new (mem_root)
          Equi_height<MYSQL_TIME>(mem_root, schema_name, table_name,
                                  column_name, Value_map_type::DATETIME);
    } else if (data_type->value() == "decimal") {
      histogram = new (mem_root)
          Equi_height<my_decimal>(mem_root, schema_name, table_name,
                                  column_name, Value_map_type::DECIMAL);
    } else {
      return nullptr; /* purecov: deadcode */
    }
  } else if (histogram_type->value() == Histogram::singleton_str()) {
    // Singleton histogram
    if (data_type->value() == "double") {
      histogram =
          new (mem_root) Singleton<double>(mem_root, schema_name, table_name,
                                           column_name, Value_map_type::DOUBLE);
    } else if (data_type->value() == "int") {
      histogram = new (mem_root) Singleton<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::INT);
    } else if (data_type->value() == "enum") {
      histogram = new (mem_root) Singleton<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::ENUM);
    } else if (data_type->value() == "set") {
      histogram = new (mem_root) Singleton<longlong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::SET);
    } else if (data_type->value() == "uint") {
      histogram = new (mem_root) Singleton<ulonglong>(
          mem_root, schema_name, table_name, column_name, Value_map_type::UINT);
    } else if (data_type->value() == "string") {
      histogram =
          new (mem_root) Singleton<String>(mem_root, schema_name, table_name,
                                           column_name, Value_map_type::STRING);
    } else if (data_type->value() == "datetime") {
      histogram = new (mem_root)
          Singleton<MYSQL_TIME>(mem_root, schema_name, table_name, column_name,
                                Value_map_type::DATETIME);
    } else if (data_type->value() == "date") {
      histogram = new (mem_root) Singleton<MYSQL_TIME>(
          mem_root, schema_name, table_name, column_name, Value_map_type::DATE);
    } else if (data_type->value() == "time") {
      histogram = new (mem_root) Singleton<MYSQL_TIME>(
          mem_root, schema_name, table_name, column_name, Value_map_type::TIME);
    } else if (data_type->value() == "decimal") {
      histogram = new (mem_root)
          Singleton<my_decimal>(mem_root, schema_name, table_name, column_name,
                                Value_map_type::DECIMAL);
    } else {
      return nullptr; /* purecov: deadcode */
    }
  } else {
    // Unsupported histogram type.
    return nullptr; /* purecov: deadcode */
  }

  if (histogram != nullptr && histogram->json_to_histogram(json_object))
    return nullptr; /* purecov: deadcode */
  return histogram;
}

/*
  All subclasses should also call this function in order to populate fields that
  are shared among all histogram types (character set, null values fraction).
*/
bool Histogram::json_to_histogram(const Json_object &json_object) {
  // The sampling rate that was used to create the histogram.
  const Json_dom *sampling_rate_dom = json_object.get(sampling_rate_str());
  if (sampling_rate_dom == nullptr ||
      sampling_rate_dom->json_type() != enum_json_type::J_DOUBLE) {
    return true; /* purecov: deadcode */
  }
  const Json_double *sampling_rate =
      down_cast<const Json_double *>(sampling_rate_dom);
  m_sampling_rate = sampling_rate->value();

  // The number of buckets originally specified by the user.
  const Json_dom *num_buckets_specified_dom =
      json_object.get(numer_of_buckets_specified_str());
  if (num_buckets_specified_dom == nullptr ||
      num_buckets_specified_dom->json_type() != enum_json_type::J_INT) {
    return true; /* purecov: deadcode */
  }
  const Json_int *num_buckets_specified =
      down_cast<const Json_int *>(num_buckets_specified_dom);
  m_num_buckets_specified = num_buckets_specified->value();

  // Fraction of SQL null-values in the original data set.
  const Json_dom *null_values_dom = json_object.get(null_values_str());
  if (null_values_dom == nullptr ||
      null_values_dom->json_type() != enum_json_type::J_DOUBLE) {
    return true; /* purecov: deadcode */
  }
  const Json_double *null_values =
      down_cast<const Json_double *>(null_values_dom);
  m_null_values_fraction = null_values->value();

  // Character set ID
  const Json_dom *charset_id_dom = json_object.get(collation_id_str());
  if (charset_id_dom == nullptr ||
      charset_id_dom->json_type() != enum_json_type::J_UINT) {
    return true; /* purecov: deadcode */
  }
  const Json_uint *charset_id = down_cast<const Json_uint *>(charset_id_dom);

  // Get the charset (my_sys.h)
  m_charset = get_charset(static_cast<uint>(charset_id->value()), MYF(0));

  return false;
}

static std::map<const Value_map_type, const std::string> value_map_type_to_str =
    {{Value_map_type::DATETIME, "datetime"}, {Value_map_type::DATE, "date"},
     {Value_map_type::TIME, "time"},         {Value_map_type::INT, "int"},
     {Value_map_type::UINT, "uint"},         {Value_map_type::DOUBLE, "double"},
     {Value_map_type::DECIMAL, "decimal"},   {Value_map_type::STRING, "string"},
     {Value_map_type::ENUM, "enum"},         {Value_map_type::SET, "set"}};

bool Histogram::histogram_data_type_to_json(Json_object *json_object) const {
  std::string foo = value_map_type_to_str[get_data_type()];
  const Json_string json_value(foo);
  return json_object->add_clone(data_type_str(), &json_value);
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom, double *out) {
  if (json_dom->json_type() != enum_json_type::J_DOUBLE)
    return true; /* purecov: deadcode */
  *out = down_cast<const Json_double *>(json_dom)->value();
  return false;
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom, String *out) {
  DBUG_ASSERT(get_character_set() != nullptr);
  if (json_dom->json_type() != enum_json_type::J_OPAQUE)
    return true; /* purecov: deadcode */
  const Json_opaque *json_opaque = down_cast<const Json_opaque *>(json_dom);

  String value(json_opaque->value(), json_opaque->size(), get_character_set());

  /*
    Make a copy of the data, since the JSON opaque will free it before we need
    it.
  */
  char *value_dup_data = value.dup(get_mem_root());
  if (value_dup_data == nullptr) {
    DBUG_ASSERT(false); /* purecov: deadcode */
    return true;        // OOM
  }

  out->set(value_dup_data, value.length(), value.charset());
  return false;
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom,
                                       ulonglong *out) {
  if (json_dom->json_type() != enum_json_type::J_UINT)
    return true; /* purecov: deadcode */
  *out = down_cast<const Json_uint *>(json_dom)->value();
  return false;
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom,
                                       longlong *out) {
  if (json_dom->json_type() != enum_json_type::J_INT)
    return true; /* purecov: deadcode */
  *out = down_cast<const Json_int *>(json_dom)->value();
  return false;
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom,
                                       MYSQL_TIME *out) {
  if (json_dom->json_type() != enum_json_type::J_DATE &&
      json_dom->json_type() != enum_json_type::J_TIME &&
      json_dom->json_type() != enum_json_type::J_DATETIME &&
      json_dom->json_type() != enum_json_type::J_TIMESTAMP)
    return true; /* purecov: deadcode */
  *out = *down_cast<const Json_datetime *>(json_dom)->value();
  return false;
}

template <>
bool Histogram::extract_json_dom_value(const Json_dom *json_dom,
                                       my_decimal *out) {
  if (json_dom->json_type() != enum_json_type::J_DECIMAL)
    return true; /* purecov: deadcode */
  *out = *down_cast<const Json_decimal *>(json_dom)->value();
  return false;
}

/**
  Check if a field is covered by a single-part unique index (primary key or
  unique index). Indexes that are marked as invisible are ignored.

  @param thd The current session.
  @param field The field to check.

  @return true if the field is covered by a single-part unique index. False
          otherwise.
*/
static bool covered_by_single_part_index(const THD *thd, const Field *field) {
  Key_map possible_keys;
  possible_keys.merge(field->table->s->usable_indexes(thd));
  possible_keys.intersect(field->key_start);
  DBUG_ASSERT(field->table->s->keys <= possible_keys.length());
  for (uint i = 0; i < field->table->s->keys; ++i) {
    if (possible_keys.is_set(i) &&
        field->table->s->key_info[i].user_defined_key_parts == 1 &&
        (field->table->s->key_info[i].flags & HA_NOSAME)) {
      return true;
    }
  }

  return false;
}

/**
  Prepare one Value_map for each field we are creating histogram statistics for.
  We will also estimate how many bytes one row will consume. For example, if we
  are creating histogram statistics for two INTEGER columns, we estimate that
  one row will consume (sizeof(longlong) * 2) bytes (16 bytes).

  @param fields              A vector with all the fields we are creating
                             histogram statistics for.
  @param[out] value_maps     A map where the Value_maps will be initialized.
  @param[out] row_size_bytes An estimation of how many bytes one row will
                             consume.

  @return true on error, false otherwise.
*/
static bool prepare_value_maps(
    std::vector<Field *, Histogram_key_allocator<Field *>> &fields,
    value_map_collection &value_maps, size_t *row_size_bytes) {
  *row_size_bytes = 0;
  for (const Field *field : fields) {
    histograms::Value_map_base *value_map = nullptr;

    const Value_map_type value_map_type =
        histograms::field_type_to_value_map_type(field);

    switch (value_map_type) {
      case histograms::Value_map_type::STRING: {
        size_t max_field_length =
            std::min(static_cast<size_t>(field->field_length),
                     histograms::HISTOGRAM_MAX_COMPARE_LENGTH);
        *row_size_bytes += max_field_length * field->charset()->mbmaxlen;
        value_map =
            new histograms::Value_map<String>(field->charset(), value_map_type);
        break;
      }
      case histograms::Value_map_type::DOUBLE: {
        value_map =
            new histograms::Value_map<double>(field->charset(), value_map_type);
        break;
      }
      case histograms::Value_map_type::INT:
      case histograms::Value_map_type::ENUM:
      case histograms::Value_map_type::SET: {
        value_map = new histograms::Value_map<longlong>(field->charset(),
                                                        value_map_type);
        break;
      }
      case histograms::Value_map_type::UINT: {
        value_map = new histograms::Value_map<ulonglong>(field->charset(),
                                                         value_map_type);
        break;
      }
      case histograms::Value_map_type::DATETIME:
      case histograms::Value_map_type::DATE:
      case histograms::Value_map_type::TIME: {
        value_map = new histograms::Value_map<MYSQL_TIME>(field->charset(),
                                                          value_map_type);
        break;
      }
      case histograms::Value_map_type::DECIMAL: {
        value_map = new histograms::Value_map<my_decimal>(field->charset(),
                                                          value_map_type);
        break;
      }
      case histograms::Value_map_type::INVALID: {
        DBUG_ASSERT(false); /* purecov: deadcode */
        return true;
      }
    }

    // Overhead for each element
    *row_size_bytes += value_map->element_overhead();

    value_maps.emplace(field->field_index,
                       std::unique_ptr<histograms::Value_map_base>(value_map));
  }

  return false;
}

/**
  Read data from a table into the provided Value_maps. We will read data using
  sampling with the provided sampling percentage.

  @param fields            A vector with the fields we are reading data from.
  @param sample_percentage The sampling percentage we will use for sampling.
                           Must be between 0.0 and 100.0.
  @param table             The table we are reading the data from.
  @param value_maps        The Value_maps we are reading data into.

  @return true on error, false otherwise.
*/
static bool fill_value_maps(
    const std::vector<Field *, Histogram_key_allocator<Field *>> &fields,
    double sample_percentage, const TABLE *table,
    value_map_collection &value_maps) {
  DBUG_ASSERT(sample_percentage > 0.0);
  DBUG_ASSERT(sample_percentage <= 100.0);
  DBUG_ASSERT(fields.size() == value_maps.size());

  std::random_device rd;
  std::uniform_int_distribution<int> dist;
  int sampling_seed = dist(rd);

  DBUG_EXECUTE_IF("histogram_force_sampling", {
    sampling_seed = 1;
    sample_percentage = 50.0;
  });

  void *scan_ctx = nullptr;

  for (auto &value_map : value_maps)
    value_map.second->set_sampling_rate(sample_percentage / 100.0);

  if (table->file->ha_sample_init(scan_ctx, sample_percentage, sampling_seed,
                                  enum_sampling_method::SYSTEM)) {
    return true;
  }

  auto handler_guard = create_scope_guard([table, scan_ctx]() {
    table->file->ha_sample_end(scan_ctx); /* purecov: deadcode */
  });

  // Read the data from each column into its own Value_map.
  int res = table->file->ha_sample_next(scan_ctx, table->record[0]);

  while (res == 0) {
    for (Field *field : fields) {
      histograms::Value_map_base *value_map =
          value_maps.at(field->field_index).get();

      switch (histograms::field_type_to_value_map_type(field)) {
        case histograms::Value_map_type::STRING: {
          StringBuffer<MAX_FIELD_WIDTH> str_buf(field->charset());
          field->val_str(&str_buf);

          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(static_cast<String>(str_buf), 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::DOUBLE: {
          double value = field->val_real();
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::INT:
        case histograms::Value_map_type::ENUM:
        case histograms::Value_map_type::SET: {
          longlong value = field->val_int();
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::UINT: {
          ulonglong value = static_cast<ulonglong>(field->val_int());
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::DATE: {
          MYSQL_TIME time_value;
          TIME_from_longlong_date_packed(&time_value,
                                         field->val_date_temporal());
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(time_value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::TIME: {
          MYSQL_TIME time_value;
          TIME_from_longlong_time_packed(&time_value,
                                         field->val_time_temporal());
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(time_value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::DATETIME: {
          MYSQL_TIME time_value;
          TIME_from_longlong_datetime_packed(&time_value,
                                             field->val_date_temporal());
          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(time_value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::DECIMAL: {
          my_decimal buffer;
          my_decimal *value;
          value = field->val_decimal(&buffer);

          if (field->is_null())
            value_map->add_null_values(1);
          else if (value_map->add_values(*value, 1))
            return true; /* purecov: deadcode */
          break;
        }
        case histograms::Value_map_type::INVALID: {
          DBUG_ASSERT(false); /* purecov: deadcode */
          break;
        }
      }
    }

    res = table->file->ha_sample_next(scan_ctx, table->record[0]);

    DBUG_EXECUTE_IF(
        "sample_read_sample_half", static uint count = 1;
        if (count == std::max(1ULL, table->file->stats.records) / 2) {
          res = HA_ERR_END_OF_FILE;
          break;
        } ++count;);
  }

  if (res != HA_ERR_END_OF_FILE) return true; /* purecov: deadcode */

  // Close the handler
  handler_guard.commit();
  if (table->file->ha_sample_end(scan_ctx)) {
    DBUG_ASSERT(false); /* purecov: deadcode */
    return true;
  }

  return false;
}

bool update_histogram(THD *thd, TABLE_LIST *table, const columns_set &columns,
                      int num_buckets, results_map &results) {
  dd::cache::Dictionary_client::Auto_releaser auto_releaser(thd->dd_client());

  // Read only should have been stopped at an earlier stage.
  DBUG_ASSERT(!check_readonly(thd, false));
  DBUG_ASSERT(!thd->tx_read_only);

  DBUG_ASSERT(results.empty());
  DBUG_ASSERT(!columns.empty());

  // Only one table should be specified in ANALYZE TABLE .. UPDATE HISTOGRAM
  DBUG_ASSERT(table->next_local == nullptr);

  if (table->table != nullptr && table->table->s->tmp_table != NO_TMP_TABLE) {
    /*
      Normally, the table we are going to read data from is not initialized at
      this point. But if table->table is not a null-pointer, it has already been
      initialized at an earlier stage. This will happen if the table is a
      temporary table.
    */
    results.emplace("", Message::TEMPORARY_TABLE);
    return true;
  }

  /*
    Create two scope guards; one for disabling autocommit and one that will do a
    rollback and ensure that any open tables are closed before returning.
  */
  Disable_autocommit_guard autocommit_guard(thd);
  auto tables_guard = create_scope_guard([thd]() {
    if (trans_rollback_stmt(thd) || trans_rollback(thd))
      DBUG_ASSERT(false); /* purecov: deadcode */
    close_thread_tables(thd);
  });

  table->reinit_before_use(thd);
  if (open_and_lock_tables(thd, table, 0)) {
    return true;
  }

  DBUG_EXECUTE_IF("histogram_fail_after_open_table", { return true; });

  if (table->is_view()) {
    results.emplace("", Message::VIEW);
    return true;
  }

  DBUG_ASSERT(table->table != nullptr);
  TABLE *tbl = table->table;

  if (tbl->s->encrypt_type.length > 0 &&
      my_strcasecmp(system_charset_info, "n", tbl->s->encrypt_type.str) != 0) {
    results.emplace("", Message::ENCRYPTED_TABLE);
    return true;
  }

  /*
    Check if the provided column names exist, and that they have a supported
    data type. If they do, mark them in the read set.
  */
  bitmap_clear_all(tbl->write_set);
  bitmap_clear_all(tbl->read_set);
  std::vector<Field *, Histogram_key_allocator<Field *>> resolved_fields;

  for (const std::string &column_name : columns) {
    Field *field = find_field_in_table_sef(tbl, column_name.c_str());

    if (field == nullptr) {
      // Field not found in table
      results.emplace(column_name, Message::FIELD_NOT_FOUND);
      continue;
    } else if (histograms::field_type_to_value_map_type(field) ==
               histograms::Value_map_type::INVALID) {
      // Unsupported data type
      results.emplace(column_name, Message::UNSUPPORTED_DATA_TYPE);
      continue;
    }

    /*
      Check if this field is covered by a single-part unique index. If it is, we
      don't want to create histogram statistics for it.
    */
    if (covered_by_single_part_index(thd, field)) {
      results.emplace(column_name,
                      Message::COVERED_BY_SINGLE_PART_UNIQUE_INDEX);
      continue;
    }
    resolved_fields.push_back(field);

    bitmap_set_bit(tbl->read_set, field->field_index);
    if (field->is_gcol()) {
      bitmap_set_bit(tbl->write_set, field->field_index);
      /*
        The base columns needs to be in the write set in case of nested
        generated columns:

        CREATE TABLE t1 (
          col1 INT,
          col2 INT AS (col1 + 1) VIRTUAL,
          col3 INT AS (col2 + 1) VIRTUAL);

        If we are reading data from "col3", we also need to update the data in
        "col2" in order for the generated value to be correct.
      */
      bitmap_union(tbl->write_set, &field->gcol_info->base_columns_map);
      bitmap_union(tbl->read_set, &field->gcol_info->base_columns_map);
    }
  }

  /*
    If we don't have any fields, we just quit here. Return "true" so we don't
    write empty transactions/statements to the binlog.
  */
  if (resolved_fields.empty()) return true;

  /*
    Prepare one Value_map for each field we are creating histogram statistics
    for. Also, estimate how many bytes one row will consume so that we can
    estimate how many rows we can fit into memory permitted by
    histogram_generation_max_mem_size.
  */
  size_t row_size_bytes = 0;
  value_map_collection value_maps;
  if (prepare_value_maps(resolved_fields, value_maps, &row_size_bytes))
    return true; /* purecov: deadcode */

  /*
    Caclulate how many rows we can fit into memory permitted by
    histogram_generation_max_mem_size.
  */
  double rows_in_memory = thd->variables.histogram_generation_max_mem_size /
                          static_cast<double>(row_size_bytes);

  /*
    Ensure that we estimate at least one row in the table, so we avoid
    division by zero error.

    NOTE: We ignore errors from "fetch_number_of_rows()" on purpose, since we
    don't consider it fatal not having the correct row estimate.
  */
  table->fetch_number_of_rows();
  ha_rows rows_in_table = std::max(1ULL, tbl->file->stats.records);

  double sample_percentage = rows_in_memory / rows_in_table * 100.0;
  sample_percentage = std::min(sample_percentage, 100.0);

  // Read data from the table into the Value_maps we have prepared.
  if (fill_value_maps(resolved_fields, sample_percentage, tbl, value_maps))
    return true; /* purecov: deadcode */

  // Create a histogram for each Value_map, and store it to persistent storage.
  for (const Field *field : resolved_fields) {
    /*
      The MEM_ROOT is transferred to the dictionary object when
      histogram->store_histogram is called.
    */
    MEM_ROOT local_mem_root;
    init_alloc_root(key_memory_histograms, &local_mem_root, 256, 0);

    std::string col_name(field->field_name);
    histograms::Histogram *histogram =
        value_maps.at(field->field_index)
            ->build_histogram(
                &local_mem_root, num_buckets,
                std::string(table->db, table->db_length),
                std::string(table->table_name, table->table_name_length),
                col_name);

    if (histogram == nullptr) {
      /* purecov: begin inspected */
      my_error(ER_UNABLE_TO_BUILD_HISTOGRAM, MYF(0), field->field_name,
               table->db, table->table_name);
      return true;
      /* purecov: end */
    } else if (histogram->store_histogram(thd)) {
      // errors have already been reported
      return true; /* purecov: deadcode */
    }

    results.emplace(col_name, Message::HISTOGRAM_CREATED);
  }

  bool ret = trans_commit_stmt(thd) || trans_commit(thd);
  close_thread_tables(thd);
  tables_guard.commit();
  return ret;
}

bool drop_all_histograms(THD *thd, const TABLE_LIST &table,
                         const dd::Table &table_definition,
                         results_map &results) {
  columns_set columns;
  for (const auto &col : table_definition.columns())
    columns.emplace(col->name().c_str());

  return drop_histograms(thd, table, columns, results);
}

bool drop_histograms(THD *thd, const TABLE_LIST &table,
                     const columns_set &columns, results_map &results) {
  dd::cache::Dictionary_client *client = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser auto_releaser(client);

  for (const std::string &column_name : columns) {
    MDL_key mdl_key;
    dd::Column_statistics::create_mdl_key(
        {table.db, table.db_length},
        {table.table_name, table.table_name_length}, column_name.c_str(),
        &mdl_key);

    if (lock_for_write(thd, mdl_key))
      return true;  // error is already reported.

    dd::String_type dd_name = dd::Column_statistics::create_name(
        {table.db, table.db_length},
        {table.table_name, table.table_name_length}, column_name.c_str());

    // Do we have an existing histogram for this column?
    const dd::Column_statistics *column_statistics = nullptr;
    if (client->acquire(dd_name, &column_statistics)) {
      // error is already reported.
      return true; /* purecov: deadcode */
    }

    if (column_statistics == nullptr) {
      results.emplace(column_name, Message::NO_HISTOGRAM_FOUND);
      continue;
    }

    if (client->drop(column_statistics)) {
      /* purecov: begin inspected */
      my_error(ER_UNABLE_TO_DROP_COLUMN_STATISTICS, MYF(0), column_name.c_str(),
               table.db, table.table_name);
      return true;
      /* purecov: end */
    }

    results.emplace(column_name, Message::HISTOGRAM_DELETED);
  }

  return false;
}

bool Histogram::store_histogram(THD *thd) const {
  dd::cache::Dictionary_client *client = thd->dd_client();

  MDL_key mdl_key;
  dd::Column_statistics::create_mdl_key(get_database_name().str,
                                        get_table_name().str,
                                        get_column_name().str, &mdl_key);

  if (lock_for_write(thd, mdl_key)) {
    // Error has already been reported
    return true; /* purecov: deadcode */
  }

  DEBUG_SYNC(thd, "store_histogram_after_write_lock");

  dd::String_type dd_name = dd::Column_statistics::create_name(
      get_database_name().str, get_table_name().str, get_column_name().str);

  // Do we have an existing histogram for this column?
  dd::Column_statistics *column_stats = nullptr;
  if (client->acquire_for_modification(dd_name, &column_stats)) {
    // Error has already been reported
    return true; /* purecov: deadcode */
  }

  if (column_stats != nullptr) {
    // Update the existing object.
    column_stats->set_histogram(this);
    if (client->update(column_stats)) {
      /* purecov: begin inspected */
      my_error(ER_UNABLE_TO_UPDATE_COLUMN_STATISTICS, MYF(0),
               get_column_name().str, get_database_name().str,
               get_table_name().str);
      return true;
      /* purecov: end */
    }
  } else {
    // Create a new object
    std::unique_ptr<dd::Column_statistics> column_statistics(
        dd::create_object<dd::Column_statistics>());

    column_statistics.get()->set_schema_name(get_database_name().str);
    column_statistics.get()->set_table_name(get_table_name().str);
    column_statistics.get()->set_column_name(get_column_name().str);
    column_statistics.get()->set_name(dd_name);
    column_statistics.get()->set_histogram(this);

    if (client->store(column_statistics.get())) {
      /* purecov: begin inspected */
      my_error(ER_UNABLE_TO_STORE_COLUMN_STATISTICS, MYF(0),
               get_column_name().str, get_database_name().str,
               get_table_name().str);
      return true;
      /* purecov: end */
    }
  }

  return false;
}

/**
  Rename a single histogram from a old schema/table name to a new schema/table
  name. It is used for instance by RENAME TABLE, where the contents of the
  histograms doesn't change.

  @param thd             Thread handler.
  @param old_schema_name The old schema name.
  @param old_table_name  The old table name.
  @param new_schema_name The new schema name.
  @param new_table_name  The new table name.
  @param column_name     The column name.
  @param results         A map where the result of the operation is stored.

  @return false on success, true on error.
*/
static bool rename_histogram(THD *thd, const char *old_schema_name,
                             const char *old_table_name,
                             const char *new_schema_name,
                             const char *new_table_name,
                             const char *column_name, results_map &results) {
  dd::cache::Dictionary_client *client = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser auto_releaser(client);

  // First find the histogram with the old name.
  MDL_key mdl_key;
  dd::Column_statistics::create_mdl_key(old_schema_name, old_table_name,
                                        column_name, &mdl_key);

  if (lock_for_write(thd, mdl_key)) {
    // Error has already been reported
    return true; /* purecov: deadcode */
  }

  dd::String_type dd_name = dd::Column_statistics::create_name(
      old_schema_name, old_table_name, column_name);

  dd::Column_statistics *column_statistics = nullptr;
  if (client->acquire_for_modification(dd_name, &column_statistics)) {
    // Error has already been reported
    return true; /* purecov: deadcode */
  }

  if (column_statistics == nullptr) {
    results.emplace(column_name, Message::NO_HISTOGRAM_FOUND);
    return false;
  }

  dd::Column_statistics::create_mdl_key(new_schema_name, new_table_name,
                                        column_name, &mdl_key);

  if (lock_for_write(thd, mdl_key)) {
    // Error has already been reported
    return true; /* purecov: deadcode */
  }

  column_statistics->set_schema_name(new_schema_name);
  column_statistics->set_table_name(new_table_name);
  column_statistics->set_column_name(column_name);
  column_statistics->set_name(column_statistics->create_name());
  if (client->update(column_statistics)) {
    /* purecov: begin inspected */
    my_error(ER_UNABLE_TO_UPDATE_COLUMN_STATISTICS, MYF(0), column_name,
             old_schema_name, old_table_name);
    return true;
    /* purecov: end */
  }

  results.emplace(column_name, Message::HISTOGRAM_DELETED);
  return false;
}

bool rename_histograms(THD *thd, const char *old_schema_name,
                       const char *old_table_name, const char *new_schema_name,
                       const char *new_table_name, results_map &results) {
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, old_schema_name,
                   old_table_name, MDL_SHARED_READ_ONLY, MDL_TRANSACTION);

  if (thd->mdl_context.acquire_lock_nsec(
          &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
    // error has already been reported
    return true; /* purecov: deadcode */
  }

  /*
    We have to look up the new table since it already will be renamed at this
    point.
  */
  const dd::Table *table_def = nullptr;
  if (thd->dd_client()->acquire(new_schema_name, new_table_name, &table_def)) {
    // error has already been reported
    return false; /* purecov: deadcode */
  }

  if (table_def == nullptr) {
    DBUG_ASSERT(false); /* purecov: deadcode */
    return false;
  }

  for (const auto &col : table_def->columns()) {
    if (rename_histogram(thd, old_schema_name, old_table_name, new_schema_name,
                         new_table_name, col->name().c_str(), results))
      return true; /* purecov: deadcode */
  }

  return false;
}

bool find_histogram(THD *thd, const std::string &schema_name,
                    const std::string &table_name,
                    const std::string &column_name,
                    const Histogram **histogram) {
  DBUG_ASSERT(*histogram == nullptr);

  if (schema_name == "mysql" || table_name == "column_statistics") return false;

  dd::String_type dd_name = dd::Column_statistics::create_name(
      schema_name.c_str(), table_name.c_str(), column_name.c_str());

  const dd::Column_statistics *column_statistics = nullptr;
  dd::cache::Dictionary_client *client = thd->dd_client();
  if (client->acquire<dd::Column_statistics>(dd_name, &column_statistics))
    return true; /* purecov: deadcode */

  if (column_statistics == nullptr) return false;

  *histogram = column_statistics->histogram();
  return false;
}

template <class T>
double Histogram::get_less_than_selectivity_dispatcher(const T &value) const {
  switch (get_histogram_type()) {
    case enum_histogram_type::SINGLETON: {
      const Singleton<T> *singleton = down_cast<const Singleton<T> *>(this);
      return singleton->get_less_than_selectivity(value);
    }
    case enum_histogram_type::EQUI_HEIGHT: {
      const Equi_height<T> *equi_height =
          down_cast<const Equi_height<T> *>(this);
      return equi_height->get_less_than_selectivity(value);
    }
  }
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return 0.0;
  /* purecov: end deadcode */
}

template <class T>
double Histogram::get_greater_than_selectivity_dispatcher(
    const T &value) const {
  switch (get_histogram_type()) {
    case enum_histogram_type::SINGLETON: {
      const Singleton<T> *singleton = down_cast<const Singleton<T> *>(this);
      return singleton->get_greater_than_selectivity(value);
    }
    case enum_histogram_type::EQUI_HEIGHT: {
      const Equi_height<T> *equi_height =
          down_cast<const Equi_height<T> *>(this);
      return equi_height->get_greater_than_selectivity(value);
    }
  }
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return 0.0;
  /* purecov: end deadcode */
}

template <class T>
double Histogram::get_equal_to_selectivity_dispatcher(const T &value) const {
  switch (get_histogram_type()) {
    case enum_histogram_type::SINGLETON: {
      const Singleton<T> *singleton = down_cast<const Singleton<T> *>(this);
      return singleton->get_equal_to_selectivity(value);
    }
    case enum_histogram_type::EQUI_HEIGHT: {
      const Equi_height<T> *equi_height =
          down_cast<const Equi_height<T> *>(this);
      return equi_height->get_equal_to_selectivity(value);
    }
  }
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return 0.0;
  /* purecov: end deadcode */
}

static bool get_temporal(Item *item, Value_map_type preferred_type,
                         MYSQL_TIME *time_value) {
  if (item->is_temporal_with_date_and_time()) {
    TIME_from_longlong_datetime_packed(time_value, item->val_date_temporal());
  } else if (item->is_temporal_with_date()) {
    TIME_from_longlong_date_packed(time_value, item->val_date_temporal());
  } else if (item->is_temporal_with_time()) {
    TIME_from_longlong_time_packed(time_value, item->val_time_temporal());
  } else {
    switch (preferred_type) {
      case Value_map_type::DATE:
      case Value_map_type::DATETIME:
        if (item->get_date_from_non_temporal(time_value, 0)) return true;
        break;
      case Value_map_type::TIME:
        if (item->get_time_from_non_temporal(time_value)) return true;
        break;
      default:
        /* purecov: begin deadcode */
        DBUG_ASSERT(0);
        break;
        /* purecov: end deadcode */
    }
  }

  return false;
}

template <class T>
double Histogram::apply_operator(const enum_operator op, const T &value) const {
  switch (op) {
    case enum_operator::LESS_THAN:
      return get_less_than_selectivity_dispatcher(value);
    case enum_operator::GREATER_THAN:
      return get_greater_than_selectivity_dispatcher(value);
    case enum_operator::EQUALS_TO:
      return get_equal_to_selectivity_dispatcher(value);
    default:
      /* purecov: begin deadcode */
      DBUG_ASSERT(false);
      return 1.0;
      /* purecov: end deadcode */
  }
}

bool Histogram::get_selectivity_dispatcher(Item *item, const enum_operator op,
                                           const TYPELIB *typelib,
                                           double *selectivity) const {
  switch (this->get_data_type()) {
    case Value_map_type::INVALID: {
      /* purecov: begin deadcode */
      DBUG_ASSERT(false);
      return true;
      /* purecov: end deadcode */
    }
    case Value_map_type::STRING: {
      // Is the character set the same? If not, we cannot use the histogram
      if (item->collation.collation->number != get_character_set()->number)
        return true;

      StringBuffer<MAX_FIELD_WIDTH> str_buf(item->collation.collation);
      const String *str = item->val_str(&str_buf);
      if (item->is_null()) return true;

      *selectivity =
          apply_operator(op, str->substr(0, HISTOGRAM_MAX_COMPARE_LENGTH));
      return false;
    }
    case Value_map_type::INT: {
      const longlong value = item->val_int();
      if (item->is_null()) return true;

      *selectivity = apply_operator(op, value);
      return false;
    }
    case Value_map_type::ENUM: {
      DBUG_ASSERT(typelib != nullptr);

      longlong value;
      if (item->data_type() == MYSQL_TYPE_VARCHAR) {
        StringBuffer<MAX_FIELD_WIDTH> str_buf(item->collation.collation);
        const String *str = item->val_str(&str_buf);
        if (item->is_null()) return true;

        // Remove any trailing whitespace
        size_t length = str->charset()->cset->lengthsp(
            str->charset(), str->ptr(), str->length());
        value = find_type2(typelib, str->ptr(), length, str->charset());
      } else {
        value = item->val_int();
        if (item->is_null()) return true;
      }

      if (op == enum_operator::EQUALS_TO) {
        *selectivity = get_equal_to_selectivity_dispatcher(value);
        return false;
      }

      return true; /* purecov: deadcode */
    }
    case Value_map_type::SET: {
      DBUG_ASSERT(typelib != nullptr);

      longlong value;
      if (item->data_type() == MYSQL_TYPE_VARCHAR) {
        StringBuffer<MAX_FIELD_WIDTH> str_buf(item->collation.collation);
        const String *str = item->val_str(&str_buf);
        if (item->is_null()) return true;

        bool got_warning;
        const char *not_used;
        uint not_used2;
        ulonglong tmp_value =
            find_set(typelib, str->ptr(), str->length(), str->charset(),
                     &not_used, &not_used2, &got_warning);

        value = static_cast<ulonglong>(tmp_value);
      } else {
        value = item->val_int();
        if (item->is_null()) return true;
      }

      if (op == enum_operator::EQUALS_TO) {
        *selectivity = get_equal_to_selectivity_dispatcher(value);
        return false;
      }

      return true; /* purecov: deadcode */
    }
    case Value_map_type::UINT: {
      const ulonglong value = static_cast<ulonglong>(item->val_int());
      if (item->is_null()) return true;

      *selectivity = apply_operator(op, value);
      return false;
    }
    case Value_map_type::DOUBLE: {
      const double value = item->val_real();
      if (item->is_null()) return true;

      *selectivity = apply_operator(op, value);
      return false;
    }
    case Value_map_type::DECIMAL: {
      my_decimal buffer;
      const my_decimal *value = item->val_decimal(&buffer);
      if (item->is_null()) return true;

      *selectivity = apply_operator(op, *value);
      return false;
    }
    case Value_map_type::DATE:
    case Value_map_type::TIME:
    case Value_map_type::DATETIME: {
      MYSQL_TIME temporal_value;
      if (get_temporal(item, get_data_type(), &temporal_value) ||
          item->is_null())
        return true;

      *selectivity = apply_operator(op, temporal_value);
      return false;
    }
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end deadcode */
}

bool Histogram::get_selectivity(Item **items, size_t item_count,
                                enum_operator op, double *selectivity) const {
  // Do some sanity checking first
  switch (op) {
    case enum_operator::EQUALS_TO:
    case enum_operator::GREATER_THAN:
    case enum_operator::LESS_THAN:
    case enum_operator::LESS_THAN_OR_EQUAL:
    case enum_operator::GREATER_THAN_OR_EQUAL:
    case enum_operator::NOT_EQUALS_TO:
      DBUG_ASSERT(item_count == 2);
      /*
        Verify that one side of the predicate is a column/field, and that the
        other side is a constant value.

        Make sure that we have the constant item as the right side argument of
        the predicate internally.
      */
      if (items[0]->const_item() && items[1]->type() == Item::FIELD_ITEM) {
        // Flip the operators as well as the operator itself.
        switch (op) {
          case enum_operator::GREATER_THAN:
            op = enum_operator::LESS_THAN;
            break;
          case enum_operator::LESS_THAN:
            op = enum_operator::GREATER_THAN;
            break;
          case enum_operator::LESS_THAN_OR_EQUAL:
            op = enum_operator::GREATER_THAN_OR_EQUAL;
            break;
          case enum_operator::GREATER_THAN_OR_EQUAL:
            op = enum_operator::LESS_THAN_OR_EQUAL;
            break;
          default:
            break;
        }
        Item *items_flipped[2];
        items_flipped[0] = items[1];
        items_flipped[1] = items[0];
        return get_selectivity(items_flipped, item_count, op, selectivity);
      } else if (items[0]->type() != Item::FIELD_ITEM ||
                 !items[1]->const_item()) {
        return true;
      }
      break;
    case enum_operator::BETWEEN:
    case enum_operator::NOT_BETWEEN:
      DBUG_ASSERT(item_count == 3);

      if (items[0]->type() != Item::FIELD_ITEM || !items[1]->const_item() ||
          !items[2]->const_item()) {
        return true;
      }
      break;
    case enum_operator::IN_LIST:
    case enum_operator::NOT_IN_LIST:
      DBUG_ASSERT(item_count >= 2);

      if (items[0]->type() != Item::FIELD_ITEM)
        return true; /* purecov: deadcode */

      // This will only work if all items are const_items
      for (size_t i = 1; i < item_count; ++i) {
        if (!items[i]->const_item()) return true;
      }
      break;
    case enum_operator::IS_NULL:
    case enum_operator::IS_NOT_NULL:
      DBUG_ASSERT(item_count == 1);
      if (items[0]->type() != Item::FIELD_ITEM) return true;
  }

  DBUG_ASSERT(items[0]->type() == Item::FIELD_ITEM);

  const TYPELIB *typelib = nullptr;
  const Item_field *item_field = down_cast<const Item_field *>(items[0]);
  if (item_field->field->real_type() == MYSQL_TYPE_ENUM ||
      item_field->field->real_type() == MYSQL_TYPE_SET) {
    const Field_enum *field_enum =
        down_cast<const Field_enum *>(item_field->field);
    typelib = field_enum->typelib;
  }

  switch (op) {
    case enum_operator::LESS_THAN:
    case enum_operator::EQUALS_TO:
    case enum_operator::GREATER_THAN: {
      return get_selectivity_dispatcher(items[1], op, typelib, selectivity);
    }
    case enum_operator::LESS_THAN_OR_EQUAL: {
      double less_than_selectivity;
      double equals_to_selectivity;
      if (get_selectivity_dispatcher(items[1], enum_operator::LESS_THAN,
                                     typelib, &less_than_selectivity) ||
          get_selectivity_dispatcher(items[1], enum_operator::EQUALS_TO,
                                     typelib, &equals_to_selectivity))
        return true;

      *selectivity = std::min(less_than_selectivity + equals_to_selectivity,
                              get_non_null_values_frequency());
      return false;
    }
    case enum_operator::GREATER_THAN_OR_EQUAL: {
      double greater_than_selectivity;
      double equals_to_selectivity;
      if (get_selectivity_dispatcher(items[1], enum_operator::GREATER_THAN,
                                     typelib, &greater_than_selectivity) ||
          get_selectivity_dispatcher(items[1], enum_operator::EQUALS_TO,
                                     typelib, &equals_to_selectivity))
        return true;

      *selectivity = std::min(greater_than_selectivity + equals_to_selectivity,
                              get_non_null_values_frequency());
      return false;
    }
    case enum_operator::NOT_EQUALS_TO: {
      double equals_to_selectivity;
      if (get_selectivity_dispatcher(items[1], enum_operator::EQUALS_TO,
                                     typelib, &equals_to_selectivity))
        return true;

      *selectivity = std::max(
          get_non_null_values_frequency() - equals_to_selectivity, 0.0);
      return false;
    }
    case enum_operator::BETWEEN: {
      double less_than_selectivity;
      double greater_than_selectivity;
      if (get_selectivity_dispatcher(items[1], enum_operator::LESS_THAN,
                                     typelib, &less_than_selectivity) ||
          get_selectivity_dispatcher(items[2], enum_operator::GREATER_THAN,
                                     typelib, &greater_than_selectivity))
        return true;

      *selectivity = this->get_non_null_values_frequency() -
                     (less_than_selectivity + greater_than_selectivity);

      /*
        Make sure that we don't return a value less than 0.0. This might happen
        with a query like:
          EXPLAIN SELECT a FROM t1 WHERE t1.a BETWEEN 3 AND 0;
      */
      *selectivity = std::max(0.0, *selectivity);
      return false;
    }
    case enum_operator::NOT_BETWEEN: {
      double less_than_selectivity;
      double greater_than_selectivity;
      if (get_selectivity_dispatcher(items[1], enum_operator::LESS_THAN,
                                     typelib, &less_than_selectivity) ||
          get_selectivity_dispatcher(items[2], enum_operator::GREATER_THAN,
                                     typelib, &greater_than_selectivity))
        return true;

      /*
        Make sure that we don't return a value greater than 1.0. This might
        happen with a query like:
          EXPLAIN SELECT a FROM t1 WHERE t1.a NOT BETWEEN 3 AND 0;
      */
      *selectivity = std::min(less_than_selectivity + greater_than_selectivity,
                              get_non_null_values_frequency());
      return false;
    }
    case enum_operator::IN_LIST: {
      *selectivity = 0.0;
      for (size_t i = 1; i < item_count; ++i) {
        double equals_to_selectivity;
        if (get_selectivity_dispatcher(items[i], enum_operator::EQUALS_TO,
                                       typelib, &equals_to_selectivity))
          return true;

        *selectivity += equals_to_selectivity;

        if (*selectivity >= get_non_null_values_frequency()) break;
      }

      /*
        Long in-lists may easily exceed a selectivity of
        get_non_null_values_frequency() in certain cases.
      */
      *selectivity = std::min(*selectivity, get_non_null_values_frequency());
      return false;
    }
    case enum_operator::NOT_IN_LIST: {
      *selectivity = this->get_non_null_values_frequency();
      for (size_t i = 1; i < item_count; ++i) {
        double equals_to_selectivity;
        if (get_selectivity_dispatcher(items[i], enum_operator::EQUALS_TO,
                                       typelib, &equals_to_selectivity)) {
          if (items[i]->null_value) {
            // WHERE col1 NOT IN (..., NULL, ...) will return zero rows.
            *selectivity = 0.0;
            return false;
          }

          return true; /* purecov: deadcode */
        }

        *selectivity -= equals_to_selectivity;
        if (*selectivity <= 0.0) break;
      }

      /*
        Long in-lists may easily estimate a selectivity less than 0.0 in certain
        cases.
      */
      *selectivity = std::max(*selectivity, 0.0);
      return false;
    }
    case enum_operator::IS_NULL:
      *selectivity = this->get_null_values_fraction();
      return false;
    case enum_operator::IS_NOT_NULL:
      *selectivity = 1.0 - this->get_null_values_fraction();
      return false;
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end deadcode */
}

// Explicit template instantiations.
template Histogram *build_histogram(MEM_ROOT *, const Value_map<double> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

template Histogram *build_histogram(MEM_ROOT *, const Value_map<String> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

template Histogram *build_histogram(MEM_ROOT *, const Value_map<ulonglong> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

template Histogram *build_histogram(MEM_ROOT *, const Value_map<longlong> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

template Histogram *build_histogram(MEM_ROOT *, const Value_map<MYSQL_TIME> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

template Histogram *build_histogram(MEM_ROOT *, const Value_map<my_decimal> &,
                                    size_t, const std::string &,
                                    const std::string &, const std::string &);

}  // namespace histograms
