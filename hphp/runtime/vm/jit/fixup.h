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

#ifndef incl_HPHP_FIXUP_H_
#define incl_HPHP_FIXUP_H_

#include <vector>
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/tread-hash-map.h"
#include "hphp/runtime/vm/vm-regs.h"
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
 *   - IndirectFixup:
 *
 *     This can be used for some shared stubs in the TC, to avoid
 *     setting up a full frame, on architectures where the calee's
 *     frame is stored immediately under the caller's sp (currently
 *     true of x64 but not arm or ppc).
 *
 *     In this case, some JIT'd code associated with the ActRec* we found made
 *     a call to a shared stub, and then that stub called C++.  The
 *     IndirectFixup record stores an offset to the saved frame pointer *two*
 *     levels deeper in C++, that says where the return IP for the call to the
 *     shared stub can be found.  I.e., we're trying to chase back two return
 *     ips into the TC.
 *
 *     Note that this means IndirectFixups will not work for C++ code
 *     paths that need to do a fixup without making at least one other
 *     C++ call (because of -momit-leaf-frame-pointers), but for the
 *     current use case this is fine.
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
 *        |         saved rvmfp()        |  push %rbp; mov %rsp, %rbp
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

  bool isValid() const { return spOffset >= 0; }

  bool operator==(const Fixup& o) const {
    return pcOffset == o.pcOffset && spOffset == o.spOffset;
  }
  bool operator!=(const Fixup& o) const {
    return pcOffset != o.pcOffset || spOffset != o.spOffset;
  }

  int32_t pcOffset{-1};
  int32_t spOffset{-1};
};

inline Fixup makeIndirectFixup(int dwordsPushed) {
  Fixup fix;
  fix.spOffset = kNativeFrameSize +
                 AROFF(m_savedRip) +
                 dwordsPushed * sizeof(uintptr_t);
  return fix;
}

namespace FixupMap {
/*
 * Record a new fixup (or overwrite an existing fixup) at tca.
 */
void recordFixup(CTCA tca, const Fixup& fixup);

/*
 * Find the fixup for tca if it exists (or return nullptr).
 */
const Fixup* findFixup(CTCA tca);

/*
 * Number of entries in the fixup map.
 */
size_t size();

/*
 * Perform a fixup of the VM registers for a stack whose first frame is `rbp`.
 *
 * Returns whether we successfully performed the fixup.  (We assert on failure
 * if `soft` is not set).
 */
bool fixupWork(ActRec* rbp, bool soft = false);

/*
 * Returns true if calls to func should use an EagerVMRegAnchor.
 */
bool eagerRecord(const Func* func);
}

namespace detail {
void syncVMRegsWork(bool soft); // internal sync work for a dirty vm state
}

/*
 * Sync VM registers for the first TC frame in the callstack.
 */
inline void syncVMRegs(bool soft = false) {
  if (tl_regState == VMRegState::CLEAN) return;
  detail::syncVMRegsWork(soft);
}

//////////////////////////////////////////////////////////////////////

}}

#endif
