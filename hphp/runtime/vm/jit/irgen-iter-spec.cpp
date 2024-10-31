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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/base/bespoke/layout-selection.h"

#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"

#include "hphp/util/struct-log.h"

namespace HPHP::jit::irgen {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

/*
 * Iterator Specialization: an explanation of the madness
 *
 * ========================================================
 * Intro: the generic case
 *
 * Before we describe iterator specialization, let's look at what the IterInit
 * and IterNext bytecodes are supposed to do. Let's assume that the bases are
 * array-likes; the object case is re-entrant and we don't specialize it.
 *
 * Pseudocode for IterInit:
 *
 *  1. Check if the base is empty; branch to done if so.
 *  2. Initialize the fields of the iterator: base, type, pos, end.
 *  3. Load and dec-ref the old val output local (and key, if applicable).
 *  4. Load, inc-ref, and store the new val (and key, if applicable).
 *  5. Continue onwards to the loop entry block.
 *
 * Pseudocode for IterNext:
 *
 *  1. Increment the iterator's pos field.
 *  2. Check if the pos is terminal; branch to done if so.
 *  3. Load and dec-ref the old val output local (and key, if applicable).
 *  4. Load, inc-ref, and store the new val (and key, if applicable).
 *  5. Check surprise flags and branch to the loop entry block.
 *
 * NOTE: It's possible that the old and new values alias (or that they point to
 * some other heap allocated values that alias). However, it's still okay to do
 * step 3 before step 4, because after step 3, any aliased values will still
 * have at least one ref-count held by the base. We'll use this fact later.
 *
 * ========================================================
 * Iter groups: the unit of specialization
 *
 * Examining the pseudocode above, steps 3 and 4 could be reused between *all*
 * IterInit and IterNext bytecodes in a given region that share the same loop
 * entry block. Given a loop entry block in a region, let's call the set of all
 * IterInit and IterNext bytecodes that share that block its "iter group".
 *
 * Some invariants on iter groups enforced at the bytecode level:
 *  - All ops have the same "type" (LocalBaseConst, LocalBaseMutable)
 *  - All ops have the same iterId, valLocalId, and keyLocalId
 *
 * And one thing that's not invariant:
 *  - The "loop exit" blocks for the ops in a group may differ. For example,
 *    it's possible that an IterInit in a iter group branches to a ReqBindJmp
 *    if the base is empty, while the IterNext in that group branches to RetC.
 *
 * In this module, we'll attempt to specialize an entire iter group or leave
 * the entire group generic. We'll store a list of SpecializedIterator structs
 * in IRGS keyed on the entry block, so that we can share certain blocks of
 * code between different ops in the same group.
 *
 * However, we will *not* guarantee that we do successfully specialize all ops
 * in the group. Instead, we'll ensure correctness by doing necessary checks
 * to ensure that the specialized code is valid before jumping into any shared
 * code blocks. We'll rely on load- and store-elim to eliminate these checks in
 * cases where we do specialize an entire iter group.
 *
 * ========================================================
 * Structure of the generated code
 *
 * There are four components in the specialized code for an iter group:
 * inits, the header, nexts, and the footer. Here's how they are arranged:
 *
 *  -----------------------   -----------------------   -----------------------
 *  | Specialized init #1 |   | Specialized init #2 |   | Specialized init #n |
 *  -----------------------   -----------------------   -----------------------
 *                       \               |               /
 *                          \            |            /
 *                             ----------------------
 *                             | Specialized header |
 *                             ----------------------
 *                                       |
 *                                       |
 *                             ----------------------
 *                             |  Loop entry block  |
 *                             | (not specialized!) |
 *                             ----------------------
 *                                       |
 *                                       |
 *       Loop body; may split control flow so there are multiple IterNexts;
 *       may throw exceptions or check types that lead to side exits, etc.
 *                |                      |                       |
 *                |                      |                       |
 *  -----------------------   -----------------------   -----------------------
 *  | Specialized next #1 |   | Specialized next #2 |   | Specialized next #n |
 *  -----------------------   -----------------------   -----------------------
 *                       \               |               /
 *                          \            |            /
 *                             ----------------------
 *                             | Specialized footer |
 *                             ----------------------
 *                                       |
 *                                       |
 *                           Jump to specialized header
 *
 * ========================================================
 * Details of the generated code
 *
 * Here's what we do in each of the components above:
 *
 *  a) Inits:  one for each IterInit in the group
 *     1. Check that the base matches the group's specialization type.
 *     2. Check that the base has non-zero size. If not, branch to done.
 *        (The done block may differ for IterInits in the same group.)
 *     3. Load and dec-ref the old val output local (and key, if applicable).
 *     4. Initialize the iter's base, type, and end fields.
 *     5. Jump to the header, phi-ing in the initial pos.
 *
 *  b) Header: a single one right before the group's loop entry
 *     1. Load, inc-ref, and store the new val (and key, if applicable)
 *     2. Continue on to the loop entry block
 *
 *  c) Nexts:  one for each IterNext in the group
 *     1. Check that the iter's type matches the group's specialization type
 *     2. Increment the iterator's pos field.
 *     3. Check if the pos is terminal. If it is, branch to done.
 *        (The done block may differ for IterNexts in the same group.)
 *     4. Jump to the footer, phi-ing in the current pos.
 *
 *  d) Footer: a single one that jumps back to the group's header
 *     1. Load and dec-ref the old val output local (and key, if applicable).
 *     2. Check surprise flags and handle the surprise if needed.
 *     3. Jump to the header, phi-ing in the current pos.
 *
 * ========================================================
 * How we do irgen
 *
 * Specializing the same iterator with multiple base types for a given loop
 * causes performance regressions. Additionally, it's unhelpful to specialize
 * an IterInit in a given region without reusing the header for an IterNext.
 *
 * To avoid hitting these pessimal cases, we store a SpecializedIterator struct
 * in IRGS for each iter group, keyed by loop entry block. (As noted above, we
 * still generate correct code if there are other bytecodes that jump into the
 * loop, because our inits and nexts are guarded on checking the base type and
 * the iter type, respectively.)
 *
 * SpecializedIterator has four fields: the specialized `iter_type`, a list of
 * `placeholders`, the shared `header` block, and the shared `footer` block.
 *
 * When we encounter the first IterInit for a given group, we'll initialize
 * this struct, choosing `iter_type` based on ArrayIterProfile profiling.
 * However, we don't know that there's an IterNext in this region yet, so we
 * emit the code behind a JmpPlaceholder and also generate generic code.
 * We store these placeholders in the `placeholders` list. We'll generate the
 * `header` block at this time, but we'll leave the `footer` block null.
 *
 * When we encounter another IterInit, if profiling suggests that we should use
 * the same type, we'll again generate specialized code and hide it behind one
 * of the `placeholders`. However, if profiling suggests that a different type
 * is better for this IterInit, we'll mark the whole group as despecialized.
 *
 * When we encounter an IterNext for a given group, if the group still has a
 * specialized `iter_type`, we'll generate specialized code with that type.
 * If this next is the first one we've seen in this group, we'll also convert
 * the placeholders for the specialized inits into jumps and emit the `footer`.
 *
 * This strategy can produce less performant code if we encounter an IterInit
 * for a group after encountering an IterNext. For instance, we might generate
 * a bunch of specialized code for iterating vecs, and then jump into this loop
 * with a dict. However, this situation is unlikely because of how we form
 * retranslation chains: if the inits are at the same bytecode offset, they're
 * likely to be part of a single chain. If this situation occurs, then we'll
 * still use the specialization if we come in through the earlier inits, but we
 * may side exit if we come in through the later ones.
 */

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_ArrayIterProfile{"ArrayIterProfile"};

//////////////////////////////////////////////////////////////////////
// Accessor for different base types.

// This struct does the iter-type-specific parts of specialized iter code-gen
// so that in the emitSpecialized* functions below, we can simply describe the
// high-level structure of the code.
struct Accessor {
  Accessor(Type arrType, bool isPtrIter)
    : m_arrType(arrType)
    , m_isPtrIter(isPtrIter)
  {}

