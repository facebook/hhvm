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

#ifndef incl_HPHP_JIT_INLINING_H_
#define incl_HPHP_JIT_INLINING_H_

#include "hphp/runtime/vm/jit/region-selection.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Func;
struct SrcKey;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct IRGS;
struct RegionDesc;

///////////////////////////////////////////////////////////////////////////////

/*
 * Inlining decision-making mechanism.
 *
 * The intended client of an InliningDecider is an arbitrary tracelet selector
 * that wishes to support inlining.  Correct usage of an InliningDecider
 * produces a guarantee (or a refusal to guarantee) that a callee's region can
 * be correctly transcluded and translated within the caller's region.
 *
 * The usage is as follows:
 *
 *    - The client region selector calls canInlineAt() on an instruction.
 *
 *    - If the check succeeds, the client produces a region for the callee at
 *      the appropriate entrypoint for the call.  How this region is selected
 *      is a policy decision left up to the client---however, the region must
 *      not itself contain inlined blocks.  (The decider has a `disabled' flag
 *      for this purpose.)
 *
 *    - The client passes this region to shouldInline() to determine whether it
 *      is inlinable.  If so, the client can mark the region as such and/or
 *      perform inlining translations as appropriate.
 */
struct InliningDecider {

  explicit InliningDecider(const Func* func) : m_topFunc(func) {}

  /////////////////////////////////////////////////////////////////////////////
  // Getters and setters.

  /*
   * Disable an inlining context.
   *
   * This should be used for the regions passed to shouldInline().
   */
  InliningDecider& disable() {
    m_disabled = true;
    return *this;
  }

  /*
   * Reset inlining state.
   *
   * Forget all current information about inlining cost and depth, preserving
   * only m_topFunc and m_disabled.
   */
  void resetState() {
    m_cost = m_callDepth = m_stackDepth = 0;
    m_costStack.clear();
  }

  /*
   * Getters for depth and disabled status.
   */
  bool disabled() const { return m_disabled; }
  int  depth()    const { return m_callDepth; }
  bool inlining() const { return depth() != 0; }

  /////////////////////////////////////////////////////////////////////////////
  // Core API.

  /*
   * Can we perform inlining of `callee' at `callSK' from within `region'?
   *
   * This is a shallow check---it asks whether `callSK' is an FCall{,D} with an
   * appropriate FPush* in the same region, and verifies that the call does not
   * block inlining (e.g., due to missing arguments, recursion, resumable
   * callee, etc.).  It does not peek into the callee's bytecode or regions,
   * and it is insensitive to inlining costs.
   *
   * This function assumes that region is fully-formed up to and including
   * `inst', because we reach backwards through the region to find the
   * corresponding FPush and validate the FPI region.
   *
   * NOTE: Inlining will fail during translation if the FPush was interpreted.
   * It is up to the client to ensure that this is not the case.
   */
  bool canInlineAt(SrcKey callSK, const Func* callee) const;

  /*
   * Check that `region' of `callee' can be inlined (possibly via other inlined
   * callees) into m_topFunc.
   *
   * If this function returns true, we guarantee (contingent on InliningDecider
   * being used correctly) that it is safe and possible to inline the callee;
   * moreover, based on global inlining heuristics, we submit that the tracelet
   * selector /ought/ to do so.
   *
   * If inlining is not performed when true is returned, registerEndInlining()
   * must be called immediately to correctly reset the internal inlining costs.
   */
  bool shouldInline(const Func* callee, const RegionDesc& region,
                    uint32_t maxTotalCost);

  /*
   * Update our context to account for the beginning of an inlined call.
   */
  void accountForInlining(const Func* callee, const RegionDesc& region);

  /*
   * Update internal state for when an inlining event ends.
   *
   * This just "pops" the call and stack depths---it should be called whenever
   * the tracelet selector is finished transcluding the blocks of an inlined
   * function (even if it's nested in another inlined function).
   */
  void registerEndInlining(const Func* callee);

  /*
   * Prevents any Func with the same fullName() as the specified callee from
   * being inlined in the future.
   */
  static void forbidInliningOf(const Func* callee);

private:
  // The function being inlined into.
  const Func* const m_topFunc;

  // If set, the decider will always refuse inlining.
  bool m_disabled{false};

  // Costs associated with inlining.
  int m_cost{0};
  int m_callDepth{0};
  int m_stackDepth{0};

  // Stack of costs, popped in registerEndInlining().
  std::vector<int> m_costStack;
};

/*
 * Select an inlining region for the call to `callee' at `sk'.
 */
RegionDescPtr selectCalleeRegion(const SrcKey& sk,
                                 const Func* callee,
                                 const IRGS& irgs,
                                 InliningDecider& inl,
                                 int32_t maxBCInstrs);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
