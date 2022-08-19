/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/json_diff.h"

#include <sys/types.h>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_byteorder.h"
#include "my_dbug.h"  // DBUG_ASSERT
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"  // current_thd
#include "sql/debug_sync.h"
#include "sql/field.h"  // Field_json
#include "sql/json_binary.h"
#include "sql/json_dom.h"   // Json_dom, Json_wrapper
#include "sql/json_path.h"  // Json_path
#include "sql/log_event.h"  // net_field_length_checked
#include "sql/psi_memory_key.h"
#include "sql/sql_const.h"
#include "sql/table.h"
#include "sql_string.h"  // StringBuffer
#include "template_utils.h"

class THD;

Json_wrapper Json_diff::value() const {
  Json_wrapper result(m_value.get());
  result.set_alias();
  return result;
}

/**
  Return the total size of a data field, plus the size of the
  preceding integer that describes the length, when the integer is
  stored in net_field_length() format

  @param length The length of the data
  @return The length of the data plus the length of the length field.
*/
static size_t length_of_length_and_string(size_t length) {
  return length + net_length_size(length);
}

/**
  Encode a String as (length, data) pair, with length being stored in
  net_field_length() format.

  @param to Buffer where length and data will be stored.
  @param from Source string containing the data.
  @return true on out of memory, false on success.
*/
static bool write_length_and_string(String *to, const String &from) {
  // Serialize length.
  size_t length = from.length();
  DBUG_EXECUTE_IF("binlog_corrupt_write_length_and_string_bad_length", {
    DBUG_SET("-d,binlog_corrupt_write_length_and_string_bad_length");
    length = 1 << 30;
  });
  char length_buf[9];
  size_t length_length =
      net_store_length((uchar *)length_buf, length) - (uchar *)length_buf;
  DBUG_PRINT("info", ("write_length_and_string: length=%lu length_length=%lu",
                      (unsigned long)length, (unsigned long)length_length));
  DBUG_EXECUTE_IF(
      "binlog_corrupt_write_length_and_string_truncate_before_string", {
        DBUG_SET(
            "-d,binlog_corrupt_write_length_and_string_truncate_before_string");
        return false;
      });
  DBUG_EXECUTE_IF("binlog_corrupt_write_length_and_string_bad_char", {
    DBUG_SET("-d,binlog_corrupt_write_length_and_string_bad_char");
    // Instead of "some text", write "\xffsome tex"
    // This is sure to corrupt both JSON paths and
    // binary JSON.
    return to->append(length_buf, length_length) || to->append(0xff) ||
           to->append(from.ptr(), from.length() - 1);
  });
  // Allocate memory and append
  return to->append(length_buf, length_length) || to->append(from);
}

size_t Json_diff::binary_length() const {
  DBUG_TRACE;

  // operation
  size_t ret = ENCODED_OPERATION_BYTES;

  /*
    It would be better to compute length without serializing the path
    and json.  And given that we serialize the path and json, it would
    be better if we dealt with out-of-memory errors in a better way.

    In the future, we should remove the need to pre-compute the size.
    Currently this is only needed by the binlog writer.  And it would
    be better to rewrite the binlog writer so that it streams rows
    directly to the thread caches instead of storing them in memory.

    And currently, this function is used from
    Row_data_memory::max_row_length, and the return value of that
    function is used in Row_data_memory::allocate_memory, which
    doesn't check out-of-memory conditions at all and might just
    dereference nullptr in case of out of memory.  So these asserts do
    not make the situation worse.
  */
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf;

  // path
  if (m_path.to_string(&buf)) DBUG_ASSERT(0); /* purecov: deadcode */
  ret += length_of_length_and_string(buf.length());

  if (operation() != enum_json_diff_operation::REMOVE) {
    // value
    buf.length(0);
    if (value().to_binary(current_thd, &buf))
      DBUG_ASSERT(0); /* purecov: deadcode */
    ret += length_of_length_and_string(buf.length());
  }

  return ret;
}