  virtual ~Accessor() {}

  // Returns whether the iterated array may contain tombstones.
  virtual bool mayContainTombstones() const = 0;

  // Branches to exit if the base doesn't match the iter's specialized type.
  virtual SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const = 0;

  // Get index of the end iteration position. This is different from Count
  // instruction in presence of tombstones.
  // Might be used only when mayContainTombstones() is true.
  virtual SSATmp* getEndIdx(IRGS& env, SSATmp* base) const = 0;

  // Given a base and a logical iter index, this method returns the value that
  // we should use as the iter's pos (e.g. a pointer, for pointer iters).
  //
  // This method assumes that we've already constrained arr to DataTypeSpecific
  // and that the type is an arrType().
  virtual SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const = 0;

  // Given a pos and a constant offset, this method returns an updated pos.
  virtual SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const = 0;

  // Given a base and a pos value, this method returns an "elm value" that we
  // can use to share arithmetic between key and val. (For example, for dict
  // index iters, we compute a pointer that's only valid for this iteration.)
  virtual SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const = 0;

  // Given a base and an "elm value", this method checks whether the elm
  // corresponds to a tombstone. If so, branches to taken.
  // Might be used only when mayContainTombstones() is true.
  virtual void checkTombstone(IRGS& env, SSATmp* arr, SSATmp* elm, Block* taken)
    const = 0;

