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
#ifndef incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_H_
#define incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit {

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

enum ServiceRequest {
#define REQ(nm) REQ_##nm,
  SERVICE_REQUESTS
#undef REQ
};

// ID to name mapping for tracing.
inline const char* serviceReqName(int req) {
  static const char* reqNames[] = {
#define REQ(nm) #nm,
    SERVICE_REQUESTS
#undef REQ
  };
  return reqNames[req];
}

#undef SERVICE_REQUESTS

/*
 * Various flags that are passed to emitServiceReq.  May be or'd
 * together.
 */
enum class SRFlags {
  None = 0,

  /*
   * Indicates if the service request is persistent. For non-persistent
   * requests, the service request stub may be reused.
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
 * Req machinery. We sometimes emit code that is unable to proceed
 * without translator assistance; e.g., a basic block whose successor is
 * unknown. We leave one of these request arg blobs in m_data, and point
 * to it at callout-time.
 */

struct ServiceReqArgInfo {
  enum {
    Immediate,
    CondCode,
    RipRelative,
  } m_kind;
  union {
    uint64_t m_imm;
    jit::ConditionCode m_cc;
  };
};

inline ServiceReqArgInfo RipRelative(TCA addr) {
  return ServiceReqArgInfo {
    ServiceReqArgInfo::RipRelative,
    { (uint64_t)addr }
  };
}

using ServiceReqArgVec = jit::vector<ServiceReqArgInfo>;

union ServiceReqArg {
  TCA tca;
  Offset offset;
  SrcKey::AtomicInt sk;
  TransFlags trflags;
  TransID transID;
  bool boolVal;
  ActRec* ar;
};

/*
 * Any changes to the size or layout of this struct must be reflected in
 * handleSRHelper() in translator-asm-helpers.S.
 */
struct ServiceReqInfo {
  ServiceRequest req;
  TCA stub;
  ServiceReqArg args[4];
};
static_assert(sizeof(ServiceReqInfo) == 0x30,
              "rsp adjustments in handleSRHelper");

//////////////////////////////////////////////////////////////////////

/*
 * Assembly stub called by translated code to pack argument registers into a
 * ServiceReqInfo, along with some other bookkeeping tasks before a service
 * request.
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
