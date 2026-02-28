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

#ifndef DD_SERIALIZE_IMPL_H_INCLUDED
#define DD_SERIALIZE_IMPL_H_INCLUDED

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>      // rapidjson::GenericValue
#include <rapidjson/prettywriter.h>  // rapidjson::PrettyWriter
#include <memory>

#include "base64.h"    // base64_encode
#include "m_string.h"  // STRING_WITH_LEN
#include "my_dbug.h"
#include "prealloced_array.h"  // Prealloced_array
#include "sql/dd/object_id.h"  // Object_id typedef

/**
  @file
  @ingroup SDI
  Internal (private) header file for the (de)serialization code. This
  file is made up of 5 parts:

  @ref int_func_decl
  @ref prealloced_typedefs
  @ref value_overloads
  @ref key_templates
  @ref special_composite_templates
*/

/**
  @defgroup int_func_decl Internal Sdi_context Functions
  @ingroup sdi

  Declarations of internal functions which operate on Sdi_context
  objects. Conceptually these are member functions of Sdi_context,
  but declaring them as such would mean that the definition of
  Sdi_context would have had to be made available in every
  translation unit where these functions get called (most data
  dictionary object implementation files).

  This is essentially a modification of the pimpl (Pointer to
  IMPLementation) idiom where we avoid the need to create separate
  api and implementation classes and avoid the extra indirection of
  going through the pimpl pointer.

  @{
*/

namespace dd {
class Column;
class Index;
class Properties;
template <typename T>
class Collection;
/**
   Factory function for creating a Property object from String_type.

   @param str string representation of properties
 */
Properties *parse_properties(const String_type &str);

class Sdi_wcontext;

/**
  Return a non-owning pointer to a char buffer which can be used
  for e.g. base64 encoding.
  @param wctx opaque context.
  @param sz size of buffer.
*/
char *buf_handle(Sdi_wcontext *wctx, size_t sz);

/**
  Returns const reference to string holding schema name to use in SDI.
  @param wctx opaque context.
  @return schema name to use.
*/

const String_type &lookup_schema_name(Sdi_wcontext *wctx);

/**
  Look up the tablespace name for a tablespace id. Returns a reference
  to the name string inside an acquired tablespace object. The
  lifetime of these tablespace objects are managed by the
  Auto_releaser in the scope where the dd store is initiated.

  @param wctx opaque context
  @param id tablespace id to look up
  @return tablespace name ref
*/

const dd::String_type &lookup_tablespace_name(Sdi_wcontext *wctx,
                                              dd::Object_id id);

class Sdi_rcontext;

/**
  Register Column objects being deserialized so that it will be
  possible to resolve references to it after deserialization has
  finished.

  @param rctx opaque context
  @param column_object object which may be referenced by other objects.
*/

void track_object(Sdi_rcontext *rctx, Column *column_object);

/**
  Register Index objects being deserialized so that it will be
  possible to resolve references to it after deserialization has
  finished.

  @param rctx opaque context
  @param index_object object which may be referenced by other objects.
*/

void track_object(Sdi_rcontext *rctx, Index *index_object);

/**
  Return an non-owning raw pointer to the deserialized Index object
  with ordinal postion index opx (ordinal position opx+1). The unused
  const Index* argument is needed for overload resolution.

  @param rctx opaque context
  @param opx ordinal position index
*/

Index *get_by_opx(Sdi_rcontext *rctx, const Index *, uint opx);

/**
  Return an non-owning raw pointer to the deserialized Column object
  with ordinal postion index opx (ordinal position opx+1). The unused
  const Column* argument is needed for overload resolution.

  @param rctx opaque context
  @param opx ordinal position index
*/

Column *get_by_opx(Sdi_rcontext *rctx, const Column *, uint opx);

/**
  Return a non-owning pointer to a char buffer which can be used
  for e.g. base64 encoding.
  @param rctx opaque context
  @param sz size of buffer
  @return non-owning pointer to buffer
*/

char *buf_handle(Sdi_rcontext *rctx, size_t sz);

/**
  Return the the Object_id of a schema name in the current data
  dictionary. Used to recreate a reference to a schema during
  deserialization.

  @param rctx opaque context.
  @param name schema name used as reference.
  @param idp [OUT] pointer to Object_id variable where result is stored.
  @return MySQL error handling.
  */

bool lookup_schema_ref(Sdi_rcontext *rctx, const String_type &name,
                       Object_id *idp);

/**
  Return the the Object_id of a tablespace name in the current data
  dictionary. Used to recreate a reference to a tablespace during
  deserialization.

  @param rctx opaque context.
  @param name schema name used as reference.
  @param idp [OUT] pointer to Object_id variable where result is stored.
  @return MySQL error handling.

  */

bool lookup_tablespace_ref(Sdi_rcontext *rctx, const String_type &name,
                           Object_id *idp);

}  // namespace dd

