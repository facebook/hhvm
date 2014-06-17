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

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"

namespace HPHP { namespace JIT {

#define SERVICE_REQUESTS \
  /*
   * Return from this nested VM invocation to the previous invocation.
   * (Ending the program if there is no previous invocation.)
   */ \
  REQ(EXIT) \
  \
  /*
   * BIND_* all are requests for the first time a call, jump, or
   * whatever is needed.  This generally involves translating new code
   * and then patching an address supplied as a service request
   * argument.
   */ \
  REQ(BIND_CALL)         \
  REQ(BIND_JMP)          \
  REQ(BIND_JCC)          \
  REQ(BIND_ADDR)         \
  REQ(BIND_SIDE_EXIT)    \
  REQ(BIND_JMPCC_FIRST)  \
  REQ(BIND_JMPCC_SECOND) \
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
   * If the max translations is reached for a SrcKey, the last
   * translation in the chain will jump to an interpret request stub.
   * This instructs enterTC to punt to the interpreter.
   */ \
  REQ(INTERPRET) \
  \
  /*
   * When the interpreter pushes an ActRec, the return address for
   * this ActRec will be set to a stub that raises POST_INTERP_RET,
   * since it doesn't have a TCA to return to.
   *
   * This request is raised in the case that translated machine code
   * executes the RetC for a frame that was pushed by the interpreter.
   */ \
  REQ(POST_INTERP_RET) \
  \
  /*
   * Raised when the execution stack overflowed.
   */ \
  REQ(STACK_OVERFLOW) \
  \
  /*
   * Resume restarts execution at the current PC.  This is used after
   * an interpOne of an instruction that changes the PC, and in some
   * cases with FCall.
   */ \
  REQ(RESUME)

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
   * Indicates the service request should be aligned.
   */
  Align = 1 << 0,

  /*
   * For some service requests (returning from interpreted frames),
   * using a ret instruction to get back to enterTCHelper will
   * unbalance the return stack buffer---in these cases use a jmp.
   */
  JmpInsteadOfRet = 1 << 1,

  /*
   * Indicates if the service request is persistent. For non-persistent
   * requests, the service request stub may be reused.
   */
  Persist = 1 << 2,
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

// REQ_BIND_CALL
struct ReqBindCall {
  SrcKey m_sourceInstr;
  JIT::TCA m_toSmash;
  int m_nArgs;
  bool m_isImmutable; // call was to known func.
};


struct ServiceReqArgInfo {
  enum {
    Immediate,
    CondCode,
    RipRelative,
  } m_kind;
  union {
    uint64_t m_imm;
    JIT::ConditionCode m_cc;
  };
};

inline ServiceReqArgInfo RipRelative(TCA addr) {
  return ServiceReqArgInfo {
    ServiceReqArgInfo::RipRelative,
    { (uint64_t)addr }
  };
}

typedef smart::vector<ServiceReqArgInfo> ServiceReqArgVec;

}}

#endif
