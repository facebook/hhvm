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
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"

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
  bool const is_empty = is_empty_catch(env.unit.blocks[p.target]);

  auto const catch_target = is_empty
    ? tc::ustubs().endCatchHelper
    : env.addrs[p.target];
  assertx(catch_target);

  env.meta.catches.emplace_back(p.instr, catch_target);
}

///////////////////////////////////////////////////////////////////////////////

static void setJmpTransID(Venv& env, TCA jmp) {
  if (!env.unit.context) return;

  env.meta.setJmpTransID(
    jmp, env.unit.context->transID, env.unit.context->kind
  );
}

static void registerFallbackJump(Venv& env, TCA jmp, ConditionCode cc) {
  auto const incoming = cc == CC_None ? IncomingBranch::jmpFrom(jmp)
                                      : IncomingBranch::jccFrom(jmp);

  env.meta.inProgressTailJumps.push_back(incoming);
}

bool emit(Venv& env, const bindjmp& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.meta, env.cb->frontier());
  env.stubs.push_back({jmp, nullptr, i});
  setJmpTransID(env, jmp);
  return true;
}

bool emit(Venv& env, const bindjcc& i) {
  auto const jcc =
    emitSmashableJcc(*env.cb, env.meta, env.cb->frontier(), i.cc);
  env.stubs.push_back({nullptr, jcc, i});
  setJmpTransID(env, jcc);
  return true;
}

bool emit(Venv& env, const bindaddr& i) {
  env.stubs.push_back({nullptr, nullptr, i});
  setJmpTransID(env, TCA(i.addr.get()));
  env.meta.codePointers.insert(i.addr.get());
  return true;
}

bool emit(Venv& env, const fallback& i) {
  auto const jmp = emitSmashableJmp(*env.cb, env.meta, env.cb->frontier());
  env.stubs.push_back({jmp, nullptr, i});
  registerFallbackJump(env, jmp, CC_None);
  return true;
}

bool emit(Venv& env, const fallbackcc& i) {
  auto const jcc =
    emitSmashableJcc(*env.cb, env.meta, env.cb->frontier(), i.cc);
  env.stubs.push_back({nullptr, jcc, i});
  registerFallbackJump(env, jcc, i.cc);
  return true;
}

bool emit(Venv& env, const retransopt& i) {
  svcreq::emit_retranslate_opt_stub(*env.cb, env.text.data(), i.spOff, i.sk);
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
        stub = svcreq::emit_bindjmp_stub(frozen, env.text.data(),
                                         env.meta, i.spOff, p.jmp,
                                         i.target, i.trflags);
      } break;

    case Vinstr::bindjcc:
      { auto const& i = p.svcreq.bindjcc_;
        assertx(!p.jmp && p.jcc);
        stub = svcreq::emit_bindjmp_stub(frozen, env.text.data(),
                                         env.meta, i.spOff, p.jcc,
                                         i.target, i.trflags);
      } break;

    case Vinstr::bindaddr:
      { auto const& i = p.svcreq.bindaddr_;
        assertx(!p.jmp && !p.jcc);
        stub = svcreq::emit_bindaddr_stub(frozen, env.text.data(),
                                          env.meta, i.spOff, i.addr.get(),
                                          i.target, TransFlags{});
        *i.addr.get() = stub;
      } break;

    case Vinstr::fallback:
      { auto const& i = p.svcreq.fallback_;
        assertx(p.jmp && !p.jcc);

        auto const srcrec = tc::findSrcRec(i.target);
        always_assert(srcrec);
        stub = i.trflags.packed
          ? svcreq::emit_retranslate_stub(frozen, env.text.data(),
                                          i.spOff, i.target, i.trflags)
          : srcrec->getFallbackTranslation();
      } break;

    case Vinstr::fallbackcc:
      { auto const& i = p.svcreq.fallbackcc_;
        assertx(!p.jmp && p.jcc);

        auto const srcrec = tc::findSrcRec(i.target);
        always_assert(srcrec);
        stub = i.trflags.packed
          ? svcreq::emit_retranslate_stub(frozen, env.text.data(),
                                          i.spOff, i.target, i.trflags)
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

}

const uint64_t* alloc_literal(Venv& env, uint64_t val) {
  // TreadHashMap doesn't support 0 as a key, and we have far more efficient
  // ways of getting 0 in a register anyway.
  always_assert(val != 0);

  if (auto addr = addrForLiteral(val)) return addr;

  auto& pending = env.meta.literals;
  auto it = pending.find(val);
  if (it != pending.end()) {
    assertx(*it->second == val);
    return it->second;
  }

  auto addr = env.text.data().alloc<uint64_t>(alignof(uint64_t));
  *addr = val;
  pending.emplace(val, addr);
  return addr;
}

}}
