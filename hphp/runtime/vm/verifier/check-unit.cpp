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
  bool checkArrays();
  bool checkSourceLocs();
  bool checkPreClasses();
  bool checkFuncs();
  bool checkBytecode();
  bool checkMetadata();
  bool checkStructor(
    const FuncEmitter* structor,
    const PreClassEmitter* preclass
  );
  bool checkClosure(const PreClassEmitter* closure);

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
  return checkStrings() &&
         checkArrays() &&
         //checkSourceLocs() &&
         checkPreClasses() &&
         checkBytecode() &&
         checkMetadata() &&
         checkFuncs();
}

template<class LitType>
bool UnitChecker::checkLiteral(size_t id,
                               const LitType* lt,
                               const char* what) {
  bool ok = true;
  if (!lt) {
    error("null %s id %zu in unit %s\n", what, id,
                 m_unit->md5().toString().c_str());
    ok = false;
  }
  if (!lt->isStatic()) {
    error("non-static %s id %zu in unit %s\n", what, id,
                 m_unit->md5().toString().c_str());
    ok = false;
  }
  return ok;
}

bool UnitChecker::checkStrings() {
  bool ok = true;
  for (size_t i = 0, n = m_unit->numLitstrs(); i < n; ++i) {
    ok &= checkLiteral(i, m_unit->lookupLitstr(i), "string");
  }
  return ok;
  // Notes
  // * Any string in repo can be null.  repo litstrId is checked on load
  // then discarded.  Each Litstr entry becomes a static string.
  // Strings are hash-commoned so presumably only one can be null per unit.
  // string_data_hash and string_data_same both crash/assert on null.
  // * If DB has dups then UnitEmitter commons them - spec should outlaw
  // dups because UE will assert if db has dups with different ids.
  // StringData statically keeps a map of loaded static strings
  // * UE keeps a (String->id) mapping and assigns dups the same id, plus
  // a table of litstrs indexed by id.  Unit stores them as
  // m_namedInfo, a vector<StringData,NamedEntity=null> of pairs.
  // * are null characters allowed inside the string?
  // * are strings utf8-encoded?
}

bool UnitChecker::checkArrays() {
  bool ok = true;
  for (size_t i = 0, n = m_unit->numArrays(); i < n; ++i) {
    ok &= checkLiteral(i, m_unit->lookupArray(i), "array");
  }
  return ok;
}

