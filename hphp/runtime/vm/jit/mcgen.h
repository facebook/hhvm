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

#ifndef incl_HPHP_JIT_MCGEN_H_
#define incl_HPHP_JIT_MCGEN_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {

struct ActRec;

namespace jit {

struct IRUnit;
struct Vunit;

namespace tc { struct ThreadTCBuffer; }

/*
 * Arguments for the translate() entry points in Translator.
 *
 * These include a variety of flags that help decide what to translate.
 */
struct TransArgs {
  explicit TransArgs(SrcKey sk) : sk{sk} {}

  SrcKey sk;
  Annotations annotations;
  TransFlags flags{0};
  TransID transId{kInvalidTransID};
  TransKind kind{TransKind::Invalid};
  RegionDescPtr region{nullptr};
};

/*
 * The state of a partially-complete translation.
 *
 * It is used to transfer context between translate() and emitTranslation()
 * when the initial phase of translation can be done without the write lease.
 */
struct TransEnv {
  explicit TransEnv(const TransArgs& args) : args(args) {}
  ~TransEnv();

  TransEnv(TransEnv&&) = default;
  TransEnv& operator=(TransEnv&&) = default;

  /*
   * Context for the translation process.
   */
  TransArgs args;
  FPInvOffset initSpOffset;
  TransID transID{kInvalidTransID};

  /*
   * hhir and vasm units. Both will be set iff bytecode -> hhir lowering was
   * successful (hhir -> vasm lowering never fails).
   */
  std::unique_ptr<IRUnit> unit;
  std::unique_ptr<Vunit> vunit;

  /*
   * Metadata collected during bytecode -> hhir lowering.
   */
  PostConditions pconds;
  Annotations annotations;
};

////////////////////////////////////////////////////////////////////////////////

namespace mcgen {

struct UseThreadLocalTC {
  UseThreadLocalTC(UseThreadLocalTC&&) = delete;
  UseThreadLocalTC& operator=(UseThreadLocalTC&&) = delete;

#ifdef NDEBUG
  explicit UseThreadLocalTC(tc::ThreadTCBuffer&) {}
#else
  explicit UseThreadLocalTC(tc::ThreadTCBuffer& buf);
  ~UseThreadLocalTC();

private:
  tc::ThreadTCBuffer& m_buf;
#endif
};

struct ReadThreadLocalTC {
  ReadThreadLocalTC(ReadThreadLocalTC&&) = delete;
  ReadThreadLocalTC& operator=(ReadThreadLocalTC&&) = delete;

#ifdef NDEBUG
  explicit ReadThreadLocalTC(const tc::ThreadTCBuffer&) {}
#else
  explicit ReadThreadLocalTC(const tc::ThreadTCBuffer& m_buf);
  ~ReadThreadLocalTC();

private:
  const tc::ThreadTCBuffer& m_buf;
#endif
};

/*
 * Look up or translate a func prologue or func body.
 */
TCA getFuncPrologue(Func* func, int nPassed);

/*
 * Create a live or profile retranslation based on args.
 *
 * Will return null if the write-lease could not be obtained or a translation
 * could not be generated.
 */
TCA retranslate(TransArgs args, const RegionContext& ctx);

/*
 * Regionize and optimize the given function using profile data.
 *
 * Returns true iff the function has been successfully optimized.
 */
bool retranslateOpt(FuncId funcId);

/*
 * In JitPGO mode, check whether enough profile data has been collected and,
 * if we haven't retranslated
 */
void checkRetranslateAll();

/*
 * Called once when the JIT is activated to initialize internal mcgen structures
 */
void processInit();

/*
 * Called once before process shutdown. May block to wait for any pending JIT
 * worker threads.
 */
void processExit();

/*
 * True iff mcgen::processInit() has been called
 */
bool initialized();

/*
 * Return the timestamp at which mcgen::processInit was called
 */
int64_t jitInitTime();

/*
 * Whether we should dump TC annotations for translations of `func' of
 * `transKind'.
 */
bool dumpTCAnnotation(const Func& func, TransKind transKind);

/*
 * Is the thread local TC in use
 */
bool isLocalTCEnabled();

/*
 * Expected size of all thread local TC buffers
 */
size_t localTCSize();

/*
 * Per-thread cached TC buffer
 */
TCA cachedLocalTCBuffer();

/*
 * Is still a pending call to retranslateAll()
 */
bool retranslateAllPending();

}}}

#endif
