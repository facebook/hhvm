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

#include <cpp/base/shared/shared_string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SharedStringData::InternMap SharedStringData::s_intern;

SharedStringData::SharedStringData(const std::string &data) : m_count(1),
                                                              m_data(data) {}

int SharedStringData::decRefCount() const {
  ASSERT(m_count > 0);
  int cur = atomic_dec(m_count);
  if (cur == 1) {
    // Possibly only left in intern map
    InternMap::accessor acc;
    bool found = s_intern.find(acc, m_data);
    ASSERT(found);
    if (!found) return 1;
    // Write lock on the map element, if count is still 1 then
    // can erase
    if (m_count == 1) {
      s_intern.erase(acc);
      m_count = 0;
      return 0;
    }
    // May technically not be 1 but doesn't matter
    return 1;
  }
  return cur;
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
  }
}

SharedString &SharedString::operator=(const std::string &data) {
  SharedStringData::InternMap::accessor acc;
  SharedStringData::Create(acc, data);
  return operator=(acc->second);
}

///////////////////////////////////////////////////////////////////////////////
}
