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
#include "hphp/hhbbc/emit.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <type_traits>

#include <folly/gen/Base.h>
#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/Memory.h>

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/tv-comparisons.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_emit);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_invoke("__invoke");

//////////////////////////////////////////////////////////////////////

struct PceInfo {
  PreClassEmitter* pce;
  Id origId;
};

struct EmitUnitState {
  explicit EmitUnitState(const Index& index, const php::Unit* unit) :
      index(index), unit(unit) {}

  /*
   * Access to the Index for this program.
   */
  const Index& index;

  /*
   * Access to the unit we're emitting
   */
  const php::Unit* unit;

  /*
   * While emitting bytecode, we keep track of the classes and funcs
   * we emit.
   */
  std::vector<Offset>  classOffsets;
  std::vector<PceInfo> pceInfo;
};

/*
 * Some bytecodes need to be mutated before being emitted. Pass those
 * bytecodes by value to their respective emit_op functions.
 */
template<typename T>
struct OpInfoHelper {
  static constexpr bool by_value = T::op == Op::CreateCl;
  using type = typename std::conditional<by_value, T, const T&>::type;
};

template<typename T>
using OpInfo = typename OpInfoHelper<T>::type;

/*
 * Helper to conditionally call fun on data provided data is of the
 * given type.
 */
template<Op op, typename F, typename Bc>
std::enable_if_t<std::remove_reference_t<Bc>::op == op>
caller(F&& fun, Bc&& data) { fun(std::forward<Bc>(data)); }

template<Op op, typename F, typename Bc>
std::enable_if_t<std::remove_reference_t<Bc>::op != op>
caller(F&&, Bc&&) {}

Id recordClass(EmitUnitState& euState, UnitEmitter& ue, Id id) {
  auto cls = euState.unit->classes[id].get();
  euState.pceInfo.push_back(
    { ue.newPreClassEmitter(cls->name->toCppString(), cls->hoistability), id }
  );
  return euState.pceInfo.back().pce->id();
}

//////////////////////////////////////////////////////////////////////

php::SrcLoc srcLoc(const php::Func& func, int32_t ix) {
  if (ix < 0) return php::SrcLoc{};
  auto const unit = func.originalUnit ? func.originalUnit : func.unit;
  return unit->srcLocs[ix];
}

/*
 * Order the blocks for bytecode emission.
 *
 * Rules about block order:
 *
 *   - Each DV funclet must have all of its blocks contiguous, with the
 *     entry block first.
 *
 *   - Main entry point must be the first block.
 *
 * It is not a requirement, but we attempt to locate all the DV entry
 * points after the rest of the primary function body.  The normal
 * case for DV initializers is that each one falls through to the
 * next, with the block jumping back to the main entry point.
 */
std::vector<BlockId> order_blocks(const php::WideFunc& f) {
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

  FTRACE(2, "      block order:{}\n",
    [&] {
      std::string ret;
      for (auto const bid : sorted) {
        ret += " ";
        ret += folly::to<std::string>(bid);
      }
      return ret;
    }()
  );
  return sorted;
}

// While emitting bytecode, we learn about some metadata that will
// need to be registered in the FuncEmitter.
struct EmitBcInfo {
  struct JmpFixup { Offset instrOff; Offset jmpImmedOff; };

  struct BlockInfo {
    BlockInfo()
      : offset(kInvalidOffset)
      , past(kInvalidOffset)
      , regionsToPop(0)
    {}

    // The offset of the block, if we've already emitted it.
    // Otherwise kInvalidOffset.
    Offset offset;

    // The offset past the end of this block.
    Offset past;

    // How many catch regions the jump at the end of this block is leaving.
    // 0 if there is no jump or if the jump is to the same catch region or a
    // child
    int regionsToPop;

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

