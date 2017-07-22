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
#include <mutex>
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
#include <folly/ScopeGuard.h>
#include <folly/Memory.h>

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
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Closure("Closure");
const StaticString s_toString("__toString");
const StaticString s_Stringish("Stringish");
const StaticString s_86cinit("86cinit");
const StaticString s_86sinit("86sinit");
const StaticString s_86pinit("86pinit");
const StaticString s_attr_Deprecated("__Deprecated");

//////////////////////////////////////////////////////////////////////

void record_const_init(php::Program& prog, uintptr_t encoded_func) {
  auto id = prog.nextConstInit.fetch_add(1, std::memory_order_relaxed);
  prog.constInits.ensureSize(id + 1);

  DEBUG_ONLY auto const oldVal = prog.constInits.exchange(id, encoded_func);
  assert(oldVal == 0);
}

//////////////////////////////////////////////////////////////////////

struct ParseUnitState {
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
  std::vector<borrowed_ptr<php::Func>> defClsMap;

  /*
   * Map from Closure index to the function(s) containing their
   * associated CreateCl opcode(s).
   */
  std::unordered_map<
    int32_t,
    std::unordered_set<borrowed_ptr<php::Func>>
  > createClMap;

  struct SrcLocHash {
    size_t operator()(const php::SrcLoc& sl) const {
      auto const h1 = ((size_t)sl.start.col << 32) | sl.start.line;
      auto const h2 = ((size_t)sl.past.col << 32) | sl.past.line;
      return hash_int64_pair(h1, h2);
    }
  };
  std::unordered_map<php::SrcLoc, int32_t, SrcLocHash> srcLocs;

  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * Must include every function with a DefCns for correctness; cinit,
   * pinit and sinit functions are added to improve overall
   * performance.
   */
  std::unordered_map<borrowed_ptr<php::Func>, int> constPassFuncs;

  /*
   * List of class aliases defined by this unit
   */
  CompactVector<std::pair<SString,SString>> classAliases;
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
    auto const breaksBB = instrIsNonCallControlFlow(op) || instrFlags(op) & TF;

    if (options.TraceBytecodes.count(op)) traceBc = true;

    if (breaksBB && !atLast) {
      markBlock(nextOff);
    }

    if (isSwitch(op)) {
      foreachSwitchTarget(pc, [&] (Offset delta) {
        markBlock(offset + delta);
      });
    } else {
      auto const target = instrJumpTarget(bc, offset);
      if (target != InvalidAbsoluteOffset) markBlock(target);
    }

    offset = nextOff;
    if (atLast) break;
  }

  /*
   * Find blocks associated with exception handlers.
   *
   *   - The start of each fault-protected region begins a block.
   *
   *   - The instruction immediately after the end of any
   *     fault-protected region begins a block.
   *
   *   - Each fault or catch entry point begins a block.
   *
   *   - The instruction immediately after the end of any
   *     fault or catch region begins a block.
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
  std::map<const EHEntEmitter*,borrowed_ptr<php::ExnNode>> ehMap;

  /*
   * Fault funclets don't actually fall in the EHEnt region for all of
   * their parent handlers in HHBC.  There may be EHEnt regions
   * covering the fault funclet, but if an exception occurs in the
   * funclet it can also propagate to any EH region from the code that
   * entered the funclet.  We want factored exit edges from the fault
   * funclets to any of these enclosing catch blocks (or other
   * enclosing funclet blocks).
   *
   * Moreover, funclet offsets can be entered from multiple protected
   * regions, so we need to keep a map of all the possible regions
   * that could have entered a given funclet, so we can add exit edges
   * to all their parent EHEnt handlers.
   */
  std::map<borrowed_ptr<php::Block>,std::vector<borrowed_ptr<php::ExnNode>>>
    funcletNodes;

  /*
   * Keep track of the start offsets for all fault funclets.  This is
   * used to find the extents of each handler for find_fault_funclets.
   * It is assumed that each fault funclet handler extends from its
   * entry offset until the next fault funclet entry offset (or end of
   * the function).
   *
   * This relies on the following bytecode invariants:
   *
   *   - All fault funclets come after the primary function body.
   *
   *   - Each fault funclet is a contiguous region of bytecode that
   *     does not jump into other fault funclets or into the primary
   *     function body.
   *
   *   - Nothing comes after the fault funclets.
   */
  std::set<Offset> faultFuncletStarts;
};

