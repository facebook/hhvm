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
 * Emit a service request stub and register a patch point as needed.
 */
void emit_svcreq_stub(Venv& env, const Venv::SvcReqPatch& p);

/*
 * Arch-independent emitters.
 *
 * Return true if the instruction was supported.
 */
template<class Inst> bool emit(Venv& env, const Inst&) { return false; }
bool emit(Venv& env, const bindjmp& i);
bool emit(Venv& env, const bindjcc& i);
bool emit(Venv& env, const bindjcc1st& i);
bool emit(Venv& env, const bindaddr& i);
bool emit(Venv& env, const fallback& i);
bool emit(Venv& env, const fallbackcc& i);
bool emit(Venv& env, const retransopt& i);

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

template<class Vemit>
void vasm_emit(const Vunit& unit, Vtext& text, AsmInfo* asm_info) {
  using namespace vasm_detail;

  Venv env { unit, text };
  env.addrs.resize(unit.blocks.size());

  auto labels = layoutBlocks(unit);

  IRMetadataUpdater irmu(env, asm_info);

  auto const area_start = [&] (Vlabel b) {
    auto area = unit.blocks[b].area;
    return text.area(area).start;
  };

  for (int i = 0, n = labels.size(); i < n; ++i) {
    assertx(checkBlockEnd(unit, labels[i]));

    auto b = labels[i];
    auto& block = unit.blocks[b];

    env.cb = &text.area(block.area).code;
    env.addrs[b] = env.cb->frontier();

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

    irmu.register_block_end();
  }

  // Emit service request stubs and register patch points.
  for (auto& p : env.stubs) emit_svcreq_stub(env, p);

  // Patch up jump targets and friends.
  Vemit::patch(env);

  // Register catch blocks.
  for (auto& p : env.catches) register_catch_block(env, p);

  if (unit.padding) Vemit::pad(text.main().code);

  irmu.finish(labels);
}

///////////////////////////////////////////////////////////////////////////////

}}
