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

#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct CollectedInfo;
struct IIndex;
struct ISS;

namespace php {
  struct Class;
  struct Const;
  struct TypeAlias;
}

//////////////////////////////////////////////////////////////////////

/*
 * Result of resolve_type_structure
 */
struct TypeStructureResolution {
  Type type; // Best known type of the *resolved* type-structure. If
             // the type-structure can be statically pre-resolved,
             // this will be a static array.
  bool mightFail; // Whether the resolution can possibly fail

  // If the resolution results in a static array with no possibility
  // of failure, return it.
  SArray sarray() const {
    if (mightFail) return nullptr;
    auto const v = tv(type);
    if (!v) return nullptr;
    assertx(tvIsDict(*v));
    assertx(val(*v).parr->isStatic());
    return val(*v).parr;
  }

  TypeStructureResolution& operator|=(const TypeStructureResolution& o) {
    type |= o.type;
    mightFail |= o.mightFail;
    return *this;
  }
};

/*
 * Attempt to resolve the given type-structure, either "anonymous",
 * from a class constant, or from a type-alias.
 */
TypeStructureResolution resolve_type_structure(const ISS&, SArray);
TypeStructureResolution resolve_type_structure(const IIndex&,
                                               const php::Const& cns,
                                               const php::Class& thiz);
TypeStructureResolution resolve_type_structure(const IIndex&,
                                               const CollectedInfo*,
                                               const php::TypeAlias&);

//////////////////////////////////////////////////////////////////////

}
