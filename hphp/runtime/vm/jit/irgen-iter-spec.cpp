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
 *  - All ops have the same "type" (NonLocal, LocalBaseConst, LocalBaseMutable)
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
// Simple getters for IterSpecialization.

Type getArrType(IterSpecialization specialization) {
  switch (specialization.base_type) {
    case IterSpecialization::Vec:    return TVec;
    case IterSpecialization::Dict:   return TDict;
  }
  always_assert(false);
}

Type getKeyType(IterSpecialization specialization) {
  switch (specialization.key_types) {
    case IterSpecialization::ArrayKey:  return TInt | TStr;
    case IterSpecialization::Int:       return TInt;
    case IterSpecialization::Str:       return TStr;
    case IterSpecialization::StaticStr: return TStaticStr;
  }
  always_assert(false);
}

ArrayIterProfile::Result getProfileResult(IRGS& env, const SSATmp* base) {
  auto const generic = ArrayIterProfile::Result{};
  assertx(!generic.top_specialization.specialized);

  auto const profile = TargetProfile<ArrayIterProfile>(
    env.context,
    makeMarker(env, curSrcKey(env)),
    s_ArrayIterProfile.get()
  );
  if (!profile.optimizing()) return generic;
  auto const result = profile.data().result();
  if (result.num_arrays == 0) return generic;

  auto const rate = 1.0 * size_t{result.top_count} / size_t{result.num_arrays};
  return rate > RO::EvalArrayIterSpecializationRate ? result : generic;
}

//////////////////////////////////////////////////////////////////////
// Accessor for different base types.

// This struct does the iter-type-specific parts of specialized iter code-gen
// so that in the emitSpecialized* functions below, we can simply describe the
// high-level structure of the code.
struct Accessor {
  Type arr_type;
  Type pos_type;
  IterSpecialization iter_type;
  ArrayLayout layout = ArrayLayout::Top();

  virtual ~Accessor() {}

  // Branches to exit if the base doesn't match the iter's specialized type.
  virtual SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const = 0;

  // Given a base and a logical iter index, this method returns the value that
  // we should use as the iter's pos (e.g. a pointer, for pointer iters).
  //
  // This method assumes that we've already constrained arr to DataTypeSpecific
  // and that the type of arr overlaps with arr_type.
  virtual SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const = 0;

  // Given a pos and a constant offset, this method returns an updated pos.
  virtual SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const = 0;

  // Given a base and a pos value, this method returns an "elm value" that we
  // can use to share arithmetic between key and val. (For example, for dict
  // index iters, we compute a pointer that's only valid for this iteration.)
  virtual SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const = 0;

  // Given a base and an "elm value", this method returns the key of that elm.
  virtual SSATmp* getKey(IRGS& env, SSATmp* arr, SSATmp* elm) const = 0;

  // Given a base and an "elm value", this method returns the val of that elm.
  virtual SSATmp* getVal(IRGS& env, SSATmp* arr, SSATmp* elm) const = 0;
};

struct VecAccessor : public Accessor {
  explicit VecAccessor(
    IterSpecialization specialization,
    bool baseConst,
    bool outputKey
  ) {
    is_ptr_iter =
      baseConst && !outputKey && VanillaVec::stores_unaligned_typed_values;
    arr_type = getArrType(specialization);
    if (allowBespokeArrayLikes()) {
      arr_type = arr_type.narrowToVanilla();
    }
    pos_type = is_ptr_iter ? TPtrToElem : TInt;
    layout = ArrayLayout::Vanilla();
    iter_type = specialization;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    return gen(env, CheckType, exit, arr_type, base);
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return is_ptr_iter ? gen(env, GetVecPtrIter, arr, idx) : idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return pos;
  }

  SSATmp* getKey(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    // is_ptr_iter is only true when the iterator doesn't output a key,
    // and this method is only called when the iterator *does* output a key.
    always_assert(!is_ptr_iter);
    return elm;
  }

  SSATmp* getVal(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    if (is_ptr_iter) {
      auto const valType = arrLikeElemType(
        arr_type,
        TInt,
        curClass(env)
      ).first;
      return gen(env, LdPtrIterVal, valType, elm);
    }
    return gen(env, LdVecElem, arr, elm);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return is_ptr_iter
      ? gen(env, AdvanceVecPtrIter, IterOffsetData{offset}, pos)
      : gen(env, AddInt, cns(env, offset), pos);
  }

  private:
    bool is_ptr_iter = false;
};

