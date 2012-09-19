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

#include <runtime/base/memory/sweepable.h>
#include <runtime/base/memory/memory_manager.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static __thread Sweepable* t_sweepList = 0;

void Sweepable::SweepAll() {
  Sweepable* persistList = 0;
  while (t_sweepList) {
    Sweepable* s = t_sweepList;
    s->unregister();

    if (s->m_persistentCount == 0) {
      s->sweep();
    } else {
      if (persistList) {
        ASSERT(persistList->m_prevSweepable = &persistList);
        persistList->m_prevSweepable = &s->m_nextSweepable;
      }
      s->m_nextSweepable = persistList;
      persistList = s;
      s->m_prevSweepable = &persistList;
    }
  }
  t_sweepList = persistList;
  if (persistList) {
    ASSERT(persistList->m_prevSweepable == &persistList);
    persistList->m_prevSweepable = &t_sweepList;
  }
}

Sweepable::Sweepable()
  : m_nextSweepable(t_sweepList)
  , m_prevSweepable(&t_sweepList)
  , m_persistentCount(0)
{
  if (t_sweepList) {
    ASSERT(t_sweepList->m_prevSweepable == &t_sweepList);
    t_sweepList->m_prevSweepable = &m_nextSweepable;
  }
  t_sweepList = this;
}

Sweepable::~Sweepable() {
  unregister();
}

void Sweepable::unregister() {
  if (m_prevSweepable) {
    if (m_nextSweepable) {
      m_nextSweepable->m_prevSweepable = m_prevSweepable;
    }
    *m_prevSweepable = m_nextSweepable;
    m_nextSweepable = 0;
    m_prevSweepable = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
}

