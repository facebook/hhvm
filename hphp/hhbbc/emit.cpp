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
#include "hphp/hhbbc/emit.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <type_traits>

#include "folly/gen/Base.h"
#include "folly/Conv.h"
#include "folly/Optional.h"
#include "folly/Memory.h"

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/preclass-emit.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_emit);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_empty("");

//////////////////////////////////////////////////////////////////////

struct EmitUnitState {
  explicit EmitUnitState(const Index& index) : index(index) {}

  /*
   * Access to the Index for this program.
   */
  const Index& index;

  /*
   * While emitting bytecode, we keep track of where the DefCls
   * opcodes for each class are.  The PreClass runtime structures
   * require knowing these offsets.
   */
  std::vector<Offset> defClsMap;
};

//////////////////////////////////////////////////////////////////////

/*
 * Order the blocks for bytecode emission.
 *
 * Rules about block order:
 *
 *   - The "primary function body" must come first.  This is all blocks
 *     that aren't part of a fault funclet.
 *
 *   - Each funclet must have all of its blocks contiguous, with the
 *     entry block first.
 *
 *   - Main entry point must be the first block.
 *
 * It is not a requirement, but we attempt to locate all the DV entry
 * points after the rest of the primary function body.  The normal
 * case for DV initializers is that each one falls through to the
 * next, with the block jumping back to the main entry point.
 */
std::vector<borrowed_ptr<php::Block>> order_blocks(const php::Func& f) {
  auto sorted = rpoSortFromMain(f);

  // Get the DV blocks, without the rest of the primary function body,
  // and then add them to the end of sorted.
  auto const dvBlocks = [&] {
    auto withDVs = rpoSortAddDVs(f);
    withDVs.erase(
      std::find(begin(withDVs), end(withDVs), sorted.front()),
      end(withDVs)
    );
    return withDVs;
  }();
  sorted.insert(end(sorted), begin(dvBlocks), end(dvBlocks));

  // This stable sort will keep the blocks only reachable from DV
  // entry points after all other main code, and move fault funclets
  // after all that.
  std::stable_sort(
    begin(sorted), end(sorted),
    [&] (borrowed_ptr<php::Block> a, borrowed_ptr<php::Block> b) {
      using T = std::underlying_type<php::Block::Section>::type;
      return static_cast<T>(a->section) < static_cast<T>(b->section);
    }
  );

  FTRACE(2, "      block order:{}\n",
    [&] {
      std::string ret;
      for (auto& b : sorted) {
        ret += " ";
        if (b->section != php::Block::Section::Main) {
          ret += "f";
        }
        ret += folly::to<std::string>(b->id);
      }
      return ret;
    }()
  );
  return sorted;
}

// While emitting bytecode, we learn about some metadata that will
// need to be registered in the FuncEmitter.
struct EmitBcInfo {
  struct FPI {
    Offset fpushOff;
    Offset fcallOff;
    int32_t fpDelta;
  };

  struct JmpFixup { Offset instrOff; Offset jmpImmedOff; };

  struct BlockInfo {
    BlockInfo()
      : offset(kInvalidOffset)
      , past(kInvalidOffset)
    {}

    // The offset of the block, if we've already emitted it.
    // Otherwise kInvalidOffset.
    Offset offset;

    // The offset past the end of this block.
    Offset past;

    // When we emit a forward jump to a block we haven't seen yet, we
    // write down where the jump was so we can fix it up when we get
    // to that block.
    std::vector<JmpFixup> forwardJumps;

    // When we see a forward jump to a block, we record the stack
    // depth at the jump site here.  This is needed to track
    // currentStackDepth correctly (and we also assert all the jumps
    // have the same depth).
    folly::Optional<uint32_t> expectedStackDepth;
  };

  std::vector<borrowed_ptr<php::Block>> blockOrder;
  uint32_t maxStackDepth;
  uint32_t maxFpiDepth;
  bool containsCalls;
  std::vector<FPI> fpiRegions;
  std::vector<BlockInfo> blockInfo;
};