struct MixedAccessor : public Accessor {
  explicit MixedAccessor(IterSpecialization specialization, bool baseConst) {
    is_ptr_iter = baseConst;
    arr_type = getArrType(specialization);
    if (allowBespokeArrayLikes()) {
      arr_type = arr_type.narrowToVanilla();
    }
    pos_type = is_ptr_iter ? TPtrToElem : TInt;
    key_type = getKeyType(specialization);
    layout = ArrayLayout::Vanilla();
    iter_type = specialization;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    auto const arr = gen(env, CheckType, exit, arr_type, base);
    gen(env, CheckDictKeys, exit, key_type, arr);
    return arr;
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return is_ptr_iter ? gen(env, GetDictPtrIter, arr, idx) : idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return is_ptr_iter ? pos : gen(env, GetDictPtrIter, arr, pos);
  }

  SSATmp* getKey(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    return gen(env, LdPtrIterKey, key_type, elm);
  }

  SSATmp* getVal(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    auto const valType = arrLikeElemType(
      is_ptr_iter ? arr_type : arr->type(),
      key_type,
      curClass(env)
    ).first;
    return gen(env, LdPtrIterVal, valType, elm);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return is_ptr_iter
      ? gen(env, AdvanceDictPtrIter, IterOffsetData{offset}, pos)
      : gen(env, AddInt, cns(env, offset), pos);
  }

private:
  bool is_ptr_iter = false;
  Type key_type;
};

struct BespokeAccessor : public Accessor {
  explicit BespokeAccessor(
      IterSpecialization specialization, ArrayLayout layout) {
    arr_type = getArrType(specialization).narrowToLayout(layout);
    pos_type = TInt;
    iter_type = specialization;
    this->layout = layout;
  }

  SSATmp* checkBase(IRGS& env, SSATmp* base, Block* exit) const override {
    auto const result = gen(env, CheckType, exit, arr_type, base);
    if (result->isA(TDict)) {
      auto const size = gen(env, Count, result);
      auto const used = gen(env, BespokeIterEnd, result);
      auto const same = gen(env, EqInt, size, used);
      gen(env, JmpZero, exit, same);
    }
    return result;
  }

  SSATmp* getPos(IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return idx;
  }

  SSATmp* getElm(IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return pos;
  }

  SSATmp* getKey(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    return gen(env, BespokeIterGetKey, arr, elm);
  }

  SSATmp* getVal(IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    return gen(env, BespokeIterGetVal, arr, elm);
  }

  SSATmp* advancePos(IRGS& env, SSATmp* pos, int16_t offset) const override {
    return gen(env, AddInt, pos, cns(env, offset));
  }
};

std::unique_ptr<Accessor> getAccessor(
    IterSpecialization type,
    ArrayLayout layout,
    const IterArgs& data,
    bool local
) {
  if (!layout.vanilla()) {
    return std::make_unique<BespokeAccessor>(type, layout);
  }
  auto const baseConst = !local || (data.flags & IterArgs::Flags::BaseConst);
  switch (type.base_type) {
    case IterSpecialization::Vec: {
      return std::make_unique<VecAccessor>(type, baseConst, data.hasKey());
    }
    case IterSpecialization::Dict: {
      return std::make_unique<MixedAccessor>(type, baseConst);
    }
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////
// Specialization helpers.

// Load the iterator base, either from the iterator itself or from a local.
// This method may only be called from one of the guarded specialized blocks,
// so we can assert that the type of the base matches the iterator type.
SSATmp* iterBase(IRGS& env, const Accessor& accessor,
                 const IterArgs& data, uint32_t baseLocalId) {
  auto const type = accessor.arr_type;
  auto const local = baseLocalId != kInvalidId;
  if (!local) return gen(env, LdIterBase, type, IterId(data.iterId), fp(env));
  gen(env, AssertLoc, type, LocalId(baseLocalId), fp(env));
  return gen(env, LdLoc, type, LocalId(baseLocalId), fp(env));
}

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

// Clear the value in an iterator's output local. Since we never specialize
// iterators in pseudo-mains, we can use LdLoc here without a problem.
void iterClear(IRGS& env, uint32_t local) {
  assertx(local != kInvalidId);
  decRef(
      env,
      gen(env, LdLoc, TCell, LocalId(local), fp(env)),
      static_cast<DecRefProfileId>(local)
  );
}

// Iterator output locals should always be cells, so we can store without
// needing to create an exit here. We check the cell invariant in debug mode.
void iterStore(IRGS& env, uint32_t local, SSATmp* cell) {
  assertx(cell->type() <= TCell);
  assertx(local != kInvalidId);
  gen(env, StLoc, LocalId(local), fp(env), cell);
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
      iterStore(env, data.valId, cns(env, TInitNull));
      if (data.hasKey()) iterStore(env, data.keyId, cns(env, TInitNull));
      auto const old = accessor.advancePos(env, pos, -1);
      gen(env, StIterPos, IterId(data.iterId), fp(env), old);
      gen(env, Jmp, makeExitSlow(env));
    }
  );
}

