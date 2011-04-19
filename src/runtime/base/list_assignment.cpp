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

#include <runtime/base/list_assignment.h>
#include <runtime/base/complex_types.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ListAssignmentElement

ListAssignmentElement::ListAssignmentElement(Variant &var, int index, ...)
  : m_var(var) {
  va_list ap;
  va_start(ap, index);
  while (index != -1) {
    m_indices.push_back(index);
    index = va_arg(ap, int);
  }
  va_end(ap);
}

void ListAssignmentElement::assign(CArrRef data) {
  ASSERT(!m_indices.empty());
  Variant tmp = data;
  unsigned int size = m_indices.size();
  for (unsigned int i = 0; i < size; i++) {
    tmp = tmp.rvalAt(m_indices[i]);
  }
  m_var = tmp;
}

///////////////////////////////////////////////////////////////////////////////
// global function

Variant list_assign(CVarRef data, ListAssignmentElement *elem, ...) {
  Array adata = data.toArray();
  vector<ListAssignmentElement *> elems;

  va_list ap;
  va_start(ap, elem);
  while (elem) {
    elems.push_back(elem);
    elem = va_arg(ap, ListAssignmentElement *);
  }
  va_end(ap);

  for (int i = elems.size() - 1; i >= 0; i--) {
    elems[i]->assign(adata);
    delete elems[i];
  }
  return data;
}

///////////////////////////////////////////////////////////////////////////////
}
