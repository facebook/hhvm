/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_VM_NAMED_ENTITY_INL_H_
#error "named-entity-inl.h should only be included by named-entity.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline NamedType::NamedType(NamedType&& ne) noexcept
  : m_cachedClass(ne.m_cachedClass)
  // Since cached type alias and cached reified generics are a union
  // the following line will set them both
  , m_cachedTypeAlias(ne.m_cachedTypeAlias)
{
  m_clsList = ne.m_clsList;
}

inline Class* NamedType::getCachedClass() const {
  return LIKELY(m_cachedClass.bound() && m_cachedClass.isInit())
    ? *m_cachedClass
    : nullptr;
}

inline ArrayData* NamedType::getCachedReifiedGenerics() const {
  return LIKELY(m_cachedReifiedGenerics.bound() &&
                m_cachedReifiedGenerics.isInit())
    ? *m_cachedReifiedGenerics
    : nullptr;
}

inline Class* NamedType::clsList() const {
  return m_clsList;
}

///////////////////////////////////////////////////////////////////////////////

inline NamedFunc::NamedFunc(NamedFunc&& ne) noexcept
  : m_cachedFunc(ne.m_cachedFunc)
{}

inline Func* NamedFunc::getCachedFunc() const {
  return LIKELY(m_cachedFunc.bound() && m_cachedFunc.isInit())
    ? *m_cachedFunc
    : nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
