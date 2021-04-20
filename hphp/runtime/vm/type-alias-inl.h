/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_TYPE_ALIAS_INL_H_
#error "type-alias-inl.h should only be included by type-alias.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct StringData;
struct ArrayData;

///////////////////////////////////////////////////////////////////////////////
// Static constructors.

inline TypeAlias TypeAlias::Invalid(const PreTypeAlias* alias) {
  TypeAlias req(alias);
  req.invalid = true;
  return req;
}

inline TypeAlias TypeAlias::From(const PreTypeAlias* alias) {
  assertx(alias->type != AnnotType::Object);

  TypeAlias req(alias);
  req.type = alias->type;
  req.nullable = alias->nullable;
  return req;
}

inline TypeAlias TypeAlias::From(TypeAlias req, const PreTypeAlias* alias) {
  assertx(alias->type == AnnotType::Object);

  req.m_preTypeAlias = alias;
  if (req.invalid) {
    return req; // Do nothing.
  }
  req.nullable |= alias->nullable;
  return req;
}

///////////////////////////////////////////////////////////////////////////////
// Serialization.

inline bool TypeAlias::serialize() const {
  if (m_serialized) return false;
  m_serialized = true;
  return true;
}

inline bool TypeAlias::wasSerialized() const {
  return m_serialized;
}

///////////////////////////////////////////////////////////////////////////////
// Comparison.

inline bool TypeAlias::same(const TypeAlias& req) const {
  return (invalid && req.invalid) ||
         (type == AnnotType::Mixed && req.type == AnnotType::Mixed) ||
         (type == req.type && nullable == req.nullable &&
          klass == req.klass && rec == req.rec);
}

inline bool operator==(const TypeAlias& l,
                       const TypeAlias& r) {
  return l.same(r);
}

inline bool operator!=(const TypeAlias& l,
                       const TypeAlias& r) {
  return !l.same(r);
}

///////////////////////////////////////////////////////////////////////////////
}
