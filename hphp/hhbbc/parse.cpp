/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <boost/next_prior.hpp>
#include <algorithm>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "folly/gen/Base.h"
#include "folly/gen/String.h"
#include "folly/ScopeGuard.h"
#include "folly/Memory.h"

#include "hphp/runtime/vm/preclass-emit.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Closure("Closure");

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
   * Map from Closure names to the function(s) containing their
   * associated CreateCl opcode(s).
   */
  std::unordered_map<
    SString,
    std::unordered_set<borrowed_ptr<php::Func>>,
    string_data_hash,
    string_data_isame
  > createClMap;
};

//////////////////////////////////////////////////////////////////////

std::set<Offset> findBasicBlocks(const FuncEmitter& fe) {
  std::set<Offset> blockStarts;
  auto markBlock = [&] (Offset off) { blockStarts.insert(off); };

  // Each entry point for a DV funclet is the start of a basic
  // block.
  for (auto& param : fe.params()) {
    if (param.hasDefaultValue()) markBlock(param.funcletOff());
  }

  // The main entry point is also a basic block start.
  markBlock(fe.base());

  /*
   * For each instruction, add it to the set if it must be the start
   * of a block.  It is the start of a block if it is:
   *
   *   - A jump target
   *
   *   - Immediatelly following a control flow instruction, other than
   *     a call.
   */
  auto offset = fe.base();
  for (;;) {
    auto const bc = fe.ue().bc();
    auto const pc = reinterpret_cast<const Op*>(bc + offset);
    auto const nextOff = offset + instrLen(pc);
    auto const atLast = nextOff == fe.past();

    if (instrIsInitialSuspend(*pc) && !atLast) {
      markBlock(nextOff);
    }

    if (instrIsNonCallControlFlow(*pc) && !atLast) {
      markBlock(nextOff);
    }

    if (isSwitch(*pc)) {
      foreachSwitchTarget(pc, [&] (Offset delta) {
        markBlock(offset + delta);
      });
    } else {
      auto const target = instrJumpTarget(
        reinterpret_cast<const Op*>(bc),
        offset
      );
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
   */
  for (auto& eh : fe.ehtab()) {
    markBlock(eh.m_base);
    markBlock(eh.m_past);
    switch (eh.m_type) {
    case EHEnt::Type::Catch:
      for (auto& centry : eh.m_catches) markBlock(centry.second);
      break;
    case EHEnt::Type::Fault:
      markBlock(eh.m_fault);
      break;
    }
  }

  // Now, each interval in blockStarts delinates a basic block.
  blockStarts.insert(fe.past());
  return blockStarts;
}

struct ExnTreeInfo {
  /*
   * Map from EHEnt to the ExnNode that will represent exception
   * behavior in that region.
   */
  std::map<const EHEnt*,borrowed_ptr<php::ExnNode>> ehMap;

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

  for (auto& eh : fe.ehtab()) {
    auto node = folly::make_unique<php::ExnNode>();
    node->id = nextExnNode++;
    node->parent = nullptr;

    switch (eh.m_type) {
    case EHEnt::Type::Fault:
      {
        auto const fault = findBlock(eh.m_fault);
        ret.funcletNodes[fault].push_back(borrow(node));
        ret.faultFuncletStarts.insert(eh.m_fault);
        node->info = php::FaultRegion { fault, eh.m_iterId, eh.m_itRef };
      }
      break;
    case EHEnt::Type::Catch:
      {
        auto treg = php::TryRegion {};
        for (auto& centry : eh.m_catches) {
          auto const catchBlk = findBlock(centry.second);
          treg.catches.emplace_back(
            fe.ue().lookupLitstr(centry.first),
            catchBlk
          );
        }
        node->info = treg;
      }
      break;
    }

    ret.ehMap[&eh] = borrow(node);

    if (eh.m_parentIndex != -1) {
      auto it = ret.ehMap.find(&fe.ehtab()[eh.m_parentIndex]);
      assert(it != end(ret.ehMap));
      node->parent = it->second;
      it->second->children.emplace_back(std::move(node));
    } else {
      func.exnNodes.emplace_back(std::move(node));
    }
  }

  ret.faultFuncletStarts.insert(fe.past());

  return ret;
}

/*
 * Instead of breaking blocks on instructions that could throw, we
 * represent the control flow edges for exception paths as a set of
 * factored edges at the end of each block.
 *
 * When we initially add them here, no attempt is made to determine if
 * the edge is actually possible to traverse.
 */
void add_factored_exits(php::Block& blk,
                        borrowed_ptr<const php::ExnNode> node) {
  for (; node; node = node->parent) {
    match<void>(
      node->info,
      [&] (const php::TryRegion& tr) {
        /*
         * Note: it seems like we should be able to stop adding edges
         * when we see a catch handler for Exception; however, fatal
         * errors don't stop there (and still run Fault handlers).
         *
         * For now we add all the edges, although we might be able to be
         * less pessimistic later.
         */
        for (auto& c : tr.catches) {
          blk.factoredExits.push_back(c.second);
        }
      },
      [&] (const php::FaultRegion& fr) {
        blk.factoredExits.push_back(fr.faultEntry);
      }
    );
  }
}

/*
 * Locate all the basic blocks associated with fault funclets, and
 * mark them as such.  Also, add factored exit edges for exceptional
 * control flow through any parent protected regions of the region(s)
 * that pointed at each fault handler.
 */
template<class BlockStarts, class FindBlock>
void find_fault_funclets(ExnTreeInfo& tinfo,
                         const php::Func& func,
                         const BlockStarts& blockStarts,
                         FindBlock findBlock) {
  auto sectionId = uint32_t{1};

  for (auto funcletStartIt = begin(tinfo.faultFuncletStarts);
      boost::next(funcletStartIt) != end(tinfo.faultFuncletStarts);
      ++funcletStartIt, ++sectionId) {
    auto const nextFunclet = *boost::next(funcletStartIt);

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
      blk->factoredExits.erase(
        std::unique(begin(blk->factoredExits), end(blk->factoredExits)),
        end(blk->factoredExits)
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

template<class FindBlock>
void populate_block(ParseUnitState& puState,
                    const FuncEmitter& fe,
                    php::Func& func,
                    php::Block& blk,
                    PC pc,
                    PC const past,
                    FindBlock findBlock) {
  auto const& ue = fe.ue();

  auto decode_minstr = [&] {
    auto const immVec = ImmVector::createFromStream(pc);
    pc += immVec.size() + sizeof(int32_t) + sizeof(int32_t);

    auto ret = MVector {};
    auto vec = immVec.vec();

    ret.lcode = static_cast<LocationCode>(*vec++);
    if (numLocationCodeImms(ret.lcode)) {
      assert(numLocationCodeImms(ret.lcode) == 1);
      ret.locBase = borrow(func.locals[decodeVariableSizeImm(&vec)]);
    }

    while (vec < pc) {
      auto elm = MElem {};
      elm.mcode = static_cast<MemberCode>(*vec++);
      switch (memberCodeImmType(elm.mcode)) {
      case MCodeImm::None: break;
      case MCodeImm::Local:
        elm.immLoc = borrow(func.locals[decodeMemberCodeImm(&vec, elm.mcode)]);
        break;
      case MCodeImm::String:
        elm.immStr = ue.lookupLitstr(decodeMemberCodeImm(&vec, elm.mcode));
        break;
      case MCodeImm::Int:
        elm.immInt = decodeMemberCodeImm(&vec, elm.mcode);
        break;
      }
      ret.mcodes.push_back(elm);
    }
    assert(vec == pc);

    return ret;
  };

  auto decode_stringvec = [&] {
    auto const vecLen = decode<int32_t>(pc);
    std::vector<SString> keys;
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
      ));
    }
    return ret;
  };

  auto decode_sswitch = [&] (PC opPC) {
    SSwitchTab ret;

    auto const vecLen = decode<int32_t>(pc);
    for (int32_t i = 0; i < vecLen - 1; ++i) {
      auto const id = decode<Id>(pc);
      auto const offset = decode<Offset>(pc);
      ret.emplace_back(
        ue.lookupLitstr(id),
        findBlock(opPC + offset - ue.bc())
      );
    }

    // Final case is the default, and must have a litstr id of -1.
    DEBUG_ONLY auto const defId = decode<Id>(pc);
    auto const defOff = decode<Offset>(pc);
    assert(defId == -1);
    ret.emplace_back(nullptr, findBlock(opPC + defOff - ue.bc()));
    return ret;
  };

  auto decode_itertab = [&] {
    IterTab ret;
    auto const vecLen = decode<int32_t>(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      auto const kind = static_cast<IterKind>(decode<int32_t>(pc));
      auto const id = decode<int32_t>(pc);
      ret.emplace_back(kind, borrow(func.iters[id]));
    }
    return ret;
  };

  auto defcls = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefCls.arg1] = &func;
  };
  auto nopdefcls = [&] (const Bytecode& b) {
    puState.defClsMap[b.NopDefCls.arg1] = &func;
  };
  auto createcl = [&] (const Bytecode& b) {
    puState.createClMap[b.CreateCl.str2].insert(&func);
  };

#define IMM_MA(n)      auto mvec = decode_minstr();
#define IMM_BLA(n)     auto targets = decode_switch(opPC);
#define IMM_SLA(n)     auto targets = decode_sswitch(opPC);
#define IMM_ILA(n)     auto iterTab = decode_itertab();
#define IMM_IVA(n)     auto arg##n = decodeVariableSizeImm(&pc);
#define IMM_I64A(n)    auto arg##n = decode<int64_t>(pc);
#define IMM_LA(n)      auto loc##n = [&] {                       \
                         auto id = decodeVariableSizeImm(&pc);   \
                         always_assert(id < func.locals.size()); \
                         return borrow(func.locals[id]);         \
                       }();
