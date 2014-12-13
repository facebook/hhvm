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
#ifndef incl_HPHP_JIT_IRGEN_STATE_H_
#define incl_HPHP_JIT_IRGEN_STATE_H_

#include <memory>
#include <vector>
#include <stack>
#include <utility>
#include <string>

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {

struct NormalizedInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

enum class IRGenMode {
  Trace,
  CFG,
};

/*
 * HHBC Translation State.
 *
 * This structure contains the main state bag for the HHIR frontend, which
 * translates HHBC into HHIR.  The parse-time state in HHIR is relatively
 * non-trivial, for two reasons: one is that we perform a number of
 * optimizations during parse time, and two is that since the IR can not
 * represent all operations on generic types some simple type analysis is
 * required to determine high-level compilation strategy.
 */
struct HTS {
  explicit HTS(TransContext);

  /*
   * TODO: refactor this code eventually so HTS doesn't own its IRUnit (or its
   * IRBuilder).  The IRUnit should be the result of running the code in the ht
   * module.
   */
  TransContext context;
  IRUnit unit;
  std::unique_ptr<IRBuilder> irb;

  /*
   * Tracks information about the current bytecode offset and which function we
   * are in. We push and pop as we deal with inlined calls.
   */
  std::vector<SrcKey> bcStateStack;

  /*
   * The id of the profiling translation for the code we're currently
   * generating, if there was one, otherwise kInvalidTransID.
   */
  TransID profTransID{kInvalidTransID};

  /*
   * Some information is only passed through the nearly-dead
   * NormalizedInstruction structure.  Don't add new uses since we're gradually
   * removing this (the long, ugly name is deliberate).
   */
  const NormalizedInstruction* currentNormalizedInstruction{nullptr};

  /*
   * True if we're on the last HHBC opcode that will be emitted for this
   * region.
   */
  bool lastBcOff{false};

  /*
   * The FPI stack is used for inlining---when we start inlining at an
   * FCall, we look in here to find a definition of the StkPtr,offset
   * that can be used after the inlined callee "returns".
   */
  std::stack<std::pair<SSATmp*,int32_t>> fpiStack;

  /*
   * When we know that a call site is being inlined we add its StkPtr
   * offset pair to this stack to prevent it from being erroneously
   * popped during an FCall.
   */
  std::stack<std::pair<SSATmp*,int32_t>> fpiActiveStack;

  /*
   * Toggles some behavior based on runtime flags.
   */
  IRGenMode mode{IRGenMode::Trace};
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug-printable string.
 */
std::string show(const HTS&);

//////////////////////////////////////////////////////////////////////

}}

#endif
