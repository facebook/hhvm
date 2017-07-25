/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_CLS_CNS_PROFILE_H_
#define incl_HPHP_JIT_CLS_CNS_PROFILE_H_

#include "hphp/runtime/base/types.h"

namespace HPHP {

struct Class;
struct TypedValue;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct ClsCnsProfile {
  ClsCnsProfile() : m_curSlot(0) {}

  ClsCnsProfile(const ClsCnsProfile& other)
    : m_curSlot(other.m_curSlot)
  {}

  std::string toString() const;

  /*
   * Obtain the profiled Slot
   */
  const Slot getSlot() const {
    return m_curSlot == 0 || m_curSlot == kInvalidSlot ?
      kInvalidSlot : m_curSlot - 1;
  }

  /*
   * Register a use of a class constant
   */
  const TypedValue* reportClsCns(const Class* cls, const StringData* cns);

  /*
   * Aggregate two ClsCnsProfiles.
   */
  static void reduce(ClsCnsProfile& a, const ClsCnsProfile& b);

  /////////////////////////////////////////////////////////////////////////////

private:
  Slot m_curSlot;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
