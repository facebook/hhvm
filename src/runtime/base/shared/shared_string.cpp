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

#include <runtime/base/shared/shared_string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SharedStringData::InternMap SharedStringData::s_intern;

SharedStringData::SharedStringData(const std::string &data) : m_data(data) {
  m_count = 1;
}

int SharedStringData::decRefCount() const {
  ASSERT(m_count > 0);
  int cur = m_count.fetch_and_decrement() - 1;
  if (cur == 0) {
    // Only left in intern map. While this data is still in the
    // the map we consider this data dying. All new Creates will
    // overwrite the SharedStringData* in the slot and not use this one.
    InternMap::accessor acc;
    bool found = s_intern.find(acc, m_data);
    ASSERT(m_count == 0);
    // May not be found or may not be acc->second if other thread overwrote.
    // Safe to just delete self.
    if (found && acc->second == this) {
      s_intern.erase(acc);
    }
    return 0;
  }
  return 1;
}

void SharedStringData::release() {
  delete this;
}

const std::string &SharedStringData::getString() const {
  return m_data;
}

void SharedStringData::Create(InternMap::accessor &acc,
                              const std::string &data) {
  if (s_intern.insert(acc, data)) {
    acc->second = new SharedStringData(acc->first);
    return;
  }
  // Danger is now that the data is dying. We must not acquire a reference
  // if it in that state.
  while (true) {
    int cur = acc->second->m_count;
    if (cur == 0) {
      acc->second = new SharedStringData(acc->first);
      break;
    }
    // Never change refcount of 0 to refcount of 1, impossible
    // to revive a dying SharedStringData.
    if (acc->second->m_count.compare_and_swap(cur + 1, cur) == cur) {
      break;
    }
    // Other thread must have inc/dec ref'd since we got cur
  }
}

SharedString &SharedString::operator=(const std::string &data) {
  reset();
  SharedStringData::InternMap::accessor acc;
  SharedStringData::Create(acc, data);
  // Ref count already incremented in Create
  m_px = acc->second;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
}
