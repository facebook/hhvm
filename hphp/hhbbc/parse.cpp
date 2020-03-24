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
#include "hphp/hhbbc/parse.h"

#include <thread>
#include <unordered_map>
#include <map>

#include <boost/variant.hpp>
#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include <folly/gen/Base.h>
#include <folly/gen/String.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/sorted_vector_types.h>

#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_parse);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Closure("Closure");
const StaticString s_toString("__toString");
const StaticString s_Stringish("Stringish");
const StaticString s_XHPChild("XHPChild");
const StaticString s_attr_Deprecated("__Deprecated");
const StaticString s___NoContextSensitiveAnalysis(
    "__NoContextSensitiveAnalysis");

//////////////////////////////////////////////////////////////////////

struct ParseUnitState {
  std::atomic<uint32_t>& nextFuncId;

  /*
   * This is computed once for each unit and stashed here.  We support
   * having either a SourceLocTable or a LineTable.  If we're
   * optimizing a repo that was already created by hphpc, it won't
   * have the full SourceLocTable information in it, so we're limited
   * to line numbers.
   */
  boost::variant< SourceLocTable
                , LineTable
                > srcLocInfo;

  /*
   * Map from class id to the function containing its DefCls
   * instruction.  We use this to compute whether classes are defined
   * at top-level.
   *
   * TODO_4: if we don't end up with a use for this, remove it.
   */
  std::vector<php::Func*> defClsMap;

  /*
   * Map from Closure index to the function(s) containing their
   * associated CreateCl opcode(s).
   */
  hphp_fast_map<
    int32_t,
    hphp_fast_set<php::Func*>
  > createClMap;

  struct SrcLocHash {
    size_t operator()(const php::SrcLoc& sl) const {
      auto const h1 = ((size_t)sl.start.col << 32) | sl.start.line;
      auto const h2 = ((size_t)sl.past.col << 32) | sl.past.line;
      return hash_int64_pair(h1, h2);
    }
  };
  hphp_fast_map<php::SrcLoc, int32_t, SrcLocHash> srcLocs;

  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * Must include every function with a DefCns for correctness; cinit,
   * pinit and sinit functions are added to improve overall
   * performance.
   */
  hphp_fast_set<php::Func*> constPassFuncs;

};

//////////////////////////////////////////////////////////////////////

std::set<Offset> findBasicBlocks(const FuncEmitter& fe) {
  std::set<Offset> blockStarts;
  auto markBlock = [&] (Offset off) { blockStarts.insert(off); };

  // Each entry point for a DV funclet is the start of a basic
  // block.
  for (auto& param : fe.params) {
    if (param.hasDefaultValue()) markBlock(param.funcletOff);
  }

  // The main entry point is also a basic block start.
  markBlock(fe.base);

  bool traceBc = false;

  /*
   * For each instruction, add it to the set if it must be the start
   * of a block.  It is the start of a block if it is:
   *
   *   - A jump target
   *
   *   - Immediatelly following a control flow instruction, other than
   *     a call.
   */
  auto offset = fe.base;
  for (;;) {
    auto const bc = fe.ue().bc();
    auto const pc = bc + offset;
    auto const nextOff = offset + instrLen(pc);
    auto const atLast = nextOff == fe.past;
    auto const op = peek_op(pc);
    auto const breaksBB =
      instrIsNonCallControlFlow(op) ||
      instrFlags(op) & TF ||
      (isFCall(op) && !instrJumpOffsets(pc).empty());

    if (options.TraceBytecodes.count(op)) traceBc = true;

    if (breaksBB && !atLast) {
      markBlock(nextOff);
    }

    auto const targets = instrJumpTargets(bc, offset);
    for (auto const& target : targets) markBlock(target);

    offset = nextOff;
    if (atLast) break;
  }

  /*
   * Find blocks associated with exception handlers.
   *
   *   - The start of each EH protected region begins a block.
   *
   *   - The instruction immediately after the end of any
   *     EH protected region begins a block.
   *
   *   - Each catch entry point begins a block.
   *
   *   - The instruction immediately after the end of any
   *     catch region begins a block.
   */
  for (auto& eh : fe.ehtab) {
    markBlock(eh.m_base);
    markBlock(eh.m_past);
    markBlock(eh.m_handler);
    if (eh.m_end != kInvalidOffset) {
      markBlock(eh.m_end);
    }
  }

  // Now, each interval in blockStarts delinates a basic block.
  blockStarts.insert(fe.past);

  if (traceBc) {
    FTRACE(0, "TraceBytecode (parse): {}::{} in {}\n",
           fe.pce() ? fe.pce()->name()->data() : "",
           fe.name, fe.ue().m_filepath);
  }

  return blockStarts;
}

