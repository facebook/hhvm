/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
// Static constructors.

inline TypeAliasReq TypeAliasReq::Invalid() {
  TypeAliasReq req;
  req.invalid = true;
  return req;
}

inline TypeAliasReq TypeAliasReq::From(const TypeAlias& alias) {
  TypeAliasReq req;

  if (alias.any) {
    req.any  = true;
    req.name = alias.name;
  } else {
    assert(alias.kind != KindOfObject);

    req.kind     = alias.kind;
    req.nullable = alias.nullable;
    req.name     = alias.name;
  }
  return req;
}

inline TypeAliasReq TypeAliasReq::From(TypeAliasReq req,
                                       const TypeAlias& alias) {
  if (req.invalid) {
    // Do nothing.
  } else if (req.any) {
    req.name = alias.name;
  } else {
    req.nullable |= alias.nullable;
    req.name = alias.name;
  }
  return req;
}

///////////////////////////////////////////////////////////////////////////////
// Comparison.

inline bool TypeAliasReq::same(const TypeAliasReq& req) const {
  return (invalid && req.invalid) ||
         (any && req.any) ||
         (kind     == req.kind &&
          nullable == req.nullable &&
          klass    == req.klass);
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
