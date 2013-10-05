/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <boost/next_prior.hpp>
#include <thread>
#include <mutex>

#include "folly/experimental/Gen.h"
#include "folly/experimental/StringGen.h"
#include "folly/ScopeGuard.h"
#include "folly/Memory.h"

#include "hphp/runtime/vm/preclass-emit.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

struct ParseUnitState {
  /*
   * This is computed once for each unit and stashed here.
   */
  SourceLocTable srcLocTable;

  /*
   * Map from class id to the function containing its DefCls
   * instruction.  We use this to compute whether classes are defined
   * at top-level.
   */
  std::vector<borrowed_ptr<php::Func>> defClsMap;
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
   * Fault funclets don't actually fall in the EHEnt region for their
   * parent handlers in HHBC.  However, we want edges from the fault
   * funclets to any enclosing catch blocks (or other enclosing
   * funclet blocks).
   *
   * For each funclet head, this records the ExnNode that points to
   * this funclet, which we will propagate to the whole funclet in
   * find_fault_funclets.
   */
  std::map<borrowed_ptr<php::Block>,borrowed_ptr<php::ExnNode>>
    funcletNodes;
};

template<class FindBlock>
ExnTreeInfo build_exn_tree(const FuncEmitter& fe,
                           php::Func& func,
                           FindBlock findBlock) {
  ExnTreeInfo ret;

  uint32_t nextExnNode = 0;

  for (auto& eh : fe.ehtab()) {
    auto node = folly::make_unique<php::ExnNode>();
    node->id = nextExnNode++;
    node->parent = nullptr;

    switch (eh.m_type) {
    case EHEnt::Type::Fault:
      {
        auto const fault = findBlock(eh.m_fault);
        assert(ret.funcletNodes[fault] == nullptr);
        ret.funcletNodes[fault] = borrow(node);

        /*
         * We know the block for this offset starts a fault funclet,
         * but we won't know its extents until we've built the cfg and
         * can look at the control flow in the funclet.  Set the block
         * type to Fault for now, but we won't propagate the value to
         * the rest of the funclet blocks until find_fault_funclets.
         */
        fault->kind = php::Block::Kind::Fault;
        node->info = php::FaultRegion { fault, eh.m_iterId, eh.m_itRef };
      }
      break;
    case EHEnt::Type::Catch:
      {
        auto treg = php::TryRegion {};
        for (auto& centry : eh.m_catches) {
          auto const catchBlk = findBlock(centry.second);
          catchBlk->kind = php::Block::Kind::CatchEntry;
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
 * Coming into this routine, we have set the block type to
 * Block::Kind::Fault on the head of each funclet, but it hasn't been
 * set on the rest of the funclet.  We also don't have factoredExits
 * from the funclet blocks to any enclosing catch handlers.
 *
 * Fault funclets may have internal control flow, but may not jump
 * into to other funclets or back into the main function body.  All
 * control flow paths through the funclet exit in blocks that end with
 * Unwind instructions.  This means if you propagate forward in RPO,
 * stopping when you see blocks that contain Unwind, the state will
 * get to the whole funclet.
 *
 * This routine propagates the block type and adds factored exits
 * along all control flow paths from the funclet head until reaching
 * an Unwind instruction.  If there were unreachable blocks in a fault
 * funclet, or if the funclet head is unreachable, they will not be
 * tagged.
 */
void find_fault_funclets(ExnTreeInfo& tinfo, const php::Func& func) {
  auto propagate = [&] (borrowed_ptr<php::Block> blk) {
    if (blk->kind != php::Block::Kind::Fault) return;

    auto factoredIt = tinfo.funcletNodes.find(blk);
    assert(factoredIt != end(tinfo.funcletNodes));
    assert(blk->factoredExits.empty());

    // Propagate the exit edges to the containing fault/try handlers,
    // if there were any.
    add_factored_exits(*blk, factoredIt->second->parent);

    // If this block ends with Unwind, don't propagate the fault
    // funclet-ness to its successors.
    if (ends_with_unwind(*blk)) return;

    // Propagate the state.  Fault funclets may have internal back
    // edges but it's harmless to propagate this to an already-visited
    // Block.
    forEachNormalSuccessor(*blk, [&] (php::Block& b) {
      b.kind = php::Block::Kind::Fault;
      tinfo.funcletNodes[&b] = factoredIt->second;
    });
  };

  // Iterate starting with the DV entries, in case a fault funclet is
  // only reachable from a DV entry.
  for (auto& blk : rpoSortAddDVs(func)) propagate(blk);
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

#define IMM_MA(n)     auto mvec = decode_minstr();
#define IMM_BLA(n)    auto targets = decode_switch(opPC);
#define IMM_SLA(n)    auto targets = decode_sswitch(opPC);
#define IMM_ILA(n)    auto iterTab = decode_itertab();
#define IMM_IVA(n)    auto arg##n = decodeVariableSizeImm(&pc);
#define IMM_I64A(n)   auto arg##n = decode<int64_t>(pc);
#define IMM_LA(n)     auto loc##n = [&] {                       \
                        auto id = decodeVariableSizeImm(&pc);   \
                        always_assert(id < func.locals.size()); \
                        return borrow(func.locals[id]);         \
                      }();
#define IMM_IA(n)     auto iter##n = [&] {                      \
                        auto id = decodeVariableSizeImm(&pc);   \
                        always_assert(id < func.iters.size());  \
                        return borrow(func.iters[id]);          \
                      }();
#define IMM_DA(n)     auto dbl##n = decode<double>(pc);
#define IMM_SA(n)     auto str##n = ue.lookupLitstr(decode<Id>(pc));
#define IMM_AA(n)     auto arr##n = ue.lookupArray(decode<Id>(pc));
#define IMM_BA(n)     assert(next == past); \
                      auto target = findBlock(  \
                        opPC + decode<Offset>(pc) - ue.bc());
#define IMM_OA(n)     auto subop = decode<uint8_t>(pc);

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
      if (Op::opcode == Op::DefCls) defcls(b);        \
      if (Op::opcode == Op::NopDefCls) nopdefcls(b);  \
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

    auto const srcLoc = [&] {
      SourceLoc sloc;
      if (getSourceLoc(puState.srcLocTable, opPC - ue.bc(), sloc)) {
        return php::SrcLoc {
          { static_cast<uint32_t>(sloc.line0),
            static_cast<uint32_t>(sloc.char0) },
          { static_cast<uint32_t>(sloc.line1),
            static_cast<uint32_t>(sloc.char1) }
        };
      }
      return php::SrcLoc{};
    }();

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
#undef IMM_OA

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
  if (blk.hhbcs.back().op == Op::Jmp) {
    blk.fallthrough = blk.hhbcs.back().Jmp.target;
    blk.hhbcs.back() = bc::Nop{};
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
      ptr->kind         = php::Block::Kind::Normal;
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
  find_fault_funclets(exnTreeInfo, func);

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
  // Note: probably need to clear AttrLeaf here eventually.
  ret->attrs                  = fe.attrs();
  ret->userAttributes         = fe.getUserAttributes();
  ret->userRetTypeConstraint  = fe.returnTypeConstraint();
  ret->originalFilename       = fe.originalFilename();

  ret->top                    = fe.top();
  ret->isClosureBody          = fe.isClosureBody();
  ret->isGeneratorBody        = fe.isGenerator();
  ret->isGeneratorFromClosure = fe.isGeneratorFromClosure();
  ret->isPairGenerator        = fe.isPairGenerator();
  ret->hasGeneratorAsBody     = fe.hasGeneratorAsBody();

  ret->nextBlockId     = 0;

  add_frame_variables(*ret, fe);
  build_cfg(puState, *ret, fe);

  return ret;
}

std::unique_ptr<php::Class> parse_class(ParseUnitState& puState,
                                        borrowed_ptr<php::Unit> unit,
                                        const PreClassEmitter& pce) {
  FTRACE(2, "  class: {}\n", pce.name()->data());

  auto ret            = folly::make_unique<php::Class>();
  ret->name           = pce.name();
  ret->srcInfo        = php::SrcInfo { pce.getLocation(),
                                       pce.docComment() };
  ret->unit           = unit;
  ret->parentName     = pce.parentName()->empty() ? nullptr : pce.parentName();
  ret->attrs          = pce.attrs();
  ret->hoistability   = pce.hositability();
  ret->userAttributes = pce.userAttributes();

  for (auto& iface : pce.interfaces()) {
    ret->interfaceNames.push_back(iface);
  }

  ret->usedTraitNames  = pce.usedTraits();
  ret->traitPrecRules  = pce.traitPrecRules();
  ret->traitAliasRules = pce.traitAliasRules();

  for (auto& me : pce.methods()) {
    ret->methods.push_back(parse_func(puState, unit, borrow(ret), *me));
  }

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
        cconst.val(),
        cconst.phpCode(),
        cconst.typeConstraint()
      }
    );
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<php::Unit> parse_unit(const UnitEmitter& ue) {
  FTRACE(2, "parse_unit {}\n", ue.getFilepath()->data());

  auto ret      = folly::make_unique<php::Unit>();
  ret->md5      = ue.md5();
  ret->filename = ue.getFilepath();

  ParseUnitState puState;
  puState.srcLocTable = ue.createSourceLocTable();
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

  // TODO_2: remove this or use it.  (Includes defClsMap.)
  // for (size_t id = 0; id < ret->classes.size(); ++id) {
  //   ret->classes[id]->definingFunc = puState.defClsMap[id];
  // }

  for (auto& ta : ue.typeAliases()) {
    ret->typeAliases.push_back(
      folly::make_unique<php::TypeAlias>(ta)
    );
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}