struct ExnTreeInfo {
  /*
   * Map from EHEnt to the ExnNode that will represent exception
   * behavior in that region.
   */
  hphp_fast_map<const EHEnt*,ExnNodeId> ehMap;
};

template<class FindBlock>
ExnTreeInfo build_exn_tree(const FuncEmitter& fe,
                           php::Func& func,
                           FindBlock findBlock) {
  ExnTreeInfo ret;
  func.exnNodes.reserve(fe.ehtab.size());
  for (auto& eh : fe.ehtab) {
    auto const catchBlk = findBlock(eh.m_handler, true);
    auto node = php::ExnNode{};
    node.idx = func.exnNodes.size();
    node.parent = NoExnNodeId;
    node.depth = 1; // 0 depth means no ExnNode
    node.region = php::CatchRegion { catchBlk, eh.m_iterId };
    ret.ehMap[&eh] = node.idx;

    if (eh.m_parentIndex != -1) {
      auto it = ret.ehMap.find(&fe.ehtab[eh.m_parentIndex]);
      assert(it != end(ret.ehMap));
      assert(it->second < node.idx);
      node.parent = it->second;
      auto& parent = func.exnNodes[node.parent];
      node.depth = parent.depth + 1;
      parent.children.emplace_back(node.idx);
    }
    func.exnNodes.emplace_back(std::move(node));
  }

  return ret;
}

template<class T> T decode(PC& pc) {
  auto const ret = *reinterpret_cast<const T*>(pc);
  pc += sizeof ret;
  return ret;
}

template<class T> void decode(PC& pc, T& val) {
  val = decode<T>(pc);
}

MKey make_mkey(const php::Func& /*func*/, MemberKey mk) {
  switch (mk.mcode) {
    case MEL: case MPL:
      return MKey{mk.mcode, mk.local};
    case MEC: case MPC:
      return MKey{mk.mcode, mk.iva};
    case MET: case MPT: case MQT:
      return MKey{mk.mcode, mk.litstr};
    case MEI:
      return MKey{mk.mcode, mk.int64};
    case MW:
      return MKey{};
  }
  not_reached();
}

template<class FindBlock>
void populate_block(ParseUnitState& puState,
                    const FuncEmitter& fe,
                    php::Func& func,
                    php::Block& blk,
                    PC pc,
                    PC const past,
                    FindBlock findBlock) {
  auto const& ue = fe.ue();

  auto decode_stringvec = [&] {
    auto const vecLen = decode_iva(pc);
    CompactVector<LSString> keys;
    for (auto i = size_t{0}; i < vecLen; ++i) {
      keys.push_back(ue.lookupLitstr(decode<int32_t>(pc)));
    }
    return keys;
  };

  auto decode_switch = [&] (PC opPC) {
    SwitchTab ret;
    auto const vecLen = decode_iva(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      ret.push_back(findBlock(
        opPC + decode<Offset>(pc) - ue.bc()
      ));
    }
    return ret;
  };

  auto decode_sswitch = [&] (PC opPC) {
    SSwitchTab ret;

    auto const vecLen = decode_iva(pc);
    for (int32_t i = 0; i < vecLen - 1; ++i) {
      auto const id = decode<Id>(pc);
      auto const offset = decode<Offset>(pc);
      ret.emplace_back(ue.lookupLitstr(id),
                       findBlock(opPC + offset - ue.bc()));
    }

    // Final case is the default, and must have a litstr id of -1.
    DEBUG_ONLY auto const defId = decode<Id>(pc);
    auto const defOff = decode<Offset>(pc);
    assert(defId == -1);
    ret.emplace_back(nullptr, findBlock(opPC + defOff - ue.bc()));
    return ret;
  };

  auto defcns = [&] () {
    puState.constPassFuncs.insert(&func);
  };
  auto defcls = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefCls.arg1] = &func;
  };
  auto defclsnop = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefClsNop.arg1] = &func;
  };
  auto createcl = [&] (const Bytecode& b) {
    puState.createClMap[b.CreateCl.arg2].insert(&func);
  };

#define IMM_BLA(n)     auto targets = decode_switch(opPC);
#define IMM_SLA(n)     auto targets = decode_sswitch(opPC);
#define IMM_IVA(n)     auto arg##n = decode_iva(pc);
#define IMM_I64A(n)    auto arg##n = decode<int64_t>(pc);
#define IMM_LA(n)      auto loc##n = [&] {                       \
                         LocalId id = decode_iva(pc);            \
                         always_assert(id < func.locals.size()); \
                         return id;                              \
                       }();
#define IMM_NLA(n)     auto nloc##n = [&] {                         \
                         NamedLocal loc = decode_named_local(pc);   \
                         always_assert(loc.id < func.locals.size());\
                         return loc;                                \
                       }();
