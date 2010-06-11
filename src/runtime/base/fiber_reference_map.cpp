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

#include <runtime/base/fiber_reference_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void FiberReferenceMap::insert(void *src, void *copy) {
  ASSERT(lookup(src) == NULL);
  ASSERT(copy == NULL || reverseLookup(copy) == NULL);
  m_forward_references[src] = copy;
  if (copy) {
    m_reverse_references[copy] = src;
  }
}

void *FiberReferenceMap::lookup(void *src) {
  PointerMap::iterator iter = m_forward_references.find(src);
  if (iter != m_forward_references.end()) {
    return iter->second;
  }
  return NULL;
}

void *FiberReferenceMap::reverseLookup(void *copy) {
  PointerMap::iterator iter = m_reverse_references.find(copy);
  if (iter != m_reverse_references.end()) {
    return iter->second;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