/** @} */  // int_func_decl

/**
  @defgroup prealloced_typedefs Prealloced_array Typedefs
  @ingroup sdi

  Defines a sub-class of Prealloced_array and some useful typedefs for use in
  (de)serialization code.
  @{
*/

typedef dd::String_type binary_t;
template <typename T, size_t PREALLOC = 16>
struct dd_vector : public Prealloced_array<T, PREALLOC> {
  dd_vector(PSI_memory_key psi_key = 0)
      : Prealloced_array<T, PREALLOC>(psi_key) {}
};

typedef dd_vector<char, 32> Byte_buffer;

/** @} */  // prealloced_typedefs

/**
  @defgroup value_overloads Value Function Overloads
  @ingroup sdi

  Defines function templates for writing a "bare" (without the key) json value.
  Each definition is overloaded on the second argument (which isn't a template
  argument) to handle each builtin type that has a corrsponding rapidjson type.
  @{
*/

template <typename W>
void write_value(W *w, bool a) {
  w->Bool(a);
}

template <typename GV>
bool read_value(bool *ap, const GV &gv) {
  if (!gv.IsBool()) {
    return true;
  }
  *ap = gv.GetBool();
  return false;
}

template <typename W>
void write_value(W *w, int a) {
  w->Int(a);
}

template <typename GV>
bool read_value(int *ap, const GV &gv) {
  if (!gv.IsInt()) {
    return true;
  }
  *ap = gv.GetInt();
  return false;
}

template <typename W>
void write_value(W *w, uint a) {
  w->Uint(a);
}

template <typename GV>
bool read_value(uint *ap, const GV &gv) {
  if (!gv.IsUint()) {
    return true;
  }
  *ap = gv.GetUint();
  return false;
}

template <typename W>
void write_value(W *w, ulong a) {
  w->Uint64(a);
}

template <typename GV>
bool read_value(ulong *ap, const GV &gv) {
  if (!gv.IsUint64()) {
    return true;
  }
  *ap = gv.GetUint64();
  return false;
}

template <typename W>
void write_value(W *w, ulonglong a) {
  w->Uint64(a);
}

template <typename GV>
bool read_value(ulonglong *ap, const GV &gv) {
  if (!gv.IsUint64()) {
    return true;
  }
  *ap = gv.GetUint64();
  return false;
}

template <typename W>
void write_value(W *w, const dd::String_type &a) {
  w->String(a.c_str(), a.size());
}

template <typename GV>
bool read_value(dd::String_type *ap, const GV &gv) {
  if (!gv.IsString()) {
    return true;
  }
  *ap = dd::String_type(gv.GetString(), gv.GetStringLength());
  return false;
}
/** @} */  // value_overloads

template <typename W>
void write_value(W *w, dd::String_type *a);

/**
  @defgroup key_templates Key-related Function Templates
  @ingroup sdi

  Defines wrapper function templates which handles the key part when
  writing and writing json.

  @{
*/

template <typename W, typename T>
void write(W *w, const T &t, const char *key, size_t key_sz) {
  w->String(key, key_sz);
  write_value(w, t);
}

template <typename T, typename GV>
bool read(T *ap, const GV &gv, const char *key) {
  if (!gv.HasMember(key)) {
    return true;
  }

  return read_value(ap, gv[key]);
}

/** @} */  // key_templates

/**
  @defgroup special_composite_templates Function Templates for Composite Types
  @ingroup sdi

  Defines function templates to handle types that do not map directly
  to a rapidjson type, and require some amount of converson/adaptation.

  @{
*/

template <typename W, typename ENUM_T>
void write_enum(W *w, ENUM_T enum_val, const char *key, size_t keysz) {
  write(w, static_cast<ulonglong>(enum_val), key, keysz);
}

template <typename ENUM_T, typename GV>
bool read_enum(ENUM_T *ep, const GV &gv, const char *key) {
  ulonglong v = 0;
  if (read(&v, gv, key)) {
    return true;
  }
  *ep = static_cast<ENUM_T>(v);
  return false;
}