  std::vector<BlockId> blockOrder;
  uint32_t maxStackDepth;
  bool containsCalls;
  std::vector<BlockInfo> blockInfo;
};

using ExnNodePtr = php::ExnNode*;

bool handleEquivalent(const php::Func& func, ExnNodeId eh1, ExnNodeId eh2) {
  auto entry = [&] (ExnNodeId eid) {
    return func.exnNodes[eid].region.catchEntry;
  };

  while (eh1 != eh2) {
    assertx(eh1 != NoExnNodeId &&
            eh2 != NoExnNodeId &&
            func.exnNodes[eh1].depth == func.exnNodes[eh2].depth);
    if (entry(eh1) != entry(eh2)) return false;
    eh1 = func.exnNodes[eh1].parent;
    eh2 = func.exnNodes[eh2].parent;
  }

  return true;
};

// The common parent P of eh1 and eh2 is the deepest region such that
// eh1 and eh2 are both handle-equivalent to P or a child of P
ExnNodeId commonParent(const php::Func& func, ExnNodeId eh1, ExnNodeId eh2) {
  if (eh1 == NoExnNodeId || eh2 == NoExnNodeId) return NoExnNodeId;
  while (func.exnNodes[eh1].depth > func.exnNodes[eh2].depth) {
    eh1 = func.exnNodes[eh1].parent;
  }
  while (func.exnNodes[eh2].depth > func.exnNodes[eh1].depth) {
    eh2 = func.exnNodes[eh2].parent;
  }
  while (!handleEquivalent(func, eh1, eh2)) {
    eh1 = func.exnNodes[eh1].parent;
    eh2 = func.exnNodes[eh2].parent;
  }
  return eh1;
};

const StaticString
  s_hhbbc_fail_verification("__hhvm_intrinsics\\hhbbc_fail_verification");

EmitBcInfo emit_bytecode(EmitUnitState& euState, UnitEmitter& ue, FuncEmitter& fe,
                         const php::WideFunc& func) {
  EmitBcInfo ret = {};
  auto& blockInfo = ret.blockInfo;
  blockInfo.resize(func.blocks().size());

  // Track the stack depth while emitting to determine maxStackDepth.
  int32_t currentStackDepth { 0 };

  // Temporary buffer for vector immediates.  (Hoisted so it's not
  // allocated in the loop.)
  std::vector<uint8_t> immVec;

  // Offset of the last emitted bytecode.
  Offset lastOff { 0 };

  bool traceBc = false;

  SCOPE_ASSERT_DETAIL("emit") {
    std::string ret;
    for (auto bid : func.blockRange()) {
      auto const block = show(*func, *func.blocks()[bid]);
      folly::format(&ret, "block #{}\n{}", bid, block);
    }

    return ret;
  };

  auto const map_local = [&] (LocalId id) {
    if (id >= func->locals.size()) return id;
    auto const loc = func->locals[id];
    assertx(!loc.killed);
    assertx(loc.id <= id);
    id = loc.id;
    return id;
  };

  auto const map_local_name = [&] (NamedLocal nl) {
    nl.id = map_local(nl.id);
    if (nl.name == kInvalidLocalName) return nl;
    if (nl.name >= func->locals.size()) return nl;
    auto const loc = func->locals[nl.name];
    if (!loc.name) {
      nl.name = kInvalidLocalName;
      return nl;
    }
    assertx(!loc.unusedName);
    nl.name = loc.nameId;
    return nl;
  };

  auto set_expected_depth = [&] (BlockId block) {
    auto& info = blockInfo[block];

    if (info.expectedStackDepth) {
      assertx(*info.expectedStackDepth == currentStackDepth);
    } else {
      info.expectedStackDepth = currentStackDepth;
    }
  };

  auto make_member_key = [&] (MKey mkey) {
    switch (mkey.mcode) {
      case MEC: case MPC:
        return MemberKey{mkey.mcode, static_cast<int32_t>(mkey.idx), mkey.rop};
      case MEL: case MPL:
        return MemberKey{mkey.mcode, map_local_name(mkey.local), mkey.rop};
      case MET: case MPT: case MQT:
        return MemberKey{mkey.mcode, mkey.litstr, mkey.rop};
      case MEI:
        return MemberKey{mkey.mcode, mkey.int64, mkey.rop};
      case MW:
        return MemberKey{};
    }
    not_reached();
  };

  auto emit_inst = [&] (const Bytecode& inst) {
    auto const startOffset = fe.bcPos();
    lastOff = startOffset;

    FTRACE(4, " emit: {} -- {} @ {}\n", currentStackDepth, show(func, inst),
           show(srcLoc(*func, inst.srcLoc)));

    if (options.TraceBytecodes.count(inst.op)) traceBc = true;

    auto emit_vsa = [&] (const CompactVector<LSString>& keys) {
      auto n = keys.size();
      fe.emitIVA(n);
      for (size_t i = 0; i < n; ++i) {
        fe.emitInt32(ue.mergeLitstr(keys[i]));
      }
    };

    auto emit_branch = [&] (BlockId id) {
      auto& info = blockInfo[id];
      if (info.offset != kInvalidOffset) {
        fe.emitInt32(info.offset - startOffset);
      } else {
        info.forwardJumps.push_back({ startOffset, fe.bcPos() });
        fe.emitInt32(0);
      }
    };

    auto emit_switch = [&] (const SwitchTab& targets) {
      fe.emitIVA(targets.size());
      for (auto t : targets) {
        set_expected_depth(t);
        emit_branch(t);
      }
    };

    auto emit_sswitch = [&] (const SSwitchTab& targets) {
      fe.emitIVA(targets.size());
      for (size_t i = 0; i < targets.size() - 1; ++i) {
        set_expected_depth(targets[i].second);
        fe.emitInt32(ue.mergeLitstr(targets[i].first));
        emit_branch(targets[i].second);
      }
      fe.emitInt32(-1);
      set_expected_depth(targets[targets.size() - 1].second);
      emit_branch(targets[targets.size() - 1].second);
    };

    auto emit_srcloc = [&] {
      auto const sl = srcLoc(*func, inst.srcLoc);
      auto const loc = sl.isValid() ?
        Location::Range(sl.start.line, sl.start.col, sl.past.line, sl.past.col)
        : Location::Range(-1,-1,-1,-1);
      fe.recordSourceLocation(loc, startOffset);
    };

    auto pop = [&] (int32_t n) {
      currentStackDepth -= n;
      assertx(currentStackDepth >= 0);
    };
    auto push = [&] (int32_t n) {
      currentStackDepth += n;
      ret.maxStackDepth =
        std::max<uint32_t>(ret.maxStackDepth, currentStackDepth);
    };

    auto ret_assert = [&] { assertx(currentStackDepth == inst.numPop()); };

    auto createcl  = [&] (auto& data) {
      auto& id = data.arg2;
      if (euState.classOffsets[id] != kInvalidOffset) {
        for (auto const& elm : euState.pceInfo) {
          if (elm.origId == id) {
            id = elm.pce->id();
            return;
          }
        }
        always_assert(false);
      }
      euState.classOffsets[id] = startOffset;
      id = recordClass(euState, ue, id);
    };

    auto emit_lar  = [&](const LocalRange& range) {
      encodeLocalRange(fe, HPHP::LocalRange{
        map_local(range.first), range.count
      });
    };

    auto emit_ita  = [&](IterArgs ita) {
      if (ita.hasKey()) ita.keyId = map_local(ita.keyId);
      ita.valId = map_local(ita.valId);
      encodeIterArgs(fe, ita);
    };

#define IMM_BLA(n)     emit_switch(data.targets);
#define IMM_SLA(n)     emit_sswitch(data.targets);
#define IMM_IVA(n)     fe.emitIVA(data.arg##n);
#define IMM_I64A(n)    fe.emitInt64(data.arg##n);
#define IMM_LA(n)      fe.emitIVA(map_local(data.loc##n));
#define IMM_NLA(n)     fe.emitNamedLocal(map_local_name(data.nloc##n));
#define IMM_ILA(n)     fe.emitIVA(map_local(data.loc##n));
#define IMM_IA(n)      fe.emitIVA(data.iter##n);
#define IMM_DA(n)      fe.emitDouble(data.dbl##n);
#define IMM_SA(n)      fe.emitInt32(ue.mergeLitstr(data.str##n));
#define IMM_RATA(n)    encodeRAT(fe, data.rat);
#define IMM_AA(n)      fe.emitInt32(ue.mergeArray(data.arr##n));
#define IMM_OA_IMPL(n) fe.emitByte(static_cast<uint8_t>(data.subop##n));
#define IMM_OA(type)   IMM_OA_IMPL
#define IMM_BA(n)      targets[numTargets++] = data.target##n; \
                       emit_branch(data.target##n);
#define IMM_VSA(n)     emit_vsa(data.keys);
#define IMM_KA(n)      encode_member_key(make_member_key(data.mkey), fe);
#define IMM_LAR(n)     emit_lar(data.locrange);
#define IMM_ITA(n)     emit_ita(data.ita);
#define IMM_FCA(n)     encodeFCallArgs(                                 \
                         fe, data.fca.base(),                           \
                         data.fca.enforceInOut(),                       \
                         [&] {                                          \
                           data.fca.applyIO(                            \
                             [&] (int numBytes, const uint8_t* inOut) { \
                               encodeFCallArgsIO(fe, numBytes, inOut);  \
                             }                                          \
                           );                                           \
                         },                                             \
                         data.fca.asyncEagerTarget() != NoBlockId,      \
                         [&] {                                          \
                           set_expected_depth(data.fca.asyncEagerTarget()); \
                           emit_branch(data.fca.asyncEagerTarget());    \
                         },                                             \
                         data.fca.context() != nullptr,                 \
                         [&] {                                          \
                           fe.emitInt32(ue.mergeLitstr(data.fca.context()));\
                         });                                            \
                       if (!data.fca.hasUnpack()) ret.containsCalls = true;

#define IMM_NA
#define IMM_ONE(x)                IMM_##x(1)
#define IMM_TWO(x, y)             IMM_##x(1); IMM_##y(2);
#define IMM_THREE(x, y, z)        IMM_TWO(x, y); IMM_##z(3);
#define IMM_FOUR(x, y, z, n)      IMM_THREE(x, y, z); IMM_##n(4);
#define IMM_FIVE(x, y, z, n, m)   IMM_FOUR(x, y, z, n); IMM_##m(5);
#define IMM_SIX(x, y, z, n, m, o) IMM_FIVE(x, y, z, n, m); IMM_##o(6);

#define POP_NOV
#define POP_ONE(x)            pop(1);
#define POP_TWO(x, y)         pop(2);
#define POP_THREE(x, y, z)    pop(3);

#define POP_MFINAL     pop(data.arg1);
#define POP_C_MFINAL(n) pop(n); pop(data.arg1);
#define POP_CMANY      pop(data.arg##1);
#define POP_SMANY      pop(data.keys.size());
#define POP_CUMANY     pop(data.arg##1);
#define POP_FCALL(nin, nobj) \
                       pop(nin + data.fca.numInputs() + 1 + data.fca.numRets());

#define PUSH_NOV
#define PUSH_ONE(x)            push(1);
#define PUSH_TWO(x, y)         push(2);
#define PUSH_THREE(x, y, z)    push(3);
#define PUSH_CMANY             push(data.arg1);
#define PUSH_FCALL             push(data.fca.numRets());

#define O(opcode, imms, inputs, outputs, flags)                 \
    auto emit_##opcode = [&] (OpInfo<bc::opcode> data) {        \
      if (RuntimeOption::EnableIntrinsicsExtension) {           \
        if (Op::opcode == Op::FCallFuncD &&                     \
            inst.FCallFuncD.str2->isame(                        \
              s_hhbbc_fail_verification.get())) {               \
          fe.emitOp(Op::CheckProp);                             \
          fe.emitInt32(                                         \
            ue.mergeLitstr(inst.FCallFuncD.str2));              \
          fe.emitOp(Op::PopC);                                  \
          ret.maxStackDepth++;                                  \
        }                                                       \
      }                                                         \
      caller<Op::CreateCl>(createcl, data);                     \
                                                                \
      if (isRet(Op::opcode)) ret_assert();                      \
      fe.emitOp(Op::opcode);                                    \
      POP_##inputs                                              \
                                                                \
      size_t numTargets = 0;                                    \
      std::array<BlockId, kMaxHhbcImms> targets;                \
                                                                \
      if (Op::opcode == Op::MemoGet) {                          \
        IMM_##imms                                              \
        assertx(numTargets == 1);                               \
        set_expected_depth(targets[0]);                         \
        PUSH_##outputs                                          \
      } else if (Op::opcode == Op::MemoGetEager) {              \
        IMM_##imms                                              \
        assertx(numTargets == 2);                               \
        set_expected_depth(targets[0]);                         \
        PUSH_##outputs                                          \
        set_expected_depth(targets[1]);                         \
      } else {                                                  \
        PUSH_##outputs                                          \
        IMM_##imms                                              \
        for (size_t i = 0; i < numTargets; ++i) {               \
          set_expected_depth(targets[i]);                       \
        }                                                       \
      }                                                         \
                                                                \
      if (flags & TF) currentStackDepth = 0;                    \
      emit_srcloc();                                            \
    };

    OPCODES

#undef O

#undef IMM_MA
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
#undef IMM_KA
#undef IMM_LAR
#undef IMM_FCA

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR
#undef IMM_FIVE
#undef IMM_SIX

#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE

#undef POP_CMANY
#undef POP_SMANY
#undef POP_CUMANY
#undef POP_FCALL
#undef POP_MFINAL
#undef POP_C_MFINAL

#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_THREE
#undef PUSH_CMANY
#undef PUSH_FCALL

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
    auto bid = *blockIt;
    auto& info = blockInfo[bid];
    auto const b = func.blocks()[bid].get();

    info.offset = fe.bcPos();
    FTRACE(2, "      block {}: {}\n", bid, info.offset);

    for (auto& fixup : info.forwardJumps) {
      fe.emitInt32(info.offset - fixup.instrOff, fixup.jmpImmedOff);
    }

    if (!info.expectedStackDepth) {
      // unreachable, or entry block
      info.expectedStackDepth = b->catchEntry ? 1 : 0;
    }

    currentStackDepth = *info.expectedStackDepth;

    auto fallthrough = b->fallthrough;
    auto end = b->hhbcs.end();
    auto flip = false;

    if (is_single_nop(*b)) {
      if (blockIt == begin(ret.blockOrder)) {
        // If the first block is just a Nop, this means that there is
        // a jump to the second block from somewhere in the
        // function. We don't want this, so we change this nop to an
        // EntryNop so it doesn't get optimized away
        emit_inst(bc_with_loc(b->hhbcs.front().srcLoc, bc::EntryNop {}));
      }
    } else {
      // If the block ends with JmpZ or JmpNZ to the next block, flip
      // the condition to make the fallthrough the next block
      if (b->hhbcs.back().op == Op::JmpZ ||
          b->hhbcs.back().op == Op::JmpNZ) {
        auto const& bc = b->hhbcs.back();
        auto const target =
          bc.op == Op::JmpNZ ? bc.JmpNZ.target1 : bc.JmpZ.target1;
        if (std::next(blockIt) != endBlockIt && blockIt[1] == target) {
          fallthrough = target;
          --end;
          flip = true;
        }
      }

      for (auto iit = b->hhbcs.begin(); iit != end; ++iit) emit_inst(*iit);
      if (flip) {
        if (end->op == Op::JmpNZ) {
          emit_inst(bc_with_loc(end->srcLoc, bc::JmpZ { b->fallthrough }));
        } else {
          emit_inst(bc_with_loc(end->srcLoc, bc::JmpNZ { b->fallthrough }));
        }
      }
    }

    info.past = fe.bcPos();

    if (fallthrough != NoBlockId) {
      set_expected_depth(fallthrough);
      if (std::next(blockIt) == endBlockIt ||
          blockIt[1] != fallthrough) {
        if (b->fallthroughNS) {
          emit_inst(bc::JmpNS { fallthrough });
        } else {
          emit_inst(bc::Jmp { fallthrough });
        }

        auto const nextExnId = func.blocks()[fallthrough]->exnNodeId;
        auto const parent = commonParent(*func, nextExnId, b->exnNodeId);

        auto depth = [&] (ExnNodeId eid) {
          return eid == NoExnNodeId ? 0 : func->exnNodes[eid].depth;
        };
        // If we are in an exn region we pop from the current region to the
        // common parent. If the common parent is null, we pop all regions
        info.regionsToPop = depth(b->exnNodeId) - depth(parent);
        assertx(info.regionsToPop >= 0);
        FTRACE(4, "      popped catch regions: {}\n", info.regionsToPop);
      }
    }

    if (b->throwExit != NoBlockId) {
      FTRACE(4, "      throw: {}\n", b->throwExit);
    }
    if (fallthrough != NoBlockId) {
      FTRACE(4, "      fallthrough: {}\n", fallthrough);
    }
    FTRACE(2, "      block {} end: {}\n", bid, info.past);
  }

