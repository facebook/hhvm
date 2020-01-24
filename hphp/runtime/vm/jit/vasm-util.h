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
#ifndef incl_HPHP_VASM_UTIL_H_
#define incl_HPHP_VASM_UTIL_H_

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * An instruction that is a "trivial nop" is always removable without changing
 * program behavior.
 */
bool is_trivial_nop(const Vinstr&);

/*
 * Splits any critical edges in `unit'.  Returns true iff the unit was modified.
 */
bool splitCriticalEdges(Vunit& unit);

///////////////////////////////////////////////////////////////////////////////

/*
 * Compute a mapping of each (reachable) block in a Vunit to its immediate
 * dominator, using the provided reverse post-order sort of the blocks.
 */
using VIdomVector = jit::vector<Vlabel>;
VIdomVector findDominators(const Vunit&, const jit::vector<Vlabel>& rpo);

/*
 * Test if b1 dominates b2
 */
bool dominates(Vlabel b1, Vlabel b2, const VIdomVector&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Compute all back edges among the (reachable) blocks in a Vunit, using the
 * provided reverse post-order sort of the blocks.
 *
 * A back-edge is a pair of blocks b1 and b2 where b2 is an immediate successor
 * of b1, and b2 also dominates b1.
 */
using BackEdgeVector = jit::vector<std::pair<Vlabel, Vlabel>>;
BackEdgeVector findBackEdges(const Vunit&,
                             const jit::vector<Vlabel>& rpo,
                             const VIdomVector&);

/*
 * Compute a mapping of loops (represented by the loop's header block) to the
 * blocks contained within that loop. The membership is inclusive, a block may
 * be in multiple loops.
 *
 * NB: This only finds reducible loops. It may not recognize irreducible loops
 * correctly, so should not be used in places where it would be unsound to miss
 * those.
 */
using LoopBlocks = jit::fast_map<Vlabel, jit::vector<Vlabel>>;
LoopBlocks findLoopBlocks(const Vunit&,
                          const PredVector&,
                          const BackEdgeVector&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Put the Vregs specified by `targets' into SSA form. Only the instances of
 * Vregs in blocks present in the reverse post-order sort will be modified.
 *
 * The Vregs will be rewritten to new Vregs, such that each new Vreg has exactly
 * one definition, and all usages of the Vregs are dominated by a
 * definition. This lets one do transformations that may not preserve SSA form,
 * and restore it after the fact.
 *
 * `blocksWithTargets' is a bitset indicating which blocks contains
 * target Vregs. Any target Vregs in blocks not marked as such will be
 * unchanged. `blocksWithTargets' will be modified as necessary to
 * reflect any Vregs added.
 *
 * A mapping of new Vregs to the Vreg they replaced is returned.
 *
 * The `ssaalias' pseudo-instruction can be used to control what a Vreg is
 * rewritten to. An example:
 *
 *   conjure %1
 *   ssaalias %1, %2
 *   copy %1, %1
 *   conjureuse %1
 *   conjureuse %2
 *
 * will be rewritten as:
 *   conjure %3
 *   copy %3, %4
 *   conjureuse %4
 *   conjureuse %3
 */
jit::fast_map<Vreg, Vreg>
restoreSSA(Vunit& unit,
           const VregSet& targets,
           boost::dynamic_bitset<>& blocksWithTargets,
           const jit::vector<Vlabel>& rpo,
           const PredVector& preds,
           MaybeVinstrId = {});

///////////////////////////////////////////////////////////////////////////////

/*
 * Return a Vloc holding the constant value represented by the given Type.
 */
Vloc make_const(Vunit&, Type);

/*
 * Move all the elements of `in' into `out', replacing `count' elements of
 * `out' starting at `idx'.  `in' is cleared at the end.
 *
 * Example: vector_splice([1, 2, 3, 4, 5], 2, 1, [10, 11, 12]) will change
 * `out' to [1, 2, 10, 11, 12, 4, 5].
 */
template<typename V>
void vector_splice(V& out, size_t idx, size_t count, V& in) {
  auto out_size = out.size();

  if (in.size() > count) {
    // Start by making room in out for the new elements.
    out.resize(out.size() + in.size() - count);

    // Move everything after the to-be-overwritten elements to the new end.
    std::move_backward(out.begin() + idx + count, out.begin() + out_size,
                       out.end());
  } else if (in.size() < count) {
    std::move(out.begin() + idx + count, out.end(),
              out.begin() + idx + in.size());
    out.resize(out.size() + in.size() - count);
  }
  // Move the new elements in.
  std::move(in.begin(), in.end(), out.begin() + idx);
  in.clear();
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Modify instruction `i' in block `b' of `unit' via the output of `modify'.
 *
 * `modify' is passed a scratch Vinstr stream to fill with instructions.  It
 * should return the number of instructions to remove from the Vunit, starting
 * at block `b' instruction `i'.  The contents of the stream are inserted in
 * place of the instructions removed.
 *
 * If `modify' both returns 0 and fails to populate the stream, nothing
 * happens, and false is returned; otherwise, return true.
 *
 * The `modify' implementation should never directly mutate instructions in the
 * stream.  Furthermore, once vmodify() is run, all pointers and references to
 * instructions in block `b' should be considered invalidated.
 */
template<typename Modify>
bool vmodify(Vunit& unit, Vlabel b, size_t i, Modify modify) {
  auto& blocks = unit.blocks;
  auto const& vinstr = blocks[b].code[i];

  auto const scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, vinstr.irctx());

  auto const nremove = modify(v);
  if (nremove == 0 && v.empty()) return false;

  vector_splice(blocks[b].code, i, nremove, blocks[scratch].code);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Determine whether any VregSF is live at the beginning of each block.
 */
std::vector<Vreg> compute_sf_livein(const Vunit& unit,
                                    const jit::vector<Vlabel>& rpo,
                                    const PredVector& preds);

/*
 * Rename all VregSFs in unit to the physical flags reg.
 */
void rename_sf_regs(Vunit& unit, const jit::fast_set<unsigned>& sf_renames);

/*
 * Compute livein set for each block, using an iterative data-flow analysis.
 */
using LiveSet = boost::dynamic_bitset<>; // Bitset of Vreg numbers.

jit::vector<LiveSet> computeLiveness(const Vunit& unit,
                                     const Abi& abi,
                                     const jit::vector<Vlabel>& blocks);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