  Type arrType() const { return m_arrType; }
  bool isPtrIter() const { return m_isPtrIter; }

private:
  const Type m_arrType;
  const bool m_isPtrIter;
};

struct VecAccessor : public Accessor {
  explicit VecAccessor(bool baseConst, bool outputKey)
    : Accessor(
        allowBespokeArrayLikes() ? TVanillaVec : TVec,
        baseConst && !outputKey && VanillaVec::stores_unaligned_typed_values
      )
  {}

  bool mayContainTombstones() const override {
    return false;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    return gen(env, CheckType, exit, arrType(), base);
  }

  SSATmp* getEndIdx(IRGS&, SSATmp*) const override {
    always_assert(false);
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return isPtrIter() ? gen(env, GetVecPtrIter, arr, idx) : idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return pos;
  }

  void checkTombstone(IRGS&, SSATmp*, SSATmp*, Block*) const override {
    always_assert(false);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return isPtrIter()
      ? gen(env, AdvanceVecPtrIter, IterOffsetData{offset}, pos)
      : gen(env, AddInt, cns(env, offset), pos);
  }
};

struct DictAccessor : public Accessor {
  explicit DictAccessor(bool baseConst, ArrayKeyTypes keyTypes)
    : Accessor(allowBespokeArrayLikes() ? TVanillaDict : TDict, baseConst)
    , m_keyTypes(keyTypes) {
    keyTypes.toJitType(m_keyJitType);
  }

  bool mayContainTombstones() const override {
    return m_keyTypes.mayIncludeTombstone();
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    auto const arr = gen(env, CheckType, exit, arrType(), base);
    if (m_keyTypes != ArrayKeyTypes::Any()) {
      auto const data = ArrayKeyTypesData{m_keyTypes};
      gen(env, CheckDictKeys, exit, data, m_keyJitType, arr);
    }
    return arr;
  }

  SSATmp* getEndIdx(IRGS& env, SSATmp* base) const override {
    assertx(mayContainTombstones());
    return gen(env, DictIterEnd, base);
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return isPtrIter() ? gen(env, GetDictPtrIter, arr, idx) : idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return isPtrIter() ? pos : gen(env, GetDictPtrIter, arr, pos);
  }

  void checkTombstone(IRGS& env, SSATmp* arr, SSATmp* elm, Block* taken)
    const override {
    assertx(mayContainTombstones());
    gen(env, CheckPtrIterTombstone, taken, arr, elm);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return isPtrIter()
      ? gen(env, AdvanceDictPtrIter, IterOffsetData{offset}, pos)
      : gen(env, AddInt, cns(env, offset), pos);
  }

private:
  const ArrayKeyTypes m_keyTypes;
  Type m_keyJitType;
};

struct KeysetAccessor : public Accessor {
  explicit KeysetAccessor(bool baseConst)
    : Accessor(allowBespokeArrayLikes() ? TVanillaKeyset : TKeyset, baseConst)
  {}

  bool mayContainTombstones() const override {
    return true;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    return gen(env, CheckType, exit, arrType(), base);
  }