bool Json_diff::write_binary(String *to) const {
  DBUG_TRACE;

  // Serialize operation
  char operation = (char)m_operation;
  DBUG_EXECUTE_IF("binlog_corrupt_json_diff_bad_op", {
    DBUG_SET("-d,binlog_corrupt_json_diff_bad_op");
    operation = 127;
  });
  if (to->append(&operation, ENCODED_OPERATION_BYTES))
    return true; /* purecov: inspected */  // OOM, error is reported
  DBUG_PRINT("info", ("wrote JSON operation=%d", (int)operation));

  /**
    @todo This first serializes in one buffer and then copies to
    another buffer.  It would be better if we could write directly to
    the output and save a round of memory allocation + copy. /Sven
  */

  // Serialize JSON path
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf;
#ifndef DBUG_OFF
  bool return_early = false;
  DBUG_EXECUTE_IF("binlog_corrupt_json_diff_truncate_before_path_length", {
    DBUG_SET("-d,binlog_corrupt_json_diff_truncate_before_path_length");
    return false;
  });
  DBUG_EXECUTE_IF("binlog_corrupt_json_diff_bad_path_length", {
    DBUG_SET("-d,binlog_corrupt_json_diff_bad_path_length");
    DBUG_SET("+d,binlog_corrupt_write_length_and_string_bad_length");
  });
  DBUG_EXECUTE_IF("binlog_corrupt_json_diff_truncate_before_path", {
    DBUG_SET("-d,binlog_corrupt_json_diff_truncate_before_path");
    DBUG_SET(
        "+d,binlog_corrupt_write_length_and_string_truncate_before_string");
    return_early = true;
  });
  DBUG_EXECUTE_IF("binlog_corrupt_json_diff_bad_path_char", {
    DBUG_SET("-d,binlog_corrupt_json_diff_bad_path_char");
    DBUG_SET("+d,binlog_corrupt_write_length_and_string_bad_char");
  });
#endif  // ifndef DBUG_OFF
  if (m_path.to_string(&buf) || write_length_and_string(to, buf))
    return true; /* purecov: inspected */  // OOM, error is reported
#ifndef DBUG_OFF
  if (return_early) return false;
#endif
  DBUG_PRINT("info", ("wrote JSON path '%s' of length %lu", buf.ptr(),
                      (unsigned long)buf.length()));

  if (m_operation != enum_json_diff_operation::REMOVE) {
    // Serialize JSON value
    buf.length(0);
#ifndef DBUG_OFF
    DBUG_EXECUTE_IF("binlog_corrupt_json_diff_truncate_before_doc_length", {
      DBUG_SET("-d,binlog_corrupt_json_diff_truncate_before_doc_length");
      return false;
    });
    DBUG_EXECUTE_IF("binlog_corrupt_json_diff_bad_doc_length", {
      DBUG_SET("-d,binlog_corrupt_json_diff_bad_doc_length");
      DBUG_SET("+d,binlog_corrupt_write_length_and_string_bad_length");
    });
    DBUG_EXECUTE_IF("binlog_corrupt_json_diff_truncate_before_doc", {
      DBUG_SET("-d,binlog_corrupt_json_diff_truncate_before_doc");
      DBUG_SET(
          "+d,binlog_corrupt_write_length_and_string_truncate_before_string");
    });
    DBUG_EXECUTE_IF("binlog_corrupt_json_diff_bad_doc_char", {
      DBUG_SET("-d,binlog_corrupt_json_diff_bad_doc_char");
      DBUG_SET("+d,binlog_corrupt_write_length_and_string_bad_char");
    });
#endif  // ifndef DBUG_OFF
    if (value().to_binary(current_thd, &buf) ||
        write_length_and_string(to, buf))
      return true; /* purecov: inspected */  // OOM, error is reported
    DBUG_PRINT("info",
               ("wrote JSON value of length %lu", (unsigned long)buf.length()));
  }

  return false;
}

Json_diff_vector::Json_diff_vector(allocator_type arg)
    : m_vector(std::vector<Json_diff, allocator_type>(arg)),
      m_binary_length(0) {}

