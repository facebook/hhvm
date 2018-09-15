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

inline TypeAliasReq TypeAliasReq::Invalid(Unit* unit) {
  assertx(unit);
  TypeAliasReq req;
  req.unit = unit;
  req.invalid = true;
  return req;
}

inline TypeAliasReq TypeAliasReq::From(Unit* unit, const TypeAlias& alias) {
  assertx(alias.type != AnnotType::Object);
  assertx(unit);

  TypeAliasReq req;
  req.unit = unit;
  req.name = alias.name;
  req.type = alias.type;
  req.nullable = alias.nullable;
  req.typeStructure = alias.typeStructure;
  req.userAttrs = alias.userAttrs;
  assertx(req.typeStructure.isDictOrDArray());
  return req;
}

inline TypeAliasReq TypeAliasReq::From(Unit* unit, TypeAliasReq req,
                                       const TypeAlias& alias) {
  assertx(alias.type == AnnotType::Object);
  assertx(unit);

  req.unit = unit;
  if (req.invalid) {
    return req; // Do nothing.
  }
  req.name = alias.name;
  req.nullable |= alias.nullable;
  req.typeStructure = alias.typeStructure;
  req.userAttrs = alias.userAttrs;
  assertx(req.typeStructure.isDictOrDArray());
  return req;
}

///////////////////////////////////////////////////////////////////////////////
// Comparison.

inline bool TypeAliasReq::same(const TypeAliasReq& req) const {
  return (invalid && req.invalid) ||
         (type == AnnotType::Mixed && req.type == AnnotType::Mixed) ||
         (type == req.type && nullable == req.nullable && klass == req.klass);
}

inline bool operator==(const TypeAliasReq& l,
                       const TypeAliasReq& r) {
  return l.same(r);
}

inline bool operator!=(const TypeAliasReq& l,
                       const TypeAliasReq& r) {
  return !l.same(r);
}

///////////////////////////////////////////////////////////////////////////////
}
