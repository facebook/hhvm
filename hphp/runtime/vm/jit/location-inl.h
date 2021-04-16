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

#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline uint32_t Location::localId() const {
  assertx(m_tag == LTag::Local);
  return m_local.locId;
}

inline SBInvOffset Location::stackIdx() const {
  assertx(m_tag == LTag::Stack);
  return m_stack.stackIdx;
}

inline bool Location::operator==(const Location& other) const {
  if (m_tag != other.m_tag) return false;

  switch (m_tag) {
    case LTag::Local:
      return localId() == other.localId();
    case LTag::Stack:
      return stackIdx() == other.stackIdx();
    case LTag::MBase:
      return true;
  }
  not_reached();
  return false;
}

inline bool Location::operator!=(const Location& other) const {
  return !(*this == other);
}

inline bool Location::operator<(const Location& other) const {
  if (m_tag < other.m_tag) return true;
  if (m_tag > other.m_tag) return false;
  switch (m_tag) {
    case LTag::Local:
      return localId() < other.localId();
    case LTag::Stack:
      return stackIdx() < other.stackIdx();
    case LTag::MBase:
      return false;
  }
  not_reached();
  return false;
}

///////////////////////////////////////////////////////////////////////////////

}}