EmitBcInfo emit_bytecode(EmitUnitState& euState,
                         UnitEmitter& ue,
                         const php::Func& func) {
  EmitBcInfo ret = {};
  auto& blockInfo = ret.blockInfo;
  blockInfo.resize(func.nextBlockId);

  // Track the stack depth while emitting to determine maxStackDepth.
  int32_t currentStackDepth { 0 };

  // Stack of in-progress fpi regions.
  std::vector<EmitBcInfo::FPI> fpiStack;

  // Temporary buffer for vector immediates.  (Hoisted so it's not
  // allocated in the loop.)
  std::vector<uint8_t> immVec;

  auto emit_inst = [&] (const Bytecode& inst) {
    auto const startOffset = ue.bcPos();

    FTRACE(4, " emit: {} -- {} @ {}\n", currentStackDepth, show(inst),
      show(inst.srcLoc));

    auto count_stack_elems = [&] (const MVector& mvec) {
      int32_t ret = numLocationCodeStackVals(mvec.lcode);
      for (auto& m : mvec.mcodes) ret += mcodeStackVals(m.mcode);
      return ret;
    };

    auto emit_mvec = [&] (const MVector& mvec) {
      immVec.clear();

      auto const lcodeImms = numLocationCodeImms(mvec.lcode);
      immVec.push_back(mvec.lcode);
      assert(lcodeImms == 0 || lcodeImms == 1);
      if (lcodeImms) encodeIvaToVector(immVec, mvec.locBase->id);

      for (auto& m : mvec.mcodes) {
        immVec.push_back(m.mcode);
        switch (memberCodeImmType(m.mcode)) {
        case MCodeImm::None:
          break;
        case MCodeImm::Local:
          encodeIvaToVector(immVec, m.immLoc->id);
          break;
        case MCodeImm::Int:
          encodeToVector(immVec, int64_t{m.immInt});
          break;
        case MCodeImm::String:
          encodeToVector(immVec, int32_t{ue.mergeLitstr(m.immStr)});
          break;
        }
      }

      ue.emitInt32(immVec.size());
      ue.emitInt32(count_stack_elems(mvec));
      for (size_t i = 0; i < immVec.size(); ++i) ue.emitByte(immVec[i]);
    };

    auto emit_vsa = [&] (const std::vector<SString>& keys) {
      auto n = keys.size();
      ue.emitInt32(n);
      for (size_t i = 0; i < n; ++i) {
        ue.emitInt32(ue.mergeLitstr(keys[i]));
      }
    };

    auto emit_branch = [&] (const php::Block& target) {
      auto& info = blockInfo[target.id];

      if (info.expectedStackDepth) {
        assert(*info.expectedStackDepth == currentStackDepth);
      } else {
        info.expectedStackDepth = currentStackDepth;
      }

      if (info.offset != kInvalidOffset) {
        ue.emitInt32(info.offset - startOffset);
      } else {
        info.forwardJumps.push_back({ startOffset, ue.bcPos() });
        ue.emitInt32(0);
      }
    };

    auto emit_switch = [&] (const SwitchTab& targets) {
      ue.emitInt32(targets.size());
      for (auto& t : targets) emit_branch(*t);
    };

    auto emit_sswitch = [&] (const SSwitchTab& targets) {
      ue.emitInt32(targets.size());
      for (size_t i = 0; i < targets.size() - 1; ++i) {
        ue.emitInt32(ue.mergeLitstr(targets[i].first));
        emit_branch(*targets[i].second);
      }
      ue.emitInt32(-1);
      emit_branch(*targets[targets.size() - 1].second);
    };

    auto emit_itertab = [&] (const IterTab& iterTab) {
      ue.emitInt32(iterTab.size());
      for (auto& kv : iterTab) {
        ue.emitInt32(kv.first);
        ue.emitInt32(kv.second->id);
      }
    };

    auto emit_srcloc = [&] {
      if (!inst.srcLoc.isValid()) return;
      Location loc;
      loc.first(inst.srcLoc.start.line, inst.srcLoc.start.col);
      loc.last(inst.srcLoc.past.line, inst.srcLoc.past.col);
      ue.recordSourceLocation(&loc, startOffset);
    };

    auto pop = [&] (int32_t n) {
      currentStackDepth -= n;
      assert(currentStackDepth >= 0);
    };
    auto push = [&] (int32_t n) {
      currentStackDepth += n;
      if (currentStackDepth > ret.maxStackDepth) {
        ret.maxStackDepth = currentStackDepth;
      }
    };

    auto fpush = [&] {
      fpiStack.push_back({startOffset, kInvalidOffset, currentStackDepth});
      ret.maxFpiDepth = std::max<uint32_t>(ret.maxFpiDepth, fpiStack.size());
    };

    auto fcall = [&] {
      auto fpi = fpiStack.back();
      fpiStack.pop_back();
      fpi.fcallOff = startOffset;
      ret.fpiRegions.push_back(fpi);
    };

    auto ret_assert = [&] { assert(currentStackDepth == 1); };

    auto defcls = [&] {
      auto const id = inst.DefCls.arg1;
      always_assert(euState.defClsMap[id] == kInvalidOffset);
      euState.defClsMap[id] = startOffset;
    };

    auto nopdefcls = [&] {
      auto const id = inst.DefCls.arg1;
      always_assert(euState.defClsMap[id] == kInvalidOffset);
      euState.defClsMap[id] = startOffset;
    };

#define IMM_MA(n)      emit_mvec(data.mvec);
#define IMM_BLA(n)     emit_switch(data.targets);
#define IMM_SLA(n)     emit_sswitch(data.targets);
#define IMM_ILA(n)     emit_itertab(data.iterTab);
#define IMM_IVA(n)     ue.emitIVA(data.arg##n);
#define IMM_I64A(n)    ue.emitInt64(data.arg##n);
#define IMM_LA(n)      ue.emitIVA(data.loc##n->id);
#define IMM_IA(n)      ue.emitIVA(data.iter##n->id);
#define IMM_DA(n)      ue.emitDouble(data.dbl##n);
#define IMM_SA(n)      ue.emitInt32(ue.mergeLitstr(data.str##n));
#define IMM_AA(n)      ue.emitInt32(ue.mergeArray(data.arr##n));
#define IMM_OA_IMPL(n) ue.emitByte(static_cast<uint8_t>(data.subop));
#define IMM_OA(type)   IMM_OA_IMPL
#define IMM_BA(n)      emit_branch(*data.target);
#define IMM_VSA(n)     emit_vsa(data.keys);

#define IMM_NA
#define IMM_ONE(x)           IMM_##x(1)
#define IMM_TWO(x, y)        IMM_##x(1);         IMM_##y(2);
#define IMM_THREE(x, y, z)   IMM_TWO(x, y);      IMM_##z(3);
#define IMM_FOUR(x, y, z, n) IMM_THREE(x, y, z); IMM_##n(4);

#define POP_NOV
#define POP_ONE(x)            pop(1);
#define POP_TWO(x, y)         pop(2);
#define POP_THREE(x, y, z)    pop(3);

#define POP_MMANY      pop(count_stack_elems(data.mvec));
#define POP_C_MMANY    pop(1); pop(count_stack_elems(data.mvec));
#define POP_R_MMANY    pop(1); pop(count_stack_elems(data.mvec));
#define POP_V_MMANY    pop(1); pop(count_stack_elems(data.mvec));
#define POP_CMANY      pop(data.arg##1);
#define POP_SMANY      pop(data.keys.size());
#define POP_FMANY      pop(data.arg##1);
#define POP_CVMANY     pop(data.arg##1);
#define POP_CVUMANY    pop(data.arg##1);

#define PUSH_NOV
#define PUSH_ONE(x)            push(1);
#define PUSH_TWO(x, y)         push(2);
#define PUSH_THREE(x, y, z)    push(3);
#define PUSH_INS_1(x)          push(1);
#define PUSH_INS_2(x)          push(1);

#define O(opcode, imms, inputs, outputs, flags)                   \
    auto emit_##opcode = [&] (const bc::opcode& data) {           \
      if (Op::opcode == Op::DefCls)    defcls();                  \
      if (Op::opcode == Op::NopDefCls) nopdefcls();               \
      if (isRet(Op::opcode))           ret_assert();              \
      ue.emitOp(Op::opcode);                                      \
      POP_##inputs                                                \
      PUSH_##outputs                                              \
      IMM_##imms                                                  \
      if (isFPush(Op::opcode))     fpush();                       \
      if (isFCallStar(Op::opcode)) fcall();                       \
      if (flags & TF) currentStackDepth = 0;                      \
      if (Op::opcode == Op::FCall || Op::opcode == Op::FCallD) {  \
        ret.containsCalls = true;                                 \
      }                                                           \
      emit_srcloc();                                              \
    };

    OPCODES

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

#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE

#undef POP_CMANY
#undef POP_SMANY
#undef POP_MMANY
#undef POP_FMANY
#undef POP_CVMANY
#undef POP_CVUMANY
#undef POP_C_MMANY
#undef POP_R_MMANY
#undef POP_V_MMANY

#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_INS_1
#undef PUSH_INS_2

#define O(opcode, ...)                                        \
    case Op::opcode:                                          \
      if (Op::opcode != Op::Nop) emit_##opcode(inst.opcode);  \
      break;
    switch (inst.op) { OPCODES }
#undef O
  };

  ret.blockOrder        = order_blocks(func);
  auto blockIt          = begin(ret.blockOrder);
  auto const endBlockIt = end(ret.blockOrder);
  for (; blockIt != endBlockIt; ++blockIt) {
    auto& b = *blockIt;
    auto& info = blockInfo[b->id];
    info.offset = ue.bcPos();
    FTRACE(2, "      block {}: {}\n", b->id, info.offset);

    for (auto& fixup : info.forwardJumps) {
      ue.emitInt32(info.offset - fixup.instrOff, fixup.jmpImmedOff);
    }

    if (info.expectedStackDepth) {
      currentStackDepth = *info.expectedStackDepth;
    }

    for (auto& inst : b->hhbcs) emit_inst(inst);

    if (b->fallthrough) {
      if (boost::next(blockIt) == endBlockIt || blockIt[1] != b->fallthrough) {
        if (b->fallthroughNS) {
          emit_inst(bc::JmpNS { b->fallthrough });
        } else {
          emit_inst(bc::Jmp { b->fallthrough });
        }
      }
    }

    info.past = ue.bcPos();
    FTRACE(2, "      block {} end: {}\n", b->id, info.past);
  }

  return ret;
}