template<class FindBlock>
ExnTreeInfo build_exn_tree(const FuncEmitter& fe,
                           php::Func& func,
                           FindBlock findBlock) {
  ExnTreeInfo ret;
  auto nextExnNode = uint32_t{0};

  for (auto& eh : fe.ehtab) {
    auto node = std::make_unique<php::ExnNode>();
    node->id = nextExnNode++;
    node->parent = nullptr;
    node->depth = 1; // 0 depth means no ExnNode

    switch (eh.m_type) {
    case EHEnt::Type::Fault:
      {
        auto const fault = findBlock(eh.m_handler);
        ret.funcletNodes[fault].push_back(borrow(node));
        ret.faultFuncletStarts.insert(eh.m_handler);
        node->info = php::FaultRegion { fault->id, eh.m_iterId, eh.m_itRef };
      }
      break;
    case EHEnt::Type::Catch:
      {
        auto const catchBlk = findBlock(eh.m_handler);
        node->info = php::CatchRegion { catchBlk->id, eh.m_iterId, eh.m_itRef };
      }
      break;
    }

    ret.ehMap[&eh] = borrow(node);

    if (eh.m_parentIndex != -1) {
      auto it = ret.ehMap.find(&fe.ehtab[eh.m_parentIndex]);
      assert(it != end(ret.ehMap));
      node->parent = it->second;
      node->depth = node->parent->depth + 1;
      it->second->children.emplace_back(std::move(node));
    } else {
      func.exnNodes.emplace_back(std::move(node));
    }
  }

  ret.faultFuncletStarts.insert(fe.past);

  return ret;
}

/*
 * Instead of breaking blocks on instructions that could throw, we
 * represent the control flow edges for exception paths as a factored
 * edge at the end of each block.
 *
 * When we initially add them here, no attempt is made to determine if
 * the edge is actually possible to traverse.
 */
void add_factored_exits(php::Block& blk,
                        borrowed_ptr<const php::ExnNode> node) {
  if (!node) return;

  match<void>(
    node->info,
    [&] (const php::CatchRegion& cr) {
      blk.factoredExits.push_back(cr.catchEntry);
    },
    [&] (const php::FaultRegion& fr) {
      blk.factoredExits.push_back(fr.faultEntry);
    }
  );
}

/*
 * Locate all the basic blocks associated with fault funclets, and
 * mark them as such.  Also, add factored exit edges for exceptional
 * control flow through any parent protected regions of the region(s)
 * that pointed at each fault handler.
 */