#define IMM_ILA(n)     auto loc##n = [&] {                       \
                         LocalId id = decode_iva(pc);            \
                         always_assert(id < func.locals.size()); \
                         return id;                              \
                       }();
#define IMM_IA(n)      auto iter##n = [&] {                      \
                         IterId id = decode_iva(pc);             \
                         always_assert(id < func.numIters);      \
                         return id;                              \
                       }();
#define IMM_DA(n)      auto dbl##n = decode<double>(pc);
#define IMM_SA(n)      auto str##n = ue.lookupLitstr(decode<Id>(pc));
#define IMM_RATA(n)    auto rat = decodeRAT(ue, pc);
#define IMM_AA(n)      auto arr##n = ue.lookupArray(decode<Id>(pc));
#define IMM_BA(n)      assert(next == past);     \
                       auto target##n = findBlock(  \
                         opPC + decode<Offset>(pc) - ue.bc());
#define IMM_OA_IMPL(n) subop##n; decode(pc, subop##n);
#define IMM_OA(type)   type IMM_OA_IMPL
#define IMM_VSA(n)     auto keys = decode_stringvec();
#define IMM_KA(n)      auto mkey = make_mkey(func, decode_member_key(pc, &ue));
#define IMM_LAR(n)     auto locrange = [&] {                             \
                         auto const range = decodeLocalRange(pc);        \
                         always_assert(range.first + range.count         \
                                       <= func.locals.size());           \
                         return LocalRange { range.first, range.count }; \
                       }();
#define IMM_ITA(n)     auto ita = decodeIterArgs(pc);
#define IMM_FCA(n)     auto fca = [&] {                                      \
                         auto const fca = decodeFCallArgs(op, pc, &ue);      \
                         auto const numBytes = (fca.numArgs + 7) / 8;        \
                         auto inoutArgs = fca.enforceInOut()                 \
                           ? std::make_unique<uint8_t[]>(numBytes)           \
                           : nullptr;                                        \
                         if (inoutArgs) {                                    \
                           memcpy(inoutArgs.get(), fca.inoutArgs, numBytes); \
                         }                                                   \
                         auto const aeOffset = fca.asyncEagerOffset;         \
                         auto const aeTarget = aeOffset != kInvalidOffset    \
                           ? findBlock(opPC + aeOffset - ue.bc())            \
                           : NoBlockId;                                      \
                         assertx(aeTarget == NoBlockId || next == past);     \
                         return FCallArgs(fca.flags, fca.numArgs,            \
                                          fca.numRets, std::move(inoutArgs), \
                                          aeTarget, fca.lockWhileUnwinding,  \
                                          fca.skipNumArgsCheck, fca.context);\
                       }();

#define IMM_NA
#define IMM_ONE(x)                IMM_##x(1)
#define IMM_TWO(x, y)             IMM_##x(1) IMM_##y(2)
#define IMM_THREE(x, y, z)        IMM_TWO(x, y) IMM_##z(3)
#define IMM_FOUR(x, y, z, n)      IMM_THREE(x, y, z) IMM_##n(4)
#define IMM_FIVE(x, y, z, n, m)   IMM_FOUR(x, y, z, n) IMM_##m(5)
#define IMM_SIX(x, y, z, n, m, o) IMM_FIVE(x, y, z, n, m) IMM_##o(6)

#define IMM_ARG(which, n)         IMM_NAME_##which(n)
#define IMM_ARG_NA
#define IMM_ARG_ONE(x)                IMM_ARG(x, 1)
#define IMM_ARG_TWO(x, y)             IMM_ARG(x, 1), IMM_ARG(y, 2)
#define IMM_ARG_THREE(x, y, z)        IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                      IMM_ARG(z, 3)
#define IMM_ARG_FOUR(x, y, z, l)      IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                      IMM_ARG(z, 3), IMM_ARG(l, 4)
#define IMM_ARG_FIVE(x, y, z, l, m)   IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                      IMM_ARG(z, 3), IMM_ARG(l, 4), \
                                      IMM_ARG(m, 5)
#define IMM_ARG_SIX(x, y, z, l, m, n) IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                      IMM_ARG(z, 3), IMM_ARG(l, 4), \
                                      IMM_ARG(m, 5), IMM_ARG(n, 6)

#define FLAGS_NF
#define FLAGS_TF
#define FLAGS_CF
#define FLAGS_FF
#define FLAGS_CF_TF
#define FLAGS_CF_FF

#define FLAGS_ARG_NF
#define FLAGS_ARG_TF
#define FLAGS_ARG_CF
#define FLAGS_ARG_FF
#define FLAGS_ARG_CF_TF
#define FLAGS_ARG_CF_FF

