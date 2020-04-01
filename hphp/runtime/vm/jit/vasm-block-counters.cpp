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

#include "hphp/runtime/vm/jit/vasm-block-counters.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/region-prof-counters.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/util/trace.h"

#include <type_traits>

TRACE_SET_MOD(vasm_block_count);

namespace HPHP { namespace jit {

namespace VasmBlockCounters {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Profile counters for vasm blocks for each optimized JIT region.  Along with
 * the counters, we keep the opcode of the first Vasm instruction in each block
 * as metadata (prior to inserting the profile counters), which is used to
 * validate the profile.
 */
using VOpRaw = std::underlying_type<Vinstr::Opcode>::type;
RegionProfCounters<int64_t, VOpRaw> s_blockCounters;

/*
 * Insert profile counters for the blocks in the given unit.
 */
void insert(Vunit& unit) {
  assertx(isJitSerializing());

  if (!unit.context) return;
  auto const regionPtr = unit.context->region;
  if (!regionPtr) return;
  const RegionEntryKey regionKey(*regionPtr);

  splitCriticalEdges(unit);

  auto const blocks = sortBlocks(unit);
  auto const livein = computeLiveness(unit, abi(), blocks);
  auto const gp_regs = abi().gpUnreserved;
  auto const rgpFallback = abi().gp().choose();
  auto const rsf = abi().sf.choose();

  for (auto b : blocks) {
    auto& block = unit.blocks[b];
    auto const counterAddr = s_blockCounters.addCounter(regionKey,
                                                        block.code.front().op);
    auto const& live_set = livein[b];
    auto const save_sf = live_set[Vreg(rsf)] ||
                         RuntimeOption::EvalJitPGOVasmBlockCountersForceSaveSF;

    // search for an available gp register to load the counter's address into it
    auto save_gp = true;
    auto rgp = rgpFallback;
    gp_regs.forEach([&](PhysReg r) {
                      if (save_gp && !live_set[Vreg(r)]) {
                        save_gp = false;
                        rgp = r;
                      }
                    });
    if (RuntimeOption::EvalJitPGOVasmBlockCountersForceSaveGP) save_gp = true;

    // emit the increment of the counter, saving/restoring the used registers on
    // the native stack before/after if needed.
    // NB: decqmlock instructions are currently used to update the counters, so
    // they actually start at zero and grow down from there.  The sign is then
    // flipped when serializing the data in RegionProfCounters::serialize().
    jit::vector<Vinstr> new_insts;
    if (save_gp) new_insts.insert(new_insts.end(), push{rgp});
    if (save_sf) new_insts.insert(new_insts.end(), pushf{rsf});
    new_insts.insert(new_insts.end(), ldimmq{counterAddr, rgp});
    new_insts.insert(new_insts.end(), decqmlock{rgp[0], rsf});
    if (save_sf) new_insts.insert(new_insts.end(), popf{rsf});
    if (save_gp) new_insts.insert(new_insts.end(), pop{rgp});

    // set irctx for the newly added instructions
    auto const irctx = block.code.front().irctx();
    for (auto& ni : new_insts) {
      ni.set_irctx(irctx);
    }

    // insert new instructions in the beginning of the block, but after any
    // existing phidef
    auto insertPt = block.code.begin();
    while (insertPt->op == Vinstr::phidef) insertPt++;
    block.code.insert(insertPt, new_insts.begin(), new_insts.end());
  }

  FTRACE(1, "VasmBlockCounters::insert: modified unit\n{}", show(unit));
}

/*
 * Checks whether the profile data in `counters' and `opcodes' matches the given
 * `unit'.  If they don't, a string with the error found is returned.  Otherwise
 * (on success), an empty string is returned.
 */
std::string checkProfile(const Vunit& unit,
                         const jit::vector<Vlabel>& sortedBlocks,
                         const jit::vector<int64_t>& counters,
                         const jit::vector<VOpRaw>& opcodes) {
  if (counters.size() == 0) return "no profile for this region";

  std::string errorMsg;

  for (size_t index = 0; index < sortedBlocks.size(); index++) {
    auto b = sortedBlocks[index];
    auto& block = unit.blocks[b];
    if (index >= counters.size()) {
      FTRACE(1, "VasmBlockCounters::checkProfile: missing block counter "
             "(index = {})\n", index);
      folly::format(
        &errorMsg,
        "missing block counter (index = {}, counters.size() = {})\n",
        index, counters.size()
      );
      break;
    }
    auto const op_prof = block.code.front().op;
    auto const op_opti = opcodes[index];
    if (op_prof != op_opti) {
      FTRACE(1, "VasmBlockCounters::checkProfile: mismatch opcode for block {}: "
             "profile was {} optimized is {}\n",
             index, vinst_names[op_prof], vinst_names[op_opti]);
      folly::format(
        &errorMsg,
        "mismatch opcode for block {}: profile was {}, optimized is {}\n",
        index, vinst_names[op_prof], vinst_names[op_opti]
      );
    }
  }

  return errorMsg;
}

/*
 * Set the weights of the blocks in the given unit based on profile data, if
 * available.
 */
void setWeights(Vunit& unit) {
  assertx(isJitDeserializing());

  if (!unit.context) return;
  auto const regionPtr = unit.context->region;
  if (!regionPtr) return;
  const RegionEntryKey regionKey(*regionPtr);

  jit::vector<VOpRaw> opcodes;
  auto counters = s_blockCounters.getCounters(regionKey, opcodes);

  splitCriticalEdges(unit);

  auto sortedBlocks = sortBlocks(unit);

  FTRACE(1, "VasmBlockCounters::setWeights: original unit\n{}", show(unit));

  std::string errorMsg = checkProfile(unit, sortedBlocks, counters, opcodes);

  if (errorMsg == "") {
    // Update the block weights.
    for (size_t index = 0; index < sortedBlocks.size(); index++) {
      auto const b = sortedBlocks[index];
      auto& block = unit.blocks[b];
      assertx(index < counters.size());
      block.weight = counters[index];
      // Drop the separation between the main, cold and frozen areas, to avoid
      // scaling of the block weights based on the area since we have accurate
      // counters.  The code-layout pass may re-split the code into
      // hot/cold/frozen later.
      block.area_idx = AreaIndex::Main;
    }
  }

  if (RuntimeOption::EvalDumpVBC) {
    unit.annotations.emplace_back("VasmBlockCounters",
                                  errorMsg != "" ? errorMsg : "matches");
  }
  FTRACE(1, "VasmBlockCounters::setWeights: modified unit\n{}", show(unit));
}

}

///////////////////////////////////////////////////////////////////////////////

void profileGuidedUpdate(Vunit& unit) {
  if (!RuntimeOption::EvalJitPGOVasmBlockCounters) return;
  if (!unit.context) return;
  if (unit.context->kind != TransKind::Optimize) return;

  if (isJitSerializing()) {
    // Insert block profile counters.
    insert(unit);
  } else if (isJitDeserializing()) {
    // Look up the information from the profile, and use it if found.
    setWeights(unit);
  }
}

///////////////////////////////////////////////////////////////////////////////

void serialize(ProfDataSerializer& ser) {
  s_blockCounters.serialize(ser);
}

///////////////////////////////////////////////////////////////////////////////

void deserialize(ProfDataDeserializer& des) {
  s_blockCounters.deserialize(des);
}

///////////////////////////////////////////////////////////////////////////////

} } }
