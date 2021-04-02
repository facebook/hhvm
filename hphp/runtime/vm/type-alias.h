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

#pragma once

#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-array-like.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/user-attributes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct Class;
struct RecordDesc;
struct StringData;
struct Unit;

///////////////////////////////////////////////////////////////////////////////

/*
 * This is the runtime representation of a type alias.  Type aliases are only
 * allowed when HipHop extensions are enabled.
 *
 * The `type' field is Object whenever the type alias is basically just a
 * name. At runtime we still might resolve this name to another type alias,
 * becoming a type alias for some other type or something in that request.
 *
 * For the per-request struct, see TypeAlias below.
 */
struct PreTypeAlias {
  Unit* unit;
  LowStringPtr name;
  LowStringPtr value;
  Attr attrs;
  AnnotType type;
  int line0;
  int line1;
  bool nullable;  // null is allowed; for ?Foo aliases
  UserAttributeMap userAttrs;
  Array typeStructure{ArrayData::CreateDict()};

  std::pair<int,int> getLocation() const {
    return std::make_pair(line0, line1);
  }
};


///////////////////////////////////////////////////////////////////////////////

/*
 * In a given request, a defined type alias is turned into a TypeAlias
 * struct.  This contains the information needed to validate parameter type
 * hints for a type alias at runtime.
 */
struct TypeAlias {

  /////////////////////////////////////////////////////////////////////////////
  // Static constructors.

  static TypeAlias Invalid(const PreTypeAlias& alias);
  static TypeAlias From(const PreTypeAlias& alias);
  static TypeAlias From(TypeAlias req,
                           const PreTypeAlias& alias);


  /////////////////////////////////////////////////////////////////////////////
  // Comparison.

  bool same(const TypeAlias& req) const;
  bool compat(const PreTypeAlias& alias) const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  // The aliased type.
  AnnotType type{AnnotType::NoReturn};
  // Overrides `type' if the alias is invalid (e.g., for a nonexistent class).
  bool invalid{false};
  // For option types, like ?Foo.
  bool nullable{false};
  // Aliased Class; nullptr if type != Object.
  LowPtr<Class> klass{nullptr};
  // Aliased RecordDesc; nullptr if type != Record.
  LowPtr<RecordDesc> rec{nullptr};
  // Needed for error messages; nullptr if not defined.
  LowStringPtr name{nullptr};
  Array typeStructure{ArrayData::CreateDict()};
  UserAttributeMap userAttrs;
  Unit* unit{nullptr};
};

bool operator==(const TypeAlias& l, const TypeAlias& r);
bool operator!=(const TypeAlias& l, const TypeAlias& r);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_TYPE_ALIAS_INL_H_
#include "hphp/runtime/vm/type-alias-inl.h"
#undef incl_HPHP_TYPE_ALIAS_INL_H_