#define O(opcode, imms, inputs, outputs, flags)                    \
  case Op::opcode:                                                 \
    {                                                              \
      auto b = [&] () -> Bytecode {                                \
        IMM_##imms /*these two macros advance the pc as required*/ \
        FLAGS_##flags                                              \
        if (isTypeAssert(op)) return bc::Nop {};                   \
        return bc::opcode { IMM_ARG_##imms FLAGS_ARG_##flags };    \
      }();                                                         \
      b.srcLoc = srcLocIx;                                         \
      if (Op::opcode == Op::DefCns)      defcns();                 \
      if (Op::opcode == Op::DefCls)      defcls(b);                \
      if (Op::opcode == Op::DefClsNop)   defclsnop(b);             \
      if (Op::opcode == Op::CreateCl)    createcl(b);              \
      blk.hhbcs.push_back(std::move(b));                           \
      assert(pc == next);                                          \
    }                                                              \
    break;

  assert(pc != past);
  do {
    auto const opPC = pc;
    auto const next = pc + instrLen(opPC);
    assert(next <= past);

    auto const srcLoc = match<php::SrcLoc>(
      puState.srcLocInfo,
      [&] (const SourceLocTable& tab) {
        SourceLoc sloc;
        if (getSourceLoc(tab, opPC - ue.bc(), sloc)) {
          return php::SrcLoc {
            { static_cast<uint32_t>(sloc.line0),
              static_cast<uint32_t>(sloc.char0) },
            { static_cast<uint32_t>(sloc.line1),
              static_cast<uint32_t>(sloc.char1) }
          };
        }
        return php::SrcLoc{};
      },
      [&] (const LineTable& tab) {
        auto const line = getLineNumber(tab, opPC - ue.bc());
        if (line != -1) {
          return php::SrcLoc {
            { static_cast<uint32_t>(line), 0 },
            { static_cast<uint32_t>(line), 0 },
          };
        };
        return php::SrcLoc{};
      }
    );

    auto const srcLocIx = puState.srcLocs.emplace(
      srcLoc, puState.srcLocs.size()).first->second;

    auto op = decode_op(pc);
    switch (op) { OPCODES }

    if (next == past) {
      if (instrAllowsFallThru(op)) {
        blk.fallthrough = findBlock(next - ue.bc());
      }
    }

    pc = next;
  } while (pc != past);

#undef O

#undef FLAGS_NF
#undef FLAGS_TF
#undef FLAGS_CF
#undef FLAGS_FF
#undef FLAGS_CF_TF
#undef FLAGS_CF_FF

#undef FLAGS_ARG_NF
#undef FLAGS_ARG_TF
#undef FLAGS_ARG_CF
#undef FLAGS_ARG_FF
#undef FLAGS_ARG_CF_TF
#undef FLAGS_ARG_CF_FF

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR
#undef IMM_FIVE

#undef IMM_ARG
#undef IMM_ARG_NA
#undef IMM_ARG_ONE
#undef IMM_ARG_TWO
#undef IMM_ARG_THREE
#undef IMM_ARG_FOUR
#undef IMM_ARG_FIVE
#undef IMM_ARG_SIX

  /*
   * If a block ends with an unconditional jump, change it to a
   * fallthrough edge.
   *
   * If the jmp is the only instruction, convert it to a Nop, to avoid
   * creating an empty block (we have an invariant that no blocks are
   * empty).
   */

  auto make_fallthrough = [&] {
    blk.fallthrough = blk.hhbcs.back().Jmp.target1;
    if (blk.hhbcs.size() == 1) {
      blk.hhbcs.back() = bc_with_loc(blk.hhbcs.back().srcLoc, bc::Nop{});
    } else {
      blk.hhbcs.pop_back();
    }
  };

  switch (blk.hhbcs.back().op) {
  case Op::Jmp:   make_fallthrough();                           break;
  case Op::JmpNS: make_fallthrough(); blk.fallthroughNS = true; break;
  default:                                                      break;
  }
}

template<class FindBlk>
void link_entry_points(php::Func& func,
                       const FuncEmitter& fe,
                       FindBlk findBlock) {
  func.dvEntries.resize(fe.params.size(), NoBlockId);
  for (size_t i = 0, sz = fe.params.size(); i < sz; ++i) {
    if (fe.params[i].hasDefaultValue()) {
      auto const dv = findBlock(fe.params[i].funcletOff);
      func.params[i].dvEntryPoint = dv;
      func.dvEntries[i] = dv;
    }
  }
  func.mainEntry = findBlock(fe.base);
}