#define IMM_IA(n)      auto iter##n = [&] {                      \
                         auto id = decodeVariableSizeImm(&pc);   \
                         always_assert(id < func.iters.size());  \
                         return borrow(func.iters[id]);          \
                       }();
#define IMM_DA(n)      auto dbl##n = decode<double>(pc);
#define IMM_SA(n)      auto str##n = ue.lookupLitstr(decode<Id>(pc));
#define IMM_AA(n)      auto arr##n = ue.lookupArray(decode<Id>(pc));
#define IMM_BA(n)      assert(next == past); \
                       auto target = findBlock(  \
                         opPC + decode<Offset>(pc) - ue.bc());
#define IMM_OA_IMPL(n) decode<uint8_t>(pc);
#define IMM_OA(type)   auto subop = (type)IMM_OA_IMPL
#define IMM_VSA(n)     auto keys = decode_stringvec();

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


#define O(opcode, imms, inputs, outputs, flags)       \
  case Op::opcode:                                    \
    {                                                 \
      ++pc;                                           \
      auto b = Bytecode {};                           \
      b.op = Op::opcode;                              \
      b.srcLoc = srcLoc;                              \
      IMM_##imms                                      \
      new (&b.opcode) bc::opcode { IMM_ARG_##imms };  \
      if (Op::opcode == Op::DefCls)    defcls(b);     \
      if (Op::opcode == Op::NopDefCls) nopdefcls(b);  \
      if (Op::opcode == Op::CreateCl)  createcl(b);   \
      blk.hhbcs.push_back(std::move(b));              \
      assert(pc == next);                             \
    }                                                 \
    break;

  assert(pc != past);
  do {
    auto const opPC = pc;
    auto const pop  = reinterpret_cast<const Op*>(pc);
    auto const next = pc + instrLen(pop);
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

    switch (*pop) { OPCODES }

    if (next == past) {
      if (instrAllowsFallThru(*pop)) {
        blk.fallthrough = findBlock(next - ue.bc());
      }
    }

    pc = next;
  } while (pc != past);

#undef O

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA

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
  func.dvEntries.resize(fe.params().size());
  for (size_t i = 0, sz = fe.params().size(); i < sz; ++i) {
    if (fe.params()[i].hasDefaultValue()) {
      auto const dv = findBlock(fe.params()[i].funcletOff());
      func.params[i].dvEntryPoint = dv;
      func.dvEntries[i] = dv;
    }
  }
  func.mainEntry = findBlock(fe.base());
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
      ptr               = folly::make_unique<php::Block>();
      ptr->id           = func.nextBlockId++;
      ptr->section      = php::Block::Section::Main;
      ptr->exnNode      = nullptr;
    }
    return borrow(ptr);
  };

  auto exnTreeInfo = build_exn_tree(fe, func, findBlock);

  for (auto it = begin(blockStarts);
      boost::next(it) != end(blockStarts);
      ++it) {
    auto const block   = findBlock(*it);
    auto const bcStart = bc + *it;
    auto const bcStop  = bc + *boost::next(it);

    if (auto const eh = findEH(fe.ehtab(), *it)) {
      auto it = exnTreeInfo.ehMap.find(eh);
      assert(it != end(exnTreeInfo.ehMap));
      block->exnNode = it->second;
      add_factored_exits(*block, block->exnNode);
    }

    populate_block(puState, fe, func, *block, bcStart, bcStop, findBlock);
  }

  link_entry_points(func, fe, findBlock);
  find_fault_funclets(exnTreeInfo, func, blockStarts, findBlock);

  for (auto& kv : blockMap) {
    func.blocks.emplace_back(std::move(kv.second));
  }
}

