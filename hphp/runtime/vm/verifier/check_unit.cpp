/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/verifier/check.h"
#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/runtime/vm/verifier/pretty.h"

namespace HPHP {
namespace Verifier {

class UnitChecker {
 public:
  UnitChecker(const Unit*, bool verbose);
  ~UnitChecker() {}
  bool verify();
 private:
  template<class T> bool checkLiteral(size_t, const T*, const char*);
  bool checkStrings();
  bool checkArrays();
  bool checkSourceLocs();
  bool checkPreConsts();
  bool checkPreClasses();
  bool checkFuncs();
  bool checkBytecode();
  bool checkMetadata();
 private:
  const Unit* m_unit;
  bool m_verbose;
};

bool checkUnit(const Unit* unit, bool verbose) {
  if (verbose) {
    verify_error("verifying unit from %s\n", unit->filepath()->data());
  }
  return UnitChecker(unit, verbose).verify();
}

// Unit contents to check:
// 1. bc
// 2. bc_meta
// 3. lines
// 4. UnitLitStr table
// 5. UnitArray table
// 6. UnitSourceLoc table
// 7. UnitPreConst table
// 8. Classes
// 9. Functions

UnitChecker::UnitChecker(const Unit* unit, bool verbose)
: m_unit(unit), m_verbose(verbose) {
}

bool UnitChecker::verify() {
  return checkStrings() &&
         checkArrays() &&
         //checkSourceLocs() &&
         //checkPreConsts() &&
         //checkPreClasses() &&
         checkBytecode() &&
         //checkMetadata() &&
         checkFuncs();
}

template<class LitType>
bool UnitChecker::checkLiteral(size_t id,
                               const LitType* lt,
                               const char* what) {
  bool ok = true;
  if (!lt) {
    verify_error("null %s id %zu in unit %s\n", what, id,
                 m_unit->md5().toString().c_str());
    ok = false;
  }
  if (!lt->isStatic()) {
    verify_error("non-static %s id %zu in unit %s\n", what, id,
                 m_unit->md5().toString().c_str());
    ok = false;
  }
  return ok;
}

bool UnitChecker::checkStrings() {
  bool ok = true;
  for (size_t i = 0, n = m_unit->numLitstrs(); i < n; ++i) {
    ok &= checkLiteral(i, m_unit->lookupLitstrId(i), "string");
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
    ok &= checkLiteral(i, m_unit->lookupArrayId(i), "array");
  }
  return ok;
}

/**
 * Check that every byte in the unit's bytecode is inside exactly one
 * function's code region.
 */
bool UnitChecker::checkBytecode() {
  bool ok = true;
  typedef std::map<Offset, const Func*> FuncMap; // ordered!
  FuncMap funcs;
  for (AllFuncs i(m_unit); !i.empty();) {
    const Func* f = i.popFront();
    if (f->past() <= f->base()) {
      if (!f->isAbstract() || f->past() < f->base()) {
        verify_error("func size <= 0 [%d:%d] in unit %s\n",
             f->base(), f->past(), m_unit->md5().toString().c_str());
        ok = false;
        continue;
      }
    }
    if (f->base() < 0 || f->past() > m_unit->bclen()) {
      verify_error("function region [%d:%d] out of unit %s bounds [%d:%d]\n",
             f->base(), f->past(), m_unit->md5().toString().c_str(),
             0, m_unit->bclen());
      ok = false;
      continue;
    }
    if (funcs.find(f->base()) != funcs.end()) {
      verify_error("duplicate function-base at %d in unit %s\n",
             f->base(), m_unit->md5().toString().c_str());
      ok = false;
      continue;
    }
    funcs.insert(FuncMap::value_type(f->base(), f));
  }
  // iterate funcs in offset order, checking for holes and overlap
  if (funcs.empty()) {
    verify_error("unit %s must have at least one func\n",
           m_unit->md5().toString().c_str());
    return false;
  }
  Offset last_past = 0;
  for (FuncMap::iterator i = funcs.begin(), e = funcs.end(); i != e; ) {
    const Func* f = (*i).second; ++i;
    if (f->base() < last_past) {
      verify_error("function overlap [%d:%d] in unit %s\n",
             f->base(), last_past, m_unit->md5().toString().c_str());
      ok = false;
    } else if (f->base() > last_past) {
      verify_error("dead bytecode space [%d:%d] in unit %s\n",
             last_past, f->base(), m_unit->md5().toString().c_str());
      ok = false;
    }
    last_past = f->past();
    if (i == e && last_past != m_unit->bclen()) {
      verify_error("dead bytecode [%d:%d] at end of unit %s\n",
             last_past, m_unit->bclen(), m_unit->md5().toString().c_str());
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
  const Func* pseudo = 0;
  bool multi = false;

  bool ok = true;
  for (AllFuncs i(m_unit); !i.empty();) {
    if (i.front()->isPseudoMain()) {
      if (pseudo) {
        multi = true;
        verify_error("unit should have exactly one pseudo-main\n");
        ok = false;
      }
      pseudo = i.front();
    }
    ok &= checkFunc(i.popFront(), m_verbose);
  }

  if (!multi && m_unit->getMain() != pseudo) {
    verify_error("funcs and unit disagree on what is the pseudo-main\n");
    ok = false;
  }

  return ok;
}

}} // HPHP::Verifier
