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

#include <runtime/base/memory/sweepable.h>
#include <runtime/base/memory/memory_manager.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(Sweepable::SweepData, Sweepable::s_sweep_data);

void Sweepable::SweepAll() {
  s_sweep_data->sweeping = true;
  SweepableSet &sweepables = s_sweep_data->sweepables;
  SweepableSet persistentObjects;
  for (SweepableSet::iterator iter = sweepables.begin();
       iter != sweepables.end(); ++iter) {
    Sweepable *obj = *iter;
    if (obj->m_persistentCount == 0) {
      obj->sweep();
    } else {
      persistentObjects.insert(obj);
    }
  }
  sweepables.clear();
  if (!persistentObjects.empty()) {
    sweepables = persistentObjects;
  }
  s_sweep_data->sweeping = false;
}

Sweepable::Sweepable() : m_persistentCount(0) {
  if (MemoryManager::TheMemoryManager()->afterCheckpoint()) {
    s_sweep_data->sweepables.insert(this);
  }
}

Sweepable::~Sweepable() {
  if (!s_sweep_data->sweeping) {
    s_sweep_data->sweepables.erase(this);
  }
}

void Sweepable::unregister() {
  s_sweep_data->sweepables.erase(this);
}

///////////////////////////////////////////////////////////////////////////////
}