  SSATmp* getEndIdx(IRGS& env, SSATmp* base) const override {
    assertx(mayContainTombstones());
    return gen(env, KeysetIterEnd, base);
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return isPtrIter() ? gen(env, GetKeysetPtrIter, arr, idx) : idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return isPtrIter() ? pos : gen(env, GetKeysetPtrIter, arr, pos);
  }

  void checkTombstone(IRGS& env, SSATmp* arr, SSATmp* elm, Block* taken)
    const override {
    assertx(mayContainTombstones());
    gen(env, CheckPtrIterTombstone, taken, arr, elm);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return isPtrIter()
      ? gen(env, AdvanceKeysetPtrIter, IterOffsetData{offset}, pos)
      : gen(env, AddInt, cns(env, offset), pos);
  }
};

struct BespokeAccessor : public Accessor {
  explicit BespokeAccessor(Type baseType)
    : Accessor(baseType, false)
  {}

  bool mayContainTombstones() const override {
    // checkBase() side-exits if we have tombstones
    return false;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    auto const result = gen(env, CheckType, exit, arrType(), base);
    auto const mayActuallyContainTombstones = [&] {
      // We don't yet support fast iteration over bespoke arrays with
      // tombstones. Currently only the MonotypeDict may contain them.
      if (!RO::EvalEmitBespokeMonotypes) return false;
      if (arrType() <= TVec) return false;
      if (arrType().arrSpec().is_struct()) return false;
      if (arrType().arrSpec().is_type_structure()) return false;
      return true;
    }();
    if (mayActuallyContainTombstones) {
      auto const size = gen(env, Count, result);
      auto const used = gen(env, BespokeIterEnd, result);
      auto const same = gen(env, EqInt, size, used);
      gen(env, JmpZero, exit, same);
    }
    return result;
  }

  SSATmp* getEndIdx(IRGS&, SSATmp*) const override {
    always_assert(false);
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return pos;
  }