void build_cfg(ParseUnitState& puState,
               php::Func& func,
               const FuncEmitter& fe) {
  auto const blockStarts = findBasicBlocks(fe);

  FTRACE(3, "    blocks are at: {}\n",
    [&]() -> std::string {
      using namespace folly::gen;
      return from(blockStarts)
        | eachTo<std::string>()
        | unsplit<std::string>(" ");
    }()
  );

  std::map<Offset,std::pair<BlockId, copy_ptr<php::Block>>> blockMap;
  auto const bc = fe.ue().bc();

  auto findBlock = [&] (Offset off, bool catchEntry = false) {
    auto& ent = blockMap[off];
    if (!ent.second) {
      auto blk         = php::Block{};
      ent.first        = blockMap.size() - 1;
      blk.exnNodeId    = NoExnNodeId;
      blk.catchEntry   = catchEntry;
      ent.second.emplace(std::move(blk));
    } else if (catchEntry) {
      ent.second.mutate()->catchEntry = true;
    }
    return ent.first;
  };

  auto exnTreeInfo = build_exn_tree(fe, func, findBlock);

  hphp_fast_map<BlockId, std::pair<int, int>> predSuccCounts;

  for (auto it = begin(blockStarts);
       std::next(it) != end(blockStarts);
       ++it) {
    auto const bid     = findBlock(*it);
    auto const block   = blockMap[*it].second.mutate();
    auto const bcStart = bc + *it;
    auto const bcStop  = bc + *std::next(it);

    if (auto const eh = Func::findEH(fe.ehtab, *it)) {
      auto it = exnTreeInfo.ehMap.find(eh);
      assert(it != end(exnTreeInfo.ehMap));
      block->exnNodeId = it->second;
      block->throwExit = func.exnNodes[it->second].region.catchEntry;
    }

    populate_block(puState, fe, func, *block, bcStart, bcStop, findBlock);
    forEachNonThrowSuccessor(*block, [&] (BlockId blkId) {
        predSuccCounts[blkId].first++;
        predSuccCounts[bid].second++;
    });
  }

  link_entry_points(func, fe, findBlock);

  func.blocks.resize(blockMap.size());
  for (auto& kv : blockMap) {
    auto const blk = kv.second.second.mutate();
    auto const id = kv.second.first;
    blk->multiSucc = predSuccCounts[id].second > 1;
    blk->multiPred = predSuccCounts[id].first > 1;
    func.blocks[id] = std::move(kv.second.second);
  }
}

void add_frame_variables(php::Func& func, const FuncEmitter& fe) {
  for (auto& param : fe.params) {
    func.params.push_back(
      php::Param {
        param.defaultValue,
        NoBlockId,
        param.typeConstraint,
        param.userType,
        param.upperBounds,
        param.phpCode,
        param.userAttributes,
        param.builtinType,
        param.isInOut(),
        param.isVariadic()
      }
    );
  }

  func.locals.reserve(fe.numLocals());
  for (LocalId id = 0; id < fe.numLocals(); ++id) {
    func.locals.push_back({
      .name = nullptr,
      .id = id,
      .killed = false,
      .nameId = id,
      .unusedName = false
    });
  }
  for (auto& kv : fe.localNameMap()) {
    func.locals[kv.second].name = kv.first;
  }

  func.numIters = fe.numIterators();
}