  if (traceBc) {
    FTRACE(0, "TraceBytecode (emit): {}::{} in {}\n",
           func->cls ? func->cls->name->data() : "",
           func->name, func->unit->filename);
  }

  return ret;
}

void emit_locals_and_params(FuncEmitter& fe, const php::Func& func,
                            const EmitBcInfo& info) {
  Id id = 0;
  for (auto const& loc : func.locals) {
    if (loc.id < func.params.size()) {
      assertx(loc.name);
      assertx(!loc.killed);
      assertx(!loc.unusedName);
      auto& param = func.params[id];
      FuncEmitter::ParamInfo pinfo;
      pinfo.defaultValue = param.defaultValue;
      pinfo.typeConstraint = param.typeConstraint;
      pinfo.userType = param.userTypeConstraint;
      pinfo.upperBounds = param.upperBounds;
      pinfo.phpCode = param.phpCode;
      pinfo.userAttributes = param.userAttributes;
      pinfo.builtinType = param.builtinType;
      if (param.inout) pinfo.setFlag(Func::ParamInfo::Flags::InOut);
      if (param.isVariadic) pinfo.setFlag(Func::ParamInfo::Flags::Variadic);
      fe.appendParam(func.locals[id].name, pinfo);
      auto const dv = param.dvEntryPoint;
      if (dv != NoBlockId) {
        fe.params[id].funcletOff = info.blockInfo[dv].offset;
      }
      ++id;
    } else {
      if (loc.killed) continue;
      if (loc.name && !loc.unusedName && loc.name) {
        fe.allocVarId(loc.name);
        assertx(fe.lookupVarId(loc.name) == id);
        assertx(loc.id == id);
        ++id;
      } else {
        fe.allocUnnamedLocal();
        ++id;
      }
    }
  }
  for (auto const& loc : func.locals) {
    if (loc.killed && !loc.unusedName && loc.name) {
      fe.allocVarId(loc.name, true);
    }
  }

  if (debug) {
    for (auto const& loc : func.locals) {
      if (!loc.killed) {
        assertx(loc.id < fe.numLocals());
      }
      if (!loc.unusedName && loc.name) {
        assertx(loc.nameId < fe.numNamedLocals());
      }
    }
  }

  assertx(fe.numLocals() == id);
  fe.setNumIterators(func.numIters);
}

