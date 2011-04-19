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
  typedef hphp_string_imap<DataType> TypePtrMap;
  static const TypePtrMap &GetTypeHintTypes();

public:
  Construct(CONSTRUCT_ARGS);
  Construct(const Location *loc);
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
  bool is() {
    return dynamic_cast<T*>(this);
  }
  template <class T>
  T *cast() {
    return dynamic_cast<T*>(this);
  }

  // only okay to call in parser phase
  template <class T>
  AstPtr<T> unsafe_cast() {
    T* p = dynamic_cast<T*>(this);
    return AstPtr<T>(p); // not thread-safe
  }

  virtual bool skipDump() const { return false;}
  virtual void dump(std::ostream &out) const = 0;

  template<class T>
  static void dumpVector(std::ostream &out, const std::vector<T> &v,
                         const char* delim = ", ") {
    bool first = true;
    for (uint i = 0; i < v.size(); i++) {
      if (!v[i]->skipDump()) {
        if (first) {
          first = false;
        } else {
          out << delim;
        }
        v[i]->dump(out);
      }
    }
  }
  const Location *loc() const { return &m_loc; }
  void setLoc(Location *loc) { m_loc = *loc;}
  void resetLoc(Parser *parser);
  void dumpLoc() const;
protected:
  Location m_loc;
private:
  int _count;

  static TypePtrMap TypeHintTypes;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_AST_CONSTRUCT_H__ */