static MEM_ROOT empty_json_diff_vector_mem_root(PSI_NOT_INSTRUMENTED, 256);
const Json_diff_vector Json_diff_vector::EMPTY_JSON_DIFF_VECTOR{
    Json_diff_vector::allocator_type{&empty_json_diff_vector_mem_root}};

void Json_diff_vector::add_diff(const Json_seekable_path &path,
                                enum_json_diff_operation operation,
                                Json_dom_ptr dom) {
  m_vector.emplace_back(path, operation, std::move(dom));
  m_binary_length += at(size() - 1).binary_length();
}

void Json_diff_vector::add_diff(const Json_seekable_path &path,
                                enum_json_diff_operation operation) {
  m_vector.emplace_back(path, operation, nullptr);
  m_binary_length += at(size() - 1).binary_length();
}

void Json_diff_vector::clear() {
  m_vector.clear();
  m_binary_length = 0;
}

size_t Json_diff_vector::binary_length(bool include_metadata) const {
  return m_binary_length + (include_metadata ? ENCODED_LENGTH_BYTES : 0);
}

bool Json_diff_vector::write_binary(String *to) const {
  // Insert placeholder where we will store the length, once that is known.
  char length_buf[ENCODED_LENGTH_BYTES] = {0, 0, 0, 0};
  if (to->append(length_buf, ENCODED_LENGTH_BYTES))
    return true; /* purecov: inspected */  // OOM, error is reported

  // Store all the diffs.
  for (const Json_diff &diff : *this)
    if (diff.write_binary(to))
      return true; /* purecov: inspected */  // OOM, error is reported

  // Store the length.
  size_t length = to->length() - ENCODED_LENGTH_BYTES;
  int4store(to->ptr(), (uint32)length);

  DBUG_PRINT("info", ("Wrote JSON diff vector length %lu=%02x %02x %02x %02x",
                      (unsigned long)length, length_buf[0], length_buf[1],
                      length_buf[2], length_buf[3]));

  return false;
}

bool Json_diff_vector::read_binary(const char **from, const TABLE *table,
                                   const char *field_name) {
  DBUG_TRACE;
  const uchar *p = pointer_cast<const uchar *>(*from);

  // Caller should have validated that the buffer is least 4 + length
  // bytes long.
  size_t length = uint4korr(p);
  p += 4;

  DBUG_PRINT("info", ("length=%d p=%p", (int)length, p));

  while (length) {
    DBUG_PRINT("info",
               ("length=%u bytes remaining to decode into Json_diff_vector",
                (uint)length));
    // Read operation
    if (length < 1) goto corrupted;
    int operation_number = *p;
    DBUG_PRINT("info", ("operation_number=%d", operation_number));
    if (operation_number >= JSON_DIFF_OPERATION_COUNT) goto corrupted;
    enum_json_diff_operation operation =
        static_cast<enum_json_diff_operation>(operation_number);
    length--;
    p++;

    // Read path length
    size_t path_length;
    if (net_field_length_checked<size_t>(&p, &length, &path_length))
      goto corrupted;
    if (length < path_length) goto corrupted;

    // Read path
    Json_path path;
    size_t bad_index;
    DBUG_PRINT("info", ("path='%.*s'", (int)path_length, p));
    if (parse_path(path_length, pointer_cast<const char *>(p), &path,
                   &bad_index))
      goto corrupted;
    p += path_length;
    length -= path_length;

    if (operation != enum_json_diff_operation::REMOVE) {
      // Read value length
      size_t value_length;
      if (net_field_length_checked<size_t>(&p, &length, &value_length))
        goto corrupted;

      if (length < value_length) goto corrupted;

      // Read value
      json_binary::Value value = json_binary::parse_binary(
          pointer_cast<const char *>(p), value_length);
      if (value.type() == json_binary::Value::ERROR) goto corrupted;
      Json_wrapper wrapper(value);
      Json_dom_ptr dom = wrapper.clone_dom(current_thd);
      if (dom == nullptr)
        return true; /* purecov: inspected */  // OOM, error is reported
      wrapper.dbug_print();

      // Store diff
      add_diff(path, operation, std::move(dom));

      p += value_length;
      length -= value_length;
    } else {
      // Store diff
      add_diff(path, operation);
    }
  }

  *from = pointer_cast<const char *>(p);
  return false;

corrupted:
  my_error(ER_CORRUPTED_JSON_DIFF, MYF(0), (int)table->s->table_name.length,
           table->s->table_name.str, field_name);
  return true;
}

