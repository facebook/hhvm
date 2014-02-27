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
#include "hphp/hhbbc/class-util.h"

#include "hphp/runtime/base/types.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_SimpleXMLElement("SimpleXMLElement"),
  s_Vector("HH\\Vector"),
  s_Map("HH\\Map"),
  s_Set("HH\\Set"),
  s_Pair("HH\\Pair"),
  s_ImmVector("HH\\ImmVector"),
  s_ImmSet("HH\\ImmSet"),
  s_ImmMap("HH\\ImmMap"),
  s_Closure("Closure");

const StaticString
  s_86pinit("86pinit"),
  s_86sinit("86sinit"),
  s_MockClass("__MockClass");

bool has_magic_bool_conversion(res::Class cls) {
  return is_collection(cls) || cls.name()->isame(s_SimpleXMLElement.get());
}

}

//////////////////////////////////////////////////////////////////////

bool is_collection(res::Class cls) {
  auto const name = cls.name();
  return name->isame(s_Vector.get()) ||
    name->isame(s_Map.get()) ||
    name->isame(s_Set.get()) ||
    name->isame(s_Pair.get()) ||
    name->isame(s_ImmVector.get()) ||
    name->isame(s_ImmSet.get()) ||
    name->isame(s_ImmMap.get());
}

bool could_have_magic_bool_conversion(Type t) {
  if (!t.couldBe(TObj)) return false;
  // TODO(#3499765): we need to handle interfaces that the collection
  // classes implement before we can ever return false here.
  // Note: exclude s_Pair if we re-enable this.
  // if (t.strictSubtypeOf(TObj)) {
  //   return has_magic_bool_conversion(dobj_of(t).cls);
  // }
  // if (is_opt(t) && unopt(t).strictSubtypeOf(TObj)) {
  //   return has_magic_bool_conversion(dobj_of(t).cls);
  // }
  return true;
}

borrowed_ptr<php::Func> find_method(borrowed_ptr<const php::Class> cls,
                                    SString name) {
  for (auto& m : cls->methods) {
    if (m->name->isame(name)) {
      return borrow(m);
    }
  }
  return nullptr;
}

bool is_special_method_name(SString name) {
  auto const p = name->data();
  return p && p[0] == '8' && p[1] == '6';
}

bool is_mock_class(borrowed_ptr<const php::Class> cls) {
  return cls->userAttributes.count(s_MockClass.get());
}

SString collectionTypeToString(uint32_t ctype) {
  switch (ctype) {
  case Collection::VectorType:
    return s_Vector.get();
  case Collection::MapType:
    return s_Map.get();
  case Collection::SetType:
    return s_Set.get();
  case Collection::PairType:
    return s_Pair.get();
  case Collection::ImmVectorType:
    return s_ImmVector.get();
  case Collection::ImmSetType:
    return s_ImmSet.get();
  case Collection::ImmMapType:
    return s_ImmMap.get();
  }
  assert(!"Unknown Collection Type");
  not_reached();
}

bool is_closure(const php::Class& c) {
  return c.parentName && c.parentName->isame(s_Closure.get());
}

//////////////////////////////////////////////////////////////////////

}}