std::unique_ptr<php::Func> parse_func(ParseUnitState& puState,
                                      php::Unit* unit,
                                      php::Class* cls,
                                      const FuncEmitter& fe) {
  FTRACE(2, "  func: {}\n",
    fe.name->data() && *fe.name->data() ? fe.name->data() : "pseudomain");

  auto ret         = std::make_unique<php::Func>();
  ret->idx         = puState.nextFuncId.fetch_add(1, std::memory_order_relaxed);
  ret->name        = fe.name;
  ret->srcInfo     = php::SrcInfo { fe.getLocation(),
                                    fe.docComment };
  ret->unit        = unit;
  ret->cls         = cls;

  ret->attrs              = static_cast<Attr>(fe.attrs & ~AttrNoOverride);
  ret->userAttributes     = fe.userAttributes;
  ret->returnUserType     = fe.retUserType;
  ret->retTypeConstraint  = fe.retTypeConstraint;
  ret->hasParamsWithMultiUBs = fe.hasParamsWithMultiUBs;
  ret->hasReturnWithMultiUBs = fe.hasReturnWithMultiUBs;
  ret->returnUBs          = fe.retUpperBounds;
  ret->originalFilename   = fe.originalFilename;

  ret->top                 = fe.top;
  ret->isClosureBody       = fe.isClosureBody;
  ret->isAsync             = fe.isAsync;
  ret->isGenerator         = fe.isGenerator;
  ret->isPairGenerator     = fe.isPairGenerator;
  ret->isMemoizeWrapper    = fe.isMemoizeWrapper;
  ret->isMemoizeWrapperLSB = fe.isMemoizeWrapperLSB;
  ret->isMemoizeImpl       = Func::isMemoizeImplName(fe.name);
  ret->isReified           = fe.userAttributes.find(s___Reified.get()) !=
                             fe.userAttributes.end();
  ret->isRxDisabled        = fe.isRxDisabled;
  ret->noContextSensitiveAnalysis = fe.userAttributes.find(
    s___NoContextSensitiveAnalysis.get()) != fe.userAttributes.end();
  ret->hasInOutArgs        = [&] {
    for (auto& a : fe.params) if (a.isInOut()) return true;
    return false;
  }();

  add_frame_variables(*ret, fe);

  if (!RuntimeOption::ConstantFunctions.empty()) {
    auto const name = [&] {
      if (!cls) return fe.name->toCppString();
      return folly::sformat("{}::{}", cls->name, ret->name);
    }();
    auto const it = RuntimeOption::ConstantFunctions.find(name);
    if (it != RuntimeOption::ConstantFunctions.end()) {
      ret->locals.resize(fe.params.size());
      ret->numIters = 0;
      ret->attrs |= AttrIsFoldable;

      auto const mainEntry = BlockId{0};

      auto blk         = php::Block{};
      blk.exnNodeId    = NoExnNodeId;

      blk.hhbcs.push_back(gen_constant(it->second));
      blk.hhbcs.push_back(bc::RetC {});
      ret->blocks.emplace_back(std::move(blk));

      ret->dvEntries.resize(fe.params.size(), NoBlockId);
      ret->mainEntry = mainEntry;

      for (size_t i = 0, sz = fe.params.size(); i < sz; ++i) {
        if (fe.params[i].hasDefaultValue()) {
          ret->params[i].dvEntryPoint = mainEntry;
          ret->dvEntries[i] = mainEntry;
        }
      }
      return ret;
    }
  }

  /*
   * Builtin functions get some extra information.  The returnType flag is only
   * non-folly::none for these, but note that something may be a builtin and
   * still have a folly::none return type.
   */
  if (fe.isNative) {
    auto const f = [&] () -> HPHP::Func* {
      if (ret->cls) {
        auto const cls = Unit::lookupClass(ret->cls->name);
        return cls ? cls->lookupMethod(ret->name) : nullptr;
      } else {
        return Unit::lookupBuiltin(ret->name);
      }
    }();

    ret->nativeInfo                   = std::make_unique<php::NativeInfo>();
    ret->nativeInfo->returnType       = fe.hniReturnType;
    if (f && ret->params.size()) {
      for (auto i = 0; i < ret->params.size(); i++) {
        auto& pi = ret->params[i];
        if (pi.isVariadic || !f->params()[i].hasDefaultValue()) continue;
        if (pi.defaultValue.m_type == KindOfUninit &&
            pi.phpCode != nullptr) {
          auto res = eval_cell_value([&] {
              auto val = f_constant(StrNR(pi.phpCode));
              val.setEvalScalar();
              return *val.asTypedValue();
            });
          if (!res) {
            FTRACE(4, "Argument {} to {}: Failed to evaluate {}\n",
                   i, f->fullName(), pi.phpCode);
            continue;
          }
          pi.defaultValue = *res;
        }
      }
    }
    if (!f || !f->nativeFuncPtr() ||
        (f->userAttributes().count(
          LowStringPtr(s_attr_Deprecated.get())))) {
      ret->attrs |= AttrNoFCallBuiltin;
    }
  }

  build_cfg(puState, *ret, fe);

  return ret;
}

void parse_methods(ParseUnitState& puState,
                   php::Class* ret,
                   php::Unit* unit,
                   const PreClassEmitter& pce) {
  std::unique_ptr<php::Func> cinit;
  for (auto& me : pce.methods()) {
    auto f = parse_func(puState, unit, ret, *me);
    if (f->name == s_86cinit.get()) {
      puState.constPassFuncs.insert(f.get());
      cinit = std::move(f);
    } else {
      if (f->name == s_86pinit.get() ||
          f->name == s_86sinit.get() ||
          f->name == s_86linit.get()) {
        puState.constPassFuncs.insert(f.get());
      }
      ret->methods.push_back(std::move(f));
    }
  }
  if (cinit) ret->methods.push_back(std::move(cinit));
}