void emit_locals_and_params(FuncEmitter& fe,
                            const php::Func& func,
                            const EmitBcInfo& info) {
  Id id = 0;

  for (auto& loc : func.locals) {
    if (id < func.params.size()) {
      auto& param = func.params[id];
      FuncEmitter::ParamInfo pinfo;
      pinfo.setDefaultValue(param.defaultValue);
      pinfo.setTypeConstraint(param.typeConstraint);
      pinfo.setUserType(param.userTypeConstraint);
      pinfo.setPhpCode(param.phpCode);
      pinfo.setUserAttributes(param.userAttributes);
      pinfo.setBuiltinType(param.builtinType);
      pinfo.setRef(param.byRef);
      fe.appendParam(func.locals[id]->name, pinfo);
      if (auto const dv = param.dvEntryPoint) {
        fe.setParamFuncletOff(id, info.blockInfo[dv->id].offset);
      }
    } else {
      if (loc->name) {
        fe.allocVarId(loc->name);
        assert(fe.lookupVarId(loc->name) == id);
      } else {
        fe.allocUnnamedLocal();
      }
    }

    ++id;
  }
  assert(fe.numLocals() == id);
  fe.setNumIterators(func.iters.size());

  for (auto& sv : func.staticLocals) {
    fe.addStaticVar(Func::SVInfo { sv.name, sv.phpCode });
  }
}

