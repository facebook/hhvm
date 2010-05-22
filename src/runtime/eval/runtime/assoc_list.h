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
#include <runtime/eval/base/eval_base.h>
#include <list>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class VarAssocPair {
public:
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(VarAssocPair);
  VarAssocPair(CStrRef s, VarAssocPair *next = NULL);
  Variant &var() { return m_var; }
  CStrRef name() const { return m_name; }
  void dump() const {}
  VarAssocPair *next() const { return m_next; }
private:
  String m_name;
  Variant m_var;
  VarAssocPair *m_next;
};

class AssocList {
public:
  AssocList();
  ~AssocList();
  Variant &prepend(CStrRef name);
  Variant &get(CStrRef name);
  Variant *getPtr(CStrRef name);
  bool exists(CStrRef name) const;
  Array toArray() const;
private:
  VarAssocPair *m_list;
};

///////////////////////////////////////////////////////////////////////////////
}
}

