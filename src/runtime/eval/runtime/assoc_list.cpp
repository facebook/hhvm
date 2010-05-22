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
#include <runtime/eval/runtime/assoc_list.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(VarAssocPair);

VarAssocPair::VarAssocPair(CStrRef s, VarAssocPair *next /* = NULL */)
  : m_name(s), m_next(next) {}

AssocList::AssocList() : m_list(NULL) {}
AssocList::~AssocList() {
  VarAssocPair *vp = m_list;
  while (vp) {
    VarAssocPair *tp = vp->next();
    DELETE(VarAssocPair)(vp);
    vp = tp;
  }
}

Variant &AssocList::prepend(CStrRef name) {
  m_list = NEW(VarAssocPair)(name, m_list);
  return m_list->var();
}

Variant &AssocList::get(CStrRef name) {
  Variant *v = getPtr(name);
  if (!v) return prepend(name);
  return *v;
}

Variant *AssocList::getPtr(CStrRef name) {
  for (VarAssocPair *vp = m_list; vp; vp = vp->next()) {
    if (name.same(vp->name())) {
      return &vp->var();
    }
  }
  return NULL;
}

bool AssocList::exists(CStrRef name) const {
  for (VarAssocPair *vp = m_list; vp; vp = vp->next()) {
    if (name.same(vp->name())) {
      return true;
    }
  }
  return false;
}

Array AssocList::toArray() const {
  Array res = Array::Create();
  for (VarAssocPair *vp = m_list; vp; vp = vp->next()) {
    if (vp->var().isInitialized()) {
      res.set(vp->name(), vp->var());
    }
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////
}
}

