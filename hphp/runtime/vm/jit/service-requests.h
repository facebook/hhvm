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

#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include <folly/Optional.h>

namespace HPHP {

struct ActRec;

namespace jit {

struct CGMeta;

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
   * bind_jmp(TCA jmp, SrcKey target)
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
   * bind_addr(TCA* addr, SrcKey target)
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
   * retranslate(Offset off)
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
   * retranslate_opt(SrcKey sk)
   *
   * A request to retranslate the function from `sk', leveraging profiling data
   * to produce a set of larger, more optimized translations.  Only used when
   * PGO is enabled. Execution will resume at `sk' whether or not retranslation
   * is successful.
   */                     \
  REQ(RETRANSLATE_OPT)

/*
 * Service request types.
 */
enum ServiceRequest : uint32_t {
#define REQ(nm) REQ_##nm,
  SERVICE_REQUESTS
#undef REQ
};

///////////////////////////////////////////////////////////////////////////////

namespace svcreq {

///////////////////////////////////////////////////////////////////////////////

/*
 * ID to name mapping for tracing.
 */
inline const char* to_name(ServiceRequest req) {
  static const char* reqNames[] = {
#define REQ(nm) #nm,
    SERVICE_REQUESTS
#undef REQ
  };
  return reqNames[req];
}

#undef SERVICE_REQUESTS

///////////////////////////////////////////////////////////////////////////////
// Emitters.

/*
 * Type-discriminated service request argument.
 *
 * Each argument is written to a register when the service request is made:
 *  - Immediates are loaded directly.
 *  - Addresses are loaded using an rip-relative lea to aid code relocation.
 *  - Condition codes produce a setcc based on the status flags passed to the
 *    emit routine.
 */
struct Arg {
  enum class Kind { Immed, Address, CondCode };

  explicit Arg(uint64_t imm) : kind{Kind::Immed}, imm{imm} {}
  explicit Arg(TCA addr) : kind{Kind::Address}, addr{addr} {}
  explicit Arg(ConditionCode cc) : kind{Kind::CondCode}, cc{cc} {}

public:
  Kind kind;
  union {
    uint64_t imm;
    TCA addr;
    ConditionCode cc;
  };
};

using ArgVec = jit::vector<Arg>;

/*
 * Service request stub emitters.
 *
 * These stubs do some shuffling of arguments before launching into the JIT
 * translator via handleSRHelper().
 *
 * Service request stubs can be either persistent or ephemeral.  The only
 * difference (besides that ephemeral service requests require a stub start
 * address) is that ephemeral requests are padded to stub_size().
 *
 * Since making a service request leaves the TC, we need to sync the current
 * bytecode eval stack pointer, given via `spOff', to vmsp.  In the cases where
 * vmsp also needs to be synced between translations (namely, in resumed
 * contexts), we do this sync inline at the site of the jump to the stub, so
 * that it still occurs once the jump gets smashed.  Otherwise (namely, in
 * non-resumed contexts), the client must pass a non-none `spOff', and we do
 * the sync in the stub to save work once the service request is completed and
 * the jump is smashed.
 */
template<typename... Args>
TCA emit_persistent(CodeBlock& cb,
                    DataBlock& data,
                    CGMeta& meta,
                    FPInvOffset spOff,
                    ServiceRequest sr,
                    Args... args);
template<typename... Args>
TCA emit_ephemeral(CodeBlock& cb,
                   DataBlock& data,
                   CGMeta& meta,
                   TCA start,
                   FPInvOffset spOff,
                   ServiceRequest sr,
                   Args... args);
/*
 * These emit service request stubs that may not be relocated.  This distinction
 * is important, because these discard metadata that allows relocation.
 */
template<typename... Args>
TCA emit_persistent(CodeBlock& cb,
                    DataBlock& data,
                    FPInvOffset spOff,
                    ServiceRequest sr,
                    Args... args);
template<typename... Args>
TCA emit_ephemeral(CodeBlock& cb,
                   DataBlock& data,
                   TCA start,
                   FPInvOffset spOff,
                   ServiceRequest sr,
                   Args... args);

/*
 * Helpers for emitting specific service requests.
 */
TCA emit_bindjmp_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                      FPInvOffset spOff, TCA jmp, SrcKey target);
TCA emit_bindaddr_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                       FPInvOffset spOff, TCA* addr, SrcKey target);
TCA emit_retranslate_opt_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                              FPInvOffset spOff, SrcKey sk);

/*
 * Emit a stub which syncs vmsp and vmpc and then calls
 * resumeHelperNoTranslate. Call are smashed to this when we know we
 * can no longer create new translations. The address of the stub is
 * returned if successful, nullptr otherwise (the creation can fail if
 * there's no space in the TC for it).
 */
TCA emit_interp_no_translate_stub(FPInvOffset spOff, SrcKey sk);

///////////////////////////////////////////////////////////////////////////////

/*
 * Maximum number of arguments a service request can accept.
 */
constexpr int kMaxArgs = 2;

namespace x64 {
  constexpr int kMovLen = 10;
  constexpr int kLeaVmSpLen = 7;
}

namespace arm {
  // vasm lea is emitted in 4 bytes.
  //   ADD imm
  constexpr int kLeaVmSpLen = 4;
  // The largest of vasm setcc, copy, or leap is emitted in 16 bytes.
  //   AND imm, MOV, LDR + B + dc32, or ADRP + ADD imm
  constexpr int kMovLen = 12;
  // The largest of vasm copy or leap is emitted in 16 bytes.
  //   MOV, LDR + B + dc32, or ADRP + ADD imm
  constexpr int kPersist = 12;
  // vasm copy and jmpi is emitted in 16 bytes.
  //   MOV + LDR + B + dc32
  constexpr int kSvcReqExit = 16;
}

/*
 * Space used by an ephemeral stub.
 *
 * All ephemeral service request stubs are sized to this fixed, architecture-
 * dependent size, which is guaranteed to fit all service request types along
 * with a terminal padding instruction.
 */
constexpr size_t stub_size() {
  // The extra args are the request type and the stub address.
  constexpr auto kTotalArgs = kMaxArgs + 2;

  switch (arch()) {
    case Arch::X64:
      return kTotalArgs * x64::kMovLen + x64::kLeaVmSpLen;
    case Arch::ARM:
      return arm::kLeaVmSpLen +
        kTotalArgs * arm::kMovLen +
        arm::kPersist + arm::kSvcReqExit;
    default:
      // GCC has a bug with throwing in a constexpr function.
      // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67371
      // throw std::logic_error("Stub size not defined on architecture.");
      break;
  }
  // Because of GCC's issue, we have this assert, and a return value.
  static_assert(arch() == Arch::X64 || arch() == Arch::ARM,
                "Stub size not defined on architecture");
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Service request metadata.
 *
 * This structure is created on the stack by handleSRHelper() from the SR
 * arguments passed in registers.  Any changes to its size or layout of must be
 * reflected in the handleSRHelper unique stub.
 */
struct ReqInfo {
  /*
   * The service request type.
   */
  ServiceRequest req;

  /*
   * Depth of the evaluation stack.
   */
  FPInvOffset spOff;

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
  } args[kMaxArgs];
};

static_assert(sizeof(ReqInfo) == 0x20,
              "rsp adjustments in handleSRHelper");

///////////////////////////////////////////////////////////////////////////////

}}}

#include "hphp/runtime/vm/jit/service-requests-inl.h"
