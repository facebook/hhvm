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

#include "hphp/runtime/base/rds-symbol.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"

#include <boost/variant.hpp>
#include <folly/Format.h>
#include <folly/Hash.h>

#include <string>
#include <type_traits>

namespace HPHP { namespace rds {

///////////////////////////////////////////////////////////////////////////////

namespace {

struct SymbolKind : boost::static_visitor<std::string> {
  std::string operator()(LinkName k) const { return k.type; }
  std::string operator()(LinkID k) const { return k.type; }
  std::string operator()(ClsConstant) const { return "ClsConstant"; }
  std::string operator()(StaticMethod) const { return "StaticMethod"; }
  std::string operator()(StaticMethodF) const { return "StaticMethodF"; }
  std::string operator()(Profile) const { return "Profile"; }
  std::string operator()(SPropCache) const { return "SPropCache"; }
  std::string operator()(StaticMemoValue) const { return "StaticMemoValue"; }
  std::string operator()(StaticMemoCache) const { return "StaticMemoCache"; }
  std::string operator()(LSBMemoValue) const { return "LSBMemoValue"; }
  std::string operator()(LSBMemoCache) const { return "LSBMemoCache"; }
  std::string operator()(TSCache) const { return "TSCache"; }
};

struct SymbolRep : boost::static_visitor<std::string> {
  std::string operator()(LinkName k) const { return k.name->data(); }
  std::string operator()(LinkID) const { return ""; }

  std::string operator()(ClsConstant k) const {
    return k.clsName->data() + std::string("::") + k.cnsName->data();
  }

  std::string operator()(StaticMethod k)  const { return k.name->data(); }
  std::string operator()(StaticMethodF k) const { return k.name->data(); }

  std::string operator()(Profile k) const {
    return folly::format(
      "{}:t{}:{}",
      k.name,
      k.transId,
      k.bcOff
    ).str();
  }
  std::string operator()(SPropCache k) const {
    return k.cls->name()->toCppString() + "::" +
           k.cls->staticProperties()[k.slot].name->toCppString();
  }

  std::string operator()(StaticMemoValue k) const {
    auto const func = Func::fromFuncId(k.funcId);
    return func->fullName()->toCppString();
  }
  std::string operator()(StaticMemoCache k) const {
    auto const func = Func::fromFuncId(k.funcId);
    return func->fullName()->toCppString();
  }

  std::string operator()(LSBMemoValue k) const {
    auto const clsName = k.cls->name()->toCppString();
    auto const funcName = Func::fromFuncId(k.funcId)->fullName()->toCppString();
    return clsName + "::" + funcName;
  }
  std::string operator()(LSBMemoCache k) const {
    auto const clsName = k.cls->name()->toCppString();
    auto const funcName = Func::fromFuncId(k.funcId)->fullName()->toCppString();
    return clsName + "::" + funcName;
  }

  std::string operator()(TSCache k) const {
    auto const func = Func::fromFuncId(k.funcId);
    return func->fullName()->toCppString();
  }
};

struct SymbolEq : boost::static_visitor<bool> {
  template<class T, class U>
  typename std::enable_if<
    !std::is_same<T,U>::value,
    bool
  >::type operator()(const T&, const U&) const { return false; }

  bool operator()(LinkName k1, LinkName k2) const {
    return strcmp(k1.type, k2.type) == 0 && k1.name->isame(k2.name);
  }
  bool operator()(LinkID k1, LinkID k2) const {
    return strcmp(k1.type, k2.type) == 0;
  }

  bool operator()(ClsConstant k1, ClsConstant k2) const {
    assertx(k1.clsName->isStatic() && k1.cnsName->isStatic());
    assertx(k2.clsName->isStatic() && k2.cnsName->isStatic());
    return k1.clsName->isame(k2.clsName) &&
           k1.cnsName == k2.cnsName;
  }

  bool operator()(Profile k1, Profile k2) const {
    assertx(k1.name->isStatic() && k2.name->isStatic());
    return k1.kind == k2.kind &&
           k1.transId == k2.transId &&
           k1.bcOff == k2.bcOff &&
           k1.name == k2.name;
  }

  template<class T>
  typename std::enable_if<
    std::is_same<T,StaticMethod>::value ||
      std::is_same<T,StaticMethodF>::value,
    bool
  >::type operator()(const T& t1, const T& t2) const {
    assertx(t1.name->isStatic() && t2.name->isStatic());
    return t1.name->isame(t2.name);
  }

  bool operator()(SPropCache k1, SPropCache k2) const {
    return k1.cls == k2.cls && k1.slot == k2.slot;
  }

  bool operator()(StaticMemoValue k1, StaticMemoValue k2) const {
    return k1.funcId == k2.funcId;
  }

  bool operator()(StaticMemoCache k1, StaticMemoCache k2) const {
    return k1.funcId == k2.funcId;
  }

  bool operator()(LSBMemoValue k1, LSBMemoValue k2) const {
    return k1.cls == k2.cls && k1.funcId == k2.funcId;
  }

  bool operator()(LSBMemoCache k1, LSBMemoCache k2) const {
    return k1.cls == k2.cls && k1.funcId == k2.funcId;
  }

  bool operator()(TSCache k1, TSCache k2) const {
    return k1.funcId == k2.funcId;
  }
};

struct SymbolHash : boost::static_visitor<size_t> {
  size_t operator()(LinkName k) const {
    return folly::hash::hash_combine(
      std::string{k.type}, k.name->hash());
  }
  size_t operator()(LinkID k) const {
    return folly::hash::hash_combine(std::string{k.type});
  }

  size_t operator()(ClsConstant k) const {
    return folly::hash::hash_128_to_64(
      k.clsName->hash(),
      k.cnsName->hash()
    );
  }

  size_t operator()(Profile k) const {
    return folly::hash::hash_combine(
      static_cast<int>(k.kind),
      k.transId,
      k.bcOff,
      k.name->hash()
    );
  }

  size_t operator()(StaticMethod k)  const { return k.name->hash(); }
  size_t operator()(StaticMethodF k) const { return k.name->hash(); }

  size_t operator()(SPropCache k) const {
    return folly::hash::hash_combine(
      k.cls.get(), k.slot
    );
  }

  size_t operator()(StaticMemoValue k) const {
    return std::hash<FuncId>()(k.funcId);
  }
  size_t operator()(StaticMemoCache k) const {
    return std::hash<FuncId>()(k.funcId);
  }

  size_t operator()(LSBMemoValue k) const {
    return folly::hash::hash_combine(
      k.cls.get(), std::hash<FuncId>()(k.funcId)
    );
  }
  size_t operator()(LSBMemoCache k) const {
    return folly::hash::hash_combine(
      k.cls.get(), std::hash<FuncId>()(k.funcId)
    );
  }

  size_t operator()(TSCache k) const {
    return std::hash<FuncId>()(k.funcId);
  }
};

}

std::string symbol_kind(const Symbol& sym) {
  return boost::apply_visitor(SymbolKind(), sym);
}
std::string symbol_rep(const Symbol& sym) {
  return boost::apply_visitor(SymbolRep(), sym);
}
bool symbol_eq(const Symbol& sym1, const Symbol& sym2) {
  return boost::apply_visitor(SymbolEq(), sym1, sym2);
}
size_t symbol_hash(const Symbol& sym) {
  return boost::apply_visitor(SymbolHash(), sym);
}

///////////////////////////////////////////////////////////////////////////////

}}