struct EHRegion {
  const php::ExnNode* node;
  EHRegion* parent;
  Offset start;
  Offset past;
};

template<class BlockInfo, class ParentIndexMap>
void emit_eh_region(FuncEmitter& fe,
                    const EHRegion* region,
                    const BlockInfo& blockInfo,
                    ParentIndexMap& parentIndexMap) {
  FTRACE(2,  "    func {}: ExnNode {}\n", fe.name, region->node->idx);

  auto const unreachable = [&] (const php::ExnNode& node) {
    return blockInfo[node.region.catchEntry].offset == kInvalidOffset;
  };

  // A region on a single empty block.
  if (region->start == region->past) {
    FTRACE(2, "    Skipping\n");
    return;
  } else if (unreachable(*region->node)) {
    FTRACE(2, "    Unreachable\n");
    return;
  }

  FTRACE(2, "    Process @ {}-{}\n", region->start, region->past);

  auto& eh = fe.addEHEnt();
  eh.m_base = region->start;
  eh.m_past = region->past;
  assertx(eh.m_past >= eh.m_base);
  assertx(eh.m_base != kInvalidOffset && eh.m_past != kInvalidOffset);

  // An unreachable parent won't be emitted (and thus its offset won't be set),
  // so find the closest reachable one.
  auto parent = region->parent;
  while (parent && unreachable(*parent->node)) parent = parent->parent;
  if (parent) {
    auto parentIt = parentIndexMap.find(parent);
    assertx(parentIt != end(parentIndexMap));
    eh.m_parentIndex = parentIt->second;
  } else {
    eh.m_parentIndex = -1;
  }
  parentIndexMap[region] = fe.ehtab.size() - 1;

  auto const& cr = region->node->region;
  eh.m_handler = blockInfo[cr.catchEntry].offset;
  eh.m_end = kInvalidOffset;
  eh.m_iterId = cr.iterId;

  assertx(eh.m_handler != kInvalidOffset);
}

