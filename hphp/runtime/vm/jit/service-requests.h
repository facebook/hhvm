/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_H_
#define incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/util/asm-x64.h"

namespace HPHP {

struct ActRec;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * A service request is a co-routine used to invoke JIT translation services.
 *
 * Most service requests are accomplished by jumping to a service request stub,
 * which rolls up arguments and calls into the JIT via handleSRHelper().
 * Afterwards, we re-enter the TC, returning control non-locally to the next
 * logical instruction.
 *
 * The implementation of the service request will return the TCA at which to
 * resume execution even if it's another stub rather than the calling function.
 */
#define SERVICE_REQUESTS  \
  /*
   * bind_jmp(TCA jmp, SrcKey target, TransFlags trflags)
   *
   * A jmp to the potentially untranslated code for `target'.
   *
   * Jump to a service request stub, which invokes the JIT and looks up the
   * translation for `target'---or creates a translation if one does not exist.
   * The address of the translation is then smashed into the immediate of the
   * jump instruction (whose address is passed via `jmp').
   */                     \
  REQ(BIND_JMP)           \
                          \
  /*
   * bind_addr(TCA* addr, SrcKey target, TransFlags trflags)
   *
   * A code pointer to the potentially untranslated `target'; used for
   * just-in-time indirect call translations.
   *
   * Similar to bind_jmp, except that the smash target is *addr instead of the
   * jmp instruction's immediate.  When we emit a bind_addr, we only emit the
   * request stub and store its address to *addr; someone else has to emit the
   * indirect jump that actually invokes the service request.
   */                     \
  REQ(BIND_ADDR)          \
                          \
  /*
   * bind_jcc_first(TCA jcc, SrcKey taken, SrcKey next, bool did_take)
   *
   * A branch between two potentially untranslated targets.
   *
   * A bind_jcc_1st is emitted as a jcc followed by a jmp, both to the same
   * stub.  When invoked, a translation of the appropriate side of the branch
   * (indicated by `did_take') is obtained, and the jcc is rewritten so that it
   * will translate the other side of the branch when the inverse condition is
   * met.
   *
   * @see: MCGenerator::bindJccFirst()
   */                     \
  REQ(BIND_JCC_FIRST)     \
                          \
  /*
   * retranslate(Offset off, TransFlags trflags)
   *
   * A request to retranslate the current function at bytecode offset `off',
   * for when no existing translations support the incoming types.
   *
   * The smash target(s) of a retranslate is stored in
   * SrcRec::m_tailFallbackJumps.
   */                     \
  REQ(RETRANSLATE)        \
                          \
  /*
   * retranslate_opt(SrcKey target, TransID transID)
   *
   * A request to retranslate a function entry `target', leveraging profiling
   * data to produce a larger, more optimized translation.  Only used when PGO
   * is enabled.
   */                     \
  REQ(RETRANSLATE_OPT)    \
                          \
  /*
   * post_interp_ret(ActRec* arg, ActRec* caller)
   * post_debugger_ret()
   *
   * post_interp_ret is invoked in the case that translated code in the TC
   * executes the return for a frame that was pushed by the interpreter---since
   * there is no TCA to return to.
   *
   * post_debugger_ret is a similar request that is used when translated code
   * returns from a frame that had its saved return address smashed by the
   * debugger.
   */                     \
  REQ(POST_INTERP_RET)    \
  REQ(POST_DEBUGGER_RET)

/*
 * Service request types.
 */
enum ServiceRequest {
#define REQ(nm) REQ_##nm,
  SERVICE_REQUESTS
#undef REQ
};

/*
 * ID to name mapping for tracing.
 */
inline const char* serviceReqName(int req) {
  static const char* reqNames[] = {
#define REQ(nm) #nm,
    SERVICE_REQUESTS
#undef REQ
  };
  return reqNames[req];
}

#undef SERVICE_REQUESTS

///////////////////////////////////////////////////////////////////////////////
// Service request emission.

/*
 * Flags for emitting service requests.
 */
enum class SRFlags {
  None = 0,

  /*
   * Whether the emitted service request stub should be persistent.
   *
   * Non-persistent stubs are called "ephemeral", and may be reused.
   */
  Persist = 1 << 0,
};

inline bool operator&(SRFlags a, SRFlags b) {
  return int(a) & int(b);
}

inline SRFlags operator|(SRFlags a, SRFlags b) {
  return SRFlags(int(a) | int(b));
}

/*
 * Type-discriminated service request argument.
 *
 * Each argument is written to a register when the service request is made:
 *  - Immediates are loaded directly.
 *  - Addresses are loaded using an rip-relative lea to aid code relocation.
 *  - Condition codes produce a setcc based on the status flags passed to the
 *    emit routine.
 */
struct SvcReqArg {
  enum class Kind { Immed, Address, CondCode };

  explicit SvcReqArg(uint64_t imm) : kind{Kind::Immed}, imm{imm} {}
  explicit SvcReqArg(TCA addr) : kind{Kind::Address}, addr{addr} {}
  explicit SvcReqArg(ConditionCode cc) : kind{Kind::CondCode}, cc{cc} {}

public:
  Kind kind;
  union {
    uint64_t imm;
    TCA addr;
    ConditionCode cc;
  };
};

using SvcReqArgVec = jit::vector<SvcReqArg>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Service request metadata.
 *
 * This structure is created on the stack by handleSRHelper() from the SR
 * arguments passed in registers.  Any changes to its size or layout of must be
 * reflected in handleSRHelper() in translator-asm-helpers.S.
 */
struct ServiceReqInfo {
  /*
   * The service request type.
   */
  ServiceRequest req;

  /*
   * Address of the service request's code stub for non-persistent requests.
   *
   * The service request handler will free this stub if it's set, after the
   * service is performed.
   */
  TCA stub;

  /*
   * Service request arguments; see SERVICE_REQUESTS for documentation.
   */
  union {
    TCA tca;
    Offset offset;
    SrcKey::AtomicInt sk;
    TransFlags trflags;
    TransID transID;
    bool boolVal;
    ActRec* ar;
  } args[4];
};

static_assert(sizeof(ServiceReqInfo) == 0x30,
              "rsp adjustments in handleSRHelper");

/*
 * Service request assembly stub.
 *
 * Called by translated code before a service request to pack argument
 * registers into a ServiceReqInfo and perform some other bookkeeping tasks.
 */
extern "C" void handleSRHelper();

///////////////////////////////////////////////////////////////////////////////

/*
 * Return the VM stack offset a service request was associated with.  This
 * function is only legal to call with service requests that were created with
 * an FPInvOffset.  (TODO: list of when we do that.)
 */
FPInvOffset serviceReqSPOff(TCA);

/*
 * A REQ_BIND_JMP service request passes an address of a jump that can be
 * patched.  This function lets you change this jump address for an existing
 * REQ_BIND_JMP stub to `newJmpIp'.  The caller must indicate whether the stub
 * was created with a target SrcKey that is a resumed function.
 *
 * Pre: the `stub' must be a REQ_BIND_JMP stub.
 */
void adjustBindJmpPatchableJmpAddress(TCA stub,
                                      bool targetIsResumed,
                                      TCA newJmpIp);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
