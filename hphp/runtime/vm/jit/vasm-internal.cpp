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

#include "hphp/runtime/vm/jit/vasm-internal.h"

#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/data-block.h"

#include <vector>

namespace HPHP::jit { namespace vasm_detail {

///////////////////////////////////////////////////////////////////////////////

IRMetadataUpdater::IRMetadataUpdater(const Venv& env, AsmInfo* asm_info)
  : m_env(env)
  , m_asm_info(asm_info)
{
  if (m_asm_info) {
    m_area_to_blockinfos.resize(m_env.text.areas().size());
    for (auto& r : m_area_to_blockinfos) r.resize(m_env.unit.blocks.size());
  }
  if (transdb::enabled() || RuntimeOption::EvalJitUseVtuneAPI) {
    m_bcmap = &env.meta.bcMap;
  }
}

void IRMetadataUpdater::register_inst(const Vinstr& inst) {
  // Update HHIR mappings for AsmInfo.
  if (m_asm_info) {
    auto& snippets = block_info();
    auto const frontier = m_env.cb->frontier();

    if (!snippets.empty()) {
      auto& snip = snippets.back();
      snip.range = TcaRange { snip.range.start(), frontier };
    }
    snippets.push_back(
      Snippet { inst.origin, TcaRange { frontier, nullptr } }
    );
  }
  m_origin = inst.origin;

  // Update HHBC mappings for the TransDB.
  if (m_bcmap && m_origin) {
    auto const sk = inst.origin->marker().sk();
    if (m_bcmap->empty() || m_bcmap->back().sk != sk) {
      m_bcmap->push_back(TransBCMapping{
        sk.unit()->sha1(),
        sk,
        m_env.text.main().code.frontier(),
        m_env.text.cold().code.frontier(),
        m_env.text.frozen().code.frontier()
      });
    }
  }
}

void IRMetadataUpdater::register_block_end() {
  if (!m_asm_info) return;
  auto& snippets = block_info();

  if (!snippets.empty()) {
    auto& snip = snippets.back();
    snip.range = TcaRange { snip.range.start(), m_env.cb->frontier() };
  }
}

void IRMetadataUpdater::finish(const jit::vector<Vlabel>& labels) {
  if (!m_asm_info) return;

  auto const& areas = m_env.text.areas();

  for (auto i = 0; i < areas.size(); ++i) {
    auto& block_infos = m_area_to_blockinfos[i];

    for (auto const b : labels) {
      auto const& snippets = block_infos[b];
      if (snippets.empty()) continue;

      const IRInstruction* origin = nullptr;

      for (auto const& snip : snippets) {
        if (origin != snip.origin && snip.origin) {
          origin = snip.origin;
        }
        m_asm_info->updateForInstruction(
          origin,
          static_cast<AreaIndex>(i),
          snip.range.start(),
          snip.range.end()
        );
      }
    }
  }
}

jit::vector<IRMetadataUpdater::Snippet>&
IRMetadataUpdater::block_info() {
  auto const b = m_env.current;
  auto const& block = m_env.unit.blocks[b];

  return m_area_to_blockinfos[size_t(block.area_idx)][b];
}

///////////////////////////////////////////////////////////////////////////////

bool is_empty_catch(const Vblock& block) {
  return block.code.size() == 2 &&
         block.code[0].op == Vinstr::landingpad &&
         block.code[1].op == Vinstr::jmpi &&
         block.code[1].jmpi_.target == tc::ustubs().endCatchHelper;
}

void register_catch_block(const Venv& env, const Venv::LabelPatch& p) {
  // If the catch block is empty, we can just let tc_unwind_resume() and
  // tc_unwind_personality() skip over our frame.
  if (is_empty_catch(env.unit.blocks[p.target])) {
    return;
  }

  auto const catch_target = env.addrs[p.target];
  assertx(catch_target);
  env.meta.catches.emplace_back(p.instr, catch_target);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * For profiling translations, record in ProfData that the control-transfer
 * instruction `jmp' is associated with the current translation being emitted.
 */
void setJmpTransID(Venv& env, TCA jmp) {
  if (!env.unit.context || env.unit.context->kind != TransKind::Profile) return;

  assertx(env.unit.context->transIDs.size() == 1);
  auto const transID = *env.unit.context->transIDs.begin();

  env.meta.setJmpTransID(jmp, transID, env.unit.context->kind);
}

void registerFallbackJump(Venv& env, TCA jmp, ConditionCode cc) {
  auto const incoming = cc == CC_None ? IncomingBranch::jmpFrom(jmp)
                                      : IncomingBranch::jccFrom(jmp);

  env.meta.inProgressTailJumps.push_back(incoming);
}

}

bool emit(Venv& env, const callphps& i) {
  const auto call = emitSmashableCall(*env.cb, env.meta, i.target);
  setJmpTransID(env, call);
  env.meta.smashableCallData[call] = PrologueID(i.func, i.nargs);
  setCallFuncId(env, call + smashableCallLen());
  return true;
}

bool emit(Venv& env, const callphpfe& i) {
  assertx(i.target.funcEntry());
  auto const call = emitSmashableCall(*env.cb, env.meta, env.cb->frontier());
  setJmpTransID(env, call);
  env.meta.smashableBinds.push_back({
    IncomingBranch::callFrom(call), i.target, SBInvOffset{0},
    false /* fallback */});
  return true;
}

bool emit(Venv& env, const bindjmp& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.meta, env.cb->frontier());
  setJmpTransID(env, jmp);
  env.meta.smashableBinds.push_back({
    IncomingBranch::jmpFrom(jmp), i.target, i.spOff, false /* fallback */});
  return true;
}

bool emit(Venv& env, const bindjcc& i) {
  auto const jcc =
    emitSmashableJcc(*env.cb, env.meta, env.cb->frontier(), i.cc);
  setJmpTransID(env, jcc);
  env.meta.smashableBinds.push_back({
    IncomingBranch::jccFrom(jcc), i.target, i.spOff, false /* fallback */});
  return true;
}

bool emit(Venv& env, const bindaddr& i) {
  setJmpTransID(env, TCA(i.addr.get()));
  env.meta.codePointers.emplace(i.addr.get());
  env.meta.smashableBinds.push_back({
    IncomingBranch::addr(i.addr.get()), i.target, i.spOff,
    false /* fallback */});
  return true;
}

bool emit(Venv& env, const ldbindaddr& i) {
  auto const mov_addr = emitSmashableMovq(*env.cb, env.meta, 0, r64(i.d));
  setJmpTransID(env, mov_addr);
  env.meta.smashableBinds.push_back({
    IncomingBranch::ldaddr(mov_addr), i.target, i.spOff,
    false /* fallback */});
  return true;
}

bool emit(Venv& env, const fallback& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.meta, env.cb->frontier());
  registerFallbackJump(env, jmp, CC_None);
  env.meta.smashableBinds.push_back({
    IncomingBranch::jmpFrom(jmp), i.target, i.spOff, true /* fallback */});
  return true;
}

bool emit(Venv& env, const fallbackcc& i) {
  auto const jcc =
    emitSmashableJcc(*env.cb, env.meta, env.cb->frontier(), i.cc);
  registerFallbackJump(env, jcc, i.cc);
  env.meta.smashableBinds.push_back({
    IncomingBranch::jccFrom(jcc), i.target, i.spOff, true /* fallback */});
  return true;
}

bool emit(Venv& env, const movqs& i) {
  auto const mov = emitSmashableMovq(*env.cb, env.meta, i.s.q(), r64(i.d));
  if (i.addr.isValid()) {
    env.vaddrs[i.addr] = mov;
  }
  return true;
}

bool emit(Venv& env, const jmps& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.meta, env.cb->frontier());
  env.jmps.push_back({jmp, i.targets[0]});
  if (i.jmp_addr.isValid()) {
    env.vaddrs[i.jmp_addr] = jmp;
  }
  if (i.taken_addr.isValid()) {
    env.pending_vaddrs.push_back({i.taken_addr, i.targets[1]});
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Computes inline frames for each block in unit. Inline frames are dominated
 * by an inlinestart instruction and post-dominated by an inlineend instruction.
 * This function annotates Vblocks with their associated frame, and populates
 * the frame vector. Additionally, inlinestart and inlineend instructions are
 * replaced by jmp instructions.
 */
void computeFrames(Vunit& unit) {
  auto const topFunc = unit.context ? unit.context->initSrcKey.func() : nullptr;

  auto const rpo = sortBlocks(unit);

  assertx(unit.frames.size() == Vframe::Root);
  unit.frames.emplace_back(
    topFunc, 0, SBInvOffset{0}, Vframe::Top, 0, unit.blocks[rpo[0]].weight
  );
  unit.blocks[rpo[0]].frame = 0;
  unit.blocks[rpo[0]].pending_frames = 0;
  for (auto const b : rpo) {
    auto& block = unit.blocks[b];
    int pending = block.pending_frames;
    assert_flog(block.frame != -1, "Block frames cannot be uninitialized.");

    if (block.code.empty()) continue;

    auto const next_frame = [&] () -> int {
      auto frame = block.frame;
      for (auto& inst : block.code) {
        switch (inst.op) {
        case Vinstr::inlinestart: {
          frame = inst.inlinestart_.id = unit.allocFrame(inst, frame,
                                                         block.weight);
          pending++;
          break;
        }
        case Vinstr::inlineend:
          frame = unit.frames[frame].parent;
          pending--;
          break;
        case Vinstr::pushframe:
          pending--;
          break;
        default: break;
        }
      }
      return frame;
    }();

    for (auto const s : succs(block)) {
      auto& sblock = unit.blocks[s];
      assert_flog(
        (sblock.frame == -1 || sblock.frame == next_frame) &&
        (sblock.pending_frames == -1 || sblock.pending_frames == pending),
        "Blocks must be dominated by a single inline frame at the same depth,"
        "{} cannot have frames {} ({}) and {} ({}) at depths {} and {}.",
        s,
        sblock.frame,
        unit.frames[sblock.frame].func
          ? unit.frames[sblock.frame].func->fullName()->data()
          : "(null)",
        next_frame,
        unit.frames[next_frame].func
          ? unit.frames[next_frame].func->fullName()->data()
          : "(null)",
        sblock.pending_frames,
        pending
      );
      sblock.frame = next_frame;
      sblock.pending_frames = pending;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////

}

const uint64_t* alloc_literal(Venv& env, uint64_t val) {
  // TreadHashMap doesn't support 0 as a key, and we have far more efficient
  // ways of getting 0 in a register anyway.
  always_assert(val != 0);

  if (auto addr = addrForLiteral(val)) return addr;

  auto& pending = env.meta.literalAddrs;
  auto it = pending.find(val);
  if (it != pending.end()) {
    DEBUG_ONLY auto realAddr =
      (uint64_t*)env.text.data().toDestAddress((TCA)it->second);
    assertx(*realAddr == val);
    return it->second;
  }

  auto addr = env.text.data().alloc<uint64_t>(alignof(uint64_t));
  auto realAddr = (uint64_t*)env.text.data().toDestAddress((TCA)addr);
  *realAddr = val;

  pending.emplace(val, addr);
  return addr;
}

///////////////////////////////////////////////////////////////////////////////

void setCallFuncId(Venv& env, TCA callRetAddr) {
  if (!env.unit.context) return;

  env.meta.setCallFuncId(callRetAddr, env.unit.context->initSrcKey.funcID(),
                         env.unit.context->kind);
}

///////////////////////////////////////////////////////////////////////////////

}