template <class BlockStarts, class FindBlock>
void find_fault_funclets(ExnTreeInfo& tinfo, const php::Func& /*func*/,
                         const BlockStarts& blockStarts, FindBlock findBlock) {
  auto sectionId = uint32_t{1};

  for (auto funcletStartIt = begin(tinfo.faultFuncletStarts);
      std::next(funcletStartIt) != end(tinfo.faultFuncletStarts);
      ++funcletStartIt, ++sectionId) {
    auto const nextFunclet = *std::next(funcletStartIt);

    auto offIt = blockStarts.find(*funcletStartIt);
    assert(offIt != end(blockStarts));

    auto const firstBlk  = findBlock(*offIt);
    auto const funcletIt = tinfo.funcletNodes.find(firstBlk);
    assert(funcletIt != end(tinfo.funcletNodes));
    assert(!funcletIt->second.empty());

    do {
      auto const blk = findBlock(*offIt);
      blk->section   = static_cast<php::Block::Section>(sectionId);

      // Propagate the exit edges to the containing fault/try handlers,
      // if there were any.
      for (auto& node : funcletIt->second) {
        add_factored_exits(*blk, node->parent);
      }

      // Fault funclets can have protected regions which may point to
      // handlers that are also listed in parents of the EH-region that
      // targets the funclet.  This means we might have duplicate
      // factored exits now, so we need to remove them.
      std::sort(begin(blk->factoredExits), end(blk->factoredExits));
      blk->factoredExits.resize(
        std::unique(begin(blk->factoredExits), end(blk->factoredExits)) -
        begin(blk->factoredExits)
      );

      ++offIt;
    } while (offIt != end(blockStarts) && *offIt < nextFunclet);
  }
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
      return MKey{mk.mcode, static_cast<LocalId>(mk.iva)};
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
    auto const vecLen = decode<int32_t>(pc);
    CompactVector<LSString> keys;
    for (auto i = size_t{0}; i < vecLen; ++i) {
      keys.push_back(ue.lookupLitstr(decode<int32_t>(pc)));
    }
    return keys;
  };

  auto decode_switch = [&] (PC opPC) {
    SwitchTab ret;
    auto const vecLen = decode<int32_t>(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      ret.push_back(findBlock(
        opPC + decode<Offset>(pc) - ue.bc()
      )->id);
    }
    return ret;
  };

  auto decode_sswitch = [&] (PC opPC) {
    SSwitchTab ret;

    auto const vecLen = decode<int32_t>(pc);
    for (int32_t i = 0; i < vecLen - 1; ++i) {
      auto const id = decode<Id>(pc);
      auto const offset = decode<Offset>(pc);
      ret.emplace_back(ue.lookupLitstr(id),
                       findBlock(opPC + offset - ue.bc())->id);
    }

    // Final case is the default, and must have a litstr id of -1.
    DEBUG_ONLY auto const defId = decode<Id>(pc);
    auto const defOff = decode<Offset>(pc);
    assert(defId == -1);
    ret.emplace_back(nullptr, findBlock(opPC + defOff - ue.bc())->id);
    return ret;
  };

  auto decode_itertab = [&] {
    IterTab ret;
    auto const vecLen = decode<int32_t>(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      auto const kind = static_cast<IterKind>(decode<int32_t>(pc));
      auto const id = decode<int32_t>(pc);
      ret.emplace_back(kind, id);
    }
    return ret;
  };

  auto defcns = [&] () {
    puState.constPassFuncs[&func] |= php::Program::ForAnalyze;
  };
  auto addelem = [&] () {
    puState.constPassFuncs[&func] |= php::Program::ForOptimize;
  };
  auto defcls = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefCls.arg1] = &func;
  };
  auto defclsnop = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefClsNop.arg1] = &func;
  };
  auto aliascls = [&] (const Bytecode& b) {
    puState.classAliases.emplace_back(b.AliasCls.str1, b.AliasCls.str2);
  };
  auto createcl = [&] (const Bytecode& b) {
    puState.createClMap[b.CreateCl.arg2].insert(&func);
  };
  auto has_call_unpack = [&] {
    auto const fpi = Func::findFPI(&*fe.fpitab.begin(),
                                   &*fe.fpitab.end(), pc - ue.bc());
    auto const op = peek_op(ue.bc() + fpi->m_fpiEndOff);
    return op == OpFCallArray || op == OpFCallUnpack;
  };

#define IMM_BLA(n)     auto targets = decode_switch(opPC);
#define IMM_SLA(n)     auto targets = decode_sswitch(opPC);
#define IMM_ILA(n)     auto iterTab = decode_itertab();
#define IMM_IVA(n)     auto arg##n = decode_iva(pc);
#define IMM_I64A(n)    auto arg##n = decode<int64_t>(pc);
#define IMM_LA(n)      auto loc##n = [&] {                       \
                         LocalId id = decode_iva(pc);            \
                         always_assert(id < func.locals.size()); \
                         return id;                              \
                       }();
#define IMM_IA(n)      auto iter##n = [&] {                      \
                         IterId id = decode_iva(pc);             \
                         always_assert(id < func.numIters);      \
                         return id;                              \
                       }();
