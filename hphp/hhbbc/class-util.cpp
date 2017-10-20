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

#include "hphp/runtime/base/collections.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_SimpleXMLElement("SimpleXMLElement"),
  s_Closure("Closure"),
  s_86pinit("86pinit"),
  s_86sinit("86sinit"),
  s_MockClass("__MockClass");
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

//////////////////////////////////////////////////////////////////////

}}
