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

//////////////////////////////////////////////////////////////////////

#define SERVICE_REQUESTS \
  /*
   * BIND_* all are requests for the first time a jump is needed.  This
   * generally involves translating new code and then patching an address
   * supplied as a service request argument.
   */ \
  REQ(BIND_JMP)          \
  REQ(BIND_ADDR)         \
  REQ(BIND_JMPCC_FIRST)  \
  \
  /*
   * When all translations don't support the incoming types, a
   * retranslate request is made.
   */ \
  REQ(RETRANSLATE) \
  \
  /*
   * When PGO is enabled, this retranslates previous translations leveraging
   * profiling data.
   */ \
  REQ(RETRANSLATE_OPT) \
  \
  /*
   * When the interpreter pushes an ActRec, the return address for
   * this ActRec will be set to a stub that raises POST_INTERP_RET,
   * since it doesn't have a TCA to return to.
   *
   * REQ_POST_INTERP_RET is raised in the case that translated machine code
   * executes the RetC for a frame that was pushed by the
   * interpreter. REQ_POST_DEBUGGER_RET is a similar request that is used when
   * translated code returns from a frame that had its saved return address
   * smashed by the debugger.
   */ \
  REQ(POST_INTERP_RET) \
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

//////////////////////////////////////////////////////////////////////
// Service request emission.

/*
 * Flags for emitting service requests.
 */
enum class SRFlags {
  None = 0,

  /*
   * Whether the service request is persistent.
   *
   * For non-persistent requests, the service request stub may be reused.
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

//////////////////////////////////////////////////////////////////////

/*
 * Service request metadata.
 *
 * This structure is created on the stack by handleSRHelper() from the SR
 * arguments passed in registers.  Any changes to its size or layout of must be
 * reflected in handleSRHelper() in translator-asm-helpers.S.
 */
struct ServiceReqInfo {
  ServiceRequest req;
  TCA stub;

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

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

}}

#endif