  void checkTombstone(IRGS&, SSATmp*, SSATmp*, Block*) const override {
    always_assert(false);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return gen(env, AddInt, pos, cns(env, offset));
  }
};

std::unique_ptr<Accessor> getAccessor(
    DataType baseDT,
    ArrayKeyTypes keyTypes,
    ArrayLayout layout,
    const IterArgs& data
) {
  if (!layout.vanilla()) {
    auto const baseType = Type(baseDT).narrowToLayout(layout);
    return std::make_unique<BespokeAccessor>(baseType);
  }

  auto const baseConst = has_flag(data.flags, IterArgs::Flags::BaseConst);
  auto const withKeys = has_flag(data.flags, IterArgs::Flags::WithKeys);
  switch (baseDT) {
    case KindOfVec:
      return std::make_unique<VecAccessor>(baseConst, withKeys);
    case KindOfDict:
      return std::make_unique<DictAccessor>(baseConst, keyTypes);
    case KindOfKeyset:
      return std::make_unique<KeysetAccessor>(baseConst);
    default:
      always_assert(false);
  }
}

//////////////////////////////////////////////////////////////////////
// Specialization helpers.

// When ifThen creates new blocks, it assigns them a profCount of curProfCount.
// curProfCount is based the bytecode we're generating code for: e.g. a
// particular IterInit or IterNext in an iter group.
//
// However, during code-gen for IterInit, we may also create the header, and
// during code-gen for IterNext, we may also create the footer. These blocks
// are shared and so have higher weight than curProfCount. We initialize their
// count correctly when we create the header and footer entry Block*, so we
// just have to propagate that incoming count forward when we do an ifThen.
template<class Branch, class Taken>
void iterIfThen(IRGS& env, Branch branch, Taken taken) {
  auto const count = env.irb->curBlock()->profCount();
  ifThen(env, branch, [&]{
    hint(env, Block::Hint::Unlikely);
    env.irb->curBlock()->setProfCount(count);
    taken();
  });
  env.irb->curBlock()->setProfCount(count);
}

// Convert an iterator position to an integer representation.
SSATmp* posAsInt(IRGS& env, const Accessor& accessor, SSATmp* pos) {
  if (!accessor.isPtrIter()) return pos;
  return gen(env, PtrToElemAsInt, pos);
}

// It's important that we dec-ref the old values before the surprise check;
// refcount-opts may fail to pair incs and decs separated by this check.
//
// However, if we do that, then when we interpret IterNext in surprise cases,
// we'll dec-ref the values again. We solve this issue by storing nulls to the
// output locals here, making the dec-refs in the surprise cases a no-op.
//
// Likewise, we really want the old position to be dead at this point so that
// register allocation can update the position in place. However, if we don't
// do anything here, store-elim is liable to push StIterPos into the exit path
// and vasm-copy is liable to realize that it can re-use the old position tmp
// instead of recomputing it. We want to increment the position register in
// place in the main flow, so we recompute the tmp here instead.
void iterSurpriseCheck(IRGS& env, const Accessor& accessor,
                       const IterArgs& data, SSATmp* pos) {
  iterIfThen(env,
    [&](Block* taken) {
      gen(env, CheckSurpriseFlags, taken, anyStackRegister(env));
    },
    [&]{
      auto const old = accessor.advancePos(env, pos, -1);
      gen(env, StIterPos, IterId(data.iterId), fp(env),
          posAsInt(env, accessor, old));
      gen(env, Jmp, makeExitSlow(env));
    }
  );
}

// At the start of each shared specialized block (header or footer), we must
// clear FrameState (we'll reuse the block) and phi in the new pos value.
SSATmp* phiIterPos(IRGS& env, const Accessor& accessor) {
  env.irb->fs().clearForUnprocessedPred();
  auto block = env.irb->curBlock();
  auto const label = env.unit.defLabel(1, block, env.irb->nextBCContext());
  auto const pos = label->dst(0);
  pos->setType(accessor.isPtrIter() ? TPtrToElem : TInt);
  return pos;
}

//////////////////////////////////////////////////////////////////////
// Specialization implementations: init, header, next, and footer.

void emitSpecializedInit(IRGS& env, const Accessor& accessor,
                         const IterArgs& data, Block* header,
                         Offset doneOffset, SSATmp* base) {
  // We don't need to specialize on key type for value-only iterators.
  // However, we still need to call accessor.check to rule out tombstones.
  auto const arr = accessor.checkBase(env, base, makeExit(env));
  auto const size = gen(env, Count, arr);

  ifThen(env,
    [&](Block* taken) { gen(env, JmpZero, taken, size); },
    [&]{ gen(env, Jmp, getBlock(env, doneOffset)); }
  );

  auto const id = IterId(data.iterId);
  auto const endIdx = accessor.mayContainTombstones()
    ? accessor.getEndIdx(env, arr)
    : size;
  auto const endPos = accessor.getPos(env, arr, endIdx);
  gen(env, StIterEnd, id, fp(env), posAsInt(env, accessor, endPos));

  auto const beginPos = accessor.getPos(env, arr, cns(env, 0));
  if (accessor.mayContainTombstones()) {
    auto const next = defBlock(env);
    gen(env, Jmp, next, beginPos);

    env.irb->appendBlock(next);
    auto const pos = phiIterPos(env, accessor);
    auto const elm = accessor.getElm(env, arr, pos);
    iterIfThen(
      env,
      [&](Block* taken) { accessor.checkTombstone(env, arr, elm, taken); },
      [&] {
        auto const nextPos = accessor.advancePos(env, pos, 1);
        gen(env, Jmp, next, nextPos);
      }
    );
    gen(env, Jmp, header, pos);
  } else {
    gen(env, Jmp, header, beginPos);
  }
}

Block* emitSpecializedHeader(IRGS& env, const Accessor& accessor,
                             const IterArgs& data, const Type& value_type,
                             SrcKey bodySk, uint32_t baseLocalId) {
  auto const bodyProfCount = env.irb->hasBlock(bodySk)
    ? getBlock(env, bodySk)->profCount()
    : curProfCount(env);
  auto const header = env.unit.defBlock(bodyProfCount);
  BlockPusher bp(*env.irb, env.irb->curMarker(), header);

  auto const pos = phiIterPos(env, accessor);
  gen(env, AssertLoc, accessor.arrType(), LocalId(baseLocalId), fp(env));

  gen(env, StIterPos, IterId(data.iterId), fp(env),
      posAsInt(env, accessor, pos));
  gen(env, Jmp, getBlock(env, bodySk));

  return header;
}

void emitSpecializedNext(IRGS& env, const Accessor& accessor,
                         const IterArgs& data, Block* footer,
                         SSATmp* base) {
  base = accessor.checkBase(env, base, makeExit(env));

  auto const asIterPosType = [&](SSATmp* iterPos) {
    if (!accessor.isPtrIter()) return iterPos;
    return gen(env, IntAsPtrToElem, iterPos);
  };

  auto const id = IterId(data.iterId);
  auto const old = asIterPosType(gen(env, LdIterPos, id, fp(env)));
  auto const end = asIterPosType(gen(env, LdIterEnd, id, fp(env)));

  auto const done = defBlock(env);
  auto const checkDone = [&] (SSATmp* pos) {
    auto const eq = accessor.isPtrIter() ? EqPtrIter : EqInt;
    gen(env, JmpNZero, done, gen(env, eq, pos, end));
  };

  if (accessor.mayContainTombstones()) {
    auto const next = defBlock(env);
    gen(env, Jmp, next, old);

    env.irb->appendBlock(next);
    auto const phi = phiIterPos(env, accessor);
    auto const cur = accessor.advancePos(env, phi, 1);
    checkDone(cur);

    auto const elm = accessor.getElm(env, base, cur);
    iterIfThen(
      env,
      [&](Block* taken) { accessor.checkTombstone(env, base, elm, taken); },
      [&] { gen(env, Jmp, next, cur); }
    );

    gen(env, Jmp, footer, cur);
  } else {
    auto const cur = accessor.advancePos(env, old, 1);
    checkDone(cur);
    gen(env, Jmp, footer, cur);
  }

  env.irb->appendBlock(done);
  auto const nextProfCount = env.irb->hasBlock(nextSrcKey(env))
    ? getBlock(env, nextSrcKey(env))->profCount()
    : curProfCount(env);
  env.irb->curBlock()->setProfCount(nextProfCount);
  gen(env, KillIter, id, fp(env));
}

Block* emitSpecializedFooter(IRGS& env, const Accessor& accessor,
                           const IterArgs& data, Block* header) {
  auto const footer = env.unit.defBlock(env.irb->curBlock()->profCount());
  BlockPusher bp{*env.irb, env.irb->curMarker(), footer};

  auto const pos = phiIterPos(env, accessor);
  iterSurpriseCheck(env, accessor, data, pos);
  gen(env, Jmp, header, pos);

  return footer;
}

ArrayLayout getBaseLayout(SSATmp* base) {
  auto const baseDT = dt_modulo_persistence(base->type().toDataType());
  if (!allowBespokeArrayLikes()) return ArrayLayout::Vanilla();
  if (!arrayTypeCouldBeBespoke(baseDT)) return ArrayLayout::Vanilla();
  return base->type().arrSpec().layout();
}

//////////////////////////////////////////////////////////////////////

}  // namespace