bool UnitChecker::checkStructor(
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

bool UnitChecker::checkClosure(const PreClassEmitter* cls){
  bool ok = true;
  if (!(cls->attrs() & AttrUnique)) {
    error("Closure %s must be uniquely named\n", cls->name()->data());
    ok = false;
  }

  auto invalidMethods = [&] {
    error("Closure %s must have a single public method named __invoke\n",
          cls->name()->data());
    ok = false;
  };

  if (!cls->hasMethod(s_invoke.get()) ||
      !(cls->lookupMethod(s_invoke.get())->attrs & AttrPublic)) {
    invalidMethods();
  } else if (cls->methods().size() == 2) {
    auto invoke = cls->lookupMethod(s_invoke.get());
    for (auto m : cls->methods()) {
      if (m != invoke && !(m->attrs & AttrIsInOutWrapper)) {
        invalidMethods();
        break;
      }
    }
  } else if (cls->methods().size() != 1) {
    invalidMethods();
  }

  if (cls->hasMethod(s_invoke.get()) &&
      !(cls->lookupMethod(s_invoke.get())->isClosureBody)) {
    error("Closure %s __invoke method must be a closure body\n",
          cls->name()->data());
    ok = false;
  }

  return ok;
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
bool UnitChecker::checkPreClasses() {
  bool ok = true;

  for (Id pceId = 0; pceId < m_unit->numPreClasses(); ++pceId) {
    auto preclass = m_unit->pce(pceId);
    auto classAttrs = preclass->attrs();

    // Closures don't need constructors
    if (preclass->parentName()->toCppString() == std::string("Closure")) {
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

    if (classAttrs & AttrSealed) {
      const auto sealed_attr =
        preclass->userAttributes().find(s___Sealed.get())->second;
      IterateV(
        sealed_attr.m_data.parr, [this, preclass, &ok](TypedValue tv) -> bool {
          if (!isStringType(tv.m_type)) {
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

    for(auto it : preclass->propMap()) {
      auto prop = preclass->propMap()[it.second];
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

      if (name == std::string("__construct") ||
          (name == className && !(classAttrs & AttrTrait))) {
        ok &= checkStructor(method, preclass);
      }

      if (name == std::string("__destruct")) {
        ok &= checkStructor(method, preclass);
      }
    }
  }

  return ok;
}

//TODO: T19445287 implement this as the fuzzer finds more bugs
bool UnitChecker::checkMetadata() {
  return true;
}

/**
 * Check that every byte in the unit's bytecode is inside exactly one
 * function's code region.
 */
bool UnitChecker::checkBytecode() {
  bool ok = true;
  typedef std::map<Offset, const FuncEmitter*> FuncMap; // ordered!
  FuncMap funcs;
  auto addFunc = [&] (const FuncEmitter* f) {
    if (f->past <= f->base) {
      if (!(f->attrs & AttrAbstract) || f->past < f->base) {
        error("func size <= 0 [%d:%d] in unit %s\n",
             f->base, f->past, m_unit->md5().toString().c_str());
        ok = false;
        return;
      }
    }
    if (f->base < 0 || f->past > m_unit->bcPos()) {
      error("function region [%d:%d] out of unit %s bounds [%d:%d]\n",
             f->base, f->past, m_unit->md5().toString().c_str(),
             0, m_unit->bcPos());
      ok = false;
      return;
    }
    if (funcs.find(f->base) != funcs.end()) {
      error("duplicate function-base at %d in unit %s\n",
             f->base, m_unit->md5().toString().c_str());
      ok = false;
      return;
    }
    funcs.insert(FuncMap::value_type(f->base, f));
  };
  for (auto f : m_unit->fevec()) addFunc(f);
  for (Id i = 0; i < m_unit->numPreClasses(); i++) {
    for (auto f : m_unit->pce(i)->methods()) addFunc(f);
  }
  // iterate funcs in offset order, checking for holes and overlap
  if (funcs.empty()) {
    error("unit %s must have at least one func\n",
           m_unit->md5().toString().c_str());
    return false;
  }
  Offset last_past = 0;
  for (FuncMap::iterator i = funcs.begin(), e = funcs.end(); i != e; ) {
    auto const f = (*i++).second;
    if (f->base < last_past) {
      error("function overlap [%d:%d] in unit %s\n",
             f->base, last_past, m_unit->md5().toString().c_str());
      ok = false;
    } else if (f->base > last_past) {
      error("dead bytecode space [%d:%d] in unit %s\n",
             last_past, f->base, m_unit->md5().toString().c_str());
      ok = false;
    }
    last_past = f->past;
    if (i == e && last_past != m_unit->bcPos()) {
      error("dead bytecode [%d:%d] at end of unit %s\n",
             last_past, m_unit->bcPos(), m_unit->md5().toString().c_str());
      ok = false;
    }
  }
  return ok;
  // Notes
  // 1. Bytecode regions for every function must not overlap and must exactly
  // divide up the bytecode of the whole unit.
  // 2. Its not an error for an abstract function to have zero size.
}

bool UnitChecker::checkFuncs() {
  const FuncEmitter* pseudo = nullptr;
  bool multi = false;
  bool ok = true;
  bool nonTops = true;

  auto doCheck = [&] (const FuncEmitter* func) {
    if (func->isPseudoMain()) {
      if(func->isMemoizeWrapper) {
        error("%s", "pseudo-main cannot be a memoize wrapper\n");
        ok = false;
      }
      if (pseudo) {
        multi = true;
        error("%s", "unit should have exactly one pseudo-main\n");
        ok = false;
      }
      pseudo = func;
    }

    if (!func->pce()) {
      if (!nonTops && !func->top) {
        error("Non-top function %s defined after a toplevel function\n",
              func->name->data());
        ok = false;
      }
      nonTops &= !func->top;
    }

    if (func->isNative) {
      ok &= checkNativeFunc(func, m_errmode);
    }

    ok &= checkFunc(func, m_errmode);
  };

  for (auto func : m_unit->fevec()) doCheck(func);

  for (Id i = 0; i < m_unit->numPreClasses(); i++) {
    for (auto f : m_unit->pce(i)->methods()) doCheck(f);
  }

  if (!multi && m_unit->getMain() != pseudo) {
    error("%s", "funcs and unit disagree on what is the pseudo-main\n");
    ok = false;
  }

  return ok;
}

}} // HPHP::Verifier
