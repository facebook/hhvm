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

#ifndef DD__RAW_RECORD_INCLUDED
#define DD__RAW_RECORD_INCLUDED

#include "my_config.h"

#include "my_inttypes.h"
#include "my_io.h"  // IWYU pragma: keep

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>

#include "sql/dd/object_id.h"    // dd::Object_id
#include "sql/dd/string_type.h"  // dd::String_type

class Json_wrapper;
class Field;
struct TABLE;

typedef long my_time_t;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Properties;

///////////////////////////////////////////////////////////////////////////

class Raw_record {
 public:
  Raw_record(TABLE *table);

 public:
  bool update();
  bool drop();

 public:
  bool store_pk_id(int field_no, Object_id id);
  bool store_ref_id(int field_no, Object_id id);
  bool store(int field_no, const String_type &s, bool is_null = false);
  bool store(int field_no, ulonglong ull, bool is_null = false);
  bool store(int field_no, longlong ll, bool is_null = false);

  bool store(int field_no, bool b, bool is_null = false) {
    return store(field_no, b ? 1ll : 0ll, is_null);
  }

  bool store(int field_no, uint v, bool is_null = false) {
    return store(field_no, (ulonglong)v, is_null);
  }

  bool store(int field_no, int v, bool is_null = false) {
    return store(field_no, (longlong)v, is_null);
  }

  bool store(int field_no, const Properties &p);

  bool store_time(int field_no, my_time_t val, bool is_null = false);

  /**
    Store timeval at field specified by field_no into the record.

    @param field_no  Field position in the record.
    @param tv        Time value to store.

    @returns
     false on success
     true on failure
  */

  bool store_timestamp(int field_no, const timeval &tv);

  bool store_json(int field_no, const Json_wrapper &json);

 public:
  bool is_null(int field_no) const;

  longlong read_int(int field_no) const;
  longlong read_int(int field_no, longlong null_value) const {
    return is_null(field_no) ? null_value : read_int(field_no);
  }

  ulonglong read_uint(int field_no) const;
  ulonglong read_uint(int field_no, ulonglong null_value) const {
    return is_null(field_no) ? null_value : read_uint(field_no);
  }

  String_type read_str(int field_no) const;
  String_type read_str(int field_no, const String_type &null_value) const {
    return is_null(field_no) ? null_value : read_str(field_no);
  }

  Object_id read_ref_id(int field_no) const;

  bool read_bool(int field_no) const { return read_int(field_no) != 0; }

  my_time_t read_time(int field_no) const;

  /**
    Read timeval stored at field specified by field_no from the record.

    @param field_no  Field position in the record.

    @returns
      timeval stored at field_no.
  */

  timeval read_timestamp(int field_no) const;

  bool read_json(int field_no, Json_wrapper *json_wrapper) const;

 protected:
  void set_null(int field_no, bool is_null);

  Field *field(
      int field_no) const;  // XXX: return non-const from const-operation

 protected:
  TABLE *m_table;
};

///////////////////////////////////////////////////////////////////////////

class Raw_new_record : public Raw_record {
 public:
  Raw_new_record(TABLE *table);

  ~Raw_new_record() { finalize(); }

 public:
  bool insert();

  Object_id get_insert_id() const;

  void finalize();
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__RAW_RECORD_INCLUDED