void exn_path(const php::Func& func,
              std::vector<const php::ExnNode*>& ret,
              ExnNodeId id) {
  if (id == NoExnNodeId) return;
  auto const& n = func.exnNodes[id];
  exn_path(func, ret, n.parent);
  ret.push_back(&n);
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
void emit_ehent_tree(FuncEmitter& fe, const php::WideFunc& func,
                     const EmitBcInfo& info) {
  hphp_fast_map<
    const php::ExnNode*,
    std::vector<std::unique_ptr<EHRegion>>
  > exnMap;

  /*
   * While walking over the blocks in layout order, we track the set
   * of "active" exnNodes.  This are a list of exnNodes that inherit
   * from each other.  When a new active node is pushed, begin an
   * EHEnt, and when it's popped, it's done.
   */
  std::vector<const php::ExnNode*> activeList;

  auto pop_active = [&] (Offset past) {
    auto p = activeList.back();
    activeList.pop_back();
    exnMap[p].back()->past = past;
  };

  auto push_active = [&] (const php::ExnNode* p, Offset start) {
    auto const parent = activeList.empty()
      ? nullptr
      : exnMap[activeList.back()].back().get();
    exnMap[p].push_back(
      std::make_unique<EHRegion>(
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
  for (auto const bid : info.blockOrder) {
    auto const b = func.blocks()[bid].get();
    auto const offset = info.blockInfo[bid].offset;

    if (b->exnNodeId == NoExnNodeId) {
      while (!activeList.empty()) pop_active(offset);
      continue;
    }

    std::vector<const php::ExnNode*> current;
    exn_path(*func, current, b->exnNodeId);

    auto const prefix = shared_prefix(current, activeList);
    for (size_t i = prefix, sz = activeList.size(); i < sz; ++i) {
      pop_active(offset);
    }
    for (size_t i = prefix, sz = current.size(); i < sz; ++i) {
      push_active(current[i], offset);
    }

    for (int i = 0; i < info.blockInfo[bid].regionsToPop; i++) {
      // If the block ended in a jump out of the catch region, this effectively
      // ends all catch regions deeper than the one we are jumping to
      pop_active(info.blockInfo[bid].past);
    }

    if (debug && !activeList.empty()) {
      current.clear();
      exn_path(*func, current, activeList.back()->idx);
      assertx(current == activeList);
    }
  }

  while (!activeList.empty()) {
    pop_active(info.blockInfo[info.blockOrder.back()].past);
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
  std::vector<EHRegion*> regions;
  for (auto& mapEnt : exnMap) {
    for (auto& region : mapEnt.second) {
      regions.push_back(region.get());
    }
  }
  std::sort(
    begin(regions), end(regions),
    [&] (const EHRegion* a, const EHRegion* b) {
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
            assertx(p == b);
          }
          return false;
        }
        return a->past > b->past;
      }
      return a->start < b->start;
    }
  );

  hphp_fast_map<const EHRegion*,uint32_t> parentIndexMap;
  for (auto& r : regions) {
    emit_eh_region(fe, r, info.blockInfo, parentIndexMap);
  }
  fe.setEHTabIsSorted();
}

void merge_repo_auth_type(UnitEmitter& ue, RepoAuthType rat) {
  using T = RepoAuthType::Tag;

  switch (rat.tag()) {
  case T::OptBool:
  case T::UninitBool:
  case T::OptInt:
  case T::UninitInt:
  case T::OptSStr:
  case T::UninitSStr:
  case T::OptStr:
  case T::UninitStr:
  case T::OptDbl:
  case T::OptRes:
  case T::OptObj:
  case T::UninitObj:
  case T::OptFunc:
  case T::OptCls:
  case T::OptClsMeth:
  case T::OptRecord:
  case T::OptLazyCls:
  case T::OptUncArrKey:
  case T::OptArrKey:
  case T::OptUncArrKeyCompat:
  case T::OptArrKeyCompat:
  case T::OptUncStrLike:
  case T::OptStrLike:
  case T::Null:
  case T::Cell:
  case T::InitUnc:
  case T::Unc:
  case T::UncArrKey:
  case T::ArrKey:
  case T::UncArrKeyCompat:
  case T::ArrKeyCompat:
  case T::UncStrLike:
  case T::StrLike:
  case T::InitCell:
  case T::Uninit:
  case T::InitNull:
  case T::Bool:
  case T::Int:
  case T::Dbl:
  case T::Res:
  case T::SStr:
  case T::Str:
  case T::Obj:
  case T::Func:
  case T::Cls:
  case T::ClsMeth:
  case T::Record:
  case T::LazyCls:
  case T::Num:
  case T::OptNum:
  case T::InitPrim:
  case T::NonNull:
    return;

  case T::OptSVec:
  case T::OptVec:
  case T::SVec:
  case T::Vec:
  case T::OptSDict:
  case T::OptDict:
  case T::SDict:
  case T::Dict:
  case T::OptSKeyset:
  case T::OptKeyset:
  case T::SKeyset:
  case T::Keyset:
  case T::SArrLike:
  case T::ArrLike:
  case T::OptSArrLike:
  case T::OptArrLike:

  case T::VecCompat:
  case T::ArrLikeCompat:
  case T::OptVecCompat:
  case T::OptArrLikeCompat:
    // NOTE: In repo mode, RAT's in Array's might only contain global litstr
    // id's. No need to merge. In non-repo mode, RAT's in Array's might contain
    // local litstr id's.
    return;

  case T::OptSubObj:
  case T::OptExactObj:
  case T::UninitSubObj:
  case T::UninitExactObj:
  case T::SubObj:
  case T::ExactObj:
  case T::OptSubCls:
  case T::OptExactCls:
  case T::SubCls:
  case T::ExactCls:
    ue.mergeLitstr(rat.clsName());
    return;

  case T::OptSubRecord:
  case T::OptExactRecord:
  case T::SubRecord:
  case T::ExactRecord:
    ue.mergeLitstr(rat.recordName());
    return;
  }
}

void emit_finish_func(EmitUnitState& state, FuncEmitter& fe,
                      php::WideFunc& wf, const EmitBcInfo& info) {
  auto const& func = *wf;
  if (info.containsCalls) fe.containsCalls = true;;

  emit_locals_and_params(fe, func, info);
  emit_ehent_tree(fe, wf, info);
  wf.blocks().clear();

  fe.userAttributes = func.userAttributes;
  fe.retUserType = func.returnUserType;
  fe.retUpperBounds = func.returnUBs;
  fe.originalFilename =
    func.originalFilename ? func.originalFilename :
    func.originalUnit ? func.originalUnit->filename : nullptr;
  fe.isClosureBody = func.isClosureBody;
  fe.isAsync = func.isAsync;
  fe.isGenerator = func.isGenerator;
  fe.isPairGenerator = func.isPairGenerator;
  fe.isNative = func.nativeInfo != nullptr;
  fe.isMemoizeWrapper = func.isMemoizeWrapper;
  fe.isMemoizeWrapperLSB = func.isMemoizeWrapperLSB;
  fe.isRxDisabled = func.isRxDisabled;
  fe.hasParamsWithMultiUBs = func.hasParamsWithMultiUBs;
  fe.hasReturnWithMultiUBs = func.hasReturnWithMultiUBs;

  for (auto& name : func.staticCoeffects) fe.staticCoeffects.push_back(name);
  for (auto& rule : func.coeffectRules)   fe.coeffectRules.push_back(rule);

  auto const retTy = state.index.lookup_return_type_raw(&func);
  if (!retTy.subtypeOf(BBottom)) {
    auto const rat = make_repo_type(*state.index.array_table_builder(), retTy);
    merge_repo_auth_type(fe.ue(), rat);
    fe.repoReturnType = rat;
  }

  if (is_specialized_wait_handle(retTy)) {
    auto const awaitedTy = wait_handle_inner(retTy);
    if (!awaitedTy.subtypeOf(BBottom)) {
      auto const rat = make_repo_type(
        *state.index.array_table_builder(),
        awaitedTy
      );
      merge_repo_auth_type(fe.ue(), rat);
      fe.repoAwaitedReturnType = rat;
    }
  }

  if (func.nativeInfo) {
    fe.hniReturnType = func.nativeInfo->returnType;
  }
  fe.retTypeConstraint = func.retTypeConstraint;

  fe.maxStackCells = info.maxStackDepth +
                     fe.numLocals() +
                     fe.numIterators() * kNumIterCells;

  fe.finish();
}

void renumber_locals(php::Func& func) {
  Id id = 0;
  Id nameId = 0;

  // We initialise local name ids in two passes, since locals that have not
  // been remapped may require their name be at the same offset as the local.
  // That's true for params, volatile locals, or locals in funcs with VarEnvs.
  //
  // In the first pass, we assume that all local names are used. Only in the
  // second pass do we apply the fact that some local names are never used.

  for (auto& loc : func.locals) {
    if (loc.killed) {
      // Make sure it's out of range, in case someone tries to read it.
      loc.id = INT_MAX;
    } else {
      loc.nameId = nameId++;
      loc.id = id++;
    }
  }
  for (auto& loc : func.locals) {
    if (loc.unusedName || !loc.name) {
      // Make sure it's out of range, in case someone tries to read it.
      loc.nameId = INT_MAX;
    } else if (loc.killed) {
      // The local uses a shared local slot, but its name is still used.
      loc.nameId = nameId++;
    }
  }
}

void emit_init_func(FuncEmitter& fe, const php::Func& func) {
  fe.init(
    std::get<0>(func.srcInfo.loc),
    std::get<1>(func.srcInfo.loc),
    func.attrs | (func.sampleDynamicCalls ? AttrDynamicallyCallable : AttrNone),
    func.srcInfo.docComment
  );
}

void emit_func(EmitUnitState& state, UnitEmitter& ue,
               FuncEmitter& fe, php::Func& f) {
  FTRACE(2,  "    func {}\n", f.name->data());
  assertx(f.attrs & AttrUnique);
  assertx(f.attrs & AttrPersistent);
  renumber_locals(f);
  emit_init_func(fe, f);
  auto func = php::WideFunc::mut(&f);
  auto const info = emit_bytecode(state, ue, fe, func);
  emit_finish_func(state, fe, func, info);
}

void emit_record(UnitEmitter& ue, const php::Record& rec) {
  assertx(rec.attrs & AttrUnique);
  assertx(rec.attrs & AttrPersistent);
  auto const re = ue.newRecordEmitter(rec.name->toCppString());
  re->init(
      std::get<0>(rec.srcInfo.loc),
      std::get<1>(rec.srcInfo.loc),
      rec.attrs,
      rec.parentName ? rec.parentName : staticEmptyString(),
      rec.srcInfo.docComment
  );
  re->setUserAttributes(rec.userAttributes);
  for (auto&& f : rec.fields) {
    re->addField(
        f.name,
        f.attrs,
        f.userType,
        f.typeConstraint,
        RecordEmitter::UpperBoundVec{},
        f.docComment,
        &f.val,
        RepoAuthType{},
        f.userAttributes
    );
  }
  ue.pushMergeableRecord(re->id());
}

void emit_class(EmitUnitState& state, UnitEmitter& ue, PreClassEmitter* pce,
                Offset offset, php::Class& cls) {
  FTRACE(2, "    class: {}\n", cls.name->data());
  assertx(cls.attrs & AttrUnique);
  assertx(cls.attrs & AttrPersistent);
  pce->init(
    std::get<0>(cls.srcInfo.loc),
    std::get<1>(cls.srcInfo.loc),
    cls.attrs |
      (cls.sampleDynamicConstruct ? AttrDynamicallyConstructible : AttrNone),
    cls.parentName ? cls.parentName : staticEmptyString(),
    cls.srcInfo.docComment
  );
  pce->setUserAttributes(cls.userAttributes);

  for (auto& x : cls.interfaceNames)     pce->addInterface(x);
  for (auto& x : cls.includedEnumNames)  pce->addEnumInclude(x);
  for (auto& x : cls.usedTraitNames)     pce->addUsedTrait(x);
  for (auto& x : cls.requirements)       pce->addClassRequirement(x);
  for (auto& x : cls.traitPrecRules)     pce->addTraitPrecRule(x);
  for (auto& x : cls.traitAliasRules)    pce->addTraitAliasRule(x);

  pce->setIfaceVtableSlot(state.index.lookup_iface_vtable_slot(&cls));

  bool needs86cinit = false;

  auto const nativeConsts = cls.attrs & AttrBuiltin ?
    Native::getClassConstants(cls.name) : nullptr;

  for (auto& cconst : cls.constants) {
    if (nativeConsts && nativeConsts->count(cconst.name)) {
      break;
    }
    if (cconst.kind == ConstModifiers::Kind::Context) {
      pce->addContextConstant(
        cconst.name,
        std::vector<LowStringPtr>(cconst.coeffects),
        cconst.isAbstract,
        cconst.isFromTrait
      );
    } else if (!cconst.val.has_value()) {
      pce->addAbstractConstant(
        cconst.name,
        cconst.typeConstraint,
        cconst.kind,
        cconst.isFromTrait
      );
    } else {
      needs86cinit |= cconst.val->m_type == KindOfUninit;

      pce->addConstant(
        cconst.name,
        cconst.typeConstraint,
        &cconst.val.value(),
        cconst.phpCode,
        cconst.kind,
        cconst.isFromTrait
      );
    }
  }

  for (auto& m : cls.methods) {
    if (!needs86cinit && m->name == s_86cinit.get()) continue;
    FTRACE(2, "    method: {}\n", m->name->data());
    auto const fe = ue.newMethodEmitter(m->name, pce);
    emit_func(state, ue, *fe, *m);
    pce->addMethod(fe);
  }

  CompactVector<Type> useVars;
  if (is_closure(cls)) {
    auto f = find_method(&cls, s_invoke.get());
    useVars = state.index.lookup_closure_use_vars(f, true);
  }
  auto uvIt = useVars.begin();

  auto const privateProps   = state.index.lookup_private_props(&cls, true);
  auto const privateStatics = state.index.lookup_private_statics(&cls, true);
  auto const publicStatics  = state.index.lookup_public_statics(&cls);
  for (auto const& prop : cls.properties) {
    auto const makeRat = [&] (const Type& ty) -> RepoAuthType {
      if (!ty.subtypeOf(BCell)) return RepoAuthType{};
      if (ty.subtypeOf(BBottom)) {
        // A property can be TBottom if no sets (nor its initial value) is
        // compatible with its type-constraint, or if its LateInit and there's
        // no sets to it. The repo auth type here doesn't particularly matter,
        // since such a prop will be inaccessible.
        return RepoAuthType{};
      }

      auto const rat = make_repo_type(*state.index.array_table_builder(), ty);
      merge_repo_auth_type(ue, rat);
      return rat;
    };

    auto const privPropTy = [&] (const PropState& ps) -> Type {
      if (is_closure(cls)) {
        // For closures use variables will be the first properties of the
        // closure object, in declaration order
        if (uvIt != useVars.end()) return *uvIt++;
        return TCell;
      }

      auto it = ps.find(prop.name);
      if (it == end(ps)) return TCell;
      return it->second.ty;
    };

    auto propTy = TCell;
    auto const attrs = prop.attrs;
    if (attrs & AttrPrivate) {
      propTy = privPropTy((attrs & AttrStatic) ? privateStatics : privateProps);
    } else if ((attrs & (AttrPublic|AttrProtected)) && (attrs & AttrStatic)) {
      propTy = [&] {
        auto const it = publicStatics.find(prop.name);
        if (it == end(publicStatics)) return TCell;
        return it->second.ty;
      }();
    }

    pce->addProperty(
      prop.name,
      prop.attrs,
      prop.userType,
      prop.typeConstraint,
      prop.ubs,
      prop.docComment,
      &prop.val,
      makeRat(propTy),
      prop.userAttributes
    );
  }
  assertx(uvIt == useVars.end());

  pce->setEnumBaseTy(cls.enumBaseTy);
}

void emit_typealias(UnitEmitter& ue, const php::TypeAlias& alias) {
  assertx(alias.attrs & AttrUnique);
  assertx(alias.attrs & AttrPersistent);
  auto const te = ue.newTypeAliasEmitter(alias.name->toCppString());
  te->init(
      std::get<0>(alias.srcInfo.loc),
      std::get<1>(alias.srcInfo.loc),
      alias.attrs,
      alias.value,
      alias.type,
      alias.nullable
  );
  te->setUserAttributes(alias.userAttrs);
  te->setTypeStructure(alias.typeStructure);

  auto const id = te->id();
  ue.pushMergeableId(Unit::MergeKind::TypeAlias, id);
}

void emit_constant(UnitEmitter& ue, const php::Constant& constant) {
  assertx(constant.attrs & AttrUnique);
  assertx(constant.attrs & AttrPersistent);
  Constant c {
    constant.name,
    constant.val,
    constant.attrs,
  };
  auto const id = ue.addConstant(c);
  ue.pushMergeableId(Unit::MergeKind::Define, id);
}

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<UnitEmitter> emit_unit(const Index& index, php::Unit& unit) {
  Trace::Bump bumper{
    Trace::hhbbc_emit, kSystemLibBump, is_systemlib_part(unit)
  };

  assertx(check(unit));

  auto ue = std::make_unique<UnitEmitter>(unit.sha1,
                                          SHA1{},
                                          Native::s_noNativeFuncs,
                                          true);
  FTRACE(1, "  unit {}\n", unit.filename->data());
  ue->m_sn = unit.sn;
  ue->m_filepath = unit.filename;
  ue->m_metaData = unit.metaData;
  ue->m_fileAttributes = unit.fileAttributes;

  if (unit.fatalInfo) {
    ue->m_fatalUnit = true;
    ue->m_fatalOp = unit.fatalInfo->fatalOp;
    ue->m_fatalLoc = unit.fatalInfo->fatalLoc;
    ue->m_fatalMsg = unit.fatalInfo->fatalMsg;
  }

  EmitUnitState state { index, &unit };
  state.classOffsets.resize(unit.classes.size(), kInvalidOffset);

  // Go thought all constant and see if they still need their matching 86cinit
  // func. In repo mode we are able to optimize away most of them away. And if
  // the const don't need them anymore we should not emit them.
  std::unordered_set<
    const StringData*,
    string_data_hash,
    string_data_same
  > const_86cinit_funcs;
  for (size_t id = 0; id < unit.constants.size(); ++id) {
    auto& c = unit.constants[id];
    if (type(c->val) != KindOfUninit) {
      const_86cinit_funcs.insert(Constant::funcNameFromName(c->name));
    }
  }

  /*
   * Top level funcs are always defined when the unit is loaded, and
   * don't have a DefFunc bytecode. Process them up front.
   */
  std::vector<std::unique_ptr<FuncEmitter> > top_fes;
  for (size_t id = 0; id < unit.funcs.size(); ++id) {
    auto const f = unit.funcs[id].get();
    if (const_86cinit_funcs.find(f->name) != const_86cinit_funcs.end()) {
      continue;
    }
    top_fes.push_back(std::make_unique<FuncEmitter>(*ue, -1, -1, f->name));
    emit_func(state, *ue, *top_fes.back(), *f);
  }

  /*
   * Find any top-level classes that need to be included due to
   * hoistability.
   */
  for (size_t id = 0; id < unit.classes.size(); ++id) {
    if (state.classOffsets[id] != kInvalidOffset) continue;
    auto const c = unit.classes[id].get();
    // Closures are AlwaysHoistable; but there's no need to include
    // them unless there's a reachable CreateCl.
    if (is_closure(*c)) continue;
    recordClass(state, *ue, id);
  }

  size_t pceId = 0;
  // Note that state.pceInfo can grow inside the loop
  while (pceId < state.pceInfo.size()) {
    auto const& pceInfo = state.pceInfo[pceId++];
    auto const id = pceInfo.origId;
    emit_class(state, *ue, pceInfo.pce,
               state.classOffsets[id], *unit.classes[id]);
  }

  for (size_t id = 0; id < unit.typeAliases.size(); ++id) {
    emit_typealias(*ue, *unit.typeAliases[id]);
  }

  for (size_t id = 0; id < unit.constants.size(); ++id) {
    emit_constant(*ue, *unit.constants[id]);
  }

  for (size_t id = 0; id < unit.records.size(); ++id) {
    emit_record(*ue, *unit.records[id]);
  }

  // Top level funcs need to go after any non-top level funcs. See
  // Unit::merge for details.
  for (auto& fe : top_fes) ue->appendTopEmitter(std::move(fe));

  return ue;
}

//////////////////////////////////////////////////////////////////////

}}
