/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

inline NamedEntity::NamedEntity(NamedEntity&& ne) noexcept
  : m_cachedClass(ne.m_cachedClass)
  , m_cachedFunc(ne.m_cachedFunc)
  , m_cachedTypeAlias(ne.m_cachedTypeAlias)
{
  m_clsList = ne.m_clsList;
}

inline rds::Handle NamedEntity::getFuncHandle() const {
  m_cachedFunc.bind();
  return m_cachedFunc.handle();
}

inline rds::Handle NamedEntity::getClassHandle() const {
  m_cachedClass.bind();
  return m_cachedClass.handle();
}

inline Class* NamedEntity::clsList() const {
  return m_clsList;
}

///////////////////////////////////////////////////////////////////////////////
}
