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

#include "hphp/runtime/vm/jit/vasm-internal.h"

#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/data-block.h"

#include <vector>

namespace HPHP { namespace jit { namespace vasm_detail {

///////////////////////////////////////////////////////////////////////////////

IRMetadataUpdater::IRMetadataUpdater(const Venv& env, AsmInfo* asm_info)
  : m_env(env)
  , m_asm_info(asm_info)
{
  if (m_asm_info) {
    m_area_to_blockinfos.resize(m_env.text.areas().size());
    for (auto& r : m_area_to_blockinfos) r.resize(m_env.unit.blocks.size());
  }
  if (mcg->tx().isTransDBEnabled() ||
      RuntimeOption::EvalJitUseVtuneAPI) {
    m_bcmap = &mcg->cgFixups().m_bcMap;
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
    if (m_bcmap->empty() ||
        m_bcmap->back().md5 != sk.unit()->md5() ||
        m_bcmap->back().bcStart != sk.offset()) {
      m_bcmap->push_back(TransBCMapping{
        sk.unit()->md5(),
        sk.offset(),
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

  return m_area_to_blockinfos[size_t(block.area)][b];
}

///////////////////////////////////////////////////////////////////////////////

bool is_empty_catch(const Vblock& block) {
  return block.code.size() == 2 &&
         block.code[0].op == Vinstr::landingpad &&
         block.code[1].op == Vinstr::jmpi &&
         block.code[1].jmpi_.target == mcg->tx().uniqueStubs.endCatchHelper;
}

void register_catch_block(const Venv& env, const Venv::LabelPatch& p) {
  bool const is_empty = is_empty_catch(env.unit.blocks[p.target]);

  auto const catch_target = is_empty
    ? mcg->tx().uniqueStubs.endCatchHelper
    : env.addrs[p.target];
  assertx(catch_target);

  mcg->registerCatchBlock(p.instr, catch_target);
}

///////////////////////////////////////////////////////////////////////////////

bool emit(Venv& env, const bindjmp& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.cb->frontier());
  env.stubs.push_back({jmp, nullptr, i});
  mcg->setJmpTransID(jmp);
  return true;
}

bool emit(Venv& env, const bindjcc& i) {
  auto const jcc = emitSmashableJcc(*env.cb, env.cb->frontier(), i.cc);
  env.stubs.push_back({nullptr, jcc, i});
  mcg->setJmpTransID(jcc);
  return true;
}

bool emit(Venv& env, const bindjcc1st& i) {
  auto const jcc_jmp =
    emitSmashableJccAndJmp(*env.cb, env.cb->frontier(), i.cc);

  env.stubs.push_back({jcc_jmp.second, jcc_jmp.first, i});

  mcg->setJmpTransID(jcc_jmp.first);
  mcg->setJmpTransID(jcc_jmp.second);
  return true;
}

bool emit(Venv& env, const bindaddr& i) {
  env.stubs.push_back({nullptr, nullptr, i});
  mcg->setJmpTransID(TCA(i.addr));
  mcg->cgFixups().m_codePointers.insert(i.addr);
  return true;
}

bool emit(Venv& env, const fallback& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.cb->frontier());
  env.stubs.push_back({jmp, nullptr, i});
  mcg->tx().getSrcRec(i.target)->registerFallbackJump(jmp, CC_None);
  return true;
}

bool emit(Venv& env, const fallbackcc& i) {
  auto const jcc = emitSmashableJcc(*env.cb, env.cb->frontier(), i.cc);
  env.stubs.push_back({nullptr, jcc, i});
  mcg->tx().getSrcRec(i.target)->registerFallbackJump(jcc, i.cc);
  return true;
}

bool emit(Venv& env, const retransopt& i) {
  svcreq::emit_retranslate_opt_stub(*env.cb, i.spOff, i.target, i.transID);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void emit_svcreq_stub(Venv& env, const Venv::SvcReqPatch& p) {
  auto& frozen = env.text.frozen().code;

  TCA stub = nullptr;

  switch (p.svcreq.op) {
    case Vinstr::bindjmp:
      { auto const& i = p.svcreq.bindjmp_;
        assertx(p.jmp && !p.jcc);
        stub = svcreq::emit_bindjmp_stub(frozen, i.spOff, p.jmp,
                                         i.target, i.trflags);
      } break;

    case Vinstr::bindjcc:
      { auto const& i = p.svcreq.bindjcc_;
        assertx(!p.jmp && p.jcc);
        stub = svcreq::emit_bindjmp_stub(frozen, i.spOff, p.jcc,
                                         i.target, i.trflags);
      } break;

    case Vinstr::bindaddr:
      { auto const& i = p.svcreq.bindaddr_;
        assertx(!p.jmp && !p.jcc);
        stub = svcreq::emit_bindaddr_stub(frozen, i.spOff, i.addr,
                                          i.target, TransFlags{});
        *i.addr = stub;
      } break;

    case Vinstr::bindjcc1st:
      { auto const& i = p.svcreq.bindjcc1st_;
        assertx(p.jmp && p.jcc);
        stub = svcreq::emit_bindjcc1st_stub(frozen, i.spOff, p.jcc,
                                            i.targets[1], i.targets[0], i.cc);
      } break;

    case Vinstr::fallback:
      { auto const& i = p.svcreq.fallback_;
        assertx(p.jmp && !p.jcc);

        auto const srcrec = mcg->tx().getSrcRec(i.target);
        stub = i.trflags.packed
          ? svcreq::emit_retranslate_stub(frozen, i.spOff, i.target, i.trflags)
          : srcrec->getFallbackTranslation();
      } break;

    case Vinstr::fallbackcc:
      { auto const& i = p.svcreq.fallbackcc_;
        assertx(!p.jmp && p.jcc);

        auto const srcrec = mcg->tx().getSrcRec(i.target);
        stub = i.trflags.packed
          ? svcreq::emit_retranslate_stub(frozen, i.spOff, i.target, i.trflags)
          : srcrec->getFallbackTranslation();
      } break;

    default: always_assert(false);
  }
  assertx(stub != nullptr);

  // Register any necessary patches by creating fake labels for the stubs.
  if (p.jmp) {
    env.jmps.push_back({p.jmp, Vlabel { env.addrs.size() }});
    env.addrs.push_back(stub);
  }
  if (p.jcc) {
    env.jccs.push_back({p.jcc, Vlabel { env.addrs.size() }});
    env.addrs.push_back(stub);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