// In profiling mode, we profile the dec-ref of the iterator output locals
// before calling the generic native helper, so that if we specialize we'll
// produce good code for them. `init` is true for *IterInit*, not *IterNext*.
void profileDecRefs(IRGS& env, const IterArgs& data, SSATmp* base,
                    const bool local, const bool init) {
  if (env.context.kind != TransKind::Profile) return;

  // We could profile the iterator's base for IterNext here, too, but loading
  // the value out of the base is tricky and it doesn't affect perf measurably.
  if (!local) {
    always_assert(init == (base != nullptr));
    if (init) gen(env, ProfileDecRef, DecRefData(), base);
  }

  auto const val = gen(env, LdLoc, TCell, LocalId(data.valId), fp(env));
  gen(env, ProfileDecRef, DecRefData(data.valId), val);
  if (!data.hasKey()) return;

  // If we haven't already guarded on the type of a string key, and it may or
  // may not be persistent, force a guard on it now. We'll lose the persistent-
  // vs-not distinction when specializing, but a DecRefProfile for a generic
  // type like TInitCell would capture this distinction.
  auto const key = [&]{
    auto const tmp = gen(env, LdLoc, TCell, LocalId(data.keyId), fp(env));
    return init ? tmp : cond(env,
      [&](Block* taken) { return gen(env, CheckType, TStr, taken, tmp); },
      [&](SSATmp* str)  { return str; },
      [&]               { return tmp; });
  }();
  gen(env, ProfileDecRef, DecRefData(data.keyId), key);
}

// At the start of each shared specialized block (header or footer), we must
// clear FrameState (we'll reuse the block) and phi in the new pos value.
SSATmp* phiIterPos(IRGS& env, const Accessor& accessor) {
  env.irb->fs().clearForUnprocessedPred();
  auto block = env.irb->curBlock();
  auto const label = env.unit.defLabel(1, block, env.irb->nextBCContext());
  auto const pos = label->dst(0);
  pos->setType(accessor.pos_type);
  return pos;
}

//////////////////////////////////////////////////////////////////////
// Specialization implementations: init, header, next, and footer.

void emitSpecializedInit(IRGS& env, const Accessor& accessor,
                         const IterArgs& data, bool local, Block* header,
                         Block* done, SSATmp* base) {
  auto const arr = accessor.checkBase(env, base, makeExitSlow(env));
  auto const size = gen(env, Count, arr);
  if (!local) discard(env, 1);

  ifThen(env,
    [&](Block* taken) { gen(env, JmpZero, taken, size); },
    [&]{
      if (!local) decRef(env, arr);
      gen(env, Jmp, done);
    }
  );

  iterClear(env, data.valId);
  if (data.hasKey()) iterClear(env, data.keyId);

  auto const id = IterId(data.iterId);
  auto const baseConst = !local || (data.flags & IterArgs::Flags::BaseConst);
  auto const ty = IterTypeData(
    data.iterId, accessor.iter_type, accessor.layout, baseConst, data.hasKey());
  gen(env, StIterBase, id, fp(env), local ? cns(env, nullptr) : arr);
  gen(env, StIterType, ty, fp(env));
  gen(env, StIterEnd,  id, fp(env), accessor.getPos(env, arr, size));
  gen(env, Jmp, header, accessor.getPos(env, arr, cns(env, 0)));
}

void emitSpecializedHeader(IRGS& env, const Accessor& accessor,
                           const IterArgs& data, const Type& value_type,
                           Block* body, uint32_t baseLocalId) {
  auto const pos = phiIterPos(env, accessor);
  auto const arr = accessor.pos_type <= TInt
    ? iterBase(env, accessor, data, baseLocalId)
    : nullptr;

  auto const finish = [&](SSATmp* elm, SSATmp* val) {
    auto const keyed = data.hasKey();
    auto const key = keyed ? accessor.getKey(env, arr, elm) : nullptr;
    iterStore(env, data.valId, val);
    if (keyed) iterStore(env, data.keyId, key);
    gen(env, StIterPos, IterId(data.iterId), fp(env), pos);
    gen(env, IncRef, val);
    if (keyed) gen(env, IncRef, key);
  };

  auto guarded_val = (SSATmp*)nullptr;
  auto const elm = accessor.getElm(env, arr, pos);
  auto const val = accessor.getVal(env, arr, elm);
  auto const guardable = relaxToGuardable(value_type);
  always_assert(guardable <= TCell);

  iterIfThen(env,
    [&](Block* taken) {
      guarded_val = gen(env, CheckType, guardable, taken, val);
    },
    [&]{
      finish(elm, val);
      gen(env, Jmp, makeExit(env, nextSrcKey(env)));
    }
  );
  finish(elm, guarded_val);
  gen(env, Jmp, body);
}

