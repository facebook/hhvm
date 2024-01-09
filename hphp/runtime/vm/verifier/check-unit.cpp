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

#include <stdio.h>
#include <algorithm>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/verifier/check.h"
#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/runtime/vm/verifier/pretty.h"

namespace HPHP {
namespace Verifier {

struct UnitChecker {
  UnitChecker(const UnitEmitter*, ErrorMode mode);
  ~UnitChecker() {}
  bool verify();

 private:
  template<class T> bool checkLiteral(size_t, const T*, const char*);
  bool checkStrings();
  bool checkSourceLocs();
  bool checkPreClasses();
  bool checkFuncs();
  bool checkMetadata();
  bool checkConstructor(
    const FuncEmitter* structor,
    const PreClassEmitter* preclass
  );
  bool checkClosure(const PreClassEmitter* closure);
  bool checkNativeData(const PreClassEmitter* closure);

 private:
  template<class... Args>
  void error(const char* const fmt, Args&&... args) {
    verify_error(
      m_unit,
      nullptr,
      m_errmode == kThrow,
      fmt,
      std::forward<Args>(args)...
    );
  }

 private:
  const UnitEmitter* m_unit;
  ErrorMode m_errmode;

  StringToStringTMap m_createCls;
};

const StaticString s_invoke("__invoke");

bool checkUnit(const UnitEmitter* unit, ErrorMode mode) {
  if (mode == kVerbose) {
    printf("verifying unit from %s\n", unit->m_filepath->data());
  }
  return UnitChecker(unit, mode).verify();
}

// Unit contents to check:
//   o bc
//   o lines
//   o UnitLitStr table
//   o UnitArray table
//   o UnitSourceLoc table
//   o Classes
//   o Functions

UnitChecker::UnitChecker(const UnitEmitter* unit, ErrorMode mode)
: m_unit(unit), m_errmode(mode) {
}

bool UnitChecker::verify() {
  return checkPreClasses() &&
         checkMetadata() &&
         checkFuncs();
}

bool UnitChecker::checkConstructor(
  const FuncEmitter* structor,
  const PreClassEmitter* preclass
) {
  bool ok = true;

  if (structor->attrs & AttrStatic) {
    error("%s in class %s cannot be static\n",
           structor->name->data(), preclass->name()->data());
    ok = false;
  }

  if (structor->isClosureBody) {
    error("%s in class %s cannot be a closure body\n",
           structor->name->data(), preclass->name()->data());
    ok = false;
  }

  return ok;
}

bool UnitChecker::checkClosure(const PreClassEmitter* cls) {
  bool ok = true;
  if (cls->methods().size() != 1 ||
      !cls->hasMethod(s_invoke.get()) ||
      !(cls->lookupMethod(s_invoke.get())->attrs & AttrPublic)) {
    error("Closure %s must have a single public method named __invoke\n",
          cls->name()->data());
    ok = false;
  }

  if (cls->hasMethod(s_invoke.get()) &&
      !(cls->lookupMethod(s_invoke.get())->isClosureBody)) {
    error("Closure %s __invoke method must be a closure body\n",
          cls->name()->data());
    ok = false;
  }

  return ok;
}

bool UnitChecker::checkNativeData(const PreClassEmitter* cls) {
  if (!RuntimeOption::EvalVerifySystemLibHasNativeImpl) {
    return true;
  }

  auto nativeData = Native::getNativeDataInfo(cls->name());
  if (!nativeData) {
    error("Class %s's NativeData is not registered\n",
          cls->name()->data());
    return false;
  }

  return true;
}

/* Check the following conditions:
   - All constructors/destructors are non-static and not closure bodies
   - Properties/Methods have exactly one access modifier
   - Preclasses do not inherit from themselves
   - Interfaces cannot be final
   - Methods cannot be both abstract and final
   - Classish cannot be both final and sealed
*/
const StaticString s___Sealed("__Sealed");
const StaticString s___NativeData("__NativeData");
const StaticString s_Closure("Closure");
bool UnitChecker::checkPreClasses() {
  bool ok = true;

  for (auto const preclass : m_unit->preclasses()) {
    auto classAttrs = preclass->attrs();
    const auto& userAttrs = preclass->userAttributes();

    // Closures don't need constructors
    if (preclass->parentName()->tsame(s_Closure.get())) {
      ok &= checkClosure(preclass);
    }

    if (!preclass->parentName()->empty() &&
          preclass->name()->equal(preclass->parentName())) {
        ok = false;
        error("Class %s inherits from itself\n", preclass->name()->data());
      }

    if (classAttrs & AttrFinal && classAttrs & AttrInterface) {
        ok = false;
        error("Class %s is a final interface\n", preclass->name()->data());
    }

    if (classAttrs & AttrIsConst) {
      if (classAttrs & (AttrEnum | AttrEnumClass | AttrInterface | AttrTrait)) {
        error("Class %s violates that interfaces, traits and enums may not be const",
              preclass->name()->data());
      }
      if (!(classAttrs & AttrForbidDynamicProps)) {
        error("Const class %s missing ForbidDynamicProps attribute",
              preclass->name()->data());
      }
    }

    if (classAttrs & AttrSealed) {
      const auto sealed_attr = userAttrs.find(s___Sealed.get())->second;
      IterateV(
        sealed_attr.m_data.parr, [this, preclass, &ok](TypedValue tv) -> bool {
          if (!isStringType(tv.m_type) &&
              !isLazyClassType(tv.m_type)) {
            ok = false;
            error("For Class %s, values in sealed whitelist must be strings\n",
                  preclass->name()->data());
            return true;
          }
          return false;
        });
      if (classAttrs & AttrFinal && !(classAttrs & AttrTrait)) {
        ok = false;
        error("Class %s is both final and sealed\n", preclass->name()->data());
      }
    }

    if (userAttrs.find(s___NativeData.get()) != userAttrs.end()) {
      ok &= checkNativeData(preclass);
    }

    for(auto& prop : preclass->propMap().ordered_range()) {
      Attr attributes = prop.attrs();
      int access_modifiers = (attributes & AttrPublic ? 1 : 0) +
                             (attributes & AttrProtected ? 1 : 0) +
                             (attributes & AttrPrivate ? 1 : 0);
      if (access_modifiers > 1) {
        ok = false;
        error("Property %s in class %s has more than one access modifier\n",
               prop.name()->data(), preclass->name()->data());
      }
      if (access_modifiers == 0) {
        ok = false;
        error("Property %s in class %s has no access modifier\n",
               prop.name()->data(), preclass->name()->data());
      }
      if (attributes & AttrIsConst) {
        if (attributes & AttrLateInit) {
          error("Const property %s in class %s may not also be late init",
                prop.name()->data(), preclass->name()->data());
        }
      }
    }

    for(auto method : preclass->methods()) {
      Attr attributes = method->attrs;
      auto name = method->name->toCppString();
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);

      int access_modifiers = (attributes & AttrPublic ? 1 : 0) +
                             (attributes & AttrProtected ? 1 : 0) +
                             (attributes & AttrPrivate ? 1 : 0);
      if (access_modifiers > 1) {
        ok = false;
        error("Method %s in class %s has more than one access modifier\n",
               method->name->data(), preclass->name()->data());
      }
      if (access_modifiers == 0) {
        ok = false;
        error("Method %s in class %s has no access modifier\n",
               method->name->data(), preclass->name()->data());
      }

      if ((attributes & AttrFinal) && (attributes & AttrAbstract)) {
        ok = false;
        error("Method %s in class %s is both abstract and final\n",
               method->name->data(), preclass->name()->data());
      }

      auto className = preclass->name()->toCppString();
      std::transform(className.begin(), className.end(), className.begin(),
                      ::tolower);

      if (name == std::string("__construct")) {
        ok &= checkConstructor(method, preclass);
      }
    }
  }

  return ok;
}

//TODO: T19445287 implement this as the fuzzer finds more bugs
bool UnitChecker::checkMetadata() {
  return true;
}

bool UnitChecker::checkFuncs() {
  bool ok = true;

  auto doCheck = [&] (const FuncEmitter* func) {
    if (func->isNative) ok &= checkNativeFunc(func, m_errmode);
    ok &= checkFunc(func, m_createCls, m_errmode);
  };

  for (auto& func : m_unit->fevec()) doCheck(func.get());

  for (auto const pce : m_unit->preclasses()) {
    for (auto f : pce->methods()) doCheck(f);
  }

  return ok;
}

}} // HPHP::Verifier
