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

#ifndef incl_HPHP_FIXUP_H_
#define incl_HPHP_FIXUP_H_

#include <vector>
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/tread-hash-map.h"
#include "hphp/util/atomic.h"
#include "hphp/util/data-block.h"

namespace HPHP {
struct ExecutionContext;
}

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * The Fixup map allows us to reconstruct the state of the VM registers (fp,
 * sp, and pc) from an up-stack invocation record.  Each range of bytes in the
 * translation cache is associated with a "distance" in both stack cells and
 * opcode bytes from the beginning of the function.  These are known at
 * translation time.
 *
 * The way this works is by chasing the native rbp chain to find a rbp that we
 * know is a VM frame (i.e. is actually a full ActRec).  Once we find that,
 * regsFromActRec is called, which looks to see if the return ip for the frame
 * before the VM frame has an entry in the fixup map (i.e. if it points into
 * the translation cache)---if so, it finds the fixup information in one of two
 * ways:
 *
 *   - Fixup: the normal case.
 *
 *     The Fixup record just stores an offset relative to the ActRec* for vmsp,
 *     and an offset from the start of the func for pc.  In the case of
 *     resumable frames the sp offset is relative to Stack::resumableStackBase.
 *
 *   - IndirectFixup: this is used for some shared stubs in the TC.
 *
 *     In this case, some JIT'd code associated with the ActRec* we found made
 *     a call to a shared stub, and then that stub called C++.  The
 *     IndirectFixup record stores an offset to the saved frame pointer *two*
 *     levels deeper in C++, that says where the return IP for the call to the
 *     shared stub can be found.  I.e., we're trying to chase back two return
 *     ips into the TC.
 *
 *     Note that this means IndirectFixups will not work for C++ code paths
 *     that need to do a fixup without making at least one other C++ call, but
 *     for the current use case this is fine.
 *
 *     Here's a picture of the native stack in the indirect fixup situation:
 *
 *        |..............................|
 *        |..............................|
 *        +------------------------------+  __enterTCHelper
 *        |       RetIP to enterTC()     |
 *        |--                          --|
 *        |          savedRbp            |
 *        |--                          --|
 *        |        <other junk>          |
 *        +------------------------------+  TC code
 *        |    RetIP to enterTCHelper    |
 *        |--                          --|
 *        |         saved %rdi           |  <from callUnaryStub>
 *        +------------------------------+  STUB (e.g. decRefGeneric)
 *        | RetIP to caller of dtor stub |
 *        |--                          --|
 *        |     <pushes in dtor stub>    |
 *        +------------------------------+  <call to C++>
 *        |    RetIP to the dtor stub    |
 *        |--                          --|
 *        |         saved rVmFp          |  push %rbp; mov %rsp, %rbp
 *    +-->|--                          --|
 *    |   |    < C++ local variables>    |
 *    |   +------------------------------+
 *    |   |   RetIP to first C++ callee  |  C++ calls another function
 *    |   |--                          --|
 *    +---|     saved native %rbp (*)    |  points as shown, from mov above
 *        |--                          --|
 *        |..............................|
 *        |..............................|
 *
 *     The offset in IndirectFixup is how to get to the "RetIP to caller of
 *     dtor stub", relative to the value in the starred stack slot shown.  We
 *     then look that IP up in the fixup map again to find a normal
 *     (non-indirect) Fixup record.
 *
 */

//////////////////////////////////////////////////////////////////////

struct Fixup {
  Fixup(int32_t pcOff, int32_t spOff) : pcOffset{pcOff}, spOffset{spOff} {
    assertx(pcOffset >= 0);
    assertx(spOffset >= 0);
  }

  Fixup() {}

  bool isValid() const { return pcOffset >= 0 && spOffset >= 0; }

  int32_t pcOffset{-1};
  int32_t spOffset{-1};
};

struct IndirectFixup {
  explicit IndirectFixup(int retIpDisp) : returnIpDisp{retIpDisp} {}

  /* FixupEntry uses magic to differentiate between IndirectFixup and Fixup. */
  int32_t magic{-1};
  int32_t returnIpDisp;
};

inline Fixup makeIndirectFixup(int dwordsPushed) {
  Fixup fix;
  fix.spOffset = (2 + dwordsPushed) * 8;
  return fix;
}

struct FixupMap {
  static constexpr unsigned kInitCapac = 128;
  TRACE_SET_MOD(fixup);

  struct VMRegs {
    const Op* pc;
    TypedValue* sp;
    const ActRec* fp;
  };

  FixupMap() : m_fixups(kInitCapac) {}

  void recordFixup(CTCA tca, const Fixup& fixup) {
    TRACE(3, "FixupMapImpl::recordFixup: tca %p -> (pcOff %d, spOff %d)\n",
          tca, fixup.pcOffset, fixup.spOffset);

    if (auto pos = m_fixups.find(tca)) {
      *pos = FixupEntry(fixup);
    } else {
      m_fixups.insert(tca, FixupEntry(fixup));
    }
  }

  const Fixup* findFixup(CTCA tca) const {
    auto ent = m_fixups.find(tca);
    if (!ent) return nullptr;
    return &ent->fixup;
  }

  bool getFrameRegs(const ActRec* ar, const ActRec* prevAr,
                    VMRegs* outVMRegs) const;

  void fixup(ExecutionContext* ec) const;
  void fixupWork(ExecutionContext* ec, ActRec* rbp) const;
  void fixupWorkSimulated(ExecutionContext* ec) const;

  static bool eagerRecord(const Func* func);

private:
  union FixupEntry {
    explicit FixupEntry(Fixup f) : fixup(f) {}

    /* Depends on the magic field in an IndirectFixup being -1. */
    bool isIndirect() const {
      static_assert(
        offsetof(IndirectFixup, magic) == offsetof(FixupEntry, firstElem),
        "Differentiates between Fixup and IndirectFixup by looking at magic."
      );

      return firstElem < 0;
    }

    int32_t firstElem;
    Fixup fixup;
    IndirectFixup indirect;
  };

private:
  PC pc(const ActRec* ar, const Func* f, const Fixup& fixup) const {
    assertx(f);
    return f->getEntry() + fixup.pcOffset;
  }

  void regsFromActRec(CTCA tca, const ActRec* ar, const Fixup& fixup,
                      VMRegs* outRegs) const {
    const Func* f = ar->m_func;
    assertx(f);
    TRACE(3, "regsFromActRec:: tca %p -> (pcOff %d, spOff %d)\n",
          (void*)tca, fixup.pcOffset, fixup.spOffset);
    assertx(fixup.spOffset >= 0);
    outRegs->pc = reinterpret_cast<const Op*>(pc(ar, f, fixup));
    outRegs->fp = ar;

    if (UNLIKELY(ar->resumed())) {
      TypedValue* stackBase = Stack::resumableStackBase(ar);
      outRegs->sp = stackBase - fixup.spOffset;
    } else {
      outRegs->sp = (TypedValue*)ar - fixup.spOffset;
    }
  }

private:
  TreadHashMap<CTCA,FixupEntry,ctca_identity_hash> m_fixups;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
