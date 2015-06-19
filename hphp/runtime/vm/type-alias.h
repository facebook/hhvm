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

#ifndef incl_HPHP_TYPE_ALIAS_H_
#define incl_HPHP_TYPE_ALIAS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/annot-type.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct StringData;

///////////////////////////////////////////////////////////////////////////////

/*
 * This is the runtime representation of a type alias.  Type aliases are only
 * allowed when HipHop extensions are enabled.
 *
 * The `type' field is Object whenever the type alias is basically just a
 * name.  At runtime we still might resolve this name to another type alias,
 * becoming a type alias for KindOfArray or something in that request.
 *
 * For the per-request struct, see TypeAliasReq below.
 */
struct TypeAlias {
  LowStringPtr name;
  LowStringPtr value;
  Attr         attrs;
  AnnotType    type;
  bool         nullable;  // null is allowed; for ?Foo aliases

  template<class SerDe> void serde(SerDe& sd) {
    sd(name)
      (value)
      (type)
      (nullable)
      (attrs)
      ;
  }
};


///////////////////////////////////////////////////////////////////////////////

/*
 * In a given request, a defined type alias is turned into a TypeAliasReq
 * struct.  This contains the information needed to validate parameter type
 * hints for a type alias at runtime.
 */
struct TypeAliasReq {

  /////////////////////////////////////////////////////////////////////////////
  // Static constructors.

  static TypeAliasReq Invalid();
  static TypeAliasReq From(const TypeAlias& alias);
  static TypeAliasReq From(TypeAliasReq req, const TypeAlias& alias);


  /////////////////////////////////////////////////////////////////////////////
  // Comparison.

  bool same(const TypeAliasReq& req) const;
  bool compat(const TypeAlias& alias) const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  // The aliased type.
  AnnotType type{AnnotType::Uninit};
  // Overrides `type' if the alias is invalid (e.g., for a nonexistent class).
  bool invalid{false};
  // For option types, like ?Foo.
  bool nullable{false};
  // Aliased Class; nullptr if type != Object.
  LowPtr<Class> klass{nullptr};
  // Needed for error messages; nullptr if not defined.
  LowStringPtr name{nullptr};
};

bool operator==(const TypeAliasReq& l, const TypeAliasReq& r);
bool operator!=(const TypeAliasReq& l, const TypeAliasReq& r);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_TYPE_ALIAS_INL_H_
#include "hphp/runtime/vm/type-alias-inl.h"
#undef incl_HPHP_TYPE_ALIAS_INL_H_

#endif
