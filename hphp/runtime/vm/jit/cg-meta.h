/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_CODE_GEN_FIXUPS_H_
#define incl_HPHP_JIT_CODE_GEN_FIXUPS_H_

#include "hphp/runtime/vm/jit/alignment.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/growable-vector.h"

#include <map>
#include <vector>

namespace HPHP { namespace jit {

/*
 * CGMeta contains a variety of different metadata information that is
 * collected during code generation.
 *
 * A major use case of this class is to expose metadata to the code relocator
 * for adjustment before it is written to global data structures (e.g., in
 * MCGenerator).
 */
struct CGMeta {
  void process(GrowableVector<IncomingBranch>* inProgressTailBranches);
  void process_only(GrowableVector<IncomingBranch>* inProgressTailBranches);
  bool empty() const;
  void clear();

  void setJmpTransID(TCA jmp, TransID transID, TransKind kind);

  /*
   * Code addresses of interest to the code generator.
   *
   * At emit-time, these should be set to point to TCAs of interest, and will
   * be updated if those addresses change.  A watchpoint set immediately
   * following an instruction is guaranteed to still follow it immediately,
   * even if updated.
   */
  std::vector<TCA*> watchpoints;

  /*
   * Pending MCGenerator table entries.
   */
  std::vector<std::pair<TCA,Fixup>> fixups;
  std::vector<std::pair<CTCA,TCA>> catches;
  std::vector<std::pair<TCA,TransID>> jmpTransIDs;
  std::unordered_map<uint64_t, const uint64_t*> literals;

  /*
   * All the alignment constraints on each code address.
   */
  std::multimap<TCA,std::pair<Alignment,AlignContext>> alignments;

  /*
   * Addresses of any allocated service request stubs.
   */
  std::vector<TCA> reusedStubs;

  /*
   * Address immediates in the generated code.
   *
   * Also contains the addresses of any mcprep{} smashable movq instructions
   * that were emitted.
   */
  std::set<TCA> addressImmediates;

  /*
   * Code addresses of interest to other code.
   *
   * These are like `watchpoints', except that the pointers point into the TC
   * data segment rather than, e.g., code generator data structures.  Used for
   * REQ_BIND_ADDR service requests.
   *
   * These also omit the "stickiness" guarantee w.r.t. previous instructions.
   */
  std::set<TCA*> codePointers;

  /*
   * Smash targets of fallback{} and fallbackcc{} instructions (e.g.,
   * REQ_RETRANSLATE service requests).
   *
   * These always correspond to the initial SrcKey of the current IR unit---
   * see cgReqRetranslate().
   */
  GrowableVector<IncomingBranch> inProgressTailJumps;

  /*
   * Smashable locations. Used on relocation to be sure a smashable instruction
   * is not optimized in size.
   */
  std::set<TCA> smashableLocations;

  /*
   * Debug-only map from bytecode to machine code address.
   */
  std::vector<TransBCMapping> bcMap;
};

/*
 * If the literal val has already been written into the TCA return its address,
 * otherwise return nullptr
 */
const uint64_t* addrForLiteral(uint64_t val);

/*
 * Look up a TCA-to-landingpad mapping.
 */
folly::Optional<TCA> getCatchTrace(CTCA ip);

/*
 * Return the number of registered catch traces
 */
size_t numCatchTraces();

/*
 * Erase catch trace at addr. If no trace is registered at addr no action is
 * performed.
 */
void eraseCatchTrace(CTCA addr);

}}

#endif