#define IMM_CAR(n)     auto slot = [&] {                                \
                         ClsRefSlotId id = decode_iva(pc);              \
                         always_assert(id >= 0 && id < func.numClsRefSlots); \
                         return id;                                     \
                       }();
#define IMM_CAW(n)     auto slot = [&] {                                \
                         ClsRefSlotId id = decode_iva(pc);              \
                         always_assert(id >= 0 && id < func.numClsRefSlots); \
                         return id;                                     \
                       }();
#define IMM_DA(n)      auto dbl##n = decode<double>(pc);
#define IMM_SA(n)      auto str##n = ue.lookupLitstr(decode<Id>(pc));
#define IMM_RATA(n)    auto rat = decodeRAT(ue, pc);
#define IMM_AA(n)      auto arr##n = ue.lookupArray(decode<Id>(pc));
#define IMM_BA(n)      assert(next == past);     \
                       auto target = findBlock(  \
                         opPC + decode<Offset>(pc) - ue.bc())->id;
#define IMM_OA_IMPL(n) subop##n; decode(pc, subop##n);
#define IMM_OA(type)   type IMM_OA_IMPL
#define IMM_VSA(n)     auto keys = decode_stringvec();
#define IMM_KA(n)      auto mkey = make_mkey(func, decode_member_key(pc, &ue));
#define IMM_LAR(n)     auto locrange = [&] {                                 \
                         auto const range = decodeLocalRange(pc);            \
                         always_assert(range.first + range.restCount         \
                                       < func.locals.size());                \
                         return LocalRange { range.first, range.restCount }; \
                       }();

#define IMM_NA
#define IMM_ONE(x)           IMM_##x(1)
#define IMM_TWO(x, y)        IMM_##x(1)          IMM_##y(2)
#define IMM_THREE(x, y, z)   IMM_TWO(x, y)       IMM_##z(3)
#define IMM_FOUR(x, y, z, n) IMM_THREE(x, y, z)  IMM_##n(4)

#define IMM_ARG(which, n)         IMM_NAME_##which(n)
#define IMM_ARG_NA
#define IMM_ARG_ONE(x)            IMM_ARG(x, 1)
#define IMM_ARG_TWO(x, y)         IMM_ARG(x, 1), IMM_ARG(y, 2)
#define IMM_ARG_THREE(x, y, z)    IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                    IMM_ARG(z, 3)
#define IMM_ARG_FOUR(x, y, z, l)  IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                   IMM_ARG(z, 3), IMM_ARG(l, 4)

#define FLAGS_NF
#define FLAGS_TF
#define FLAGS_CF
#define FLAGS_FF
#define FLAGS_PF auto hu = has_call_unpack();
#define FLAGS_CF_TF
#define FLAGS_CF_FF

#define FLAGS_ARG_NF
#define FLAGS_ARG_TF
#define FLAGS_ARG_CF
#define FLAGS_ARG_FF
#define FLAGS_ARG_PF ,hu
#define FLAGS_ARG_CF_TF
#define FLAGS_ARG_CF_FF