void add_frame_variables(php::Func& func, const FuncEmitter& fe) {
  for (auto& param : fe.params()) {
    func.params.push_back(
      php::Param {
        param.defaultValue(),
        nullptr,
        param.typeConstraint(),
        param.userType(),
        param.phpCode(),
        param.userAttributes(),
        param.builtinType(),
        param.ref()
      }
    );
  }

  func.locals.resize(fe.numLocals());
  for (size_t id = 0; id < func.locals.size(); ++id) {
    auto& loc = func.locals[id];
    loc = folly::make_unique<php::Local>();
    loc->id = id;
    loc->name = nullptr;
  }
  for (auto& kv : fe.localNameMap()) {
    func.locals[kv.second]->name = kv.first;
  }

  func.iters.resize(fe.numIterators());
  for (uint32_t i = 0; i < func.iters.size(); ++i) {
    func.iters[i] = folly::make_unique<php::Iter>();
    func.iters[i]->id = i;
  }

  func.staticLocals.reserve(fe.svInfo().size());
  for (auto& sv : fe.svInfo()) {
    func.staticLocals.push_back(
      php::StaticLocalInfo { sv.name, sv.phpCode }
    );
  }
}

std::unique_ptr<php::Func> parse_func(ParseUnitState& puState,
                                      borrowed_ptr<php::Unit> unit,
                                      borrowed_ptr<php::Class> cls,
                                      const FuncEmitter& fe) {
  FTRACE(2, "  func: {}\n",
    fe.name()->data() && *fe.name()->data() ? fe.name()->data()
                                            : "pseudomain");

  auto ret             = folly::make_unique<php::Func>();
  ret->name            = fe.name();
  ret->srcInfo         = php::SrcInfo { fe.getLocation(),
                                        fe.getDocComment() };
  ret->unit            = unit;
  ret->cls             = cls;
  ret->nextBlockId     = 0;

  ret->attrs                  = fe.attrs();
  ret->userAttributes         = fe.getUserAttributes();
  ret->returnUserType         = fe.returnUserType();
  ret->retTypeConstraint      = fe.returnTypeConstraint();
  ret->originalFilename       = fe.originalFilename();

  ret->top                    = fe.top();
  ret->isClosureBody          = fe.isClosureBody();
  ret->isAsync                = fe.isAsync();
  ret->isGenerator            = fe.isGenerator();
  ret->isPairGenerator        = fe.isPairGenerator();

  /*
   * HNI-style native functions get some extra information.
   */
  if (fe.isHNINative()) {
    ret->nativeInfo             = folly::make_unique<php::NativeInfo>();
    ret->nativeInfo->returnType = fe.getReturnType();
  }

  add_frame_variables(*ret, fe);
  build_cfg(puState, *ret, fe);

  return ret;
}