void add_stringish(php::Class* cls) {
  // The runtime adds Stringish to any class providing a __toString() function,
  // so we mirror that here to make sure analysis of interfaces is correct.
  // All Stringish are also XHPChild, so handle it here as well.
  if (cls->attrs & AttrInterface && cls->name->isame(s_Stringish.get())) {
    return;
  }

  bool hasXHP = false;
  for (auto& iface : cls->interfaceNames) {
    if (iface->isame(s_Stringish.get())) return;
    if (iface->isame(s_XHPChild.get())) { hasXHP = true; }
  }

  for (auto& func : cls->methods) {
    if (func->name->isame(s_toString.get())) {
      FTRACE(2, "Adding Stringish and XHPChild to {}\n", cls->name->data());
      cls->interfaceNames.push_back(s_Stringish.get());
      if (!hasXHP && !cls->name->isame(s_XHPChild.get())) {
        cls->interfaceNames.push_back(s_XHPChild.get());
      }
      return;
    }
  }
}

std::unique_ptr<php::Record> parse_record(php::Unit* unit,
                                          const RecordEmitter& re) {
  FTRACE(2, "  record: {}\n", re.name()->data());

  auto ret                = std::make_unique<php::Record>();
  ret->unit               = unit;
  ret->srcInfo            = php::SrcInfo {re.getLocation(), re.docComment()};
  ret->name               = re.name();
  ret->attrs              = static_cast<Attr>(re.attrs() & ~AttrNoOverride);
  ret->parentName         = re.parentName()->empty()? nullptr: re.parentName();
  ret->id                 = re.id();
  ret->userAttributes     = re.userAttributes();

  auto& fieldMap = re.fieldMap();
  for (size_t idx = 0; idx < fieldMap.size(); ++idx) {
    auto& field = fieldMap[idx];
    ret->fields.push_back(
      php::RecordField {
        field.name(),
        field.attrs(),
        field.userType(),
        field.docComment(),
        field.val(),
        field.typeConstraint(),
        field.userAttributes()
      }
    );
  }
  return ret;
}

std::unique_ptr<php::Class> parse_class(ParseUnitState& puState,
                                        php::Unit* unit,
                                        const PreClassEmitter& pce) {
  FTRACE(2, "  class: {}\n", pce.name()->data());

  auto ret                = std::make_unique<php::Class>();
  ret->name               = pce.name();
  ret->srcInfo            = php::SrcInfo { pce.getLocation(),
                                           pce.docComment() };
  ret->unit               = unit;
  ret->closureContextCls  = nullptr;
  ret->parentName         = pce.parentName()->empty() ? nullptr
                                                      : pce.parentName();
  ret->attrs              = static_cast<Attr>(pce.attrs() & ~AttrNoOverride);
  ret->hoistability       = pce.hoistability();
  ret->userAttributes     = pce.userAttributes();
  ret->id                 = pce.id();
  ret->hasReifiedGenerics = ret->userAttributes.find(s___Reified.get()) !=
                            ret->userAttributes.end();
  ret->hasConstProp       = false;

  for (auto& iface : pce.interfaces()) {
    ret->interfaceNames.push_back(iface);
  }

  copy(ret->usedTraitNames,  pce.usedTraits());
  copy(ret->traitPrecRules,  pce.traitPrecRules());
  copy(ret->traitAliasRules, pce.traitAliasRules());
  copy(ret->requirements,    pce.requirements());

  parse_methods(puState, ret.get(), unit, pce);
  add_stringish(ret.get());

  auto& propMap = pce.propMap();
  for (size_t idx = 0; idx < propMap.size(); ++idx) {
    auto& prop = propMap[idx];
    ret->properties.push_back(
      php::Prop {
        prop.name(),
        prop.attrs(),
        prop.userAttributes(),
        prop.docComment(),
        prop.userType(),
        prop.typeConstraint(),
        prop.val()
      }
    );
    if ((prop.attrs() & (AttrStatic | AttrIsConst)) == AttrIsConst) {
      ret->hasConstProp = true;
    }
  }

  auto& constMap = pce.constMap();
  for (size_t idx = 0; idx < constMap.size(); ++idx) {
    auto& cconst = constMap[idx];
    // Set all constants as NoOverride, we'll clear this while building
    // the index
    ret->constants.push_back(
      php::Const {
        cconst.name(),
        ret.get(),
        cconst.valOption(),
        cconst.phpCode(),
        cconst.typeConstraint(),
        cconst.isTypeconst(),
        true // NoOverride
      }
    );
  }

  if (ret->attrs & AttrBuiltin) {
    if (auto nativeConsts = Native::getClassConstants(ret->name)) {
      for (auto const& cnsMap : *nativeConsts) {
        TypedValueAux tvaux;
        tvCopy(cnsMap.second, tvaux);
        tvaux.constModifiers() = {};
        ret->constants.push_back(
          php::Const {
            cnsMap.first,
            ret.get(),
            tvaux,
            staticEmptyString(),
            staticEmptyString(),
            false
          }
        );
      }
    }
  }

  ret->enumBaseTy = pce.enumBaseTy();

  return ret;
}

