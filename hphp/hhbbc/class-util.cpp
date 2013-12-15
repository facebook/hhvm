/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/class-util.h"

#include "hphp/runtime/base/types.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_SimpleXMLElement("SimpleXMLElement"),
  s_Vector("Vector"),
  s_Map("Map"),
  s_StableMap("StableMap"),
  s_Set("Set"),
  s_FrozenVector("FrozenVector"),
  s_FrozenSet("FrozenSet");

bool has_magic_bool_conversion(res::Class cls) {
  return is_collection(cls) || cls.name()->isame(s_SimpleXMLElement.get());
}

}

//////////////////////////////////////////////////////////////////////

bool is_collection(res::Class cls) {
  auto const name = cls.name();
  return name->isame(s_Vector.get()) ||
         name->isame(s_Map.get()) ||
         name->isame(s_StableMap.get()) ||
         name->isame(s_Set.get()) ||
         name->isame(s_FrozenVector.get()) ||
         name->isame(s_FrozenSet.get());
}

bool could_have_magic_bool_conversion(Type t) {
  if (!t.couldBe(TObj)) return false;
  if (t.strictSubtypeOf(TObj)) {
    return has_magic_bool_conversion(dobj_of(t).cls);
  }
  if (is_opt(t) && unopt(t).strictSubtypeOf(TObj)) {
    return has_magic_bool_conversion(dobj_of(t).cls);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

}}

