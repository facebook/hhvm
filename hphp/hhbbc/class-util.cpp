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
#include "hphp/hhbbc/class-util.h"

#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"

#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/vm/preclass-emitter.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_SimpleXMLElement("SimpleXMLElement"),
  s_Closure("Closure"),
  s_MockClass("__MockClass"),
  s_NoFlatten("__NoFlatten");
}

//////////////////////////////////////////////////////////////////////

bool has_magic_bool_conversion(SString clsName) {
  return
    collections::isTypeName(clsName) ||
    clsName->isame(s_SimpleXMLElement.get());
}

bool is_collection(res::Class cls) {
  auto const name = cls.name();
  return collections::isTypeName(name);
}

php::Func* find_method(const php::Class* cls, SString name) {
  for (auto& m : cls->methods) {
    if (m->name->isame(name)) {
      return m.get();
    }
  }
  return nullptr;
}

bool is_special_method_name(SString name) {
  auto const p = name->data();
  return p && p[0] == '8' && p[1] == '6';
}

bool is_mock_class(const php::Class* cls) {
  return cls->userAttributes.count(s_MockClass.get());
}

bool is_noflatten_trait(const php::Class* cls) {
  assertx(cls->attrs & AttrTrait);
  return cls->userAttributes.count(s_NoFlatten.get());
}

bool is_closure(const php::Class& c) {
  return c.parentName && c.parentName->isame(s_Closure.get());
}

bool is_unused_trait(const php::Class& c) {
  return
    (c.attrs & (AttrTrait | AttrNoOverride)) == (AttrTrait | AttrNoOverride);
}

bool is_used_trait(const php::Class& c) {
  return
    (c.attrs & (AttrTrait | AttrNoOverride)) == AttrTrait;
}

std::string normalized_class_name(const php::Class& cls) {
  auto const name = cls.name->toCppString();
  if (!PreClassEmitter::IsAnonymousClassName(name)) return name;
  return name.substr(0, name.find_last_of(';'));
}

bool prop_might_have_bad_initial_value(const Index& index,
                                       const php::Class& cls,
                                       const php::Prop& prop) {
  assertx(!is_used_trait(cls));

  if (is_closure(cls)) return false;
  if (prop.attrs & (AttrSystemInitialValue | AttrLateInit)) return false;

  auto const initial = from_cell(prop.val);
  if (initial.subtypeOf(BUninit)) return true;

  auto const ctx = Context{ cls.unit, nullptr, &cls };
  if (!index.satisfies_constraint(ctx, initial, prop.typeConstraint)) {
    return true;
  }

  return std::any_of(
    prop.ubs.begin(), prop.ubs.end(),
    [&] (TypeConstraint ub) {
      applyFlagsToUB(ub, prop.typeConstraint);
      return !index.satisfies_constraint(ctx, initial, ub);
    }
  );
}

//////////////////////////////////////////////////////////////////////

namespace php {

//////////////////////////////////////////////////////////////////////

ClassBase::ClassBase(const ClassBase& other) {
  for (auto& m : other.methods) {
    methods.push_back(std::make_unique<php::Func>(*m));
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}}