/**
  Find the value at the specified path in a JSON DOM. The path should
  not contain any wildcard or ellipsis, only simple array cells or
  member names. Auto-wrapping is not performed.

  @param dom        the root of the DOM
  @param first_leg  the first path leg
  @param last_leg   the last path leg (exclusive)
  @return the JSON DOM at the given path, or `nullptr` if the path is not found
*/
static Json_dom *seek_exact_path(Json_dom *dom,
                                 const Json_path_iterator &first_leg,
                                 const Json_path_iterator &last_leg) {
  for (auto it = first_leg; it != last_leg; ++it) {
    const Json_path_leg *leg = *it;
    const auto leg_type = leg->get_type();
    DBUG_ASSERT(leg_type == jpl_member || leg_type == jpl_array_cell);
    switch (dom->json_type()) {
      case enum_json_type::J_ARRAY: {
        const auto array = down_cast<Json_array *>(dom);
        if (leg_type != jpl_array_cell) return nullptr;
        Json_array_index idx = leg->first_array_index(array->size());
        if (!idx.within_bounds()) return nullptr;
        dom = (*array)[idx.position()];
        continue;
      }
      case enum_json_type::J_OBJECT: {
        const auto object = down_cast<Json_object *>(dom);
        if (leg_type != jpl_member) return nullptr;
        dom = object->get(leg->get_member_name());
        if (dom == nullptr) return nullptr;
        continue;
      }
      default:
        return nullptr;
    }
  }

  return dom;
}