//////////////////////////////////////////////////////////////////////
// The public API for iterator specialization.

// Generate specialized code for this IterInit. Returns true on success.
bool specializeIterInit(IRGS& env, Offset doneOffset,
                        const IterArgs& data, SSATmp* base,
                        uint32_t baseLocalId,
                        ArrayIterProfile::Result profiledResult) {
  assertx(base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj));
  if (base->isA(TObj)) return false;

  auto const bodySk = nextSrcKey(env);
  auto const bodyKey = env.irb->hasBlock(bodySk)
    ? getBlock(env, bodySk)
    : nullptr;
  auto const keyTypes = profiledResult.key_types;
  auto const layout = getBaseLayout(base);

  FTRACE(2, "Trying to specialize IterInit: {} @ {}\n",
         keyTypes.show(), layout.describe());
  if (!layout.vanilla() && !layout.monotype() && !layout.is_struct()) {
    FTRACE(2, "Failure: not a vanilla, monotype, or struct layout.\n");
    return false;
  }

  // We're committing to the specialization.
  TRACE(2, "Success! Generating specialized code.\n");

  // If the body block is part of this translation, record the first profile
  // for this DataType specialization. We should almost always have just one,
  // but in case regionizeFunc() did something weird and produced multiple
  // IterInits of the same base DataType pointing to the same body block,
  // the first one likely has the highest weight, so we pick that one as
  // a hint for IterNext.
  auto const baseDT = dt_modulo_persistence(base->type().toDataType());
  if (bodyKey != nullptr) {
    auto const iterProfileKey = std::make_pair(bodyKey, baseDT);
    env.iterProfiles.emplace(iterProfileKey, IterProfileInfo{layout, keyTypes});
  }

  auto const accessor = getAccessor(baseDT, keyTypes, layout, data);
  assertx(base->type().maybe(accessor->arrType()));

  auto const header = [&] {
    auto const emitHeader = [&] {
      return emitSpecializedHeader(
        env, *accessor, data, profiledResult.value_type, bodySk, baseLocalId);
    };

    if (bodyKey == nullptr) return emitHeader();

    auto const iterKey = std::make_tuple(
      bodyKey, baseDT, layout.toUint16(), keyTypes.toBits());
    if (env.iters.contains(iterKey)) return env.iters[iterKey].header;

    auto const res = emitHeader();
    env.iters.emplace(iterKey, IterSpecInfo{res, nullptr});
    return res;
  }();

  emitSpecializedInit(env, *accessor, data, header, doneOffset, base);
  return true;
}

