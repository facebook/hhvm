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

#include "string_bag.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringBag::StringBag(int reserve_count /* = 0 */) {
  if (reserve_count > 0) {
    m_strings.reserve(reserve_count);
  }
}

StringBag::~StringBag() {
  for (unsigned int i = 0; i < m_strings.size(); i++) free(m_strings[i]);
}

const char *StringBag::add(const char *s) {
  char *allocated = strdup(s);
  m_strings.push_back(allocated);
  return allocated;
}

const char *StringBag::at(unsigned int index) const {
  ASSERT(index < m_strings.size());
  return m_strings[index];
}

///////////////////////////////////////////////////////////////////////////////
}