enum_json_diff_status apply_json_diffs(Field_json *field,
                                       const Json_diff_vector *diffs) {
  DBUG_TRACE;
  // Cannot apply a diff to NULL.
  if (field->is_null()) return enum_json_diff_status::REJECTED;

  DBUG_EXECUTE_IF("simulate_oom_in_apply_json_diffs", {
    DBUG_SET("+d,simulate_out_of_memory");
    DBUG_SET("-d,simulate_oom_in_apply_json_diffs");
  });

  Json_wrapper doc;
  if (field->val_json(&doc))
    return enum_json_diff_status::ERROR; /* purecov: inspected */

  doc.dbug_print("apply_json_diffs: before-doc");

  // Should we collect logical diffs while applying them?
  const bool collect_logical_diffs =
      field->table->is_logical_diff_enabled(field);

  // Should we try to perform the update in place using binary diffs?
  bool binary_inplace_update = field->table->is_binary_diff_enabled(field);

  StringBuffer<STRING_BUFFER_USUAL_SIZE> buffer;

  const THD *thd = field->table->in_use;

  for (const Json_diff &diff : *diffs) {
    Json_wrapper val = diff.value();

    auto &path = diff.path();

    if (path.leg_count() == 0) {
      /*
        Cannot replace the root (then a full update will be used
        instead of creating a diff), or insert the root, or remove the
        root, so reject this diff.
      */
      return enum_json_diff_status::REJECTED;
    }

    if (collect_logical_diffs)
      field->table->add_logical_diff(field, path, diff.operation(), &val);

    if (binary_inplace_update) {
      if (diff.operation() == enum_json_diff_operation::REPLACE) {
        bool partially_updated = false;
        bool replaced_path = false;
        if (doc.attempt_binary_update(field, path, &val, false, &buffer,
                                      &partially_updated, &replaced_path))
          return enum_json_diff_status::ERROR; /* purecov: inspected */

        if (partially_updated) {
          if (!replaced_path) return enum_json_diff_status::REJECTED;
          DBUG_EXECUTE_IF("rpl_row_jsondiff_binarydiff", {
            const char act[] =
                "now SIGNAL signal.rpl_row_jsondiff_binarydiff_created";
            DBUG_ASSERT(opt_debug_sync_timeout > 0);
            DBUG_ASSERT(
                !debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
          };);
          continue;
        }
      } else if (diff.operation() == enum_json_diff_operation::REMOVE) {
        Json_wrapper_vector hits(key_memory_JSON);
        bool found_path = false;
        if (doc.binary_remove(field, path, &buffer, &found_path))
          return enum_json_diff_status::ERROR; /* purecov: inspected */
        if (!found_path) return enum_json_diff_status::REJECTED;
        continue;
      }

      // Couldn't update in place, so try full update.
      binary_inplace_update = false;
      field->table->disable_binary_diffs_for_current_row(field);
    }

    Json_dom *dom = doc.to_dom(thd);
    if (doc.to_dom(thd) == nullptr)
      return enum_json_diff_status::ERROR; /* purecov: inspected */

    switch (diff.operation()) {
      case enum_json_diff_operation::REPLACE: {
        DBUG_ASSERT(path.leg_count() > 0);
        Json_dom *old = seek_exact_path(dom, path.begin(), path.end());
        if (old == nullptr) return enum_json_diff_status::REJECTED;
        DBUG_ASSERT(old->parent() != nullptr);
        old->parent()->replace_dom_in_container(old, val.clone_dom(thd));
        continue;
      }
      case enum_json_diff_operation::INSERT: {
        DBUG_ASSERT(path.leg_count() > 0);
        Json_dom *parent = seek_exact_path(dom, path.begin(), path.end() - 1);
        if (parent == nullptr) return enum_json_diff_status::REJECTED;
        const Json_path_leg *last_leg = path.last_leg();
        if (parent->json_type() == enum_json_type::J_OBJECT &&
            last_leg->get_type() == jpl_member) {
          auto obj = down_cast<Json_object *>(parent);
          if (obj->get(last_leg->get_member_name()) != nullptr)
            return enum_json_diff_status::REJECTED;
          if (obj->add_alias(last_leg->get_member_name(), val.clone_dom(thd)))
            return enum_json_diff_status::ERROR; /* purecov: inspected */
          continue;
        }
        if (parent->json_type() == enum_json_type::J_ARRAY &&
            last_leg->get_type() == jpl_array_cell) {
          auto array = down_cast<Json_array *>(parent);
          Json_array_index idx = last_leg->first_array_index(array->size());
          if (array->insert_alias(idx.position(), val.clone_dom(thd)))
            return enum_json_diff_status::ERROR; /* purecov: inspected */
          continue;
        }
        return enum_json_diff_status::REJECTED;
      }
      case enum_json_diff_operation::REMOVE: {
        DBUG_ASSERT(path.leg_count() > 0);
        Json_dom *parent = seek_exact_path(dom, path.begin(), path.end() - 1);
        if (parent == nullptr) return enum_json_diff_status::REJECTED;
        const Json_path_leg *last_leg = path.last_leg();
        if (parent->json_type() == enum_json_type::J_OBJECT) {
          auto object = down_cast<Json_object *>(parent);
          if (last_leg->get_type() != jpl_member ||
              !object->remove(last_leg->get_member_name()))
            return enum_json_diff_status::REJECTED;
        } else if (parent->json_type() == enum_json_type::J_ARRAY) {
          if (last_leg->get_type() != jpl_array_cell)
            return enum_json_diff_status::REJECTED;
          auto array = down_cast<Json_array *>(parent);
          Json_array_index idx = last_leg->first_array_index(array->size());
          if (!idx.within_bounds() || !array->remove(idx.position()))
            return enum_json_diff_status::REJECTED;
        } else {
          return enum_json_diff_status::REJECTED;
        }
        continue;
      }
    }

    DBUG_ASSERT(false); /* purecov: deadcode */
  }

  if (field->store_json(&doc) != TYPE_OK)
    return enum_json_diff_status::ERROR; /* purecov: inspected */

  return enum_json_diff_status::SUCCESS;
}