// `baseLocalId` is only valid for local iters. Returns true on specialization.
bool specializeIterNext(IRGS& env, Offset bodyOffset,
                        const IterArgs& data, SSATmp* base,
                        uint32_t baseLocalId) {
  assertx(base->type().subtypeOfAny(TVec, TDict, TKeyset, TObj));
  if (base->isA(TObj)) return false;

  auto const bodySk = SrcKey{curSrcKey(env), bodyOffset};
  auto const bodyKey = env.irb->hasBlock(bodySk)
    ? getBlock(env, bodySk)
    : nullptr;
  auto const baseDT = dt_modulo_persistence(base->type().toDataType());
  auto const iterProfileKey = std::make_pair(bodyKey, baseDT);
  auto const it = env.iterProfiles.find(iterProfileKey);
  auto const keyTypes = it != env.iterProfiles.end()
    ? it->second.keyTypes
    : baseDT == KindOfVec ? ArrayKeyTypes::Ints() : ArrayKeyTypes::ArrayKeys();
  auto const layout = [&] {
    if (it != env.iterProfiles.end()) {
      // If IterInit provided a profiling hint and it doesn't contradict what
      // we know about the type, use it.
      auto const l = it->second.layout & base->type().arrSpec().layout();
      if (l != ArrayLayout::Bottom()) return l;
    }
    return getBaseLayout(base);
  }();

  FTRACE(2, "Trying to specialize IterNext: {} @ {}\n",
         keyTypes.show(), layout.describe());
  if (!layout.vanilla() && !layout.monotype() && !layout.is_struct()) {
    FTRACE(2, "Failure: not a vanilla, monotype, or struct layout.\n");
    return false;
  }

  auto const accessor = getAccessor(baseDT, keyTypes, layout, data);
  assertx(base->type().maybe(accessor->arrType()));

  auto iterSpecInfoUncached = IterSpecInfo{nullptr, nullptr};
  auto& iterSpecInfo = [&]() -> IterSpecInfo& {
    if (bodyKey == nullptr) return iterSpecInfoUncached;
    auto const iterKey = std::make_tuple(
      bodyKey, baseDT, layout.toUint16(), keyTypes.toBits());
    return env.iters[iterKey];
  }();
  auto const header = [&] {
    if (iterSpecInfo.header != nullptr) return iterSpecInfo.header;
    auto const res = emitSpecializedHeader(
      env, *accessor, data, TInitCell, bodySk, baseLocalId);
    iterSpecInfo.header = res;
    return res;
  }();
  auto const footer = [&] {
    if (iterSpecInfo.footer != nullptr) return iterSpecInfo.footer;
    auto const res = emitSpecializedFooter(env, *accessor, data, header);
    iterSpecInfo.footer = res;
    return res;
  }();

  emitSpecializedNext(env, *accessor, data, footer, base);
  return true;
}

//////////////////////////////////////////////////////////////////////

}