//////////////////////////////////////////////////////////////////////

void assign_closure_context(const ParseUnitState&, php::Class*);

php::Class*
find_closure_context(const ParseUnitState& puState,
                     php::Func* createClFunc) {
  if (auto const cls = createClFunc->cls) {
    if (cls->parentName &&
        cls->parentName->isame(s_Closure.get())) {
      // We have a closure created by a closure's invoke method, which
      // means it should inherit the outer closure's context, so we
      // have to know that first.
      assign_closure_context(puState, cls);
      return cls->closureContextCls;
    }
    return cls;
  }
  return nullptr;
}

void assign_closure_context(const ParseUnitState& puState,
                            php::Class* clo) {
  if (clo->closureContextCls) return;

  auto clIt = puState.createClMap.find(clo->id);
  if (clIt == end(puState.createClMap)) {
    // Unused closure class.  Technically not prohibited by the spec.
    return;
  }

  /*
   * Any route to the closure context must yield the same class, or
   * things downstream won't understand.  We try every route and
   * assert they are all the same here.
   *
   * See bytecode.specification for CreateCl for the relevant
   * invariants.
   */
  always_assert(!clIt->second.empty());
  auto it = begin(clIt->second);
  auto const representative = find_closure_context(puState, *it);
  if (debug) {
    for (++it; it != end(clIt->second); ++it) {
      assert(find_closure_context(puState, *it) == representative);
    }
  }
  clo->closureContextCls = representative;
}

void find_additional_metadata(const ParseUnitState& puState,
                              php::Unit* unit) {
  for (auto& c : unit->classes) {
    if (!c->parentName || !c->parentName->isame(s_Closure.get())) {
      continue;
    }
    assign_closure_context(puState, c.get());
  }
}

//////////////////////////////////////////////////////////////////////

}

void parse_unit(php::Program& prog, const UnitEmitter* uep) {
  Trace::Bump bumper{Trace::hhbbc_parse, kSystemLibBump, uep->isASystemLib()};
  FTRACE(2, "parse_unit {}\n", uep->m_filepath->data());

  if (RuntimeOption::EvalAbortBuildOnVerifyError && !uep->check(false)) {
    fprintf(
      stderr,
      "The unoptimized unit for %s did not pass verification, "
      "bailing because Eval.AbortBuildOnVerifyError is set\n",
      uep->m_filepath->data()
    );
    _Exit(1);
  }

  auto const& ue = *uep;

  auto ret      = std::make_unique<php::Unit>();
  ret->filename = ue.m_filepath;
  ret->isHHFile = ue.m_isHHFile;
  ret->metaData = ue.m_metaData;
  ret->fileAttributes = ue.m_fileAttributes;

  ParseUnitState puState{ prog.nextFuncId };
  if (ue.hasSourceLocInfo()) {
    puState.srcLocInfo = ue.createSourceLocTable();
  } else {
    puState.srcLocInfo = ue.lineTable();
  }
  puState.defClsMap.resize(ue.numPreClasses(), nullptr);

  for (size_t i = 0; i < ue.numPreClasses(); ++i) {
    auto cls = parse_class(puState, ret.get(), *ue.pce(i));
    ret->classes.push_back(std::move(cls));
  }

  for (size_t i = 0; i < ue.numRecords(); ++i) {
    auto rec = parse_record(ret.get(), *ue.re(i));
    ret->records.push_back(std::move(rec));
  }

  for (auto& fe : ue.fevec()) {
    auto func = parse_func(puState, ret.get(), nullptr, *fe);
    assert(!fe->pce());
    if (fe->isPseudoMain()) {
      ret->pseudomain = std::move(func);
    } else {
      ret->funcs.push_back(std::move(func));
    }
  }

  ret->srcLocs.resize(puState.srcLocs.size());
  for (auto& srcInfo : puState.srcLocs) {
    ret->srcLocs[srcInfo.second] = srcInfo.first;
  }

  for (auto& ta : ue.typeAliases()) {
    ret->typeAliases.push_back(
      std::make_unique<php::TypeAlias>(ta)
    );
  }

  for (auto& c : ue.constants()) {
    ret->constants.push_back(
      std::make_unique<Constant>(c)
    );
  }

  find_additional_metadata(puState, ret.get());

  assert(check(*ret));

  std::lock_guard<std::mutex> _{prog.lock};
  for (auto const item : puState.constPassFuncs) {
    prog.constInits.push_back(item);
  }
  ret->sha1 = SHA1 { prog.units.size() };
  prog.units.push_back(std::move(ret));
}

//////////////////////////////////////////////////////////////////////

}}
