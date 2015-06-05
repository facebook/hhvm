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
#include <functional>

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {

struct NormalizedInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

struct FPIInfo {
  SSATmp* returnSP;
  FPInvOffset returnSPOff; // return's logical sp offset; stkptr might differ
  IRInstruction* spillFrame;
};

/*
 * IR-Generation State.
 *
 * This structure contains the main state bag for the HHIR frontend, which
 * translates HHBC into HHIR.  The parse-time state in HHIR is relatively
 * non-trivial, for two reasons: one is that we perform a number of
 * optimizations during parse time, and two is that since the IR can not
 * represent all operations on generic types some simple type analysis is
 * required to determine high-level compilation strategy.
 */
struct IRGS {
  explicit IRGS(TransContext ctx, TransFlags);

  /*
   * TODO: refactor this code eventually so IRGS doesn't own its IRUnit (or its
   * IRBuilder).  The IRUnit should be the result of running the code in the ht
   * module.
   */
  TransContext context;
  TransFlags transFlags;
  IRUnit unit;
  std::unique_ptr<IRBuilder> irb;

  /*
   * Tracks information about the current bytecode offset and which function we
   * are in. We push and pop as we deal with inlined calls.
   */
  std::vector<SrcKey> bcStateStack;

  /*
   * The current inlining level.  0 means we're not inlining.
   */
  uint16_t inlineLevel{0};

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
   * True if we're on the first HHBC instruction that will be executed
   * for this instruction.  This is the first bytecode instruction in
   * either the region entry block or any other block in its
   * retranslation chain (i.e. that can be reached due to guard
   * failures before advancing VM state for any bytecode instruction).
   */
  bool firstBcInst{true};

  /*
   * True if we're on the last HHBC instruction that will be emitted
   * for this region.
   */
  bool lastBcInst{false};

  /*
   * The FPI stack is used for inlining---when we start inlining at an FCall,
   * we look in here to find a definition of the StkPtr,offset that can be used
   * after the inlined callee "returns".
   */
  std::stack<FPIInfo> fpiStack;

  /*
   * When we know that a call site is being inlined we add its StkPtr
   * offset pair to this stack to prevent it from being erroneously
   * popped during an FCall.
   */
  std::stack<std::pair<SSATmp*,FPInvOffset>> fpiActiveStack;

  /*
   * The function to use to create catch blocks when instructions that can
   * throw are created with no catch block.  The default (when this function is
   * null) is to spill the stack and then leave.  We allow a non-default
   * basically for an minstr use case.  This is reset every time we
   * prepareForNextHHBC.
   */
  std::function<Block* ()> catchCreator;
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug-printable string.
 */
std::string show(const IRGS&);

//////////////////////////////////////////////////////////////////////

}}

#endif
