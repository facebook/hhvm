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
#include "hphp/runtime/vm/jit/trans-prof-counters.h"
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
 * Profile counters for vasm blocks for each optimized JIT region and prologue.
 * Along with the counters, we keep the opcode of the first Vasm instruction
 * in each block as metadata (prior to inserting the profile counters), which
 * is used to validate the profile.
 */
using VOpRaw = std::underlying_type<Vinstr::Opcode>::type;
TransProfCounters<int64_t, VOpRaw> s_blockCounters;

/*
 * Insert profile counters for the blocks in the given unit.
 */
template <typename T>
void insert(Vunit& unit, const T& key) {
  assertx(isJitSerializing());

  splitCriticalEdges(unit);

  auto const blocks = sortBlocks(unit);
  auto const livein = computeLiveness(unit, abi(), blocks);
  auto const gp_regs = abi().gpUnreserved;
  auto const rgpFallback = abi().gp().choose();
  auto const rsf = abi().sf.choose();

  for (auto b : blocks) {
    auto& block = unit.blocks[b];
    auto const counterAddr = s_blockCounters.addCounter(key, block.code.front().op);
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
    // flipped when serializing the data in TransProfCounters::serialize().
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
 * `unit'. If they don't, a string with the error found is returned. Otherwise
 * (on success), an empty string is returned.
 */
std::string checkProfile(const Vunit& unit,
                         const jit::vector<Vlabel>& sortedBlocks,
                         const jit::vector<int64_t>& counters,
                         const jit::vector<VOpRaw>& opcodes) {
  auto const kind = unit.context->kind == TransKind::OptPrologue ? "prologue" : "region";
  if (counters.size() == 0) return folly::sformat("no profile for this {}", kind);

  std::string errorMsg;
  size_t opcodeMismatches=0;

  auto report = [&] {
    if (!errorMsg.empty()) return;
    if (unit.context->kind == TransKind::OptPrologue) {
      errorMsg = show(unit.context->pid) + "\n";
      FTRACE(1, "VasmBlockCounters::checkProfile: PrologueID={}", errorMsg);
    } else {
      errorMsg = show(unit.context->region->entry()->start()) + "\n";
      FTRACE(1, "VasmBlockCounters::checkProfile: SrcKey={}", errorMsg);
    }
  };

  for (size_t index = 0; index < sortedBlocks.size(); index++) {
    auto b = sortedBlocks[index];
    auto& block = unit.blocks[b];
    if (index >= counters.size()) {
      report();
      auto const msg = folly::sformat(
        "missing block counter for B{} (index = {}, counters.size() = {})\n",
        size_t{b}, index, counters.size());
      FTRACE(1, "VasmBlockCounters::checkProfile: {}", msg);
      errorMsg += msg;
      return errorMsg;
    }
    auto const op_opti = block.code.front().op;
    auto const op_prof = opcodes[index];
    if (op_prof != op_opti) {
      report();
      auto const msg = folly::sformat(
        "mismatch opcode for B{} (index = {}): "
        "profile was {} optimized is {}\n",
        size_t{b}, index, vinst_names[op_prof], vinst_names[op_opti]);
      FTRACE(1, "VasmBlockCounters::checkProfile: {}", msg);
      errorMsg += msg;
      opcodeMismatches++;
    }
  }

  // Consider the profile to match even if we have some opcode mismatches.
  if (opcodeMismatches <=
      RuntimeOption::EvalJitPGOVasmBlockCountersMaxOpMismatches) {
    return "";
  }
  return errorMsg;
}

/*
 * Set the weights of the blocks in the given unit based on profile data, if
 * available.
 */
template <typename T>
void setWeights(Vunit& unit, const T& key) {
  assertx(isJitDeserializing());

  jit::vector<VOpRaw> opcodes;
  auto counters = s_blockCounters.getCounters(key, opcodes);

  splitCriticalEdges(unit);

  auto sortedBlocks = sortBlocks(unit);

  FTRACE(1, "VasmBlockCounters::setWeights: original unit\n{}", show(unit));

  std::string errorMsg = checkProfile(unit, sortedBlocks, counters, opcodes);

  auto DEBUG_ONLY prefix = "un";
  bool enoughProfile = false;

  if (errorMsg == "") {
    // Check that enough profile was collected.
    if (counters[0] >= RuntimeOption::EvalJitPGOVasmBlockCountersMinEntryValue) {
      prefix = "";
      enoughProfile = true;
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
  }

  if (RuntimeOption::EvalDumpVBC) {
    unit.annotations.emplace_back("VasmBlockCounters",
                                  errorMsg != "" ? errorMsg :
                                  enoughProfile ? "matches" :
                                  "matches, but not enough profile data");
  }
  FTRACE(1, "VasmBlockCounters::setWeights: {}modified unit\n{}",
         prefix, show(unit));
}

}

///////////////////////////////////////////////////////////////////////////////

Optional<uint64_t> getRegionWeight(const RegionDesc& region) {
  if (!RO::EvalJitPGOVasmBlockCounters || !isJitDeserializing()) {
    return std::nullopt;
  }
  auto const key = RegionEntryKey(region);
  auto const weight = s_blockCounters.getFirstCounter(key);
  if (!weight) return std::nullopt;
  return safe_cast<uint64_t>(*weight);
}

template <typename T>
void update(Vunit& unit, const T& key){
  if (isJitSerializing()) {
    // Insert block profile counters.
    insert(unit, key);
  } else if (isJitDeserializing()) {
    // Look up the information from the profile, and use it if found.
    setWeights(unit, key);
  }
}

void profileGuidedUpdate(Vunit& unit) {
  if (!RuntimeOption::EvalJitPGOVasmBlockCounters) return;
  if (!unit.context) return;
  auto const optimizePrologue = unit.context->kind == TransKind::OptPrologue &&
    RuntimeOption::EvalJitPGOVasmBlockCountersOptPrologue;

  if (unit.context->kind == TransKind::Optimize) {
    auto const regionPtr = unit.context->region;
    if (!regionPtr) return;
    const RegionEntryKey regionKey(*regionPtr);
    update(unit, regionKey);
  } else if (optimizePrologue) {
    auto const pid = unit.context->pid;
    if (pid.funcId() == FuncId::Invalid) return;
    update(unit, pid);
  }
}

///////////////////////////////////////////////////////////////////////////////

void serialize(ProfDataSerializer& ser) {
  s_blockCounters.serialize(ser);
}

void deserialize(ProfDataDeserializer& des) {
  s_blockCounters.deserialize(des);
}

void free() {
  s_blockCounters.freeCounters();
}

///////////////////////////////////////////////////////////////////////////////

} } }