struct EHRegion {
  borrowed_ptr<const php::ExnNode> node;
  borrowed_ptr<EHRegion> parent;
  Offset start;
  Offset past;
};

template<class BlockInfo, class ParentIndexMap>
void emit_eh_region(FuncEmitter& fe,
                    borrowed_ptr<const EHRegion> region,
                    const BlockInfo& blockInfo,
                    ParentIndexMap& parentIndexMap) {
  // A region on a single empty block.
  if (region->start == region->past) return;

  auto& eh = fe.addEHEnt();
  eh.m_base = region->start;
  eh.m_past = region->past;
  assert(eh.m_past >= eh.m_base);
  assert(eh.m_base != kInvalidOffset && eh.m_past != kInvalidOffset);

  if (region->parent) {
    auto parentIt = parentIndexMap.find(region->parent);
    assert(parentIt != end(parentIndexMap));
    eh.m_parentIndex = parentIt->second;
  } else {
    eh.m_parentIndex = -1;
  }
  parentIndexMap[region] = fe.ehtab().size() - 1;

  match<void>(
    region->node->info,
    [&] (const php::TryRegion& tr) {
      eh.m_type = EHEnt::Type::Catch;
      for (auto& c : tr.catches) {
        eh.m_catches.emplace_back(
          fe.ue().mergeLitstr(c.first),
          blockInfo[c.second->id].offset
        );
      }
      eh.m_fault = kInvalidOffset;
      eh.m_iterId = -1;
      eh.m_itRef = false;
    },
    [&] (const php::FaultRegion& fr) {
      eh.m_type = EHEnt::Type::Fault;
      eh.m_fault = blockInfo[fr.faultEntry->id].offset;
      eh.m_iterId = fr.iterId;
      eh.m_itRef = fr.itRef;
    }
  );
}