void parse_methods(ParseUnitState& puState,
                   borrowed_ptr<php::Class> ret,
                   borrowed_ptr<php::Unit> unit,
                   const PreClassEmitter& pce) {
  for (auto& me : pce.methods()) {
    auto f = parse_func(puState, unit, ret, *me);
    ret->methods.push_back(std::move(f));
  }
}

std::unique_ptr<php::Class> parse_class(ParseUnitState& puState,
                                        borrowed_ptr<php::Unit> unit,
                                        const PreClassEmitter& pce) {
  FTRACE(2, "  class: {}\n", pce.name()->data());

  auto ret               = folly::make_unique<php::Class>();
  ret->name              = pce.name();
  ret->srcInfo           = php::SrcInfo { pce.getLocation(),
                                          pce.docComment() };
  ret->unit              = unit;
  ret->closureContextCls = nullptr;
  ret->parentName        = pce.parentName()->empty() ? nullptr
                                                     : pce.parentName();
  ret->attrs             = pce.attrs();
  ret->hoistability      = pce.hoistability();
  ret->userAttributes    = pce.userAttributes();

  for (auto& iface : pce.interfaces()) {
    ret->interfaceNames.push_back(iface);
  }

  ret->usedTraitNames    = pce.usedTraits();
  ret->traitPrecRules    = pce.traitPrecRules();
  ret->traitAliasRules   = pce.traitAliasRules();
  ret->traitRequirements = pce.traitRequirements();

  parse_methods(puState, borrow(ret), unit, pce);

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
        cconst.val(),
        cconst.phpCode(),
        cconst.typeConstraint()
      }
    );
  }

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

  auto clIt = puState.createClMap.find(clo->name);
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

std::unique_ptr<php::Unit> parse_unit(const UnitEmitter& ue) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, ue.isASystemLib()};
  FTRACE(2, "parse_unit {}\n", ue.getFilepath()->data());

  auto ret      = folly::make_unique<php::Unit>();
  ret->md5      = ue.md5();
  ret->filename = ue.getFilepath();

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

  for (auto& ta : ue.typeAliases()) {
    ret->typeAliases.push_back(
      folly::make_unique<php::TypeAlias>(ta)
    );
  }

  find_additional_metadata(puState, borrow(ret));

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