#define O(opcode, imms, inputs, outputs, flags)                    \
  case Op::opcode:                                                 \
    {                                                              \
      auto b = [&] () -> Bytecode {                                \
        IMM_##imms /*these two macros advance the pc as required*/ \
        FLAGS_##flags                                              \
        if (isTypeAssert(op)) bc::Nop {};                          \
        return bc::opcode { IMM_ARG_##imms FLAGS_ARG_##flags };    \
      }();                                                         \
      b.srcLoc = srcLocIx;                                         \
      if (Op::opcode == Op::DefCns) defcns();                      \
      if (Op::opcode == Op::AddElemC ||                            \
          Op::opcode == Op::AddNewElemC) addelem();                \
      if (Op::opcode == Op::DefCls)    defcls(b);                  \
      if (Op::opcode == Op::DefClsNop) defclsnop(b);               \
      if (Op::opcode == Op::AliasCls) aliascls(b);                 \
      if (Op::opcode == Op::CreateCl)  createcl(b);                \
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

    auto const op = decode_op(pc);
    switch (op) { OPCODES }

    if (next == past) {
      if (instrAllowsFallThru(op)) {
        blk.fallthrough = findBlock(next - ue.bc())->id;
      }
    }

    pc = next;
  } while (pc != past);

#undef O

#undef FLAGS_NF
#undef FLAGS_TF
#undef FLAGS_CF
#undef FLAGS_FF
#undef FLAGS_PF
#undef FLAGS_CF_TF
#undef FLAGS_CF_FF

#undef FLAGS_ARG_NF
#undef FLAGS_ARG_TF
#undef FLAGS_ARG_CF
#undef FLAGS_ARG_FF
#undef FLAGS_ARG_PF
#undef FLAGS_ARG_CF_TF
#undef FLAGS_ARG_CF_FF

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA
#undef IMM_LAR

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR

#undef IMM_ARG
#undef IMM_ARG_NA
#undef IMM_ARG_ONE
#undef IMM_ARG_TWO
#undef IMM_ARG_THREE
#undef IMM_ARG_FOUR

  /*
   * If a block ends with an unconditional jump, change it to a
   * fallthrough edge.
   *
   * Just convert the opcode to a Nop, because this could create an
   * empty block and we have an invariant that no blocks are empty.
   */

  auto make_fallthrough = [&] {
    blk.fallthrough = blk.hhbcs.back().Jmp.target;
    blk.hhbcs.back() = bc_with_loc(blk.hhbcs.back().srcLoc, bc::Nop{});
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
      auto const dv = findBlock(fe.params[i].funcletOff)->id;
      func.params[i].dvEntryPoint = dv;
      func.dvEntries[i] = dv;
    }
  }
  func.mainEntry = findBlock(fe.base)->id;
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

  std::map<Offset,std::unique_ptr<php::Block>> blockMap;
  auto const bc = fe.ue().bc();

  auto findBlock = [&] (Offset off) {
    auto& ptr = blockMap[off];
    if (!ptr) {
      ptr               = std::make_unique<php::Block>();
      ptr->id           = blockMap.size() - 1;
      ptr->section      = php::Block::Section::Main;
      ptr->exnNode      = nullptr;
    }
    return borrow(ptr);
  };

  auto exnTreeInfo = build_exn_tree(fe, func, findBlock);

  for (auto it = begin(blockStarts);
       std::next(it) != end(blockStarts);
       ++it) {
    auto const block   = findBlock(*it);
    auto const bcStart = bc + *it;
    auto const bcStop  = bc + *std::next(it);

    if (auto const eh = Func::findEH(fe.ehtab, *it)) {
      auto it = exnTreeInfo.ehMap.find(eh);
      assert(it != end(exnTreeInfo.ehMap));
      block->exnNode = it->second;
      add_factored_exits(*block, block->exnNode);
    }

    populate_block(puState, fe, func, *block, bcStart, bcStop, findBlock);
  }

  link_entry_points(func, fe, findBlock);
  find_fault_funclets(exnTreeInfo, func, blockStarts, findBlock);

  func.blocks.resize(blockMap.size());
  for (auto& kv : blockMap) {
    auto const id = kv.second->id;
    func.blocks[id] = std::move(kv.second);
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
        param.phpCode,
        param.userAttributes,
        param.builtinType,
        param.byRef,
        param.byRef,
        param.variadic
      }
    );
  }

  func.locals.reserve(fe.numLocals());
  for (LocalId id = 0; id < fe.numLocals(); ++id) {
    func.locals.push_back({nullptr, id, false});
  }
  for (auto& kv : fe.localNameMap()) {
    func.locals[kv.second].name = kv.first;
  }

  func.numIters = fe.numIterators();
  func.numClsRefSlots = fe.numClsRefSlots();

  func.staticLocals.reserve(fe.staticVars.size());
  for (auto& sv : fe.staticVars) {
    func.staticLocals.push_back(
      php::StaticLocalInfo { sv.name }
    );
  }
}