void exn_path(std::vector<const php::ExnNode*>& ret, const php::ExnNode* n) {
  if (!n) return;
  exn_path(ret, n->parent);
  ret.push_back(n);
}

// Return the count of shared elements in the front of two forward
// ranges.
template<class ForwardRange1, class ForwardRange2>
size_t shared_prefix(ForwardRange1& r1, ForwardRange2& r2) {
  auto r1it = begin(r1);
  auto r2it = begin(r2);
  auto const r1end = end(r1);
  auto const r2end = end(r2);
  auto ret = size_t{0};
  while (r1it != r1end && r2it != r2end && *r1it == *r2it) {
    ++ret; ++r1it; ++r2it;
  }
  return ret;
}

/*
 * Traverse the actual block layout, and find out the intervals for
 * each exception region in the tree.
 *
 * The basic idea here is that we haven't constrained block layout
 * based on the exception tree, but adjacent blocks are still
 * reasonably likely to have the same ExnNode.  Try to coalesce the EH
 * regions we create for in those cases.
 */
void emit_ehent_tree(FuncEmitter& fe,
                     const php::Func& func,
                     const EmitBcInfo& info) {
  std::map<
    borrowed_ptr<const php::ExnNode>,
    std::vector<std::unique_ptr<EHRegion>>
  > exnMap;

  /*
   * While walking over the blocks in layout order, we track the set
   * of "active" exnNodes.  This are a list of exnNodes that inherit
   * from each other.  When a new active node is pushed, begin an
   * EHEnt, and when it's popped, it's done.
   */
  std::vector<borrowed_ptr<const php::ExnNode>> activeList;

  auto pop_active = [&] (Offset past) {
    auto p = activeList.back();
    activeList.pop_back();
    exnMap[p].back()->past = past;
  };

  auto push_active = [&] (const php::ExnNode* p, Offset start) {
    auto const parent = activeList.empty()
      ? nullptr
      : borrow(exnMap[activeList.back()].back());
    exnMap[p].push_back(
      folly::make_unique<EHRegion>(
        EHRegion { p, parent, start, kInvalidOffset }
      )
    );
    activeList.push_back(p);
  };

  /*
   * Walk over the blocks, and compare the new block's exnNode path to
   * the active one.  Find the least common ancestor of the two paths,
   * then modify the active list by popping and then pushing nodes to
   * set it to the new block's path.
   */
  for (auto& b : info.blockOrder) {
    auto const offset = info.blockInfo[b->id].offset;

    if (!b->exnNode) {
      while (!activeList.empty()) pop_active(offset);
      continue;
    }

    std::vector<borrowed_ptr<const php::ExnNode>> current;
    exn_path(current, b->exnNode);

    auto const prefix = shared_prefix(current, activeList);
    for (size_t i = prefix, sz = activeList.size(); i < sz; ++i) {
      pop_active(offset);
    }
    for (size_t i = prefix, sz = current.size(); i < sz; ++i) {
      push_active(current[i], offset);
    }

    if (debug && !activeList.empty()) {
      current.clear();
      exn_path(current, activeList.back());
      assert(current == activeList);
    }
  }

  while (!activeList.empty()) {
    pop_active(info.blockInfo[info.blockOrder.back()->id].past);
  }

  /*
   * We've created all our regions, but we need to sort them instead
   * of trying to get the UnitEmitter to do it.
   *
   * The UnitEmitter expects EH regions that look a certain way
   * (basically the way emitter.cpp likes them).  There are some rules
   * about the order it needs to have at runtime, which we set up
   * here.
   *
   * Essentially, an entry a is less than an entry b iff:
   *
   *   - a starts before b
   *   - a starts at the same place, but encloses b entirely
   *   - a has the same extents as b, but is a parent of b
   */
  std::vector<borrowed_ptr<EHRegion>> regions;
  for (auto& mapEnt : exnMap) {
    for (auto& region : mapEnt.second) {
      regions.push_back(borrow(region));
    }
  }
  std::sort(
    begin(regions), end(regions),
    [&] (borrowed_ptr<const EHRegion> a, borrowed_ptr<const EHRegion> b) {
      if (a == b) return false;
      if (a->start == b->start) {
        if (a->past == b->past) {
          // When regions exactly overlap, the parent is less than the
          // child.
          for (auto p = b->parent; p != nullptr; p = p->parent) {
            if (p == a) return true;
          }
          // If a is not a parent of b, and they have the same region;
          // then b better be a parent of a.
          if (debug) {
            auto p = a->parent;
            for (; p != b && p != nullptr; p = p->parent) continue;
            assert(p == b);
          }
          return false;
        }
        return a->past > b->past;
      }
      return a->start < b->start;
    }
  );

  std::map<borrowed_ptr<const EHRegion>,uint32_t> parentIndexMap;
  for (auto& r : regions) {
    emit_eh_region(fe, r, info.blockInfo, parentIndexMap);
  }
  fe.setEhTabIsSorted();
}