void emitSpecializedNext(IRGS& env, const Accessor& accessor,
                         const IterArgs& data, Block* footer,
                         uint32_t baseLocalId) {
  auto const exit = makeExitSlow(env);
  auto const local = baseLocalId != kInvalidId;
  auto const baseConst = !local || (data.flags & IterArgs::Flags::BaseConst);
  auto const type = IterTypeData(
    data.iterId, accessor.iter_type, accessor.layout, baseConst, data.hasKey());
  gen(env, CheckIter, exit, type, fp(env));

  // For LocalBaseMutable iterators specialized on bespoke array-like bases,
  // we unfortunately need to check the layout again at each IterNext.
  if (!baseConst && type.type.bespoke) {
    auto const dt = accessor.arr_type.unspecialize();
    gen(env, AssertLoc, dt, LocalId(baseLocalId), fp(env));
    auto const base = gen(env, LdLoc, dt, LocalId(baseLocalId), fp(env));
    gen(env, CheckType, exit, accessor.arr_type, base);
  }

  auto const id = IterId(data.iterId);
  auto const old = gen(env, LdIterPos, accessor.pos_type, id, fp(env));
  auto const end = gen(env, LdIterEnd, accessor.pos_type, id, fp(env));
  auto const pos = accessor.advancePos(env, old, 1);

  ifThen(env,
    [&](Block* taken) {
      auto const eq = accessor.pos_type <= TInt ? EqInt : EqPtrIter;
      auto const done = gen(env, eq, pos, end);
      gen(env, JmpNZero, taken, done);
      gen(env, Jmp, footer, pos);
    },
    [&]{
      auto const next = getBlock(env, nextSrcKey(env));
      env.irb->curBlock()->setProfCount(next->profCount());

      if (local) {
        gen(env, KillIter, id, fp(env));
      } else {
        // Load and dec-ref the base for non-local iters. We don't want to do
        // this load in the loop, because it's dead there for pointer iters,
        auto const base = iterBase(env, accessor, data, baseLocalId);
        gen(env, KillIter, id, fp(env));
        decRef(env, base);
      }
    }
  );
}

void emitSpecializedFooter(IRGS& env, const Accessor& accessor,
                           const IterArgs& data, Block* header) {
  auto const pos = phiIterPos(env, accessor);
  iterClear(env, data.valId);
  if (data.hasKey()) iterClear(env, data.keyId);
  iterSurpriseCheck(env, accessor, data, pos);
  gen(env, Jmp, header, pos);
}

//////////////////////////////////////////////////////////////////////

}  // namespace

//////////////////////////////////////////////////////////////////////
// The public API for iterator specialization.

