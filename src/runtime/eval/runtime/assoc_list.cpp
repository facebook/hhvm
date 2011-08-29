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
#include <runtime/eval/runtime/assoc_list.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(VarAssocPair);

VarAssocPair::VarAssocPair(CStrRef s, VarAssocPair *next /* = NULL */)
  : m_name(s), m_next(next) {}

AssocList::AssocList() : m_head(NULL), m_tail(NULL), m_count(0) {}
AssocList::~AssocList() {
  if (m_head == NULL) return;
  // randomize destruction of variables, so as to make
  // order-dependent coding problems apparent
  int start = rand() % m_count;
  ASSERT(0 <= start && start < m_count);
  VarAssocPair *vp;
  int i = 0;
  for (vp = m_head; i < start; vp = vp->m_next, i++);
  VarAssocPair *startvp = vp;
  ASSERT(startvp);
  VarAssocPair *next;
  for (; vp; vp = next) {
    next = vp->m_next;
    DELETE(VarAssocPair)(vp);
  }
  for (vp = m_head; vp != startvp; vp = next) {
    next = vp->m_next;
    DELETE(VarAssocPair)(vp);
  }
}

Variant &AssocList::append(CStrRef name) {
  if (UNLIKELY(m_head == NULL)) {
    ASSERT(m_tail == NULL);
    ASSERT(m_count == 0);
    m_head  = m_tail = NEW(VarAssocPair)(name);
    m_count = 1;
    return m_head->var();
  }
  ASSERT(m_tail != NULL && m_tail->m_next == NULL);
  ASSERT(m_count > 0);
  VarAssocPair *t = NEW(VarAssocPair)(name);
  m_tail->m_next = t;
  m_tail = t;
  m_count++;
  return m_tail->var();
}

Variant &AssocList::get(CStrRef name) {
  Variant *v = getPtr(name);
  if (!v) return append(name);
  return *v;
}

Variant *AssocList::getPtr(CStrRef name) {
  for (VarAssocPair *vp = m_head; vp; vp = vp->m_next) {
    StringData *s1 = name.get();
    StringData *s2 = vp->name().get();
    ASSERT(s1 && s2);
    if (s1 == s2) return &vp->var();
    // static strings are unique
    if (LIKELY(s1->isStatic() && s2->isStatic())) continue;
    if (s1->same(s2)) return &vp->var();
  }
  return NULL;
}

bool AssocList::exists(CStrRef name, bool checkInit /* = false */) const {
  VarAssocPair *vp;
  for (vp = m_head; vp; vp = vp->m_next) {
    StringData *s1 = name.get();
    StringData *s2 = vp->name().get();
    ASSERT(s1 && s2);
    if (s1 == s2) break;
    // static strings are unique
    if (LIKELY(s1->isStatic() && s2->isStatic())) continue;
    if (s1->same(s2)) break;
  }
  return (vp && (!checkInit || vp->var().isInitialized()));
}

Array AssocList::toArray() const {
  Array res = Array::Create();
  for (VarAssocPair *vp = m_head; vp; vp = vp->m_next) {
    if (vp->var().isInitialized() && vp->name() != "GLOBALS") {
      res.lval(vp->name()).setWithRef(vp->var());
    }
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////
}
}