void emit_finish_func(const php::Func& func,
                      FuncEmitter& fe,
                      const EmitBcInfo& info) {
  fe.setMaxStackCells(
    info.maxStackDepth + fe.numLocals() +
      info.maxFpiDepth * kNumActRecCells
  );
  if (info.containsCalls) fe.setContainsCalls();

  for (auto& fpi : info.fpiRegions) {
    auto& e = fe.addFPIEnt();
    e.m_fpushOff = fpi.fpushOff;
    e.m_fcallOff = fpi.fcallOff;
    e.m_fpOff    = fpi.fpDelta;
  }

  emit_locals_and_params(fe, func, info);
  emit_ehent_tree(fe, func, info);

  fe.setUserAttributes(func.userAttributes);
  fe.setReturnUserType(func.returnUserType);
  fe.setOriginalFilename(func.originalFilename);
  fe.setIsClosureBody(func.isClosureBody);
  fe.setIsAsync(func.isAsync);
  fe.setIsGenerator(func.isGenerator);
  fe.setIsPairGenerator(func.isPairGenerator);
  if (func.nativeInfo) {
    fe.setReturnType(func.nativeInfo->returnType);
  }
  fe.setReturnTypeConstraint(func.retTypeConstraint);

  fe.finish(fe.ue().bcPos(), false /* load */);
  fe.ue().recordFunction(&fe);
}

void emit_init_func(FuncEmitter& fe, const php::Func& func) {
  fe.init(
    std::get<0>(func.srcInfo.loc),
    std::get<1>(func.srcInfo.loc),
    fe.ue().bcPos(),
    func.attrs,
    func.top,
    func.srcInfo.docComment
  );
}

void emit_func(EmitUnitState& state, UnitEmitter& ue, const php::Func& func) {
  FTRACE(2,  "    func {}\n", func.name->data());
  auto const fe = ue.newFuncEmitter(func.name);
  emit_init_func(*fe, func);
  auto const info = emit_bytecode(state, ue, func);
  emit_finish_func(func, *fe, info);
}

void emit_pseudomain(EmitUnitState& state,
                     UnitEmitter& ue,
                     const php::Unit& unit) {
  FTRACE(2,  "    pseudomain\n");
  auto& pm = *unit.pseudomain;
  ue.initMain(std::get<0>(pm.srcInfo.loc),
              std::get<1>(pm.srcInfo.loc));
  auto const fe = ue.getMain();
  auto const info = emit_bytecode(state, ue, pm);
  emit_finish_func(pm, *fe, info);
}

RepoAuthType make_repo_type(UnitEmitter& ue, const Type& t) {
  using T = RepoAuthType::Tag;

  if (t.strictSubtypeOf(TObj) || (is_opt(t) && t.strictSubtypeOf(TOptObj))) {
    auto const dobj = dobj_of(t);
    auto const tag =
      is_opt(t)
        ? (dobj.type == DObj::Exact ? T::OptExactObj : T::OptSubObj)
        : (dobj.type == DObj::Exact ? T::ExactObj    : T::SubObj);
    ue.mergeLitstr(dobj.cls.name());
    return RepoAuthType { tag, dobj.cls.name() };
  }

#define ASSERTT_OP(x) if (t.subtypeOf(T##x)) return RepoAuthType{T::x};
  ASSERTT_OPS
#undef ASSERTT_OP
  return RepoAuthType{};
}