std::unique_ptr<php::Func> parse_func(ParseUnitState& puState,
                                      borrowed_ptr<php::Unit> unit,
                                      borrowed_ptr<php::Class> cls,
                                      const FuncEmitter& fe) {
  FTRACE(2, "  func: {}\n",
    fe.name->data() && *fe.name->data() ? fe.name->data() : "pseudomain");

  auto ret             = std::make_unique<php::Func>();
  ret->name            = fe.name;
  ret->srcInfo         = php::SrcInfo { fe.getLocation(),
                                        fe.docComment };
  ret->unit            = unit;
  ret->cls             = cls;

  ret->attrs              = static_cast<Attr>(fe.attrs & ~AttrNoOverride);
  ret->userAttributes     = fe.userAttributes;
  ret->returnUserType     = fe.retUserType;
  ret->retTypeConstraint  = fe.retTypeConstraint;
  ret->originalFilename   = fe.originalFilename;

  ret->top                = fe.top;
  ret->isClosureBody      = fe.isClosureBody;
  ret->isAsync            = fe.isAsync;
  ret->isGenerator        = fe.isGenerator;
  ret->isPairGenerator    = fe.isPairGenerator;
  ret->isMemoizeWrapper   = fe.isMemoizeWrapper;
  ret->isMemoizeImpl      = Func::isMemoizeImplName(fe.name);

  add_frame_variables(*ret, fe);

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
    ret->nativeInfo->dynCallWrapperId = fe.dynCallWrapperId;
    if (f && ret->params.size()) {
      if (!f->cls()) {
        // There are a handful of functions whose first parameter is by
        // ref, but which don't require a reference.  There is also
        // array_multisort, which has this property for all its
        // parameters; but that
        if (ret->params[0].byRef && !f->mustBeRef(0)) {
          ret->params[0].mustBeRef = false;
        }
      }
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
                   borrowed_ptr<php::Class> ret,
                   borrowed_ptr<php::Unit> unit,
                   const PreClassEmitter& pce) {
  std::unique_ptr<php::Func> cinit;
  for (auto& me : pce.methods()) {
    auto f = parse_func(puState, unit, ret, *me);
    if (f->name == s_86cinit.get()) {
      puState.constPassFuncs[borrow(f)] |= php::Program::ForAnalyze;
      cinit = std::move(f);
    } else {
      if (f->name == s_86pinit.get() || f->name == s_86sinit.get()) {
        puState.constPassFuncs[borrow(f)] |= php::Program::ForAnalyze;
      }
      ret->methods.push_back(std::move(f));
    }
  }
  if (cinit) ret->methods.push_back(std::move(cinit));
}

void add_stringish(borrowed_ptr<php::Class> cls) {
  // The runtime adds Stringish to any class providing a __toString() function,
  // so we mirror that here to make sure analysis of interfaces is correct.
  if (cls->attrs & AttrInterface && cls->name->isame(s_Stringish.get())) {
    return;
  }

  for (auto& iface : cls->interfaceNames) {
    if (iface->isame(s_Stringish.get())) return;
  }

  for (auto& func : cls->methods) {
    if (func->name->isame(s_toString.get())) {
      FTRACE(2, "Adding Stringish to {}\n", cls->name->data());
      cls->interfaceNames.push_back(s_Stringish.get());
      return;
    }
  }
}

std::unique_ptr<php::Class> parse_class(ParseUnitState& puState,
                                        borrowed_ptr<php::Unit> unit,
                                        const PreClassEmitter& pce) {
  FTRACE(2, "  class: {}\n", pce.name()->data());

  auto ret               = std::make_unique<php::Class>();
  ret->name              = pce.name();
  ret->srcInfo           = php::SrcInfo { pce.getLocation(),
                                          pce.docComment() };
  ret->unit              = unit;
  ret->closureContextCls = nullptr;
  ret->parentName        = pce.parentName()->empty() ? nullptr
                                                     : pce.parentName();
  ret->attrs             = static_cast<Attr>(pce.attrs() & ~AttrNoOverride);
  ret->hoistability      = pce.hoistability();
  ret->userAttributes    = pce.userAttributes();
  ret->id                = pce.id();

  for (auto& iface : pce.interfaces()) {
    ret->interfaceNames.push_back(iface);
  }

  copy(ret->usedTraitNames,  pce.usedTraits());
  copy(ret->traitPrecRules,  pce.traitPrecRules());
  copy(ret->traitAliasRules, pce.traitAliasRules());
  copy(ret->requirements,    pce.requirements());

  ret->numDeclMethods = pce.numDeclMethods();

  parse_methods(puState, borrow(ret), unit, pce);
  add_stringish(borrow(ret));

  auto& propMap = pce.propMap();
  for (size_t idx = 0; idx < propMap.size(); ++idx) {
    auto& prop = propMap[idx];
    ret->properties.push_back(
      php::Prop {
        prop.name(),
        prop.attrs(),
        prop.docComment(),
        prop.typeConstraint(),
        prop.val()
      }
    );
  }

  auto& constMap = pce.constMap();
  for (size_t idx = 0; idx < constMap.size(); ++idx) {
    auto& cconst = constMap[idx];
    ret->constants.push_back(
      php::Const {
        cconst.name(),
        borrow(ret),
        cconst.valOption(),
        cconst.phpCode(),
        cconst.typeConstraint(),
        cconst.isTypeconst()
      }
    );
  }

  ret->enumBaseTy = pce.enumBaseTy();

  return ret;
}

//////////////////////////////////////////////////////////////////////

void assign_closure_context(const ParseUnitState&, borrowed_ptr<php::Class>);

borrowed_ptr<php::Class>
find_closure_context(const ParseUnitState& puState,
                     borrowed_ptr<php::Func> createClFunc) {
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
                            borrowed_ptr<php::Class> clo) {
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
    ++it;
    for (; it != end(clIt->second); ++it) {
      assert(find_closure_context(puState, *it) == representative);
    }
  }
  clo->closureContextCls = representative;
}

