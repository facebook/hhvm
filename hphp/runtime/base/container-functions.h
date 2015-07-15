/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_CONTAINER_FUNCTIONS_H_
#define incl_HPHP_CONTAINER_FUNCTIONS_H_

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline bool isContainer(const Cell c) {
  assert(cellIsPlausible(c));
  return c.m_type == KindOfArray ||
         (c.m_type == KindOfObject && c.m_data.pobj->isCollection());
}

inline bool isContainer(const Variant& v) {
  return isContainer(*v.asCell());
}

inline bool isContainerOrNull(const Cell c) {
  assert(cellIsPlausible(c));
  return IS_NULL_TYPE(c.m_type) || c.m_type == KindOfArray ||
         (c.m_type == KindOfObject && c.m_data.pobj->isCollection());
}

inline bool isContainerOrNull(const Variant& v) {
  return isContainerOrNull(*v.asCell());
}

inline bool isMutableContainer(const Cell c) {
  assert(cellIsPlausible(c));
  return c.m_type == KindOfArray ||
         (c.m_type == KindOfObject && c.m_data.pobj->isMutableCollection());
}

inline bool isMutableContainer(const Variant& v) {
  return isMutableContainer(*v.asCell());
}

inline size_t getContainerSize(const Cell c) {
  assert(isContainer(c));
  if (c.m_type == KindOfArray) {
    return c.m_data.parr->size();
  }
  assert(c.m_type == KindOfObject && c.m_data.pobj->isCollection());
  return getCollectionSize(c.m_data.pobj);
}

inline size_t getContainerSize(const Variant& v) {
  return getContainerSize(*v.asCell());
}

inline bool isPackedContainer(const Cell c) {
  assert(isContainer(c));
  if (c.m_type == KindOfArray) {
    return c.m_data.parr->isPacked();
  }

  return isVectorCollection(c.m_data.pobj->collectionType());
}

//////////////////////////////////////////////////////////////////////

}

#endif