// Speculatively generate specialized code for this IterInit.
void specializeIterInit(IRGS& env, Offset doneOffset,
                        const IterArgs& data, uint32_t baseLocalId) {
  auto const local = baseLocalId != kInvalidId;
  auto const base = local ? ldLoc(env, baseLocalId, DataTypeIterBase)
                          : topC(env, BCSPRelOffset{0}, DataTypeIterBase);
  profileDecRefs(env, data, base, local, /*init=*/true);

  // `body` and `done` are at a different stack depth for non-local IterInits.
  if (!local) env.irb->fs().decBCSPDepth();
  auto const body = getBlock(env, nextSrcKey(env));
  auto const done = getBlock(env, bcOff(env) + doneOffset);
  if (!local) env.irb->fs().incBCSPDepth();
  auto const iter = env.iters.contains(body) ? env.iters[body].get() : nullptr;

  // We mark this iter group as being despecialized if we fail to specialize.
  auto const despecialize = [&]{
    if (iter == nullptr) {
      auto const def = SpecializedIterator{
          ArrayLayout::Top(), IterSpecialization::generic()};
      env.iters[body] = std::make_unique<SpecializedIterator>(def);
    } else {
      iter->iter_type = IterSpecialization::generic();
    }
    assertx(!env.iters[body]->iter_type.specialized);
  };

  // We don't need to specialize on key type for value-only iterators.
  // However, we still need to call accessor.check to rule out tombstones.
  auto result = getProfileResult(env, base);
  auto& iter_type = result.top_specialization;

  // Use bespoke profiling (if enabled) to choose a layout.
  auto const layout = [&]{
    if (!allowBespokeArrayLikes()) return ArrayLayout::Vanilla();
    auto const dt = getArrType(iter_type).toDataType();
    if (!arrayTypeCouldBeBespoke(dt)) return ArrayLayout::Vanilla();
    auto const sl = bespoke::layoutsForSink(env.profTransIDs, curSrcKey(env));
    assertx(sl.layouts.size() == 1);
    return sl.sideExit ? sl.layouts[0].layout : ArrayLayout::Top();
  }();
  iter_type.bespoke = layout.bespoke();
  auto const accessor = getAccessor(iter_type, layout, data, local);

  // Check all the conditions for iterator specialization, with logging.
  FTRACE(2, "Trying to specialize IterInit: {} @ {}\n",
         show(iter_type), layout.describe());
  if (!iter_type.specialized) {
    FTRACE(2, "Failure: no profiled specialization.\n");
    return despecialize();
  } else if (!layout.vanilla() && !layout.monotype() && !layout.is_struct()) {
    FTRACE(2, "Failure: not a vanilla, monotype, or struct layout.\n");
    return despecialize();
  } else if (iter && iter->iter_type.as_byte != iter_type.as_byte) {
    FTRACE(2, "Failure: specialization mismatch: {}\n", show(iter->iter_type));
    return despecialize();
  } else if (iter && iter->layout != layout) {
    FTRACE(2, "Failure: layout mismatch: {}\n", layout.describe());
    return despecialize();
  } else if (!base->type().maybe(accessor->arr_type)) {
    FTRACE(2, "Failure: incoming type mismatch: {}\n", base->type());
    return despecialize();
  }
  TRACE(2, "Success! Generating specialized code.\n");

  // We're committing to the specialization. Hide the specialized code behind
  // a placeholder so that we won't use it unless we also specialize the next.
  auto const main = env.unit.defBlock(env.irb->curBlock()->profCount());
  auto const init = env.unit.defBlock(env.irb->curBlock()->profCount());
  gen(env, JmpPlaceholder, init);
  auto const inst = &env.irb->curBlock()->back();
  always_assert(inst->is(JmpPlaceholder));
  gen(env, Jmp, main);

  env.irb->appendBlock(init);
  auto const header = iter == nullptr ? env.unit.defBlock(body->profCount())
                                      : iter->header;
  emitSpecializedInit(env, *accessor, data, local, header, done, base);

  if (iter != nullptr) {
    iter->placeholders.push_back(inst);
  } else {
    env.irb->appendBlock(header);
    auto const& value_type = result.value_type;
    emitSpecializedHeader(env, *accessor, data, value_type, body, baseLocalId);
    auto const def = SpecializedIterator{
        layout, iter_type, {inst}, header, nullptr};
    env.iters[body] = std::make_unique<SpecializedIterator>(def);
  }

  env.irb->appendBlock(main);
}

// `baseLocalId` is only valid for local iters. Returns true on specialization.
bool specializeIterNext(IRGS& env, Offset loopOffset,
                        const IterArgs& data, uint32_t baseLocalId) {
  auto const local = baseLocalId != kInvalidId;
  profileDecRefs(env, data, nullptr, local, /*init=*/false);

  auto const body = getBlock(env, bcOff(env) + loopOffset);
  if (!env.iters.contains(body)) return false;

  auto const iter = env.iters[body].get();
  if (!iter->iter_type.specialized) return false;
  auto const accessor =
    getAccessor(iter->iter_type, iter->layout, data, local);
  if (baseLocalId != kInvalidId) {
    auto const type = env.irb->fs().local(baseLocalId).type;
    if (!type.maybe(accessor->arr_type)) return false;
  }

  // We're committing to specialization for this loop. Replace the placeholders
  // for the inits with uncondition jumps into the specialized code.
  for (auto inst : iter->placeholders) {
    assertx(inst->is(JmpPlaceholder));
    env.unit.replace(inst, Jmp, inst->taken());
  }
  iter->placeholders.clear();

  assertx(iter->header != nullptr);
  auto const footer = iter->footer == nullptr
    ? env.unit.defBlock(env.irb->curBlock()->profCount())
    : iter->footer;
  emitSpecializedNext(env, *accessor, data, footer, baseLocalId);
  if (iter->footer == nullptr) {
    BlockPusher pushBlock{*env.irb, env.irb->curMarker(), footer};
    emitSpecializedFooter(env, *accessor, data, iter->header);
    iter->footer = footer;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

}