void find_additional_metadata(const ParseUnitState& puState,
                              borrowed_ptr<php::Unit> unit) {
  for (auto& c : unit->classes) {
    if (!c->parentName || !c->parentName->isame(s_Closure.get())) {
      continue;
    }
    assign_closure_context(puState, borrow(c));
  }
}

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<php::Unit> parse_unit(php::Program& prog,
                                      std::unique_ptr<UnitEmitter> uep) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, uep->isASystemLib()};
  FTRACE(2, "parse_unit {}\n", uep->m_filepath->data());

  auto const& ue = *uep;

  auto ret      = std::make_unique<php::Unit>();
  ret->md5      = ue.md5();
  ret->filename = ue.m_filepath;
  ret->preloadPriority = ue.m_preloadPriority;
  ret->isHHFile = ue.m_isHHFile;
  ret->useStrictTypes = ue.m_useStrictTypes;
  ret->useStrictTypesForBuiltins = ue.m_useStrictTypesForBuiltins;

  ParseUnitState puState;
  if (ue.hasSourceLocInfo()) {
    puState.srcLocInfo = ue.createSourceLocTable();
  } else {
    puState.srcLocInfo = ue.lineTable();
  }
  puState.defClsMap.resize(ue.numPreClasses(), nullptr);

  for (size_t i = 0; i < ue.numPreClasses(); ++i) {
    auto cls = parse_class(puState, borrow(ret), *ue.pce(i));
    ret->classes.push_back(std::move(cls));
  }

  for (auto& fe : ue.fevec()) {
    auto func = parse_func(puState, borrow(ret), nullptr, *fe);
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

  ret->classAliases = std::move(puState.classAliases);

  find_additional_metadata(puState, borrow(ret));

  for (auto const item : puState.constPassFuncs) {
    auto encoded_val = reinterpret_cast<uintptr_t>(item.first) | item.second;
    record_const_init(prog, encoded_val);
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
