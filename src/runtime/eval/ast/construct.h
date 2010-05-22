/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EVAL_AST_CONSTRUCT_H__
#define __EVAL_AST_CONSTRUCT_H__

#include <runtime/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Construct);

class Parser;

#define CONSTRUCT_ARGS Parser *parser
#define CONSTRUCT_PASS parser

/**
 * Not deriving from Countable because I don't want _count to be mutable
 */
class Construct {
public:
  Construct(CONSTRUCT_ARGS);
  virtual ~Construct() {}
  void release();
  void incRefCount() {
    ++_count;
  }
  int decRefCount() {
    ASSERT(_count > 0);
    return --_count;
  }
  int getCount() const {
    return _count;
  }

  template <class T>
  AstPtr<T> cast() {
    T* p = dynamic_cast<T*>(this);
    return AstPtr<T>(p);
  }
  virtual void dump() const;

  template<class T>
  static void dumpVector(const std::vector<T> &v, const char* delim) {
    bool first = true;
    for (uint i = 0; i < v.size(); i++) {
      if (first) {
        first = false;
      } else {
        printf("%s", delim);
      }
      v[i]->dump();
    }
  }
  const Location *loc() const { return &m_loc; }
  void dumpLoc() const;
protected:
  Location m_loc;
private:
  int _count;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_CONSTRUCT_H__ */