template <typename W>
void write_binary(dd::Sdi_wcontext *wctx, W *w, const binary_t &b,
                  const char *key, size_t keysz) {
  int binsz = static_cast<int>(b.size());
  int b64sz = base64_needed_encoded_length(binsz);

  char *bp = dd::buf_handle(wctx, static_cast<size_t>(b64sz));
  DBUG_ASSERT(bp);

  base64_encode(b.c_str(), binsz, bp);
  w->String(key, keysz);
  w->String(bp);
}

template <typename GV>
bool read_binary(dd::Sdi_rcontext *rctx, binary_t *b, const GV &gv,
                 const char *key) {
  if (!gv.HasMember(key)) {
    return true;
  }

  const GV &a_gv = gv[key];

  if (!a_gv.IsString()) {
    return true;
  }

  const char *b64 = a_gv.GetString();
  size_t b64sz = a_gv.GetStringLength();
  int binsz = base64_needed_decoded_length(b64sz);

  char *bp = dd::buf_handle(rctx, static_cast<size_t>(binsz));
  binsz = base64_decode(b64, b64sz, bp, nullptr, 0);
  *b = binary_t(bp, binsz);
  return false;
}

template <typename W, typename PP>
void write_properties(W *w, const PP &p, const char *key, size_t keysz) {
  write(w, p.raw_string(), key, keysz);
}

template <typename PP, typename GV>
bool read_properties(PP *p, const GV &gv, const char *key) {
  dd::String_type raw_string;
  if (read(&raw_string, gv, key)) {
    return true;
  }
  p->insert_values(raw_string);
  return false;
}

template <typename W, typename PP>
void write_opx_reference(W *w, const PP &p, const char *key, size_t keysz) {
  uint opx = 0;
  if (p) {
    DBUG_ASSERT(p->ordinal_position() > 0);
    opx = p->ordinal_position() - 1;
    write(w, opx, key, keysz);
  }
}

template <typename PP, typename GV>
bool read_opx_reference(dd::Sdi_rcontext *rctx, PP *p, const GV &gv,
                        const char *key) {
  uint opx = 0;
  if (read(&opx, gv, key)) {
    return true;
  }
  *p = get_by_opx(rctx, *p, opx);
  return false;
}

template <typename GV>
bool deserialize_schema_ref(dd::Sdi_rcontext *rctx, dd::Object_id *p,
                            const GV &gv, const char *key) {
  dd::String_type schema_name;
  return (read(&schema_name, gv, key) ||
          lookup_schema_ref(rctx, schema_name, p));
}

template <typename W>
void serialize_tablespace_ref(dd::Sdi_wcontext *wctx, W *w,
                              dd::Object_id tablespace_id, const char *key,
                              size_t keysz) {
  if (tablespace_id == dd::INVALID_OBJECT_ID) {
    // There is no name to look up (will be the case for SEs not using
    // tablespaces
    return;
  }
  const dd::String_type &tablespace_name =
      lookup_tablespace_name(wctx, tablespace_id);

  if (tablespace_name.empty()) {
    return;
  }
  write(w, tablespace_name, key, keysz);
}

template <typename GV>
bool deserialize_tablespace_ref(dd::Sdi_rcontext *rctx, dd::Object_id *p,
                                const GV &gv, const char *key) {
  dd::String_type tablespace_name;
  if (read(&tablespace_name, gv, key)) {
    return false;  // Ok not to have this
  }
  return lookup_tablespace_ref(rctx, tablespace_name, p);
}

template <typename W, typename C>
void serialize_each(dd::Sdi_wcontext *wctx, W *w, const dd::Collection<C *> &cp,
                    const char *key, size_t keysz) {
  w->String(key, keysz);
  w->StartArray();
  for (const C *vp : cp) {
    vp->serialize(wctx, w);
  }
  w->EndArray(cp.size());
}

template <typename ADD_BINDER, typename GV>
bool deserialize_each(dd::Sdi_rcontext *rctx, ADD_BINDER add_binder,
                      const GV &obj_gv, const char *key) {
  if (!obj_gv.HasMember(key)) {
    return true;
  }

  const GV &array_gv = obj_gv[key];
  if (!array_gv.IsArray()) {
    return true;
  }

  const typename GV::ConstValueIterator end = array_gv.End();
  for (typename GV::ConstValueIterator it = array_gv.Begin(); it != end; ++it) {
    if (add_binder()->deserialize(rctx, *it)) {
      return true;
    }
  }
  return false;
}
/** @} */  // special_composite_templates

//} // namespace dd_sdi_impl

#endif /* DD_SERIALIZE_IMPL_H_INCLUDED */
