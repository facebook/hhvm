/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_BCMARKER_H_
#define incl_HPHP_JIT_BCMARKER_H_

#include <string>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit {

struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * BCMarker holds the location of a specific bytecode instruction, along with
 * the offset from vmfp to vmsp at the beginning of the instruction.  Every
 * IRInstruction has a marker to keep track of which bytecode instruction it
 * came from.  If we're doing an optimized translation, it also holds the
 * TransID for the profiling translation associated with this piece of code.
 */
struct BCMarker {
  /*
   * This is for use by test code that needs to provide BCMarkers but is not
   * deriving them from an actual bytecode region. It is always valid().
   */
  static BCMarker Dummy() {
    return BCMarker {
      SrcKey(DummyFuncId, 0, false),
      FPInvOffset{0},
      kInvalidTransID,
      nullptr
    };
  }

  explicit BCMarker() = default;

  BCMarker(SrcKey sk, FPInvOffset sp, TransID tid, SSATmp* fp)
    : m_sk(sk)
    , m_spOff(sp)
    , m_profTransID{tid}
    , m_fp{fp}
  {
    assertx(valid());
  }

  bool operator==(const BCMarker& b) const {
    return b.m_sk == m_sk &&
           b.m_spOff == m_spOff &&
           b.m_profTransID == m_profTransID &&
           b.m_fp == m_fp;
  }
  bool operator!=(const BCMarker& b) const { return !operator==(b); }

  std::string show() const;
  bool valid() const;
  bool isDummy() const { return m_sk.valid() &&
                                m_sk.funcID() == DummyFuncId; }
  bool hasFunc() const { return valid() && !isDummy(); }

  SrcKey      sk()          const { assertx(valid());   return m_sk;           }
  const Func* func()        const { assertx(hasFunc()); return m_sk.func();    }
  Offset      bcOff()       const { assertx(valid());   return m_sk.offset();  }
  bool        resumed()     const { assertx(valid());   return m_sk.resumed(); }
  FPInvOffset spOff()       const { assertx(valid());   return m_spOff;        }
  TransID     profTransID() const { assertx(valid());   return m_profTransID;  }
  SSATmp*     fp()          const { assertx(valid());   return m_fp;           }

  // Return a copy of this marker with an updated sp or fp.
  BCMarker adjustSP(FPInvOffset sp) const {
    return BCMarker { m_sk, sp, m_profTransID, m_fp };
  }
  BCMarker adjustFP(SSATmp* fp) const {
    return BCMarker { m_sk, m_spOff, m_profTransID, fp };
  }

private:
  SrcKey m_sk;
  FPInvOffset m_spOff{0};
  TransID m_profTransID{kInvalidTransID};
  SSATmp* m_fp{nullptr};
};

//////////////////////////////////////////////////////////////////////

}}

#endif
