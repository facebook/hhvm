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

#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/tiny-vector.h"

#include <folly/Range.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct Class;
struct StringData;
struct Unit;

///////////////////////////////////////////////////////////////////////////////

enum class AliasKind {
  TypeAlias,
  CaseType
};

/*
 * This is the runtime representation of a type alias.
 *
 * At runtime we still might resolve this name to another type alias,
 * becoming a type alias for some other type or something in that request.
 *
 * For the per-request struct, see TypeAlias below.
 */
struct PreTypeAlias {
  Unit* unit;
  LowStringPtr name;
  Attr attrs;
  TypeConstraint value;
  int line0;
  int line1;
  AliasKind kind;
  UserAttributeMap userAttrs;
  Array typeStructure;
  // If !isNull(), contains m_typeStructure in post-resolved form from
  // HHBBC.
  Array resolvedTypeStructure;

  std::pair<int,int> getLocation() const {
    return std::make_pair(line0, line1);
  }

  bool isPersistent() const {
    return attrs & AttrPersistent;
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
  // Comparison.

  bool same(const TypeAlias& req) const;
  bool compat(const PreTypeAlias& alias) const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  TypeConstraint value;
  // Overrides `type' if the alias is invalid (e.g., for a nonexistent class).
  bool invalid{false};

  explicit TypeAlias(const PreTypeAlias* preTypeAlias)
    : m_preTypeAlias(preTypeAlias)
  {}

  const PreTypeAlias* preTypeAlias() const { return m_preTypeAlias; }
  const StringData* name() const { return m_preTypeAlias->name; }
  const Unit* unit() const { return m_preTypeAlias->unit; }
  UserAttributeMap userAttrs() const { return m_preTypeAlias->userAttrs; }
  const Array& typeStructure() const { return m_preTypeAlias->typeStructure; }
  const Array& resolvedTypeStructureRaw() const {
    return m_preTypeAlias->resolvedTypeStructure;
  }

  // Return the type-structure, possibly as a logging array
  const Array resolvedTypeStructure() const;

  // Should only be used to change the existing type-structure to
  // a bespoke array layout.
  void setResolvedTypeStructure(ArrayData* ad);

  // A hash for this class that will remain constant across process restarts.
  size_t stableHash() const;

  /*
   * Define the type alias given by `id', binding it to the appropriate
   * NamedType for this request.
   *
   * Raises a fatal error if type alias already defined or cannot be defined
   * unless failIsFatal is unset
   */
  static const TypeAlias* def(const PreTypeAlias* thisType, bool failIsFatal = true);

  /*
   * Look up without autoloading a type alias named `name'. Returns nullptr
   * if one cannot be found.
   *
   * If the type alias is found and `persistent' is provided, it will be set to
   * whether or not the TypeAlias's RDS handle is persistent.
   */
  static const TypeAlias* lookup(const StringData* name,
                                 bool* persistent = nullptr);

  /*
   * Look up or attempt to autoload a type alias named `name'. Returns nullptr
   * if one cannot be found or autoloaded.
   *
   * If the type alias is found and `persistent' is provided, it will be set to
   * whether or not the TypeAlias's RDS handle is persistent.
   */
  static const TypeAlias* load(const StringData* name,
                               bool* persistent = nullptr);

private:
  const PreTypeAlias* m_preTypeAlias{nullptr};
};

bool operator==(const TypeAlias& l, const TypeAlias& r);
bool operator!=(const TypeAlias& l, const TypeAlias& r);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_TYPE_ALIAS_INL_H_
#include "hphp/runtime/vm/type-alias-inl.h"
#undef incl_HPHP_TYPE_ALIAS_INL_H_
