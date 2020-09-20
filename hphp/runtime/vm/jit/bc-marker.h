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

#pragma once

#include <string>

#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit {

struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * BCMarker holds the location of a specific bytecode instruction, along with
 * the offset from vmfp to vmsp at the beginning of the instruction.  Every
 * IRInstruction has a marker to keep track of which bytecode instruction it
 * came from.  If we're doing an optimized translation, it also holds the
 * TransIDs for the profiling translations associated with this piece of code.
 * Note that there may be multiple corresponding profile translations in case
 * guard relaxation merges multiple profile translations.
 */
struct BCMarker {
  /*
   * This is for use by test code that needs to provide BCMarkers but is not
   * deriving them from an actual bytecode region. It is always valid().
   */
  static BCMarker Dummy() {
    return BCMarker {
      SrcKey(DummyFuncId, 0, ResumeMode::None),
      FPInvOffset{0},
      TransIDSet{},
      nullptr
    };
  }

  BCMarker() = default;

  BCMarker(SrcKey sk, FPInvOffset sp, const TransIDSet& tids, SSATmp* fp)
    : m_sk(sk)
    , m_spOff(sp)
    , m_profTransIDs{tids}
    , m_fp{fp}
    , m_fixupSk(sk)
  {
    assertx(valid());
  }

  bool operator==(const BCMarker& b) const {
    return b.m_sk == m_sk &&
           b.m_spOff == m_spOff &&
           b.m_profTransIDs == m_profTransIDs &&
           b.m_fp == m_fp;
  }
  bool operator!=(const BCMarker& b) const { return !operator==(b); }

  std::string show() const;
  bool valid() const;
  bool isDummy() const { return m_sk.valid() &&
                                m_sk.funcID() == DummyFuncId; }
  bool hasFunc() const { return valid() && !isDummy(); }

  SrcKey      sk()       const { assertx(valid()); return m_sk;            }
  const Func* func()     const { assertx(hasFunc()); return m_sk.func();   }
  Offset      bcOff()    const { assertx(valid()); return m_sk.offset();   }
  bool        hasThis()  const { assertx(valid()); return m_sk.hasThis();  }
  bool        prologue() const { assertx(valid()); return m_sk.prologue(); }
  FPInvOffset spOff()    const { assertx(valid()); return m_spOff;         }
  SSATmp*     fp()       const { assertx(valid()); return m_fp;            }
  SrcKey      fixupSk()  const { assertx(valid()); return m_fixupSk;       }

  const TransIDSet& profTransIDs() const {
    assertx(valid());
    return m_profTransIDs;
  }

  // Use the FixupSK resumeMode, if we've elided the frame we will need to
  // ensure that the sp is tracked properly for a resumed function.
  ResumeMode resumeMode() const {
    assertx(valid());
    return m_fixupSk.resumeMode();
  }

  const Func* fixupFunc() const {
    assertx(valid());
    return m_fixupSk.func();
  }

  Offset fixupBcOff() const {
    assertx(valid());
    return m_fixupSk.offset();
  }

  // Return a copy of this marker with an updated sp, fp, or sk.
  BCMarker adjustSP(FPInvOffset sp) const {
    return BCMarker { m_sk, sp, m_profTransIDs, m_fp };
  }
  BCMarker adjustFP(SSATmp* fp) const {
    return BCMarker { m_sk, m_spOff, m_profTransIDs, fp };
  }
  BCMarker adjustFixupSK(SrcKey sk) const {
    auto ret = *this;
    ret.m_fixupSk = sk;
    return ret;
  }
  BCMarker setPrologue() const {
    auto ret = *this;
    ret.m_sk = SrcKey{func(), bcOff(), SrcKey::PrologueTag{}};
    return ret;
  }

  void reset() {
    m_sk = SrcKey();
    m_spOff = FPInvOffset{0};
    TransIDSet emptyTransIDSet;
    m_profTransIDs.swap(emptyTransIDSet);
    m_fp = nullptr;
    m_fixupSk = SrcKey();
  }

private:
  SrcKey      m_sk;
  FPInvOffset m_spOff{0};
  TransIDSet  m_profTransIDs;
  SSATmp*     m_fp{nullptr};

  // Normally the fixup SrcKey is the same as the SrcKey for the marker,
  // however, when inlining has dropped an inner frame the fixup SrcKey will
  // be inside the parent frame so that its offset is relative to the live
  // ActRec
  SrcKey      m_fixupSk;
};

//////////////////////////////////////////////////////////////////////

}}