void emit_class(EmitUnitState& state,
                UnitEmitter& ue,
                const php::Class& cls) {
  FTRACE(2, "    class: {}\n", cls.name->data());
  auto const pce = ue.newPreClassEmitter(
    cls.name,
    cls.hoistability
  );
  pce->init(
    std::get<0>(cls.srcInfo.loc),
    std::get<1>(cls.srcInfo.loc),
    ue.bcPos(),
    cls.attrs,
    cls.parentName ? cls.parentName : s_empty.get(),
    cls.srcInfo.docComment
  );
  pce->setUserAttributes(cls.userAttributes);

  for (auto& x : cls.interfaceNames)     pce->addInterface(x);
  for (auto& x : cls.usedTraitNames)     pce->addUsedTrait(x);
  for (auto& x : cls.traitRequirements)  pce->addTraitRequirement(x);
  for (auto& x : cls.traitPrecRules)     pce->addTraitPrecRule(x);
  for (auto& x : cls.traitAliasRules)    pce->addTraitAliasRule(x);

  for (auto& m : cls.methods) {
    FTRACE(2, "    method: {}\n", m->name->data());
    auto const fe = ue.newMethodEmitter(m->name, pce);
    emit_init_func(*fe, *m);
    pce->addMethod(fe);
    auto const info = emit_bytecode(state, ue, *m);
    emit_finish_func(*m, *fe, info);
  }

  auto const privateProps   = state.index.lookup_private_props(&cls);
  auto const privateStatics = state.index.lookup_private_statics(&cls);
  for (auto& prop : cls.properties) {
    auto const repoTy = [&] (const PropState& ps) {
      // TODO(#3599292): we don't currently infer closure use var types.
      if (is_closure(cls)) return RepoAuthType{};
      auto it = ps.find(prop.name);
      return it == end(ps) ? RepoAuthType{} : make_repo_type(ue, it->second);
    };

    pce->addProperty(
      prop.name,
      prop.attrs,
      prop.typeConstraint,
      prop.docComment,
      &prop.val,
      (prop.attrs & AttrStatic) ? repoTy(privateStatics) : repoTy(privateProps)
    );
  }

  for (auto& cconst : cls.constants) {
    pce->addConstant(
      cconst.name,
      cconst.typeConstraint,
      &cconst.val,
      cconst.phpCode
    );
  }
}

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<UnitEmitter> emit_unit(const Index& index,
                                       const php::Unit& unit) {
  auto const is_systemlib = is_systemlib_part(unit);
  Trace::Bump bumper{Trace::hhbbc_emit, kSystemLibBump, is_systemlib};

  auto ue = folly::make_unique<UnitEmitter>(unit.md5);
  FTRACE(1, "  unit {}\n", unit.filename->data());
  ue->setFilepath(unit.filename);

  EmitUnitState state { index };
  state.defClsMap.resize(unit.classes.size(), kInvalidOffset);

  /*
   * Unfortunate special case for Systemlib units.
   *
   * We need to ensure these units end up mergeOnly, at runtime there
   * are things that assume this (right now no other HHBBC units end
   * up being merge only, because of the returnSeen stuff below).
   *
   * (Merge-only-ness provides no measurable perf win in repo mode now
   * that we have persistent classes, so we're not too worried about
   * this.)
   */
  if (is_systemlib) {
    ue->setMergeOnly(true);
    auto const tv = make_tv<KindOfInt64>(1);
    ue->setMainReturn(&tv);
  } else {
    /*
     * TODO(#3017265): UnitEmitter is very coupled to emitter.cpp, and
     * expects classes and things to be added in an order that isn't
     * quite clear.  If you don't set returnSeen things relating to
     * hoistability break.
     */
    ue->returnSeen();
  }

  emit_pseudomain(state, *ue, unit);
  for (auto& c : unit.classes)     emit_class(state, *ue, *c);
  for (auto& f : unit.funcs)       emit_func(state, *ue, *f);
  for (auto& t : unit.typeAliases) ue->addTypeAlias(*t);

  for (size_t id = 0; id < unit.classes.size(); ++id) {
    // We may not have a DefCls PC if we're a closure, or a
    // non-top-level class declaration is DCE'd.
    if (state.defClsMap[id] != kInvalidOffset) {
      ue->pce(id)->setOffset(state.defClsMap[id]);
    }
  }

  return ue;
}

//////////////////////////////////////////////////////////////////////

}}
