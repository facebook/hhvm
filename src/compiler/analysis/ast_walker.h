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

#ifndef __AST_WALKER_H__
#define __AST_WALKER_H__

#include <compiler/hphp.h>
#include <compiler/construct.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AstWalkerState {
public:
  AstWalkerState() : index(0) {}
  AstWalkerState(ConstructRawPtr c) : cp(c), index(0) {}

  friend bool operator==(const AstWalkerState &s1,
                         const AstWalkerState &s2) {
    return s1.cp == s2.cp && s1.index == s2.index;
  }

  ConstructRawPtr       cp;
  int                   index;
};

class AstWalkerStateVec : public std::vector<AstWalkerState> {
  public:
  AstWalkerStateVec() {}
  AstWalkerStateVec(ConstructRawPtr cp) {
    push_back(AstWalkerState(cp));
  }
};

class AstWalker {
  public:
  enum {
    WalkContinue,
    WalkSkip,
    WalkStop
  };
  int before(ConstructRawPtr) { return WalkContinue; }
  int beforeEach(ConstructRawPtr, int, ConstructRawPtr) { return WalkContinue; }
  int afterEach(ConstructRawPtr, int, ConstructRawPtr) { return WalkContinue; }
  int after(ConstructRawPtr) { return WalkContinue; }

  template<class T>
  static void walk(T &functor, AstWalkerStateVec &state,
                   ConstructRawPtr endBefore, ConstructRawPtr endAfter) {
    int size = state.size();
    if (!size) return;
    int flag;

    AstWalkerState *cfs = &state.back();
    while (true) {
      ConstructRawPtr cur = cfs->cp;
      int ix = cfs->index;
      if (!ix) {
        if (cur == endBefore) break;
        flag = functor.before(cur);
        if (flag == WalkStop) break;
        if (flag == WalkSkip) {
          state.pop_back();
          if (!--size) break;
          cfs = &state.back();
        }
        cfs->index++;
        continue;
      }
      int ii = (ix - 1) >> 1;
      if (ii < cur->getKidCount()) {
        if (ConstructRawPtr kid = cur->getNthKid(ii)) {
          if (ix & 1) {
            flag = functor.beforeEach(cur, ii, kid);
            if (flag == WalkStop) break;
            if (flag != WalkSkip) {
              state.push_back(AstWalkerState(kid));
              size++;
              cfs = &state.back();
              continue;
            }
          } else {
            flag = functor.afterEach(cur, ii, kid);
            if (flag == WalkStop) break;
          }
        }
        cfs->index++;
        continue;
      }
      if (functor.after(cur) == WalkStop || cur == endAfter) break;
      state.pop_back();
      if (!--size) break;
      cfs = &state.back();
      cfs->index++;
    }
  }
};

class FunctionWalker : public AstWalker {
  public:
  int before(ConstructRawPtr cp);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __AST_WALKER_H__
