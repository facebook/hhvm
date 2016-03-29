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

#ifndef incl_HPHP_JIT_LOCATION_H_
#define incl_HPHP_JIT_LOCATION_H_

#include "hphp/runtime/vm/jit/stack-offsets.h"

#include <cstdint>
#include <string>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

enum class LTag : uint32_t {
  Local,
  Stack,
};

/*
 * An HHBC-visible location that we track during irgen---for use in guards,
 * hints, region post-conditions, etc.
 *
 * This is currently either local variables or stack slots.  Local variables
 * are addressed by local id, and stack slots are addressed by offset from the
 * frame pointer.
 */
struct Location {
  struct Local { uint32_t locId; };
  struct Stack { FPInvOffset stackIdx; };

  /* implicit */ Location(Local l) : m_tag{LTag::Local}, m_local(l) {}
  /* implicit */ Location(Stack s) : m_tag{LTag::Stack}, m_stack(s) {}

  LTag tag() const { return m_tag; };

  uint32_t localId() const;
  FPInvOffset stackIdx() const;

  bool operator==(const Location& other) const;
  bool operator!=(const Location& other) const;

  /*
   * Comparison.
   *
   * This has no semantic meaning, and only exists so Location can be used as
   * an ordered key type.
   */
  bool operator<(const Location& other) const;

  struct Hash { size_t operator()(Location loc) const; };

private:
  LTag m_tag;
  union {
    Local m_local;
    Stack m_stack;
  };
};

std::string show(Location);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/location-inl.h"

#endif
