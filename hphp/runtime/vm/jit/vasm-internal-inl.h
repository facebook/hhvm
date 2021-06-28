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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/data-block.h"

#include <vector>

namespace HPHP { namespace jit {

struct IRInstruction;

///////////////////////////////////////////////////////////////////////////////

namespace vasm_detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * Nouned verb class used to hide mostly-debug metadata updates from the main
 * body of vasm_emit().
 *
 * This is invoked on every Vinstr encountered in order to accumulate mappings
 * from higher-level representations.
 */
struct IRMetadataUpdater {
  IRMetadataUpdater(const Venv& env, AsmInfo* asm_info);

  /*
   * Update IR mappings for a Vinstr.
   */
  void register_inst(const Vinstr& inst);

  /*
   * Update IR mappings at the end of a block.
   */
  void register_block_end();

  /*
   * Update AsmInfo after the Vunit has been fully emitted.
   */
  void finish(const jit::vector<Vlabel>& labels);

private:
  struct Snippet {
    const IRInstruction* origin;
    TcaRange range;
  };

  /*
   * Get HHIR mapping info for the current block in `m_env'.
   */
  jit::vector<Snippet>& block_info();

private:
  const Venv& m_env;
  AsmInfo* m_asm_info;
  const IRInstruction* m_origin{nullptr};
  jit::vector<jit::vector<jit::vector<Snippet>>> m_area_to_blockinfos;
  std::vector<TransBCMapping>* m_bcmap{nullptr};
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Is `block' an empty catch block?
 */
bool is_empty_catch(const Vblock& block);

/*
 * Register catch blocks for fixups.
 */
void register_catch_block(const Venv& env, const Venv::LabelPatch& p);

/*
 * Arch-independent emitters.
 *
 * Return true if the instruction was supported.
 */
template <class Inst>
bool emit(Venv& /*env*/, const Inst&) {
  return false;
}
bool emit(Venv& env, const callphps& i);
bool emit(Venv& env, const bindjmp& i);
bool emit(Venv& env, const bindjcc& i);
bool emit(Venv& env, const bindaddr& i);
bool emit(Venv& env, const ldbindaddr& i);
bool emit(Venv& env, const fallback& i);
bool emit(Venv& env, const fallbackcc& i);
bool emit(Venv& env, const movqs& i);
bool emit(Venv& env, const jmps& i);

inline bool emit(Venv& env, const pushframe&) {
  if (env.frame == -1) return true; // unreachable block
  assertx(env.pending_frames > 0);

  --env.pending_frames;
  return true;
}

inline bool emit(Venv& env, const popframe&) {
  if (env.frame == -1) return true; // unreachable block

  ++env.pending_frames;
  return true;
}

inline bool emit(Venv& env, const recordbasenativesp& i) {
  return true;
}

inline bool emit(Venv& env, const unrecordbasenativesp& i) {
  return true;
}

inline bool emit(Venv& env, const recordstack& i) {
  env.record_inline_stack(i.fakeAddress);
  return true;
}

inline void record_frame(Venv& env) {
  auto const& block = env.unit.blocks[env.current];
  auto const frame = env.frame;
  auto const start = env.framestart;
  auto& frames = env.unit.frames;
  auto const size = env.cb->frontier() - start;
  // It's possible that (a) this block is empty, and (b) cb is full, so the
  // frontier from the start of the block is actually the first byte after the
  // block. This is particularly likely when RetranslateAll is in use as
  // ephemeral code blocks are resizable.
  always_assert(!size || env.cb->contains(start));
  always_assert((int64_t)size >= 0);
  auto const area = static_cast<uint8_t>(block.area_idx);
  frames[frame].sections[area].exclusive += size;
  for (auto f = block.frame; f != Vframe::Top; f = frames[f].parent) {
    frames[f].sections[area].inclusive += size;
  }
  env.framestart = env.cb->frontier();
}

inline bool emit(Venv& env, const inlinestart& i) {
  if (env.frame == -1) return true; // unreachable block

  ++env.pending_frames;
  always_assert(0 <= i.id && i.id < env.unit.frames.size());
  record_frame(env);
  env.frame = i.id;
  return true;
}
inline bool emit(Venv& env, const inlineend&) {
  if (env.frame == -1) return true; // unreachable block
  assertx(env.pending_frames > 0);

  --env.pending_frames;
  record_frame(env);
  env.frame = env.unit.frames[env.frame].parent;
  always_assert(0 <= env.frame && env.frame < env.unit.frames.size());
  return true;
}

template<class Vemit>
void check_nop_interval(Venv& env, const Vinstr& inst,
                        int& nop_counter, int nop_interval) {
  if (LIKELY(nop_counter < 0)) return;

  switch (inst.op) {
    // These instructions are for exception handling or state syncing and do
    // not represent any actual work, so they're excluded from the nop counter.
    case Vinstr::landingpad:
    case Vinstr::nothrow:
    case Vinstr::syncpoint:
    case Vinstr::unwind:
      break;

    default:
      if (--nop_counter == 0) {
        // We use a special emit_nop() function rather than emit(nop{}) because
        // many performance counters exclude nops from their count of retired
        // instructions. It's up the to arch-specific backends to emit some
        // real work with no visible side-effects.
        Vemit(env).emit_nop();
        nop_counter = nop_interval;
      }
      break;
  }
}

template<class Vemit>
void emitLdBindRetAddrStubs(Venv& env) {
  jit::fast_map<SrcKey, CodeAddress, SrcKey::Hasher> stubs;
  env.cb = &env.text.areas().back().code;

  for (auto const& ldbindretaddr : env.ldbindretaddrs) {
    CodeAddress stub = [&] {
      auto const i = stubs.find(ldbindretaddr.target);
      if (i != stubs.end()) return i->second;

      auto const start = env.cb->frontier();
      stubs.insert({ldbindretaddr.target, start});

      // Store return value to the stack.
      Vemit(env).emit(store{rret_data(), rvmsp()[TVOFF(m_data)]});
      Vemit(env).emit(store{rret_type(), rvmsp()[TVOFF(m_type)]});

      // Bind jump to the translation.
      emit(env, bindjmp{
        ldbindretaddr.target,
        ldbindretaddr.spOff,
        cross_trace_regs_resumed()
      });

      return start;
    }();

    auto const addr = env.unit.makeAddr();
    assertx(env.vaddrs.size() == addr);
    env.vaddrs.push_back(stub);
    env.leas.push_back({ldbindretaddr.instr, addr});
  }
}

void computeFrames(Vunit& unit);

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

inline Venv::Venv(Vunit& unit, Vtext& text, CGMeta& meta)
  : unit(unit)
  , text(text)
  , meta(meta)
{
  vaddrs.resize(unit.next_vaddr);
}

inline void Venv::record_inline_stack(TCA addr) {
  // Do not record stack if we are not inlining or the code is unreachable.
  if (frame <= 0) return;

  uint32_t callOff = 0;
  auto const func = unit.frames[frame].func;
  auto const in_builtin = func && func->isCPPBuiltin();
  if (origin && !in_builtin) {
    callOff = origin->marker().bcOff();
  }
  stacks.emplace_back(addr, IStack{frame - 1, pending_frames, callOff});
}

template<class Vemit>
void vasm_emit(Vunit& unit, Vtext& text, CGMeta& fixups,
               AsmInfo* asm_info) {
  using namespace vasm_detail;

  // Lower inlinestart and inlineend instructions to jmps, and annotate blocks
  // with inlined function parents
  if (unit.needsFramesComputed()) computeFrames(unit);

  Venv env { unit, text, fixups };
  env.addrs.resize(unit.blocks.size());

  auto labels = layoutBlocks(unit);

  IRMetadataUpdater irmu(env, asm_info);

  auto const area_start = [&] (Vlabel b) {
    auto area = unit.blocks[b].area_idx;
    return text.area(area).start;
  };

  // We don't want to put nops in Vunits representing stubs, and those Vunits
  // don't have a context set.
  auto const nop_interval =
    unit.context ? RuntimeOption::EvalJitNopInterval : 0;
  auto nop_counter = nop_interval;

  for (int i = 0, n = labels.size(); i < n; ++i) {
    assertx(checkBlockEnd(unit, labels[i]));

    auto b = labels[i];
    auto& block = unit.blocks[b];

    env.cb = &text.area(block.area_idx).code;
    env.addrs[b] = env.cb->frontier();
    env.framestart = env.cb->frontier();
    env.frame = block.frame;
    env.pending_frames = std::max<int32_t>(block.pending_frames, 0);

    { // Compute the next block we will emit into the current area.
      auto const cur_start = area_start(labels[i]);
      auto j = i + 1;
      while (j < labels.size() &&
             cur_start != area_start(labels[j])) {
        j++;
      }
      env.next = j < labels.size() ? labels[j] : Vlabel(unit.blocks.size());
      env.current = b;
    }

    // We'll replace exception edges to empty catch blocks with the catch
    // helper unique stub.
    if (is_empty_catch(block)) continue;

    for (auto& inst : block.code) {
      irmu.register_inst(inst);
      env.origin = inst.origin;

      check_nop_interval<Vemit>(env, inst, nop_counter, nop_interval);

      switch (inst.op) {
#define O(name, imms, uses, defs)               \
        case Vinstr::name:                      \
          if (emit(env, inst.name##_)) break;   \
          Vemit(env).emit(inst.name##_);        \
          break;
        VASM_OPCODES
#undef O
      }
    }

    if (block.frame != -1) record_frame(env);
    irmu.register_block_end();
  }

  emitLdBindRetAddrStubs<Vemit>(env);

  Vemit::emitVeneers(env);

  Vemit::handleLiterals(env);

  // Bind any Vaddrs that correspond to Vlabels.
  for (auto const& p : env.pending_vaddrs) {
    assertx(env.addrs[p.target]);
    env.vaddrs[p.vaddr] = env.addrs[p.target];
  }

  // Retarget smashable binds.
  Vemit::retargetBinds(env);

  // Patch up jump targets and friends.
  Vemit::patch(env);

  // Register catch blocks.
  for (auto& p : env.catches) register_catch_block(env, p);

  // Register inline frames.
  for (auto& f : unit.frames) {
    if (f.parent == Vframe::Top) continue; // skip the top frame
    fixups.inlineFrames.emplace_back(IFrame{f.func, f.callOff, f.parent - 1});
  }

  // Register inline stacks.
  fixups.inlineStacks = std::move(env.stacks);

  if (unit.padding) Vemit::pad(text.main().code);

  irmu.finish(labels);
}

///////////////////////////////////////////////////////////////////////////////

}}
