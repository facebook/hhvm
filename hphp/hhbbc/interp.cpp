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
#include "hphp/hhbbc/interp.h"

#include <algorithm>
#include <vector>
#include <string>
#include <iterator>

#include <folly/Optional.h>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/runtime/ext/hh/ext_hh.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-ops.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

#include "hphp/hhbbc/interp-internal.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_PHP_Incomplete_Class("__PHP_Incomplete_Class");
const StaticString s_IMemoizeParam("HH\\IMemoizeParam");
const StaticString s_getInstanceKey("getInstanceKey");
const StaticString s_Closure("Closure");
const StaticString s_this("HH\\this");

bool poppable(Op op) {
  switch (op) {
    case Op::Dup:
    case Op::Null:
    case Op::False:
    case Op::True:
    case Op::Int:
    case Op::Double:
    case Op::String:
    case Op::Array:
    case Op::Vec:
    case Op::Dict:
    case Op::Keyset:
    case Op::NewArray:
    case Op::NewDArray:
    case Op::NewMixedArray:
    case Op::NewDictArray:
    case Op::NewLikeArrayL:
    case Op::NewCol:
      return true;
    default:
      return false;
  }
}

void interpStep(ISS& env, const Bytecode& bc);

void record(ISS& env, const Bytecode& bc) {
  if (bc.srcLoc != env.srcLoc) {
    Bytecode tmp = bc;
    tmp.srcLoc = env.srcLoc;
    return record(env, tmp);
  }

  if (!env.replacedBcs.size() &&
      env.unchangedBcs < env.blk.hhbcs.size() &&
      bc == env.blk.hhbcs[env.unchangedBcs]) {
    env.unchangedBcs++;
    return;
  }

  ITRACE(2, "  => {}\n", show(env.ctx.func, bc));
  env.replacedBcs.push_back(bc);
}

// The number of pops as seen by interp.
uint32_t numPop(const Bytecode& bc) {
  if (bc.op == Op::CGetL2) return 1;
  return bc.numPop();
}

// The number of pushes as seen by interp.
uint32_t numPush(const Bytecode& bc) {
  if (bc.op == Op::CGetL2) return 2;
  return bc.numPush();
}

void reprocess(ISS& env) {
  env.reprocess = true;
}

ArrayData** add_elem_array(ISS& env) {
  auto const idx = env.trackedElems.back().idx;
  if (idx < env.unchangedBcs) {
    auto const DEBUG_ONLY& bc = env.blk.hhbcs[idx];
    assertx(bc.op == Op::Concat);
    return nullptr;
  }
  assertx(idx >= env.unchangedBcs);
  auto& bc = env.replacedBcs[idx - env.unchangedBcs];
  auto arr = [&] () -> const ArrayData** {
    switch (bc.op) {
      case Op::Array:   return &bc.Array.arr1;
      case Op::Dict:    return &bc.Dict.arr1;
      case Op::Keyset:  return &bc.Keyset.arr1;
      case Op::Vec:     return &bc.Vec.arr1;
      case Op::Concat: return nullptr;
      default:          not_reached();
    }
  }();
  return const_cast<ArrayData**>(arr);
}

bool start_add_elem(ISS& env, Type& ty, Op op) {
  auto value = tvNonStatic(ty);
  if (!value || !isArrayLikeType(value->m_type)) return false;

  if (op == Op::AddElemC) {
    reduce(env, bc::PopC {}, bc::PopC {}, bc::PopC {});
  } else {
    reduce(env, bc::PopC {}, bc::PopC {});
  }
  env.trackedElems.emplace_back(
    env.state.stack.size(),
    env.unchangedBcs + env.replacedBcs.size()
  );

  auto const arr = value->m_data.parr;
  env.replacedBcs.push_back(
    [&] () -> Bytecode {
      if (arr->isKeyset()) {
        return bc::Keyset { arr };
      }
      if (arr->isVecArray()) {
        return bc::Vec { arr };
      }
      if (arr->isDict()) {
        return bc::Dict { arr };
      }
      if (arr->isPHPArray()) {
        return bc::Array { arr };
      }

      not_reached();
    }()
  );
  env.replacedBcs.back().srcLoc = env.srcLoc;
  ITRACE(2, "(addelem* -> {}\n",
         show(env.ctx.func, env.replacedBcs.back()));
  push(env, std::move(ty));
  effect_free(env);
  return true;
}

void finish_tracked_elem(ISS& env) {
  auto const arr = add_elem_array(env);
  env.trackedElems.pop_back();
  if (arr) ArrayData::GetScalarArray(arr);
}

void finish_tracked_elems(ISS& env, size_t depth) {
  while (!env.trackedElems.empty() && env.trackedElems.back().depth >= depth) {
    finish_tracked_elem(env);
  }
}

uint32_t id_from_slot(ISS& env, int slot) {
  auto const id = (env.state.stack.end() - (slot + 1))->id;
  assertx(id == StackElem::NoId ||
          id < env.unchangedBcs + env.replacedBcs.size());
  return id;
}

const Bytecode* op_from_id(ISS& env, uint32_t id) {
  if (id == StackElem::NoId) return nullptr;
  if (id < env.unchangedBcs) return &env.blk.hhbcs[id];
  auto const off = id - env.unchangedBcs;
  assertx(off < env.replacedBcs.size());
  return &env.replacedBcs[off];
}

void ensure_mutable(ISS& env, uint32_t id) {
  if (id < env.unchangedBcs) {
    auto const delta = env.unchangedBcs - id;
    env.replacedBcs.resize(env.replacedBcs.size() + delta);
    for (auto i = env.replacedBcs.size(); i-- > delta; ) {
      env.replacedBcs[i] = std::move(env.replacedBcs[i - delta]);
    }
    for (auto i = 0; i < delta; i++) {
      env.replacedBcs[i] = env.blk.hhbcs[id + i];
    }
    env.unchangedBcs = id;
  }
}

/*
 * Turn the instruction that wrote the slot'th element from the top of
 * the stack into a Nop, adjusting the stack appropriately. If its the
 * previous instruction, just rewind.
 */
int kill_by_slot(ISS& env, int slot) {
  auto const id = id_from_slot(env, slot);
  assertx(id != StackElem::NoId);
  auto const sz = env.state.stack.size();
  // if its the last bytecode we processed, we can rewind and avoid
  // the reprocess overhead.
  if (id == env.unchangedBcs + env.replacedBcs.size() - 1) {
    rewind(env, 1);
    return env.state.stack.size() - sz;
  }
  ensure_mutable(env, id);
  auto& bc = env.replacedBcs[id - env.unchangedBcs];
  auto const pop = numPop(bc);
  auto const push = numPush(bc);
  ITRACE(2, "kill_by_slot: slot={}, id={}, was {}\n",
         slot, id, show(env.ctx.func, bc));
  bc = bc_with_loc(bc.srcLoc, bc::Nop {});
  env.state.stack.kill(pop, push, id);
  reprocess(env);
  return env.state.stack.size() - sz;
}

/*
 * Check whether an instruction can be inserted immediately after the
 * slot'th stack entry was written. This is only possible if slot was
 * the last thing written by the instruction that wrote it (ie some
 * bytecodes push more than one value - there's no way to insert a
 * bytecode that will write *between* those values on the stack).
 */
bool can_insert_after_slot(ISS& env, int slot) {
  auto const it = env.state.stack.end() - (slot + 1);
  if (it->id == StackElem::NoId) return false;
  if (auto const next = it.next_elem(1)) {
    return next->id != it->id;
  }
  return true;
}

/*
 * Insert a sequence of bytecodes after the instruction that wrote the
 * slot'th element from the top of the stack.
 *
 * The entire sequence pops numPop, and pushes numPush stack
 * elements. Only the last bytecode can push anything onto the stack,
 * and the types it pushes are pointed to by types (if you have more
 * than one bytecode that pushes, call this more than once).
 */
void insert_after_slot(ISS& env, int slot,
                       int numPop, int numPush, const Type* types,
                       const BytecodeVec& bcs) {
  assertx(can_insert_after_slot(env, slot));
  auto const id = id_from_slot(env, slot);
  assertx(id != StackElem::NoId);
  ensure_mutable(env, id + 1);
  env.state.stack.insert_after(numPop, numPush, types, bcs.size(), id);
  env.replacedBcs.insert(env.replacedBcs.begin() + (id + 1 - env.unchangedBcs),
                         bcs.begin(), bcs.end());
  using namespace folly::gen;
  ITRACE(2, "insert_after_slot: slot={}, id={}  [{}]\n",
         slot, id,
         from(bcs) |
         map([&] (const Bytecode& bc) { return show(env.ctx.func, bc); }) |
         unsplit<std::string>(", "));
}

Bytecode& mutate_last_op(ISS& env) {
  assertx(will_reduce(env));

  if (!env.replacedBcs.size()) {
    assertx(env.unchangedBcs);
    env.replacedBcs.push_back(env.blk.hhbcs[--env.unchangedBcs]);
  }
  return env.replacedBcs.back();
}

/*
 * Can be used to replace one op with another when rewind/reduce isn't
 * safe (eg to change a SetL to a PopL - its not safe to rewind/reduce
 * because the SetL changed both the Type and the equiv of its local).
 */
void replace_last_op(ISS& env, Bytecode&& bc) {
  auto& last = mutate_last_op(env);
  auto const newPush = numPush(bc);
  auto const oldPush = numPush(last);
  auto const newPops = numPop(bc);
  auto const oldPops = numPop(last);

  assertx(newPush <= oldPush);
  assertx(newPops <= oldPops);

  if (newPush != oldPush || newPops != oldPops) {
    env.state.stack.rewind(oldPops - newPops, oldPush - newPush);
  }
  ITRACE(2, "(replace: {}->{}\n",
         show(env.ctx.func, last), show(env.ctx.func, bc));
  last = bc_with_loc(last.srcLoc, bc);
}

}

//////////////////////////////////////////////////////////////////////

const Bytecode* op_from_slot(ISS& env, int slot, int prev /* = 0 */) {
  if (!will_reduce(env)) return nullptr;
  auto const id = id_from_slot(env, slot);
  if (id == StackElem::NoId) return nullptr;
  if (id < prev) return nullptr;
  return op_from_id(env, id - prev);
}

const Bytecode* last_op(ISS& env, int idx /* = 0 */) {
  if (!will_reduce(env)) return nullptr;

  if (env.replacedBcs.size() > idx) {
    return &env.replacedBcs[env.replacedBcs.size() - idx - 1];
  }

  idx -= env.replacedBcs.size();
  if (env.unchangedBcs > idx) {
    return &env.blk.hhbcs[env.unchangedBcs - idx - 1];
  }
  return nullptr;
}

/*
 * Assuming bc was just interped, rewind to the state immediately
 * before it was interped.
 *
 * This is rarely what you want. Its used for constprop, where the
 * bytecode has been interped, but not yet committed to the bytecode
 * stream. We want to undo its effects, the spit out pops for its
 * inputs, and commit a constant-generating bytecode.
 */
void rewind(ISS& env, const Bytecode& bc) {
  ITRACE(2, "(rewind: {}\n", show(env.ctx.func, bc));
  env.state.stack.rewind(numPop(bc), numPush(bc));
}

/*
 * Used for peephole opts. Will undo the *stack* effects of the last n
 * committed byte codes, and remove them from the bytecode stream, in
 * preparation for writing out an optimized replacement sequence.
 *
 * WARNING: Does not undo other changes to state, such as local types,
 * local equivalency, and thisType. Take care when rewinding such
 * things.
 */
void rewind(ISS& env, int n) {
  assertx(n);
  while (env.replacedBcs.size()) {
    rewind(env, env.replacedBcs.back());
    env.replacedBcs.pop_back();
    if (!--n) return;
  }
  while (n--) {
    rewind(env, env.blk.hhbcs[--env.unchangedBcs]);
  }
}

void impl_vec(ISS& env, bool reduce, BytecodeVec&& bcs) {
  if (!will_reduce(env)) reduce = false;

  if (reduce) {
    using namespace folly::gen;
    ITRACE(2, "(reduce: {}\n",
           from(bcs) |
           map([&] (const Bytecode& bc) { return show(env.ctx.func, bc); }) |
           unsplit<std::string>(", "));
    if (bcs.size()) {
      auto ef = !env.flags.reduced || env.flags.effectFree;
      Trace::Indent _;
      for (auto const& bc : bcs) {
        assert(
          env.flags.jmpDest == NoBlockId &&
          "you can't use impl with branching opcodes before last position"
        );
        interpStep(env, bc);
        if (!env.flags.effectFree) ef = false;
        if (env.state.unreachable || env.flags.jmpDest != NoBlockId) break;
      }
      env.flags.effectFree = ef;
    } else if (!env.flags.reduced) {
      effect_free(env);
    }
    env.flags.reduced = true;
    return;
  }

  env.analyzeDepth++;
  SCOPE_EXIT { env.analyzeDepth--; };

  // We should be at the start of a bytecode.
  assertx(env.flags.wasPEI &&
          !env.flags.canConstProp &&
          !env.flags.effectFree);

  env.flags.wasPEI          = false;
  env.flags.canConstProp    = true;
  env.flags.effectFree      = true;

  for (auto const& bc : bcs) {
    assert(env.flags.jmpDest == NoBlockId &&
           "you can't use impl with branching opcodes before last position");

    auto const wasPEI = env.flags.wasPEI;
    auto const canConstProp = env.flags.canConstProp;
    auto const effectFree = env.flags.effectFree;

    ITRACE(3, "    (impl {}\n", show(env.ctx.func, bc));
    env.flags.wasPEI          = true;
    env.flags.canConstProp    = false;
    env.flags.effectFree      = false;
    default_dispatch(env, bc);

    if (env.flags.canConstProp) {
      [&] {
        if (env.flags.effectFree && !env.flags.wasPEI) return;
        auto stk = env.state.stack.end();
        for (auto i = bc.numPush(); i--; ) {
          --stk;
          if (!is_scalar(stk->type)) return;
        }
        env.flags.effectFree = true;
        env.flags.wasPEI = false;
      }();
    }

    // If any of the opcodes in the impl list said they could throw,
    // then the whole thing could throw.
    env.flags.wasPEI = env.flags.wasPEI || wasPEI;
    env.flags.canConstProp = env.flags.canConstProp && canConstProp;
    env.flags.effectFree = env.flags.effectFree && effectFree;
    if (env.state.unreachable || env.flags.jmpDest != NoBlockId) break;
  }
}

LocalId equivLocalRange(ISS& env, const LocalRange& range) {
  auto bestRange = range.first;
  auto equivFirst = findLocEquiv(env, range.first);
  if (equivFirst == NoLocalId) return bestRange;
  do {
    if (equivFirst < bestRange) {
      auto equivRange = [&] {
        // local equivalency includes differing by Uninit, so we need
        // to check the types.
        if (peekLocRaw(env, equivFirst) != peekLocRaw(env, range.first)) {
          return false;
        }

        for (uint32_t i = 1; i < range.count; ++i) {
          if (!locsAreEquiv(env, equivFirst + i, range.first + i) ||
              peekLocRaw(env, equivFirst + i) !=
              peekLocRaw(env, range.first + i)) {
            return false;
          }
        }

        return true;
      }();

      if (equivRange) {
        bestRange = equivFirst;
      }
    }
    equivFirst = findLocEquiv(env, equivFirst);
    assert(equivFirst != NoLocalId);
  } while (equivFirst != range.first);

  return bestRange;
}

SString getNameFromType(const Type& t) {
  if (!t.subtypeOf(BStr)) return nullptr;
  if (is_specialized_reifiedname(t)) return dreifiedname_of(t).name;
  if (is_specialized_string(t)) return sval_of(t);
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

namespace {

ArrayData*
resolveTSListStatically(ISS& env, SArray tsList, const php::Class* declaringCls,
                        bool checkArrays) {
  auto arr = Array::attach(const_cast<ArrayData*>(tsList));
  for (auto i = 0; i < arr.size(); i++) {
    auto elemArr = arr[i].getArrayData();
    auto elem = resolveTSStatically(env, elemArr, declaringCls, checkArrays);
    if (!elem) return nullptr;
    arr.set(i, Variant(elem));
  }
  return arr.detach();
}

} // namespace

ArrayData*
resolveTSStatically(ISS& env, SArray ts, const php::Class* declaringCls,
                    bool checkArrays) {
  auto const addModifiers = [&](ArrayData* result) {
    auto a = Array::attach(result);
    if (is_ts_like(ts) && !is_ts_like(a.get())) {
      a.set(s_like, true_varNR.tv());
    }
    if (is_ts_nullable(ts) && !is_ts_nullable(a.get())) {
      a.set(s_nullable, true_varNR.tv());
    }
    if (is_ts_soft(ts) && !is_ts_soft(a.get())) {
      a.set(s_soft, true_varNR.tv());
    }
    return a.detach();
  };
  auto const finish = [&](const ArrayData* result) {
    auto r = const_cast<ArrayData*>(result);
    ArrayData::GetScalarArray(&r);
    return r;
  };
  switch (get_ts_kind(ts)) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_resource:
      return finish(ts);
    case TypeStructure::Kind::T_typevar:
      if (ts->exists(s_name.get()) &&
          get_ts_name(ts)->equal(s_wildcard.get())) {
        return finish(ts);
      }
      return nullptr;
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
      if (!checkArrays || isTSAllWildcards(ts)) return finish(ts);
      return nullptr;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_enum:
      // Generics for these must have been resolved already as we'd never set
      // the TS Kind to be one of these until resolution
      return finish(ts);
    case TypeStructure::Kind::T_tuple: {
      auto const elems = get_ts_elem_types(ts);
      auto relems =
        resolveTSListStatically(env, elems, declaringCls, checkArrays);
      if (!relems) return nullptr;
      auto result = const_cast<ArrayData*>(ts);
      return finish(result->set(s_elem_types.get(), Variant(relems)));
    }
    case TypeStructure::Kind::T_shape:
      // TODO(T31677864): We can also optimize this but shapes could have
      // optional fields or they could allow unknown fields, so this one is
      // slightly more tricky
      return nullptr;
    case TypeStructure::Kind::T_unresolved: {
      assertx(ts->exists(s_classname));
      auto result = const_cast<ArrayData*>(ts);
      if (ts->exists(s_generic_types)) {
        auto const generics = get_ts_generic_types(ts);
        auto rgenerics =
          resolveTSListStatically(env, generics, declaringCls, checkArrays);
        if (!rgenerics) return nullptr;
        result = result->set(s_generic_types.get(), Variant(rgenerics));
      }
      auto const rcls = env.index.resolve_class(env.ctx, get_ts_classname(ts));
      if (!rcls || !rcls->resolved()) return nullptr;
      auto const attrs = rcls->cls()->attrs;
      auto const kind = [&] {
        if (attrs & AttrEnum)      return TypeStructure::Kind::T_enum;
        if (attrs & AttrTrait)     return TypeStructure::Kind::T_trait;
        if (attrs & AttrInterface) return TypeStructure::Kind::T_interface;
        return TypeStructure::Kind::T_class;
      }();
      return finish(result->set(s_kind.get(),
                                Variant(static_cast<uint8_t>(kind))));
    }
    case TypeStructure::Kind::T_typeaccess: {
      auto const accList = get_ts_access_list(ts);
      auto const size = accList->size();
      auto clsName = get_ts_root_name(ts);
      auto checkNoOverrideOnFirst = false;
      if (declaringCls) {
        if (clsName->isame(s_self.get())) {
          clsName = declaringCls->name;
        } else if (clsName->isame(s_parent.get()) && declaringCls->parentName) {
          clsName = declaringCls->parentName;
        } else if (clsName->isame(s_this.get())) {
          clsName = declaringCls->name;
          checkNoOverrideOnFirst = true;
        }
      }
      ArrayData* typeCnsVal = nullptr;
      for (auto i = 0; i < size; i++) {
        auto const rcls = env.index.resolve_class(env.ctx, clsName);
        if (!rcls || !rcls->resolved()) return nullptr;
        auto const cnsName = accList->at(i);
        if (!tvIsString(&cnsName)) return nullptr;
        auto const cnst = env.index.lookup_class_const_ptr(env.ctx, *rcls,
                                                           cnsName.m_data.pstr,
                                                           true);
        if (!cnst || !cnst->val || !cnst->isTypeconst ||
            !tvIsDictOrDArray(&*cnst->val)) {
          return nullptr;
        }
        if (checkNoOverrideOnFirst && i == 0 && !cnst->isNoOverride) {
          return nullptr;
        }
        typeCnsVal = resolveTSStatically(env, cnst->val->m_data.parr, cnst->cls,
                                         checkArrays);
        if (!typeCnsVal) return nullptr;
        if (i == size - 1) break;
        auto const kind = get_ts_kind(typeCnsVal);
        if (kind != TypeStructure::Kind::T_class &&
            kind != TypeStructure::Kind::T_interface) {
          return nullptr;
        }
        clsName = get_ts_classname(typeCnsVal);
      }
      if (!typeCnsVal) return nullptr;
      return finish(addModifiers(typeCnsVal));
    }
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_reifiedtype:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_trait:
      return nullptr;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

namespace interp_step {

void in(ISS& env, const bc::Nop&)  { reduce(env); }
void in(ISS& env, const bc::DiscardClsRef& op) {
  nothrow(env);
  takeClsRefSlot(env, op.slot);
}
void in(ISS& env, const bc::PopC&) {
  if (auto const last = last_op(env)) {
    if (poppable(last->op)) {
      rewind(env, 1);
      return reduce(env);
    }
    if (last->op == Op::This) {
      // can't rewind This because it removed null from thisType (so
      // CheckThis at this point is a no-op) - and note that it must
      // have *been* nullable, or we'd have turned it into a
      // `BareThis NeverNull`
      replace_last_op(env, bc::CheckThis {});
      return reduce(env);
    }
    if (last->op == Op::SetL) {
      // can't rewind a SetL because it changes local state
      replace_last_op(env, bc::PopL { last->SetL.loc1 });
      return reduce(env);
    }
    if (last->op == Op::CGetL2) {
      auto loc = last->CGetL2.loc1;
      rewind(env, 1);
      return reduce(env, bc::PopC {}, bc::CGetL { loc });
    }
  }

  effect_free(env);
  popC(env);
}

void in(ISS& env, const bc::PopU&) {
  if (auto const last = last_op(env)) {
    if (last->op == Op::NullUninit) {
      rewind(env, 1);
      return reduce(env);
    }
  }
  effect_free(env); popU(env);
}

void in(ISS& env, const bc::PopU2&) {
  effect_free(env);
  auto equiv = topStkEquiv(env);
  auto val = popC(env);
  popU(env);
  push(env, std::move(val), equiv != StackDupId ? equiv : NoLocalId);
}
void in(ISS& env, const bc::PopV&) { nothrow(env); popV(env); }

void in(ISS& env, const bc::EntryNop&) { effect_free(env); }

void in(ISS& env, const bc::Dup& /*op*/) {
  effect_free(env);
  auto equiv = topStkEquiv(env);
  auto val = popC(env);
  push(env, val, equiv);
  push(env, std::move(val), StackDupId);
}

void in(ISS& env, const bc::AssertRATL& op) {
  mayReadLocal(env, op.loc1);
  effect_free(env);
}

void in(ISS& env, const bc::AssertRATStk&) {
  effect_free(env);
}

void in(ISS& env, const bc::BreakTraceHint&) { effect_free(env); }

void in(ISS& env, const bc::CGetCUNop&) {
  effect_free(env);
  auto const t = popCU(env);
  push(env, remove_uninit(t));
}

void in(ISS& env, const bc::UGetCUNop&) {
  effect_free(env);
  popCU(env);
  push(env, TUninit);
}

void in(ISS& env, const bc::Null&) {
  effect_free(env);
  push(env, TInitNull);
}

void in(ISS& env, const bc::NullUninit&) {
  effect_free(env);
  push(env, TUninit);
}

void in(ISS& env, const bc::True&) {
  effect_free(env);
  push(env, TTrue);
}

void in(ISS& env, const bc::False&) {
  effect_free(env);
  push(env, TFalse);
}

void in(ISS& env, const bc::Int& op) {
  effect_free(env);
  push(env, ival(op.arg1));
}

void in(ISS& env, const bc::Double& op) {
  effect_free(env);
  push(env, dval(op.dbl1));
}

void in(ISS& env, const bc::String& op) {
  effect_free(env);
  push(env, sval(op.str1));
}

void in(ISS& env, const bc::Array& op) {
  assert(op.arr1->isPHPArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || op.arr1->isNotDVArray());
  effect_free(env);
  push(env, aval(op.arr1));
}

void in(ISS& env, const bc::Vec& op) {
  assert(op.arr1->isVecArray());
  effect_free(env);
  push(env, vec_val(op.arr1));
}

void in(ISS& env, const bc::Dict& op) {
  assert(op.arr1->isDict());
  effect_free(env);
  push(env, dict_val(op.arr1));
}

void in(ISS& env, const bc::Keyset& op) {
  assert(op.arr1->isKeyset());
  effect_free(env);
  push(env, keyset_val(op.arr1));
}

void in(ISS& env, const bc::NewArray& op) {
  effect_free(env);
  push(env, op.arg1 == 0 ? aempty() : some_aempty());
}

void in(ISS& env, const bc::NewDictArray& op) {
  effect_free(env);
  push(env, op.arg1 == 0 ? dict_empty() : some_dict_empty());
}

void in(ISS& env, const bc::NewMixedArray& op) {
  effect_free(env);
  push(env, op.arg1 == 0 ? aempty() : some_aempty());
}

void in(ISS& env, const bc::NewPackedArray& op) {
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  push(env, arr_packed(std::move(elems)));
  constprop(env);
}

void in(ISS& env, const bc::NewVArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  push(env, arr_packed_varray(std::move(elems)));
  effect_free(env);
  constprop(env);
}

void in(ISS& env, const bc::NewDArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  effect_free(env);
  push(env, op.arg1 == 0 ? aempty_darray() : some_aempty_darray());
}

void in(ISS& env, const bc::NewRecord& op) {
  discard(env, op.keys.size());
  push(env, TRecord);
}

void in(ISS& env, const bc::NewStructArray& op) {
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, arr_map(std::move(map)));
  effect_free(env);
  constprop(env);
}

void in(ISS& env, const bc::NewStructDArray& op) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, arr_map_darray(std::move(map)));
  effect_free(env);
  constprop(env);
}

void in(ISS& env, const bc::NewStructDict& op) {
  auto map = MapElems{};
  for (auto it = op.keys.end(); it != op.keys.begin(); ) {
    map.emplace_front(make_tv<KindOfPersistentString>(*--it), popC(env));
  }
  push(env, dict_map(std::move(map)));
  effect_free(env);
  constprop(env);
}

void in(ISS& env, const bc::NewVecArray& op) {
  auto elems = std::vector<Type>{};
  elems.reserve(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    elems.push_back(std::move(topC(env, op.arg1 - i - 1)));
  }
  discard(env, op.arg1);
  effect_free(env);
  constprop(env);
  push(env, vec(std::move(elems)));
}

void in(ISS& env, const bc::NewKeysetArray& op) {
  assert(op.arg1 > 0);
  auto map = MapElems{};
  auto ty = TBottom;
  auto useMap = true;
  auto bad = false;
  auto mayThrow = false;
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    auto k = disect_strict_key(popC(env));
    mayThrow |= k.mayThrow;
    if (k.type == TBottom) {
      bad = true;
      useMap = false;
    }
    if (useMap) {
      if (auto const v = k.tv()) {
        map.emplace_front(*v, k.type);
      } else {
        useMap = false;
      }
    }
    ty |= std::move(k.type);
  }
  if (!mayThrow) effect_free(env);
  if (useMap) {
    push(env, keyset_map(std::move(map)));
    if (!mayThrow) constprop(env);
  } else if (!bad) {
    push(env, keyset_n(ty));
  } else {
    unreachable(env);
    push(env, TBottom);
  }
}

void in(ISS& env, const bc::NewLikeArrayL& op) {
  locAsCell(env, op.loc1);
  push(env, some_aempty());
}

void in(ISS& env, const bc::AddElemC& /*op*/) {
  auto const v = topC(env, 0);
  auto const k = topC(env, 1);

  auto inTy = (env.state.stack.end() - 3).unspecialize();

  auto outTy = [&] (Type ty) ->
    folly::Optional<std::pair<Type,ThrowMode>> {
    if (ty.subtypeOf(BArr)) {
      return array_set(std::move(ty), k, v);
    }
    if (ty.subtypeOf(BDict)) {
      return dict_set(std::move(ty), k, v);
    }
    return folly::none;
  }(std::move(inTy));

  if (outTy && outTy->second == ThrowMode::None && will_reduce(env)) {
    if (!env.trackedElems.empty() &&
        env.trackedElems.back().depth + 3 == env.state.stack.size()) {
      auto const handled = [&] {
        if (!k.subtypeOf(BArrKey)) return false;
        auto ktv = tv(k);
        if (!ktv) return false;
        auto vtv = tv(v);
        if (!vtv) return false;
        auto const arr = add_elem_array(env);
        if (!arr) return false;
        *arr = (*arr)->set(*ktv, *vtv);
        return true;
      }();
      if (handled) {
        (env.state.stack.end() - 3)->type = std::move(outTy->first);
        reduce(env, bc::PopC {}, bc::PopC {});
        ITRACE(2, "(addelem* -> {}\n",
               show(env.ctx.func,
                    env.replacedBcs[env.trackedElems.back().idx - env.unchangedBcs]));
        return;
      }
    } else {
      if (start_add_elem(env, outTy->first, Op::AddElemC)) {
        return;
      }
    }
  }

  discard(env, 3);
  finish_tracked_elems(env, env.state.stack.size());

  if (!outTy) {
    return push(env, union_of(TArr, TDict));
  }

  if (outTy->first.subtypeOf(BBottom)) {
    unreachable(env);
  } else if (outTy->second == ThrowMode::None) {
    effect_free(env);
    constprop(env);
  }
  push(env, std::move(outTy->first));
}

void in(ISS& env, const bc::AddNewElemC&) {
  auto v = topC(env);
  auto inTy = (env.state.stack.end() - 2).unspecialize();

  auto outTy = [&] (Type ty) -> folly::Optional<Type> {
    if (ty.subtypeOf(BArr)) {
      return array_newelem(std::move(ty), std::move(v)).first;
    }
    if (ty.subtypeOf(BVec)) {
      return vec_newelem(std::move(ty), std::move(v)).first;
    }
    if (ty.subtypeOf(BKeyset)) {
      return keyset_newelem(std::move(ty), std::move(v)).first;
    }
    return folly::none;
  }(std::move(inTy));

  if (outTy && will_reduce(env)) {
    if (!env.trackedElems.empty() &&
        env.trackedElems.back().depth + 2 == env.state.stack.size()) {
      auto const handled = [&] {
        auto vtv = tv(v);
        if (!vtv) return false;
        auto const arr = add_elem_array(env);
        if (!arr) return false;
        *arr = (*arr)->append(*vtv);
        return true;
      }();
      if (handled) {
        (env.state.stack.end() - 2)->type = std::move(*outTy);
        reduce(env, bc::PopC {});
        ITRACE(2, "(addelem* -> {}\n",
               show(env.ctx.func,
                    env.replacedBcs[env.trackedElems.back().idx - env.unchangedBcs]));
        return;
      }
    } else {
      if (start_add_elem(env, *outTy, Op::AddNewElemC)) {
        return;
      }
    }
  }

  discard(env, 2);
  finish_tracked_elems(env, env.state.stack.size());

  if (!outTy) {
    return push(env, TInitCell);
  }

  if (outTy->subtypeOf(BBottom)) {
    unreachable(env);
  } else {
    constprop(env);
  }
  push(env, std::move(*outTy));
}

void in(ISS& env, const bc::NewCol& op) {
  auto const type = static_cast<CollectionType>(op.subop1);
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
  effect_free(env);
}

void in(ISS& env, const bc::NewPair& /*op*/) {
  popC(env); popC(env);
  auto const name = collections::typeToString(CollectionType::Pair);
  push(env, objExact(env.index.builtin_class(name)));
  effect_free(env);
}

void in(ISS& env, const bc::ColFromArray& op) {
  auto const src = popC(env);
  auto const type = static_cast<CollectionType>(op.subop1);
  assertx(type != CollectionType::Pair);
  if (type == CollectionType::Vector || type == CollectionType::ImmVector) {
    if (src.subtypeOf(TVec)) effect_free(env);
  } else {
    assertx(type == CollectionType::Map ||
            type == CollectionType::ImmMap ||
            type == CollectionType::Set ||
            type == CollectionType::ImmSet);
    if (src.subtypeOf(TDict)) effect_free(env);
  }
  auto const name = collections::typeToString(type);
  push(env, objExact(env.index.builtin_class(name)));
}

void in(ISS& env, const bc::CnsE& op) {
  if (!options.HardConstProp) return push(env, TInitCell);
  auto t = env.index.lookup_constant(env.ctx, op.str1);
  if (!t) {
    // There's no entry for this constant in the index. It must be
    // the first iteration, so we'll add a dummy entry to make sure
    // there /is/ something next time around.
    Cell val;
    val.m_type = kReadOnlyConstant;
    env.collect.cnsMap.emplace(op.str1, val);
    t = TInitCell;
    // make sure we're re-analyzed
    env.collect.readsUntrackedConstants = true;
  } else if (t->strictSubtypeOf(TInitCell)) {
    // constprop will take care of nothrow *if* its a constant; and if
    // its not, we might trigger autoload.
    constprop(env);
  }
  push(env, std::move(*t));
}

void in(ISS& env, const bc::ClsCns& op) {
  auto const& t1 = peekClsRefSlot(env, op.slot);
  if (is_specialized_cls(t1)) {
    auto const dcls = dcls_of(t1);
    auto const finish = [&] {
      reduce(env, bc::DiscardClsRef { op.slot },
                  bc::ClsCnsD { op.str1, dcls.cls.name() });
    };
    if (dcls.type == DCls::Exact) return finish();
    auto const cnst = env.index.lookup_class_const_ptr(env.ctx, dcls.cls,
                                                       op.str1, false);
    if (cnst && cnst->isNoOverride) return finish();
  }
  takeClsRefSlot(env, op.slot);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ClsCnsD& op) {
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str2)) {
    auto t = env.index.lookup_class_constant(env.ctx, *rcls, op.str1, false);
    if (options.HardConstProp) constprop(env);
    push(env, std::move(t));
    return;
  }
  push(env, TInitCell);
}

void in(ISS& env, const bc::File&)   { effect_free(env); push(env, TSStr); }
void in(ISS& env, const bc::Dir&)    { effect_free(env); push(env, TSStr); }
void in(ISS& env, const bc::Method&) { effect_free(env); push(env, TSStr); }

void in(ISS& env, const bc::ClsRefName& op) {
  auto ty = peekClsRefSlot(env, op.slot);
  if (is_specialized_cls(ty)) {
    auto const dcls = dcls_of(ty);
    if (dcls.type == DCls::Exact) {
      return reduce(env,
                    bc::DiscardClsRef { op.slot },
                    bc::String { dcls.cls.name() });
    }
  }
  nothrow(env);
  takeClsRefSlot(env, op.slot);
  push(env, TSStr);
}

void concatHelper(ISS& env, uint32_t n) {
  auto changed = false;
  auto side_effects = false;
  if (will_reduce(env)) {
    auto litstr = [&] (SString next, uint32_t i) -> SString {
      auto const t = topC(env, i);
      auto const v = tv(t);
      if (!v) return nullptr;
      if (!isStringType(v->m_type)   &&
          v->m_type != KindOfNull    &&
          v->m_type != KindOfBoolean &&
          v->m_type != KindOfInt64   &&
          v->m_type != KindOfDouble) {
        return nullptr;
      }
      auto const cell = eval_cell_value(
        [&] {
          auto const s = makeStaticString(
            next ?
            StringData::Make(tvAsCVarRef(&*v).toString().get(), next) :
            tvAsCVarRef(&*v).toString().get());
          return make_tv<KindOfString>(s);
        }
      );
      if (!cell) return nullptr;
      return cell->m_data.pstr;
    };

    auto fold = [&] (uint32_t slot, uint32_t num, SString result) {
      auto const cell = make_tv<KindOfPersistentString>(result);
      auto const ty = from_cell(cell);
      BytecodeVec bcs{num, bc::PopC {}};
      if (num > 1) bcs.push_back(gen_constant(cell));
      if (slot == 0) {
        reduce(env, std::move(bcs));
      } else {
        insert_after_slot(env, slot, num, num > 1 ? 1 : 0, &ty, bcs);
        reprocess(env);
      }
      n -= num - 1;
      changed = true;
    };

    for (auto i = 0; i < n; i++) {
      if (topC(env, i).couldBe(BObj | BArrLike | BRes)) {
        side_effects = true;
        break;
      }
    }

    if (!side_effects) {
      for (auto i = 0; i < n; i++) {
        auto const tracked = !env.trackedElems.empty() &&
          env.trackedElems.back().depth + i + 1 == env.state.stack.size();
        if (tracked) finish_tracked_elems(env, env.trackedElems.back().depth);
        auto const prev = op_from_slot(env, i);
        if (!prev) continue;
        if ((prev->op == Op::Concat && tracked) || prev->op == Op::ConcatN) {
          auto const extra = kill_by_slot(env, i);
          changed = true;
          n += extra;
          i += extra;
        }
      }
    }

    SString result = nullptr;
    uint32_t i = 0;
    uint32_t nlit = 0;
    while (i < n) {
      // In order to collapse literals, we need to be able to insert
      // pops, and a constant after the sequence that generated the
      // literals. We can always insert after the last instruction
      // though, and we only need to check the first slot of a
      // sequence.
      auto const next = !i || result || can_insert_after_slot(env, i) ?
        litstr(result, i) : nullptr;
      if (next == staticEmptyString()) {
        if (n == 1) break;
        assertx(nlit == 0);
        fold(i, 1, next);
        n--;
        continue;
      }
      if (!next) {
        if (nlit > 1) {
          fold(i - nlit, nlit, result);
          i -= nlit - 1;
        }
        nlit = 0;
      } else {
        nlit++;
      }
      result = next;
      i++;
    }
    if (nlit > 1) fold(i - nlit, nlit, result);
  }

  if (!changed) {
    discard(env, n);
    if (n == 2 && !side_effects && will_reduce(env)) {
      env.trackedElems.emplace_back(
        env.state.stack.size(),
        env.unchangedBcs + env.replacedBcs.size()
      );
    }
    push(env, TStr);
    return;
  }

  if (n == 1) {
    if (!topC(env).subtypeOf(BStr)) {
      return reduce(env, bc::CastString {});
    }
    return reduce(env);
  }

  reduce(env);
  // We can't reduce the emitted concats, or we'll end up with
  // infinite recursion.
  env.flags.wasPEI = true;
  env.flags.effectFree = false;
  env.flags.canConstProp = false;

  auto concat = [&] (uint32_t num) {
    discard(env, num);
    push(env, TStr);
    if (num == 2) {
      record(env, bc::Concat {});
    } else {
      record(env, bc::ConcatN { num });
    }
  };

  while (n >= 4) {
    concat(4);
    n -= 3;
  }
  if (n > 1) concat(n);
}

void in(ISS& env, const bc::Concat& /*op*/) {
  concatHelper(env, 2);
}

void in(ISS& env, const bc::ConcatN& op) {
  if (op.arg1 == 2) return reduce(env, bc::Concat {});
  concatHelper(env, op.arg1);
}

template <class Op, class Fun>
void arithImpl(ISS& env, const Op& /*op*/, Fun fun) {
  constprop(env);
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  push(env, fun(t2, t1));
}

void in(ISS& env, const bc::Add& op)    { arithImpl(env, op, typeAdd); }
void in(ISS& env, const bc::Sub& op)    { arithImpl(env, op, typeSub); }
void in(ISS& env, const bc::Mul& op)    { arithImpl(env, op, typeMul); }
void in(ISS& env, const bc::Div& op)    { arithImpl(env, op, typeDiv); }
void in(ISS& env, const bc::Mod& op)    { arithImpl(env, op, typeMod); }
void in(ISS& env, const bc::Pow& op)    { arithImpl(env, op, typePow); }
void in(ISS& env, const bc::BitAnd& op) { arithImpl(env, op, typeBitAnd); }
void in(ISS& env, const bc::BitOr& op)  { arithImpl(env, op, typeBitOr); }
void in(ISS& env, const bc::BitXor& op) { arithImpl(env, op, typeBitXor); }
void in(ISS& env, const bc::AddO& op)   { arithImpl(env, op, typeAddO); }
void in(ISS& env, const bc::SubO& op)   { arithImpl(env, op, typeSubO); }
void in(ISS& env, const bc::MulO& op)   { arithImpl(env, op, typeMulO); }
void in(ISS& env, const bc::Shl& op)    { arithImpl(env, op, typeShl); }
void in(ISS& env, const bc::Shr& op)    { arithImpl(env, op, typeShr); }

void in(ISS& env, const bc::BitNot& /*op*/) {
  auto const t = popC(env);
  auto const v = tv(t);
  if (v) {
    constprop(env);
    auto cell = eval_cell([&] {
      auto c = *v;
      cellBitNot(c);
      return c;
    });
    if (cell) return push(env, std::move(*cell));
  }
  push(env, TInitCell);
}

namespace {

bool couldBeHackArr(Type t) {
  return t.couldBe(BVec | BDict | BKeyset);
}

template<bool NSame>
std::pair<Type,bool> resolveSame(ISS& env) {
  auto const l1 = topStkEquiv(env, 0);
  auto const t1 = topC(env, 0);
  auto const l2 = topStkEquiv(env, 1);
  auto const t2 = topC(env, 1);

  // EvalHackArrCompatNotices will notice on === and !== between PHP arrays and
  // Hack arrays. We can't really do better than this in general because of
  // arrays inside these arrays.
  auto warningsEnabled =
    (RuntimeOption::EvalHackArrCompatNotices ||
     RuntimeOption::EvalHackArrCompatDVCmpNotices);

  auto const result = [&] {
    auto const v1 = tv(t1);
    auto const v2 = tv(t2);

    if (l1 == StackDupId ||
        (l1 == l2 && l1 != NoLocalId) ||
        (l1 <= MaxLocalId && l2 <= MaxLocalId && locsAreEquiv(env, l1, l2))) {
      if (!t1.couldBe(BDbl) || !t2.couldBe(BDbl) ||
          (v1 && (v1->m_type != KindOfDouble || !std::isnan(v1->m_data.dbl))) ||
          (v2 && (v2->m_type != KindOfDouble || !std::isnan(v2->m_data.dbl)))) {
        return NSame ? TFalse : TTrue;
      }
    }

    if (v1 && v2) {
      if (auto r = eval_cell_value([&]{ return cellSame(*v2, *v1); })) {
        // we wouldn't get here if cellSame raised a warning
        warningsEnabled = false;
        return r != NSame ? TTrue : TFalse;
      }
    }

    return NSame ? typeNSame(t1, t2) : typeSame(t1, t2);
  }();

  if (warningsEnabled && result == (NSame ? TFalse : TTrue)) {
    warningsEnabled = false;
  }
  return { result, warningsEnabled && compare_might_raise(t1, t2) };
}

template<bool Negate>
void sameImpl(ISS& env) {
  if (auto const last = last_op(env)) {
    if (last->op == Op::Null) {
      rewind(env, 1);
      reduce(env, bc::IsTypeC { IsTypeOp::Null });
      if (Negate) reduce(env, bc::Not {});
      return;
    }
    if (auto const prev = last_op(env, 1)) {
      if (prev->op == Op::Null &&
          (last->op == Op::CGetL || last->op == Op::CGetL2)) {
        auto const loc = last->op == Op::CGetL ?
          last->CGetL.loc1 : last->CGetL2.loc1;
        rewind(env, 2);
        reduce(env, bc::IsTypeL { loc, IsTypeOp::Null });
        if (Negate) reduce(env, bc::Not {});
        return;
      }
    }
  }

  auto pair = resolveSame<Negate>(env);
  discard(env, 2);

  if (!pair.second) {
    nothrow(env);
    constprop(env);
  }

  push(env, std::move(pair.first));
}

template<class JmpOp>
bool sameJmpImpl(ISS& env, Op sameOp, const JmpOp& jmp) {
  const StackElem* elems[2];
  env.state.stack.peek(2, elems, 1);

  auto const loc0 = elems[1]->equivLoc;
  auto const loc1 = elems[0]->equivLoc;
  // If loc0 == loc1, either they're both NoLocalId, so there's
  // nothing for us to deduce, or both stack elements are the same
  // value, so the only thing we could deduce is that they are or are
  // not NaN. But we don't track that, so just bail.
  if (loc0 == loc1 || loc0 == StackDupId) return false;

  auto const ty0 = elems[1]->type;
  auto const ty1 = elems[0]->type;
  auto const val0 = tv(ty0);
  auto const val1 = tv(ty1);

  assertx(!val0 || !val1);
  if ((loc0 == NoLocalId && !val0 && ty1.subtypeOf(ty0)) ||
      (loc1 == NoLocalId && !val1 && ty0.subtypeOf(ty1))) {
    return false;
  }

  // Same currently lies about the distinction between Func/Cls/Str
  if (ty0.couldBe(BFunc | BCls) && ty1.couldBe(BStr)) return false;
  if (ty1.couldBe(BFunc | BCls) && ty0.couldBe(BStr)) return false;

  // We need to loosen away the d/varray bits here because array comparison does
  // not take into account the difference.
  auto isect = intersection_of(
    loosen_dvarrayness(ty0),
    loosen_dvarrayness(ty1)
  );

  // Unfortunately, floating point negative zero and positive zero are
  // different, but are identical using as far as Same is concerened. We should
  // avoid refining a value to 0.0 because it compares identically to 0.0
  if (isect.couldBe(dval(0.0)) || isect.couldBe(dval(-0.0))) {
    isect = union_of(isect, TDbl);
  }

  discard(env, 1);

  auto handle_same = [&] {
    // Currently dce uses equivalency to prove that something isn't
    // the last reference - so we can only assert equivalency here if
    // we know that won't be affected. Its irrelevant for uncounted
    // things, and for TObj and TRes, $x === $y iff $x and $y refer to
    // the same thing.
    if (loc0 <= MaxLocalId &&
        (ty0.subtypeOf(BObj | BRes | BPrim) ||
         ty1.subtypeOf(BObj | BRes | BPrim) ||
         (ty0.subtypeOf(BUnc) && ty1.subtypeOf(BUnc)))) {
      if (loc1 == StackDupId) {
        setStkLocal(env, loc0, 0);
      } else if (loc1 <= MaxLocalId && !locsAreEquiv(env, loc0, loc1)) {
        auto loc = loc0;
        while (true) {
          auto const other = findLocEquiv(env, loc);
          if (other == NoLocalId) break;
          killLocEquiv(env, loc);
          addLocEquiv(env, loc, loc1);
          loc = other;
        }
        addLocEquiv(env, loc, loc1);
      }
    }
    return refineLocation(env, loc1 != NoLocalId ? loc1 : loc0, [&] (Type ty) {
      if (!ty.couldBe(BUninit) || !isect.couldBe(BNull)) {
        auto ret = intersection_of(std::move(ty), isect);
        return ty.subtypeOf(BUnc) ? ret : loosen_staticness(ret);
      }

      if (isect.subtypeOf(BNull)) {
        return ty.couldBe(BInitNull) ? TNull : TUninit;
      }

      return ty;
    });
  };

  auto handle_differ_side = [&] (LocalId location, const Type& ty) {
    if (!ty.subtypeOf(BInitNull) && !ty.strictSubtypeOf(TBool)) return true;
    return refineLocation(env, location, [&] (Type t) {
      if (ty.subtypeOf(BNull)) {
        t = remove_uninit(std::move(t));
        if (is_opt(t)) t = unopt(std::move(t));
        return t;
      } else if (ty.strictSubtypeOf(TBool) && t.subtypeOf(BBool)) {
        return ty == TFalse ? TTrue : TFalse;
      }
      return t;
    });
  };

  auto handle_differ = [&] {
    return
      (loc0 == NoLocalId || handle_differ_side(loc0, ty1)) &&
      (loc1 == NoLocalId || handle_differ_side(loc1, ty0));
  };

  auto const sameIsJmpTarget =
    (sameOp == Op::Same) == (JmpOp::op == Op::JmpNZ);

  auto save = env.state;
  auto const target_reachable = sameIsJmpTarget ?
    handle_same() : handle_differ();
  if (!target_reachable) jmp_nevertaken(env);
  // swap, so we can restore this state if the branch is always taken.
  env.state.swap(save);
  if (!(sameIsJmpTarget ? handle_differ() : handle_same())) {
    jmp_setdest(env, jmp.target1);
    env.state.copy_from(std::move(save));
  } else if (target_reachable) {
    env.propagate(jmp.target1, &save);
  }

  return true;
}

bc::JmpNZ invertJmp(const bc::JmpZ& jmp) { return bc::JmpNZ { jmp.target1 }; }
bc::JmpZ invertJmp(const bc::JmpNZ& jmp) { return bc::JmpZ { jmp.target1 }; }

}

void in(ISS& env, const bc::Same&)  { sameImpl<false>(env); }
void in(ISS& env, const bc::NSame&) { sameImpl<true>(env); }

template<class Fun>
void binOpBoolImpl(ISS& env, Fun fun) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    if (auto r = eval_cell_value([&]{ return fun(*v2, *v1); })) {
      constprop(env);
      return push(env, *r ? TTrue : TFalse);
    }
  }
  // TODO_4: evaluate when these can throw, non-constant type stuff.
  push(env, TBool);
}

template<class Fun>
void binOpInt64Impl(ISS& env, Fun fun) {
  auto const t1 = popC(env);
  auto const t2 = popC(env);
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) {
    if (auto r = eval_cell_value([&]{ return ival(fun(*v2, *v1)); })) {
      constprop(env);
      return push(env, std::move(*r));
    }
  }
  // TODO_4: evaluate when these can throw, non-constant type stuff.
  push(env, TInt);
}

void in(ISS& env, const bc::Eq&) {
  auto rs = resolveSame<false>(env);
  if (rs.first == TTrue) {
    if (!rs.second) constprop(env);
    discard(env, 2);
    return push(env, TTrue);
  }
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellEqual(c1, c2); });
}
void in(ISS& env, const bc::Neq&) {
  auto rs = resolveSame<false>(env);
  if (rs.first == TTrue) {
    if (!rs.second) constprop(env);
    discard(env, 2);
    return push(env, TFalse);
  }
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return !cellEqual(c1, c2); });
}
void in(ISS& env, const bc::Lt&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellLess(c1, c2); });
}
void in(ISS& env, const bc::Gt&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) { return cellGreater(c1, c2); });
}
void in(ISS& env, const bc::Lte&) { binOpBoolImpl(env, cellLessOrEqual); }
void in(ISS& env, const bc::Gte&) { binOpBoolImpl(env, cellGreaterOrEqual); }

void in(ISS& env, const bc::Cmp&) {
  binOpInt64Impl(env, [&] (Cell c1, Cell c2) { return cellCompare(c1, c2); });
}

void in(ISS& env, const bc::Xor&) {
  binOpBoolImpl(env, [&] (Cell c1, Cell c2) {
    return cellToBool(c1) ^ cellToBool(c2);
  });
}

void castBoolImpl(ISS& env, const Type& t, bool negate) {
  nothrow(env);
  constprop(env);

  auto const e = emptiness(t);
  switch (e) {
    case Emptiness::Empty:
    case Emptiness::NonEmpty:
      return push(env, (e == Emptiness::Empty) == negate ? TTrue : TFalse);
    case Emptiness::Maybe:
      break;
  }

  push(env, TBool);
}

void in(ISS& env, const bc::Not&) {
  castBoolImpl(env, popC(env), true);
}

void in(ISS& env, const bc::CastBool&) {
  auto const t = topC(env);
  if (t.subtypeOf(BBool)) return reduce(env);
  castBoolImpl(env, popC(env), false);
}

void in(ISS& env, const bc::CastInt&) {
  auto const t = topC(env);
  if (t.subtypeOf(BInt)) return reduce(env);
  constprop(env);
  popC(env);
  // Objects can raise a warning about converting to int.
  if (!t.couldBe(BObj)) nothrow(env);
  if (auto const v = tv(t)) {
    auto cell = eval_cell([&] {
      return make_tv<KindOfInt64>(cellToInt(*v));
    });
    if (cell) return push(env, std::move(*cell));
  }
  push(env, TInt);
}

// Handle a casting operation, where "target" is the type being casted to. If
// "fn" is provided, it will be called to cast any constant inputs. If "elide"
// is set to true, if the source type is the same as the destination, the cast
// will be optimized away.
void castImpl(ISS& env, Type target, void(*fn)(TypedValue*)) {
  auto const t = topC(env);
  if (t.subtypeOf(target)) return reduce(env);
  popC(env);
  if (fn) {
    if (auto val = tv(t)) {
      if (auto result = eval_cell([&] { fn(&*val); return *val; })) {
        constprop(env);
        target = *result;
      }
    }
  }
  push(env, std::move(target));
}

void in(ISS& env, const bc::CastDouble&) {
  castImpl(env, TDbl, tvCastToDoubleInPlace);
}

void in(ISS& env, const bc::CastString&) {
  castImpl(env, TStr, tvCastToStringInPlace);
}

void in(ISS& env, const bc::CastArray&)  {
  castImpl(env, TPArr, tvCastToArrayInPlace);
}

void in(ISS& env, const bc::CastObject&) { castImpl(env, TObj, nullptr); }

void in(ISS& env, const bc::CastDict&)   {
  castImpl(env, TDict, tvCastToDictInPlace);
}

void in(ISS& env, const bc::CastVec&)    {
  castImpl(env, TVec, tvCastToVecInPlace);
}

void in(ISS& env, const bc::CastKeyset&) {
  castImpl(env, TKeyset, tvCastToKeysetInPlace);
}

void in(ISS& env, const bc::CastVArray&)  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  castImpl(env, TVArr, tvCastToVArrayInPlace);
}

void in(ISS& env, const bc::CastDArray&)  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  castImpl(env, TDArr, tvCastToDArrayInPlace);
}

void in(ISS& env, const bc::DblAsBits&) {
  nothrow(env);
  constprop(env);

  auto const ty = popC(env);
  if (!ty.couldBe(BDbl)) return push(env, ival(0));

  if (auto val = tv(ty)) {
    assertx(isDoubleType(val->m_type));
    val->m_type = KindOfInt64;
    push(env, from_cell(*val));
    return;
  }

  push(env, TInt);
}

void in(ISS& env, const bc::Print& /*op*/) {
  popC(env);
  push(env, ival(1));
}

void in(ISS& env, const bc::Clone& /*op*/) {
  auto val = popC(env);
  if (!val.subtypeOf(BObj)) {
    val = is_opt(val) ? unopt(std::move(val)) : TObj;
  }
  push(env, std::move(val));
}

void in(ISS& env, const bc::Exit&)  { popC(env); push(env, TInitNull); }
void in(ISS& env, const bc::Fatal&) { popC(env); }

void in(ISS& /*env*/, const bc::JmpNS&) {
  always_assert(0 && "blocks should not contain JmpNS instructions");
}

void in(ISS& /*env*/, const bc::Jmp&) {
  always_assert(0 && "blocks should not contain Jmp instructions");
}

void in(ISS& env, const bc::Select& op) {
  auto const cond = topC(env);
  auto const t = topC(env, 1);
  auto const f = topC(env, 2);

  nothrow(env);
  constprop(env);

  switch (emptiness(cond)) {
    case Emptiness::Maybe:
      discard(env, 3);
      push(env, union_of(t, f));
      return;
    case Emptiness::NonEmpty:
      discard(env, 3);
      push(env, t);
      return;
    case Emptiness::Empty:
      return reduce(env, bc::PopC {}, bc::PopC {});
  }
  not_reached();
}

namespace {

template<class JmpOp>
bool isTypeHelper(ISS& env,
                  IsTypeOp typeOp,
                  LocalId location,
                  Op op,
                  const JmpOp& jmp) {
  if (typeOp == IsTypeOp::Scalar || typeOp == IsTypeOp::ArrLike) {
    return false;
  }

  auto const val = [&] {
    if (op != Op::IsTypeC) return locRaw(env, location);
    const StackElem* elem;
    env.state.stack.peek(1, &elem, 1);
    location = elem->equivLoc;
    return elem->type;
  }();

  if (location == NoLocalId || !val.subtypeOf(BCell)) return false;

  // If the type could be ClsMeth and Arr/Vec, skip location refining.
  // Otherwise, refine location based on the testType.
  auto testTy = type_of_istype(typeOp);
  if (RuntimeOption::EvalIsCompatibleClsMethType && val.couldBe(BClsMeth)) {
    assertx(RuntimeOption::EvalEmitClsMethPointers);
    if (RuntimeOption::EvalHackArrDVArrs) {
      if ((typeOp == IsTypeOp::Vec) || (typeOp == IsTypeOp::VArray)) {
        if (val.couldBe(BVec | BVArr)) return false;
        testTy = TClsMeth;
      }
    } else {
      if ((typeOp == IsTypeOp::Arr) || (typeOp == IsTypeOp::VArray)) {
        if (val.couldBe(BArr | BVArr)) return false;
        testTy = TClsMeth;
      }
    }
  }

  assertx(val.couldBe(testTy) &&
          (!val.subtypeOf(testTy) || val.subtypeOf(BObj)));

  discard(env, 1);

  if (op == Op::IsTypeC) {
    if (!is_type_might_raise(testTy, val)) nothrow(env);
  } else if (op == Op::IssetL) {
    nothrow(env);
  } else if (!locCouldBeUninit(env, location) &&
             !is_type_might_raise(testTy, val)) {
    nothrow(env);
  }

  auto const negate = (jmp.op == Op::JmpNZ) == (op != Op::IssetL);
  auto const was_true = [&] (Type t) {
    if (testTy.subtypeOf(BNull)) return intersection_of(t, TNull);
    assertx(!testTy.couldBe(BNull));
    return intersection_of(t, testTy);
  };
  auto const was_false = [&] (Type t) {
    auto tinit = remove_uninit(t);
    if (testTy.subtypeOf(BNull)) {
      return is_opt(tinit) ? unopt(tinit) : tinit;
    }
    if (is_opt(tinit)) {
      assertx(!testTy.couldBe(BNull));
      if (unopt(tinit).subtypeOf(testTy)) return TNull;
    }
    return t;
  };

  auto const pre = [&] (Type t) {
    return negate ? was_true(std::move(t)) : was_false(std::move(t));
  };

  auto const post = [&] (Type t) {
    return negate ? was_false(std::move(t)) : was_true(std::move(t));
  };

  refineLocation(env, location, pre, jmp.target1, post);
  return true;
}

// If the current function is a memoize wrapper, return the inferred return type
// of the function being wrapped along with if the wrapped function is effect
// free.
std::pair<Type, bool> memoizeImplRetType(ISS& env) {
  always_assert(env.ctx.func->isMemoizeWrapper);

  // Lookup the wrapped function. This should always resolve to a precise
  // function but we don't rely on it.
  auto const memo_impl_func = [&] {
    if (env.ctx.func->cls) {
      auto const clsTy = selfClsExact(env);
      return env.index.resolve_method(
        env.ctx,
        clsTy ? *clsTy : TCls,
        memoize_impl_name(env.ctx.func)
      );
    }
    return env.index.resolve_func(env.ctx, memoize_impl_name(env.ctx.func));
  }();

  // Infer the return type of the wrapped function, taking into account the
  // types of the parameters for context sensitive types.
  auto const numArgs = env.ctx.func->params.size();
  CompactVector<Type> args{numArgs};
  for (auto i = LocalId{0}; i < numArgs; ++i) {
    args[i] = locAsCell(env, i);
  }

  // Determine the context the wrapped function will be called on.
  auto const ctxType = [&]() -> Type {
    if (env.ctx.func->cls) {
      if (env.ctx.func->attrs & AttrStatic) {
        // The class context for static methods is the method's class,
        // if LSB is not specified.
        auto const clsTy =
          env.ctx.func->isMemoizeWrapperLSB ?
          selfCls(env) :
          selfClsExact(env);
        return clsTy ? *clsTy : TCls;
      } else {
        return thisTypeNonNull(env);
      }
    }
    return TBottom;
  }();

  auto retTy = env.index.lookup_return_type(
    env.ctx,
    args,
    ctxType,
    memo_impl_func
  );
  auto const effectFree = env.index.is_effect_free(memo_impl_func);
  // Regardless of anything we know the return type will be an InitCell (this is
  // a requirement of memoize functions).
  if (!retTy.subtypeOf(BInitCell)) return { TInitCell, effectFree };
  return { retTy, effectFree };
}

template<class JmpOp>
bool instanceOfJmpImpl(ISS& env,
                       const bc::InstanceOfD& inst,
                       const JmpOp& jmp) {

  const StackElem* elem;
  env.state.stack.peek(1, &elem, 1);

  auto const locId = elem->equivLoc;
  if (locId == NoLocalId || interface_supports_non_objects(inst.str1)) {
    return false;
  }
  auto const rcls = env.index.resolve_class(env.ctx, inst.str1);
  if (!rcls) return false;

  auto const val = elem->type;
  auto const instTy = subObj(*rcls);
  assertx(!val.subtypeOf(instTy) && val.couldBe(instTy));

  // If we have an optional type, whose unopt is guaranteed to pass
  // the instanceof check, then failing to pass implies it was null.
  auto const fail_implies_null = is_opt(val) && unopt(val).subtypeOf(instTy);

  discard(env, 1);
  auto const negate = jmp.op == Op::JmpNZ;
  auto const result = [&] (Type t, bool pass) {
    return pass ? instTy : fail_implies_null ? TNull : t;
  };
  auto const pre  = [&] (Type t) { return result(t, negate); };
  auto const post = [&] (Type t) { return result(t, !negate); };
  refineLocation(env, locId, pre, jmp.target1, post);
  return true;
}

template<class JmpOp>
bool isTypeStructCJmpImpl(ISS& env,
                          const bc::IsTypeStructC& inst,
                          const JmpOp& jmp) {

  const StackElem* elems[2];
  env.state.stack.peek(2, elems, 1);

  auto const locId = elems[0]->equivLoc;
  if (locId == NoLocalId) return false;

  auto const a = tv(elems[1]->type);
  if (!a) return false;
  // if it wasn't valid, the JmpOp wouldn't be reachable
  assertx(isValidTSType(*a, false));

  auto const is_nullable_ts = is_ts_nullable(a->m_data.parr);
  auto const ts_kind = get_ts_kind(a->m_data.parr);
  // type_of_type_structure does not resolve these types.  It is important we
  // do resolve them here, or we may have issues when we reduce the checks to
  // InstanceOfD checks.  This logic performs the same exact refinement as
  // instanceOfD will.
  if (is_nullable_ts ||
      (ts_kind != TypeStructure::Kind::T_class &&
       ts_kind != TypeStructure::Kind::T_interface &&
       ts_kind != TypeStructure::Kind::T_xhp &&
       ts_kind != TypeStructure::Kind::T_unresolved)) {
    return false;
  }

  auto const clsName = get_ts_classname(a->m_data.parr);
  auto const rcls = env.index.resolve_class(env.ctx, clsName);
  if (!rcls ||
      !rcls->resolved() ||
      rcls->cls()->attrs & AttrEnum ||
      interface_supports_non_objects(clsName)) {
    return false;
  }

  auto const val = elems[0]->type;
  auto const instTy = subObj(*rcls);
  if (val.subtypeOf(instTy) || !val.couldBe(instTy)) {
    return false;
  }

  // If we have an optional type, whose unopt is guaranteed to pass
  // the instanceof check, then failing to pass implies it was null.
  auto const fail_implies_null = is_opt(val) && unopt(val).subtypeOf(instTy);

  discard(env, 1);

  auto const negate = jmp.op == Op::JmpNZ;
  auto const result = [&] (Type t, bool pass) {
    return pass ? instTy : fail_implies_null ? TNull : t;
  };
  auto const pre  = [&] (Type t) { return result(t, negate); };
  auto const post = [&] (Type t) { return result(t, !negate); };
  refineLocation(env, locId, pre, jmp.target1, post);
  return true;
}

template<class JmpOp>
void jmpImpl(ISS& env, const JmpOp& op) {
  auto const Negate = std::is_same<JmpOp, bc::JmpNZ>::value;
  auto const location = topStkEquiv(env);
  auto const e = emptiness(topC(env));
  if (e == (Negate ? Emptiness::NonEmpty : Emptiness::Empty)) {
    reduce(env, bc::PopC {});
    return jmp_setdest(env, op.target1);
  }

  if (e == (Negate ? Emptiness::Empty : Emptiness::NonEmpty) ||
      (next_real_block(*env.ctx.func, env.blk.fallthrough) ==
       next_real_block(*env.ctx.func, op.target1))) {
    return reduce(env, bc::PopC{});
  }

  auto fix = [&] {
    if (env.flags.jmpDest == NoBlockId) return;
    auto const jmpDest = env.flags.jmpDest;
    env.flags.jmpDest = NoBlockId;
    rewind(env, op);
    reduce(env, bc::PopC {});
    env.flags.jmpDest = jmpDest;
  };

  if (auto const last = last_op(env)) {
    if (last->op == Op::Not) {
      rewind(env, 1);
      return reduce(env, invertJmp(op));
    }
    if (last->op == Op::Same || last->op == Op::NSame) {
      if (sameJmpImpl(env, last->op, op)) return fix();
    } else if (last->op == Op::IssetL) {
      if (isTypeHelper(env,
                       IsTypeOp::Null,
                       last->IsTypeL.loc1,
                       last->op,
                       op)) {
        return fix();
      }
    } else if (last->op == Op::IsTypeL) {
      if (isTypeHelper(env,
                       last->IsTypeL.subop2,
                       last->IsTypeL.loc1,
                       last->op,
                       op)) {
        return fix();
      }
    } else if (last->op == Op::IsTypeC) {
      if (isTypeHelper(env,
                       last->IsTypeC.subop1,
                       NoLocalId,
                       last->op,
                       op)) {
        return fix();
      }
    } else if (last->op == Op::InstanceOfD) {
      if (instanceOfJmpImpl(env, last->InstanceOfD, op)) return fix();
    } else if (last->op == Op::IsTypeStructC) {
      if (isTypeStructCJmpImpl(env, last->IsTypeStructC, op)) return fix();
    }
  }

  popC(env);
  nothrow(env);

  if (location == NoLocalId) return env.propagate(op.target1, &env.state);

  refineLocation(env, location,
                 Negate ? assert_nonemptiness : assert_emptiness,
                 op.target1,
                 Negate ? assert_emptiness : assert_nonemptiness);
  return fix();
}

} // namespace

void in(ISS& env, const bc::JmpNZ& op) { jmpImpl(env, op); }
void in(ISS& env, const bc::JmpZ& op)  { jmpImpl(env, op); }

void in(ISS& env, const bc::Switch& op) {
  auto v = tv(topC(env));

  if (v) {
    auto go = [&] (BlockId blk) {
      reduce(env, bc::PopC {});
      return jmp_setdest(env, blk);
    };
    auto num_elems = op.targets.size();
    if (op.subop1 == SwitchKind::Unbounded) {
      if (v->m_type == KindOfInt64 &&
          v->m_data.num >= 0 && v->m_data.num < num_elems) {
        return go(op.targets[v->m_data.num]);
      }
    } else {
      assertx(num_elems > 2);
      num_elems -= 2;
      for (auto i = size_t{}; ; i++) {
        if (i == num_elems) {
          return go(op.targets.back());
        }
        auto match = eval_cell_value([&] {
            return cellEqual(*v, static_cast<int64_t>(op.arg2 + i));
        });
        if (!match) break;
        if (*match) {
          return go(op.targets[i]);
        }
      }
    }
  }

  popC(env);
  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, &env.state);
  });
}

void in(ISS& env, const bc::SSwitch& op) {
  auto v = tv(topC(env));

  if (v) {
    for (auto& kv : op.targets) {
      auto match = eval_cell_value([&] {
        return !kv.first || cellEqual(*v, kv.first);
      });
      if (!match) break;
      if (*match) {
        reduce(env, bc::PopC {});
        return jmp_setdest(env, kv.second);
      }
    }
  }

  popC(env);
  forEachTakenEdge(op, [&] (BlockId id) {
      env.propagate(id, &env.state);
  });
}

void in(ISS& env, const bc::RetC& /*op*/) {
  auto const locEquiv = topStkLocal(env);
  doRet(env, popC(env), false);
  if (locEquiv != NoLocalId && locEquiv < env.ctx.func->params.size()) {
    env.flags.retParam = locEquiv;
  }
}
void in(ISS& env, const bc::RetM& op) {
  std::vector<Type> ret(op.arg1);
  for (int i = 0; i < op.arg1; i++) {
    ret[op.arg1 - i - 1] = popC(env);
  }
  doRet(env, vec(std::move(ret)), false);
}

void in(ISS& env, const bc::RetCSuspended&) {
  always_assert(env.ctx.func->isAsync && !env.ctx.func->isGenerator);

  auto const t = popC(env);
  doRet(
    env,
    is_specialized_wait_handle(t) ? wait_handle_inner(t) : TInitCell,
    false
  );
}

void in(ISS& env, const bc::Throw& /*op*/) {
  popC(env);
}

void in(ISS& env, const bc::ChainFaults&) {
  popC(env);
}

void in(ISS& env, const bc::NativeImpl&) {
  killLocals(env);
  mayUseVV(env);

  if (is_collection_method_returning_this(env.ctx.cls, env.ctx.func)) {
    auto const resCls = env.index.builtin_class(env.ctx.cls->name);
    return doRet(env, objExact(resCls), true);
  }

  if (env.ctx.func->nativeInfo) {
    return doRet(env, native_function_return_type(env.ctx.func), true);
  }
  doRet(env, TInitCell, true);
}

void in(ISS& env, const bc::CGetL& op) {
  if (locIsThis(env, op.loc1)) {
    auto const& ty = peekLocRaw(env, op.loc1);
    if (!ty.subtypeOf(BInitNull)) {
      auto const subop = ty.couldBe(BUninit) ?
        BareThisOp::Notice : ty.couldBe(BNull) ?
        BareThisOp::NoNotice : BareThisOp::NeverNull;
      return reduce(env, bc::BareThis { subop });
    }
  }
  if (auto const last = last_op(env)) {
    if (!is_pseudomain(env.ctx.func) && last->op == Op::PopL &&
        op.loc1 == last->PopL.loc1) {
      reprocess(env);
      rewind(env, 1);
      if (!locCouldBeRef(env, op.loc1)) setLocRaw(env, op.loc1, TCell);
      return reduce(env, bc::SetL { op.loc1 });
    }
  }
  if (!peekLocCouldBeUninit(env, op.loc1)) {
    auto const minLocEquiv = findMinLocEquiv(env, op.loc1, false);
    if (minLocEquiv != NoLocalId) {
      return reduce(env, bc::CGetL { minLocEquiv });
    }

    nothrow(env);
    constprop(env);
  }
  mayReadLocal(env, op.loc1);
  push(env, locAsCell(env, op.loc1), op.loc1);
}

void in(ISS& env, const bc::CGetQuietL& op) {
  if (locIsThis(env, op.loc1)) {
    return reduce(env, bc::BareThis { BareThisOp::NoNotice });
  }
  auto const minLocEquiv = findMinLocEquiv(env, op.loc1, true);
  if (minLocEquiv != NoLocalId) {
    return reduce(env, bc::CGetQuietL { minLocEquiv });
  }

  nothrow(env);
  constprop(env);
  mayReadLocal(env, op.loc1);
  push(env, locAsCell(env, op.loc1), op.loc1);
}

void in(ISS& env, const bc::CUGetL& op) {
  auto ty = locRaw(env, op.loc1);
  if (ty.subtypeOf(BUninit)) {
    return reduce(env, bc::NullUninit {});
  }
  nothrow(env);
  if (!ty.couldBe(BUninit)) constprop(env);
  if (!ty.subtypeOf(BCell)) ty = TCell;
  push(env, std::move(ty), op.loc1);
}

void in(ISS& env, const bc::PushL& op) {
  if (auto val = tv(peekLocRaw(env, op.loc1))) {
    return reduce(env, bc::UnsetL { op.loc1 }, gen_constant(*val));
  }

  auto const minLocEquiv = findMinLocEquiv(env, op.loc1, false);
  if (minLocEquiv != NoLocalId) {
    return reduce(env, bc::CGetL { minLocEquiv }, bc::UnsetL { op.loc1 });
  }

  if (auto const last = last_op(env)) {
    if (last->op == Op::PopL &&
        last->PopL.loc1 == op.loc1) {
      // rewind is ok, because we're just going to unset the local
      // (and note the unset can't be a no-op because the PopL set it
      // to an InitCell). But its possible that before the PopL, the
      // local *was* unset, so maybe would have killed the no-op. The
      // only way to fix that is to reprocess the block with the new
      // instruction sequence and see what happens.
      reprocess(env);
      rewind(env, 1);
      return reduce(env, bc::UnsetL { op.loc1 });
    }
  }

  impl(env, bc::CGetL { op.loc1 }, bc::UnsetL { op.loc1 });
}

void in(ISS& env, const bc::CGetL2& op) {
  if (auto const last = last_op(env)) {
    if ((poppable(last->op) && !numPop(*last)) ||
        (last->op == Op::CGetL && !peekLocCouldBeUninit(env, op.loc1))) {
      auto const other = *last;
      rewind(env, 1);
      return reduce(env, bc::CGetL { op.loc1 }, other);
    }
  }

  if (!peekLocCouldBeUninit(env, op.loc1)) {
    auto const minLocEquiv = findMinLocEquiv(env, op.loc1, false);
    if (minLocEquiv != NoLocalId) {
      return reduce(env, bc::CGetL2 { minLocEquiv });
    }
    effect_free(env);
  }
  mayReadLocal(env, op.loc1);
  auto loc = locAsCell(env, op.loc1);
  auto topEquiv = topStkLocal(env);
  auto top = popT(env);
  push(env, std::move(loc), op.loc1);
  push(env, std::move(top), topEquiv);
}

void in(ISS& env, const bc::CGetG&) { popC(env); push(env, TInitCell); }
void in(ISS& env, const bc::CGetQuietG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::CGetS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (vname && vname->m_type == KindOfPersistentString &&
      self && tcls.subtypeOf(*self)) {
    if (auto ty = selfPropAsCell(env, vname->m_data.pstr)) {
      // Only nothrow when we know it's a private declared property (and thus
      // accessible here), class initialization won't throw, and its not a
      // LateInit prop (which will throw if not initialized).
      if (!classInitMightRaise(env, tcls) &&
          !isMaybeLateInitSelfProp(env, vname->m_data.pstr)) {
        nothrow(env);

        // We can only constprop here if we know for sure this is exactly the
        // correct class.  The reason for this is that you could have a LSB
        // class attempting to access a private static in a derived class with
        // the same name as a private static in this class, which is supposed to
        // fatal at runtime (for an example see test/quick/static_sprop2.php).
        auto const selfExact = selfClsExact(env);
        if (selfExact && tcls.subtypeOf(*selfExact)) constprop(env);
      }

      if (ty->subtypeOf(BBottom)) unreachable(env);
      return push(env, std::move(*ty));
    }
  }

  auto indexTy = env.index.lookup_public_static(env.ctx, tcls, tname);
  if (indexTy.subtypeOf(BInitCell)) {
    /*
     * Constant propagation here can change when we invoke autoload, so it's
     * considered HardConstProp.  It's safe not to check anything about private
     * or protected static properties, because you can't override a public
     * static property with a private or protected one---if the index gave us
     * back a constant type, it's because it found a public static and it must
     * be the property this would have read dynamically.
     */
    if (options.HardConstProp &&
        !classInitMightRaise(env, tcls) &&
        !env.index.lookup_public_static_maybe_late_init(tcls, tname)) {
      constprop(env);
    }
    if (indexTy.subtypeOf(BBottom)) unreachable(env);
    return push(env, std::move(indexTy));
  }

  push(env, TInitCell);
}

void in(ISS& env, const bc::VGetL& op) {
  nothrow(env);
  setLocRaw(env, op.loc1, TRef);
  push(env, TRef);
}

void clsRefGetImpl(ISS& env, Type t1, ClsRefSlotId slot) {
  auto cls = [&]{
    if (auto const clsname = getNameFromType(t1)) {
      auto const rcls = env.index.resolve_class(env.ctx, clsname);
      if (rcls) return clsExact(*rcls);
    }
    if (t1.subtypeOf(BObj)) {
      nothrow(env);
      return objcls(t1);
    }
    return TCls;
  }();
  putClsRefSlot(env, slot, std::move(cls));
}

void in(ISS& env, const bc::ClsRefGetC& op) {
  clsRefGetImpl(env, popC(env), op.slot);
}

void in(ISS& env, const bc::ClsRefGetTS& op) {
  // TODO(T31677864): implement real optimizations
  auto const ts = popC(env);
  auto const requiredTSType = RuntimeOption::EvalHackArrDVArrs ? BDict : BDArr;
  if (!ts.couldBe(requiredTSType)) {
    push(env, TBottom);
    return;
  }
  clsRefGetImpl(env, TStr, op.slot);
}

void in(ISS& env, const bc::AKExists& /*op*/) {
  auto const base = popC(env);
  auto const key  = popC(env);

  // Bases other than array-like or object will raise a warning and return
  // false.
  if (!base.couldBeAny(TArr, TVec, TDict, TKeyset, TObj)) {
    return push(env, TFalse);
  }

  // Push the returned type and annotate effects appropriately, taking into
  // account if the base might be null. Allowing for a possibly null base lets
  // us capture more cases.
  auto const finish = [&] (const Type& t, bool mayThrow) {
    if (base.couldBe(BInitNull)) return push(env, union_of(t, TFalse));
    if (!mayThrow) {
      constprop(env);
      effect_free(env);
    }
    if (base.subtypeOf(BBottom)) unreachable(env);
    return push(env, t);
  };

  // Helper for Hack arrays. "validKey" is the set of key types which can return
  // a value from AKExists. "silentKey" is the set of key types which will
  // silently return false (anything else throws). The Hack array elem functions
  // will treat values of "silentKey" as throwing, so we must identify those
  // cases and deal with them.
  auto const hackArr = [&] (std::pair<Type, ThrowMode> elem,
                            const Type& validKey,
                            const Type& silentKey) {
    switch (elem.second) {
      case ThrowMode::None:
        assertx(key.subtypeOf(validKey));
        return finish(TTrue, false);
      case ThrowMode::MaybeMissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(TBool, false);
      case ThrowMode::MissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(TFalse, false);
      case ThrowMode::MaybeBadKey:
        assertx(key.couldBe(validKey));
        return finish(
          elem.first.subtypeOf(BBottom) ? TFalse : TBool,
          !key.subtypeOf(BOptArrKey)
        );
      case ThrowMode::BadOperation:
        assertx(!key.couldBe(validKey));
        return finish(key.couldBe(silentKey) ? TFalse : TBottom, true);
    }
  };

  // Vecs will throw for any key other than Int, Str, or Null, and will silently
  // return false for the latter two.
  if (base.subtypeOrNull(BVec)) {
    if (key.subtypeOrNull(BStr)) return finish(TFalse, false);
    return hackArr(vec_elem(base, key, TBottom), TInt, TOptStr);
  }

  // Dicts and keysets will throw for any key other than Int, Str, or Null,
  // and will silently return false for Null.
  if (base.subtypeOfAny(TOptDict, TOptKeyset)) {
    if (key.subtypeOf(BInitNull)) return finish(TFalse, false);
    auto const elem = base.subtypeOrNull(BDict)
      ? dict_elem(base, key, TBottom)
      : keyset_elem(base, key, TBottom);
    return hackArr(elem, TArrKey, TInitNull);
  }

  if (base.subtypeOrNull(BArr)) {
    // Unlike Idx, AKExists will transform a null key on arrays into the static
    // empty string, so we don't need to do any fixups here.
    auto const elem = array_elem(base, key, TBottom);
    switch (elem.second) {
      case ThrowMode::None:                return finish(TTrue, false);
      case ThrowMode::MaybeMissingElement: return finish(TBool, false);
      case ThrowMode::MissingElement:      return finish(TFalse, false);
      case ThrowMode::MaybeBadKey:
        return finish(elem.first.subtypeOf(BBottom) ? TFalse : TBool, true);
      case ThrowMode::BadOperation:        always_assert(false);
    }
  }

  // Objects or other unions of possible bases
  push(env, TBool);
}

void in(ISS& env, const bc::GetMemoKeyL& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);

  auto const rclsIMemoizeParam = env.index.builtin_class(s_IMemoizeParam.get());
  auto const tyIMemoizeParam = subObj(rclsIMemoizeParam);

  auto const inTy = locAsCell(env, op.loc1);

  // If the local could be uninit, we might raise a warning (as
  // usual). Converting an object to a memo key might invoke PHP code if it has
  // the IMemoizeParam interface, and if it doesn't, we'll throw.
  if (!locCouldBeUninit(env, op.loc1) &&
      !inTy.couldBeAny(TObj, TArr, TVec, TDict)) {
    nothrow(env); constprop(env);
  }

  // If type constraints are being enforced and the local being turned into a
  // memo key is a parameter, then we can possibly using the type constraint to
  // infer a more efficient memo key mode.
  using MK = MemoKeyConstraint;
  folly::Optional<res::Class> resolvedCls;
  auto const mkc = [&] {
    if (op.loc1 >= env.ctx.func->params.size()) return MK::None;
    auto tc = env.ctx.func->params[op.loc1].typeConstraint;
    if (tc.type() == AnnotType::Object) {
      auto res = env.index.resolve_type_name(tc.typeName());
      if (res.type != AnnotType::Object) {
        tc.resolveType(res.type, res.nullable || tc.isNullable());
      } else {
        resolvedCls = env.index.resolve_class(env.ctx, tc.typeName());
      }
    }
    return memoKeyConstraintFromTC(tc);
  }();

  // Use the type-constraint to reduce this operation to a more efficient memo
  // mode. Some of the modes can be reduced to simple bytecode operations
  // inline. Even with the type-constraints, we still need to check the inferred
  // type of the local. Something may have possibly clobbered the local between
  // the type-check and this op.
  switch (mkc) {
    case MK::Int:
      // Always an int, so the key is always an identity mapping
      if (inTy.subtypeOf(BInt)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::Bool:
      // Always a bool, so the key is the bool cast to an int
      if (inTy.subtypeOf(BBool)) {
        return reduce(env, bc::CGetL { op.loc1 }, bc::CastInt {});
      }
      break;
    case MK::Str:
      // Always a string, so the key is always an identity mapping
      if (inTy.subtypeOf(BStr)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::IntOrStr:
      // Either an int or string, so the key can be an identity mapping
      if (inTy.subtypeOf(BArrKey)) return reduce(env, bc::CGetL { op.loc1 });
      break;
    case MK::StrOrNull:
      // A nullable string. The key will either be the string or the integer
      // zero.
      if (inTy.subtypeOrNull(BStr)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::Int { 0 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::IntOrNull:
      // A nullable int. The key will either be the integer, or the static empty
      // string.
      if (inTy.subtypeOrNull(BInt)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::String { staticEmptyString() },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::BoolOrNull:
      // A nullable bool. The key will either be 0, 1, or 2.
      if (inTy.subtypeOrNull(BBool)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::CastInt {},
          bc::Int { 2 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::Dbl:
      // The double will be converted (losslessly) to an integer.
      if (inTy.subtypeOf(BDbl)) {
        return reduce(env, bc::CGetL { op.loc1 }, bc::DblAsBits {});
      }
      break;
    case MK::DblOrNull:
      // A nullable double. The key will be an integer, or the static empty
      // string.
      if (inTy.subtypeOrNull(BDbl)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::DblAsBits {},
          bc::String { staticEmptyString() },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::Object:
      // An object. If the object is definitely known to implement IMemoizeParam
      // we can simply call that method, casting the output to ensure its always
      // a string (which is what the generic mode does). If not, it will use the
      // generic mode, which can handle collections or classes which don't
      // implement getInstanceKey.
      if (resolvedCls &&
          resolvedCls->mustBeSubtypeOf(rclsIMemoizeParam) &&
          inTy.subtypeOf(tyIMemoizeParam)) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::NullUninit {},
          bc::NullUninit {},
          bc::FCallObjMethodD {
            FCallArgs(0),
            staticEmptyString(),
            ObjMethodOp::NullThrows,
            s_getInstanceKey.get()
          },
          bc::CastString {}
        );
      }
      break;
    case MK::ObjectOrNull:
      // An object or null. We can use the null safe version of a function call
      // when invoking getInstanceKey and then select from the result of that,
      // or the integer 0. This might seem wasteful, but the JIT does a good job
      // inlining away the call in the null case.
      if (resolvedCls &&
          resolvedCls->mustBeSubtypeOf(rclsIMemoizeParam) &&
          inTy.subtypeOf(opt(tyIMemoizeParam))) {
        return reduce(
          env,
          bc::CGetL { op.loc1 },
          bc::NullUninit {},
          bc::NullUninit {},
          bc::FCallObjMethodD {
            FCallArgs(0),
            staticEmptyString(),
            ObjMethodOp::NullSafe,
            s_getInstanceKey.get()
          },
          bc::CastString {},
          bc::Int { 0 },
          bc::IsTypeL { op.loc1, IsTypeOp::Null },
          bc::Select {}
        );
      }
      break;
    case MK::None:
      break;
  }

  // No type constraint, or one that isn't usuable. Use the generic memoization
  // scheme which can handle any type:

  if (auto const val = tv(inTy)) {
    auto const key = eval_cell(
      [&]{ return HHVM_FN(serialize_memoize_param)(*val); }
    );
    if (key) return push(env, *key);
  }

  // Integer keys are always mapped to themselves
  if (inTy.subtypeOf(BInt)) return reduce(env, bc::CGetL { op.loc1 });
  if (inTy.subtypeOrNull(BInt)) {
    return reduce(
      env,
      bc::CGetL { op.loc1 },
      bc::String { s_nullMemoKey.get() },
      bc::IsTypeL { op.loc1, IsTypeOp::Null },
      bc::Select {}
    );
  }
  if (inTy.subtypeOf(BBool)) {
    return reduce(
      env,
      bc::String { s_falseMemoKey.get() },
      bc::String { s_trueMemoKey.get() },
      bc::CGetL { op.loc1 },
      bc::Select {}
    );
  }

  // A memo key can be an integer if the input might be an integer, and is a
  // string otherwise. Booleans and nulls are always static strings.
  auto keyTy = [&]{
    if (inTy.subtypeOrNull(BBool)) return TSStr;
    if (inTy.couldBe(BInt))        return union_of(TInt, TStr);
    return TStr;
  }();
  push(env, std::move(keyTy));
}

void in(ISS& env, const bc::IssetL& op) {
  if (locIsThis(env, op.loc1)) {
    return reduce(env,
                  bc::BareThis { BareThisOp::NoNotice },
                  bc::IsTypeC { IsTypeOp::Null },
                  bc::Not {});
  }
  nothrow(env);
  constprop(env);
  auto const loc = locAsCell(env, op.loc1);
  if (loc.subtypeOf(BNull))  return push(env, TFalse);
  if (!loc.couldBe(BNull))   return push(env, TTrue);
  push(env, TBool);
}

void in(ISS& env, const bc::EmptyL& op) {
  nothrow(env);
  constprop(env);
  castBoolImpl(env, locAsCell(env, op.loc1), true);
}

void in(ISS& env, const bc::EmptyS& op) {
  takeClsRefSlot(env, op.slot);
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::IssetS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (self && tcls.subtypeOf(*self) &&
      vname && vname->m_type == KindOfPersistentString) {
    if (auto const t = selfPropAsCell(env, vname->m_data.pstr)) {
      if (isMaybeLateInitSelfProp(env, vname->m_data.pstr)) {
        if (!classInitMightRaise(env, tcls)) constprop(env);
        return push(env, t->subtypeOf(BBottom) ? TFalse : TBool);
      }
      if (t->subtypeOf(BNull)) {
        if (!classInitMightRaise(env, tcls)) constprop(env);
        return push(env, TFalse);
      }
      if (!t->couldBe(BNull)) {
        if (!classInitMightRaise(env, tcls)) constprop(env);
        return push(env, TTrue);
      }
    }
  }

  auto const indexTy = env.index.lookup_public_static(env.ctx, tcls, tname);
  if (indexTy.subtypeOf(BInitCell)) {
    // See the comments in CGetS about constprop for public statics.
    if (options.HardConstProp && !classInitMightRaise(env, tcls)) {
      constprop(env);
    }
    if (env.index.lookup_public_static_maybe_late_init(tcls, tname)) {
      return push(env, indexTy.subtypeOf(BBottom) ? TFalse : TBool);
    }
    if (indexTy.subtypeOf(BNull))  { return push(env, TFalse); }
    if (!indexTy.couldBe(BNull))   { return push(env, TTrue); }
  }

  push(env, TBool);
}

void in(ISS& env, const bc::EmptyG&) { popC(env); push(env, TBool); }
void in(ISS& env, const bc::IssetG&) { popC(env); push(env, TBool); }

void isTypeImpl(ISS& env, const Type& locOrCell, const Type& test) {
  if (locOrCell.subtypeOf(test))  return push(env, TTrue);
  if (!locOrCell.couldBe(test))   return push(env, TFalse);
  push(env, TBool);
}

void isTypeObj(ISS& env, const Type& ty) {
  if (!ty.couldBe(BObj)) return push(env, TFalse);
  if (ty.subtypeOf(BObj)) {
    auto const incompl = objExact(
      env.index.builtin_class(s_PHP_Incomplete_Class.get()));
    if (!ty.couldBe(incompl))  return push(env, TTrue);
    if (ty.subtypeOf(incompl)) return push(env, TFalse);
  }
  push(env, TBool);
}

void isTypeArrLike(ISS& env, const Type& ty) {
  if (ty.subtypeOf(BArr | BVec | BDict | BKeyset | BClsMeth))  {
    return push(env, TTrue);
  }
  if (!ty.couldBe(BArr | BVec | BDict | BKeyset | BClsMeth))  {
    return push(env, TFalse);
  }
  push(env, TBool);
}

namespace {
bool isCompactTypeClsMeth(ISS& env, IsTypeOp op, const Type& t) {
  assertx(RuntimeOption::EvalEmitClsMethPointers);
  if (t.couldBe(BClsMeth)) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      if (op == IsTypeOp::Vec || op == IsTypeOp::VArray) {
        if (t.subtypeOf(
            op == IsTypeOp::Vec ? BClsMeth | BVec : BClsMeth | BVArr)) {
          push(env, TTrue);
        } else if (t.couldBe(op == IsTypeOp::Vec ? BVec : BVArr)) {
          push(env, TBool);
        } else {
          isTypeImpl(env, t, TClsMeth);
        }
        return true;
      }
    } else {
      if (op == IsTypeOp::Arr || op == IsTypeOp::VArray) {
        if (t.subtypeOf(
            op == IsTypeOp::VArray ? BClsMeth | BVArr : BClsMeth | BArr)) {
          push(env, TTrue);
        } else if (t.couldBe(op == IsTypeOp::VArray ? BVArr : BArr)) {
          push(env, TBool);
        } else {
          isTypeImpl(env, t, TClsMeth);
        }
        return true;
      }
    }
  }
  return false;
}
}

template<class Op>
void isTypeLImpl(ISS& env, const Op& op) {
  auto const loc = locAsCell(env, op.loc1);
  if (!locCouldBeUninit(env, op.loc1) && !is_type_might_raise(op.subop2, loc)) {
    constprop(env);
    nothrow(env);
  }

  if (RuntimeOption::EvalIsCompatibleClsMethType &&
      isCompactTypeClsMeth(env, op.subop2, loc)) {
    return;
  }

  switch (op.subop2) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, loc);
  case IsTypeOp::ArrLike: return isTypeArrLike(env, loc);
  default: return isTypeImpl(env, loc, type_of_istype(op.subop2));
  }
}

template<class Op>
void isTypeCImpl(ISS& env, const Op& op) {
  auto const t1 = popC(env);
  if (!is_type_might_raise(op.subop1, t1)) {
    constprop(env);
    nothrow(env);
  }

  if (RuntimeOption::EvalIsCompatibleClsMethType &&
      isCompactTypeClsMeth(env, op.subop1, t1)) {
    return;
  }

  switch (op.subop1) {
  case IsTypeOp::Scalar: return push(env, TBool);
  case IsTypeOp::Obj: return isTypeObj(env, t1);
  case IsTypeOp::ArrLike: return isTypeArrLike(env, t1);
  default: return isTypeImpl(env, t1, type_of_istype(op.subop1));
  }
}

void in(ISS& env, const bc::IsTypeC& op) { isTypeCImpl(env, op); }
void in(ISS& env, const bc::IsTypeL& op) { isTypeLImpl(env, op); }

void in(ISS& env, const bc::InstanceOfD& op) {
  auto t1 = topC(env);
  // Note: InstanceOfD can do autoload if the type might be a type
  // alias, so it's not nothrow unless we know it's an object type.
  if (auto const rcls = env.index.resolve_class(env.ctx, op.str1)) {
    auto result = [&] (const Type& r) {
      nothrow(env);
      if (r != TBool) constprop(env);
      popC(env);
      push(env, r);
    };
    if (!interface_supports_non_objects(rcls->name())) {
      auto testTy = subObj(*rcls);
      if (t1.subtypeOf(testTy)) return result(TTrue);
      if (!t1.couldBe(testTy)) return result(TFalse);
      if (is_opt(t1)) {
        t1 = unopt(std::move(t1));
        if (t1.subtypeOf(testTy)) {
          return reduce(env, bc::IsTypeC { IsTypeOp::Null }, bc::Not {});
        }
      }
      return result(TBool);
    }
  }
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::InstanceOf& /*op*/) {
  auto const t1 = topC(env);
  auto const v1 = tv(t1);
  if (v1 && v1->m_type == KindOfPersistentString) {
    return reduce(env, bc::PopC {},
                       bc::InstanceOfD { v1->m_data.pstr });
  }

  if (t1.subtypeOf(BObj) && is_specialized_obj(t1)) {
    auto const dobj = dobj_of(t1);
    switch (dobj.type) {
    case DObj::Sub:
      break;
    case DObj::Exact:
      return reduce(env, bc::PopC {},
                         bc::InstanceOfD { dobj.cls.name() });
    }
  }

  popC(env);
  popC(env);
  push(env, TBool);
}

namespace {

bool isValidTypeOpForIsAs(const IsTypeOp& op) {
  switch (op) {
    case IsTypeOp::Null:
    case IsTypeOp::Bool:
    case IsTypeOp::Int:
    case IsTypeOp::Dbl:
    case IsTypeOp::Str:
    case IsTypeOp::Obj:
      return true;
    case IsTypeOp::Res:
    case IsTypeOp::Arr:
    case IsTypeOp::Vec:
    case IsTypeOp::Dict:
    case IsTypeOp::Keyset:
    case IsTypeOp::VArray:
    case IsTypeOp::DArray:
    case IsTypeOp::ArrLike:
    case IsTypeOp::Scalar:
    case IsTypeOp::ClsMeth:
      return false;
  }
  not_reached();
}

template<bool asExpression>
void isAsTypeStructImpl(ISS& env, SArray inputTS) {
  auto const resolvedTS = resolveTSStatically(env, inputTS, env.ctx.cls, true);
  auto const ts = resolvedTS ? resolvedTS : inputTS;
  auto const t = topC(env, 1); // operand to is/as

  bool may_raise = true;
  auto result = [&] (
    const Type& out,
    const folly::Optional<Type>& test = folly::none
  ) {
    if (asExpression && out.subtypeOf(BTrue)) {
      return reduce(env, bc::PopC {});
    }
    auto const location = topStkEquiv(env, 1);
    popC(env); // type structure
    popC(env); // operand to is/as
    if (!asExpression) {
      constprop(env);
      if (!may_raise) nothrow(env);
      return push(env, out);
    }
    if (out.subtypeOf(BFalse)) {
      push(env, t);
      return unreachable(env);
    }

    assertx(out == TBool);
    if (!test) return push(env, t);
    auto const newT = intersection_of(*test, t);
    if (newT == TBottom || !refineLocation(env, location, [&] (Type t) {
          auto ret = intersection_of(*test, t);
          if (test->couldBe(BInitNull) && t.couldBe(BUninit)) {
            ret |= TUninit;
          }
          return ret;
        })) {
      unreachable(env);
    }
    return push(env, newT);
  };

  auto check = [&] (
    const folly::Optional<Type> type,
    const folly::Optional<Type> deopt = folly::none
  ) {
    if (!type || is_type_might_raise(*type, t)) return result(TBool);
    auto test = type.value();
    if (t.couldBe(BClsMeth)) {
      if (RuntimeOption::EvalHackArrDVArrs) {
        if (test == TVec) {
          if (t.subtypeOf(BClsMeth | BVec)) return result(TTrue);
          else if (t.couldBe(BVec)) return result(TBool);
          else test = TClsMeth;
        } else if (test == TVArr) {
          if (t.subtypeOf(BClsMeth | BVArr)) return result(TTrue);
          else if (t.couldBe(BVArr)) return result(TBool);
          else test = TClsMeth;
        }
      } else {
        if (test == TVArr) {
          if (t.subtypeOf(BClsMeth | BVArr)) return result(TTrue);
          else if (t.couldBe(BVArr)) return result(TBool);
          else test = TClsMeth;
        } else if (test == TArr) {
          if (t.subtypeOf(BClsMeth | BArr)) return result(TTrue);
          else if (t.couldBe(BArr)) return result(TBool);
          else test = TClsMeth;
        }
      }
    }
    if (t.subtypeOf(test)) return result(TTrue);
    if (!t.couldBe(test) && (!deopt || !t.couldBe(deopt.value()))) {
      return result(TFalse);
    }
    auto const op = type_to_istypeop(test);
    if (asExpression || !op || !isValidTypeOpForIsAs(op.value())) {
      return result(TBool, test);
    }
    return reduce(env, bc::PopC {}, bc::IsTypeC { *op });
  };

  auto const is_nullable_ts = is_ts_nullable(ts);
  auto const is_definitely_null = t.subtypeOf(BNull);
  auto const is_definitely_not_null = !t.couldBe(BNull);

  if (is_nullable_ts && is_definitely_null) return result(TTrue);

  auto const ts_type = type_of_type_structure(ts);

  if (is_nullable_ts && !is_definitely_not_null && ts_type == folly::none) {
    // Ts is nullable and we know that t could be null but we dont know for sure
    // Also we didn't get a type out of the type structure
    return result(TBool);
  }

  if (!asExpression) {
    if (ts_type && !is_type_might_raise(*ts_type, t)) may_raise = false;
  }
  switch (get_ts_kind(ts)) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_null:
      return check(ts_type);
    case TypeStructure::Kind::T_tuple:
      return RuntimeOption::EvalHackArrCompatIsArrayNotices
        ? check(ts_type, TDArr)
        : check(ts_type);
    case TypeStructure::Kind::T_shape:
      return RuntimeOption::EvalHackArrCompatIsArrayNotices
        ? check(ts_type, TVArr)
        : check(ts_type);
    case TypeStructure::Kind::T_dict:
      return check(ts_type, TDArr);
    case TypeStructure::Kind::T_vec:
      return check(ts_type, TVArr);
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
      return result(TFalse);
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
      return result(TTrue);
    case TypeStructure::Kind::T_nonnull:
      if (is_definitely_null) return result(TFalse);
      if (is_definitely_not_null) return result(TTrue);
      if (!asExpression) {
        return reduce(env,
                      bc::PopC {},
                      bc::IsTypeC { IsTypeOp::Null },
                      bc::Not {});
      }
      return result(TBool);
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      if (asExpression) return result(TBool);
      auto clsname = get_ts_classname(ts);
      auto const rcls = env.index.resolve_class(env.ctx, clsname);
      if (!rcls || !rcls->resolved() || (ts->exists(s_generic_types) &&
                                         (rcls->cls()->hasReifiedGenerics ||
                                         !isTSAllWildcards(ts)))) {
        // If it is a reified class or has non wildcard generics,
        // we need to bail
        return result(TBool);
      }
      return reduce(env, bc::PopC {}, bc::InstanceOfD { clsname });
    }
    case TypeStructure::Kind::T_unresolved: {
      if (asExpression) return result(TBool);
      auto classname = get_ts_classname(ts);
      auto const has_generics = ts->exists(s_generic_types);
      if (!has_generics && classname->isame(s_this.get())) {
        return reduce(env, bc::PopC {}, bc::IsLateBoundCls {});
      }
      auto const rcls = env.index.resolve_class(env.ctx, classname);
      // We can only reduce to instance of if we know for sure that this class
      // can be resolved since instanceof undefined class does not throw
      if (!rcls || !rcls->resolved() || rcls->cls()->attrs & AttrEnum) {
        return result(TBool);
      }
      if (has_generics &&
         (rcls->cls()->hasReifiedGenerics || !isTSAllWildcards(ts))) {
          // If it is a reified class or has non wildcard generics,
          // we need to bail
        return result(TBool);
      }
      return reduce(env, bc::PopC {}, bc::InstanceOfD { rcls->name() });
    }
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
      // TODO(T29232862): implement
      return result(TBool);
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_reifiedtype:
      return result(TBool);
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
      // We will error on these at the JIT
      return result(TBool);
  }

  not_reached();
}

bool canReduceToDontResolve(SArray ts) {
  switch (get_ts_kind(ts)) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_resource:
    // Following ones don't reify, so no need to check the generics
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
      return true;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_enum:
      // If it does not have generics, then we can reduce
      // If it has generics but all of them are wildcards, then we can reduce
      // Otherwise, we can't.
      return isTSAllWildcards(ts);
    case TypeStructure::Kind::T_tuple: {
      auto result = true;
      IterateV(
        get_ts_elem_types(ts),
        [&](TypedValue v) {
          assertx(isArrayLikeType(v.m_type));
          result &= canReduceToDontResolve(v.m_data.parr);
           // when result is false, we can short circuit
          return !result;
        }
      );
      return result;
    }
    case TypeStructure::Kind::T_shape: {
      auto result = true;
      IterateV(
        get_ts_fields(ts),
        [&](TypedValue v) {
          assertx(isArrayLikeType(v.m_type));
          auto const arr = v.m_data.parr;
          if (arr->exists(s_is_cls_cns)) {
            result = false;
            return true; // short circuit
          }
          result &= canReduceToDontResolve(get_ts_value(arr));
           // when result is false, we can short circuit
          return !result;
        }
      );
      return result;
    }
    // Following needs to be resolved
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    // Following cannot be used in is/as expressions, we need to error on them
    // Currently erroring happens as a part of the resolving phase,
    // so keep resolving them
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_reifiedtype:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
      return false;
  }
  not_reached();
}

} // namespace

void in(ISS& env, const bc::IsLateBoundCls& op) {
  auto const cls = env.ctx.cls;
  if (cls && !(cls->attrs & AttrTrait)) effect_free(env);
  popC(env);
  return push(env, TBool);
}

void in(ISS& env, const bc::IsTypeStructC& op) {
  auto const requiredTSType = RuntimeOption::EvalHackArrDVArrs ? BDict : BDArr;
  if (!topC(env).couldBe(requiredTSType)) {
    popC(env);
    popC(env);
    return unreachable(env);
  }
  auto const a = tv(topC(env));
  if (!a || !isValidTSType(*a, false)) {
    popC(env);
    popC(env);
    return push(env, TBool);
  }
  if (op.subop1 == TypeStructResolveOp::Resolve &&
      canReduceToDontResolve(a->m_data.parr)) {
    return reduce(env, bc::IsTypeStructC { TypeStructResolveOp::DontResolve });
  }
  isAsTypeStructImpl<false>(env, a->m_data.parr);
}

void in(ISS& env, const bc::ThrowAsTypeStructException& op) {
  popC(env);
  popC(env);
  unreachable(env);
  return;
}

void in(ISS& env, const bc::CombineAndResolveTypeStruct& op) {
  assertx(op.arg1 > 0);
  auto valid = true;
  auto const requiredTSType = RuntimeOption::EvalHackArrDVArrs ? BDict : BDArr;
  auto const first = tv(topC(env));
  if (first && isValidTSType(*first, false)) {
    auto const ts = first->m_data.parr;
    // Optimize single input that does not need any combination
    if (op.arg1 == 1) {
      if (canReduceToDontResolve(ts)) return reduce(env);
      if (auto const resolved = resolveTSStatically(env, ts, env.ctx.cls,
                                                    false)) {
        return RuntimeOption::EvalHackArrDVArrs
          ? reduce(env, bc::PopC {}, bc::Dict  { resolved })
          : reduce(env, bc::PopC {}, bc::Array { resolved });
      }
    }
    // Optimize double input that needs a single combination and looks of the
    // form ?T, @T or ~T
    if (op.arg1 == 2 && get_ts_kind(ts) == TypeStructure::Kind::T_reifiedtype) {
      BytecodeVec instrs { bc::PopC {} };
      auto const tv_true = gen_constant(make_tv<KindOfBoolean>(true));
      if (ts->exists(s_like.get())) {
        instrs.push_back(gen_constant(make_tv<KindOfString>(s_like.get())));
        instrs.push_back(tv_true);
        instrs.push_back(bc::AddElemC {});
      }
      if (ts->exists(s_nullable.get())) {
        instrs.push_back(gen_constant(make_tv<KindOfString>(s_nullable.get())));
        instrs.push_back(tv_true);
        instrs.push_back(bc::AddElemC {});
      }
      if (ts->exists(s_soft.get())) {
        instrs.push_back(gen_constant(make_tv<KindOfString>(s_soft.get())));
        instrs.push_back(tv_true);
        instrs.push_back(bc::AddElemC {});
      }
      return reduce(env, std::move(instrs));
    }
  }

  for (int i = 0; i < op.arg1; ++i) {
    auto const t = popC(env);
    valid &= t.couldBe(requiredTSType);
  }
  if (!valid) return unreachable(env);
  nothrow(env);
  push(env, Type{requiredTSType});
}

void in(ISS& env, const bc::RecordReifiedGeneric& op) {
  // TODO(T31677864): implement real optimizations
  auto const t = popC(env);
  if (!t.couldBe(RuntimeOption::EvalHackArrDVArrs ? BVec : BVArr)) {
    return unreachable(env);
  }
  nothrow(env);
  push(env, RuntimeOption::EvalHackArrDVArrs ? TSVec : TSVArr);
}

void in(ISS& env, const bc::ReifiedName& op) {
  // TODO(T31677864): implement real optimizations
  auto const t = popC(env);
  if (!t.couldBe(RuntimeOption::EvalHackArrDVArrs ? BVec : BVArr)) {
    return unreachable(env);
  }
  nothrow(env);
  return push(env, rname(op.str1));
}

void in(ISS& env, const bc::CheckReifiedGenericMismatch& op) {
  // TODO(T31677864): implement real optimizations
  popC(env);
}

namespace {

/*
 * If the value on the top of the stack is known to be equivalent to the local
 * its being moved/copied to, return folly::none without modifying any
 * state. Otherwise, pop the stack value, perform the set, and return a pair
 * giving the value's type, and any other local its known to be equivalent to.
 */
template <typename Set>
folly::Optional<std::pair<Type, LocalId>> moveToLocImpl(ISS& env,
                                                        const Set& op) {
  if (auto const prev = last_op(env, 1)) {
    if (prev->op == Op::CGetL2 &&
        prev->CGetL2.loc1 == op.loc1 &&
        last_op(env)->op == Op::Concat) {
      rewind(env, 2);
      reduce(env, bc::SetOpL { op.loc1, SetOpOp::ConcatEqual });
      return folly::none;
    }
  }

  auto equivLoc = topStkEquiv(env);
  // If the local could be a Ref, don't record equality because the stack
  // element and the local won't actually have the same type.
  if (!locCouldBeRef(env, op.loc1)) {
    if (equivLoc == StackThisId && env.state.thisLoc != NoLocalId) {
      if (env.state.thisLoc == op.loc1 ||
                 locsAreEquiv(env, env.state.thisLoc, op.loc1)) {
        return folly::none;
      } else {
        equivLoc = env.state.thisLoc;
      }
    }
    assertx(!is_volatile_local(env.ctx.func, op.loc1));
    if (equivLoc <= MaxLocalId) {
      if (equivLoc == op.loc1 ||
          locsAreEquiv(env, equivLoc, op.loc1)) {
        // We allow equivalency to ignore Uninit, so we need to check
        // the types here.
        if (peekLocRaw(env, op.loc1) == topC(env)) {
          return folly::none;
        }
      }
    } else if (equivLoc == NoLocalId) {
      equivLoc = op.loc1;
    }
    if (!any(env.collect.opts & CollectionOpts::Speculating)) {
      effect_free(env);
    }
  } else {
    equivLoc = NoLocalId;
  }
  nothrow(env);
  auto val = popC(env);
  setLoc(env, op.loc1, val);
  if (equivLoc == StackThisId) {
    assertx(env.state.thisLoc == NoLocalId);
    equivLoc = env.state.thisLoc = op.loc1;
  }
  if (equivLoc == StackDupId) {
    setStkLocal(env, op.loc1);
  } else if (equivLoc != op.loc1 && equivLoc != NoLocalId) {
    addLocEquiv(env, op.loc1, equivLoc);
  }
  return { std::make_pair(std::move(val), equivLoc) };
}

}

void in(ISS& env, const bc::PopL& op) {
  // If the same value is already in the local, do nothing but pop
  // it. Otherwise, the set has been done by moveToLocImpl.
  if (!moveToLocImpl(env, op)) return reduce(env, bc::PopC {});
}

void in(ISS& env, const bc::SetL& op) {
  // If the same value is already in the local, do nothing because SetL keeps
  // the value on the stack. If it isn't, we need to push it back onto the stack
  // because moveToLocImpl popped it.
  if (auto p = moveToLocImpl(env, op)) {
    push(env, std::move(p->first), p->second);
  } else {
    reduce(env);
  }
}

void in(ISS& env, const bc::SetG&) {
  auto t1 = popC(env);
  popC(env);
  push(env, std::move(t1));
}

void in(ISS& env, const bc::SetS& op) {
  auto const t1    = popC(env);
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      mergeSelfProp(env, vname->m_data.pstr, t1);
    } else {
      mergeEachSelfPropRaw(env, [&] (Type) { return t1; });
    }
  }

  env.collect.publicSPropMutations.merge(env.index, env.ctx, tcls, tname, t1);

  push(env, std::move(t1));
}

void in(ISS& env, const bc::SetOpL& op) {
  auto const t1     = popC(env);
  auto const v1     = tv(t1);
  auto const loc    = locAsCell(env, op.loc1);
  auto const locVal = tv(loc);
  if (v1 && locVal) {
    // Can't constprop at this eval_cell, because of the effects on
    // locals.
    auto resultTy = eval_cell([&] {
      Cell c = *locVal;
      Cell rhs = *v1;
      setopBody(&c, op.subop2, &rhs);
      return c;
    });
    if (!resultTy) resultTy = TInitCell;

    // We may have inferred a TSStr or TSArr with a value here, but
    // at runtime it will not be static.  For now just throw that
    // away.  TODO(#3696042): should be able to loosen_staticness here.
    if (resultTy->subtypeOf(BStr)) resultTy = TStr;
    else if (resultTy->subtypeOf(BArr)) resultTy = TArr;
    else if (resultTy->subtypeOf(BVec)) resultTy = TVec;
    else if (resultTy->subtypeOf(BDict)) resultTy = TDict;
    else if (resultTy->subtypeOf(BKeyset)) resultTy = TKeyset;

    setLoc(env, op.loc1, *resultTy);
    push(env, *resultTy);
    return;
  }

  auto resultTy = typeSetOp(op.subop2, loc, t1);
  setLoc(env, op.loc1, resultTy);
  push(env, std::move(resultTy));
}

void in(ISS& env, const bc::SetOpG&) {
  popC(env); popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::SetOpS& op) {
  popC(env);
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      mergeSelfProp(env, vname->m_data.pstr, TInitCell);
    } else {
      loseNonRefSelfPropTypes(env);
    }
  }

  env.collect.publicSPropMutations.merge(
    env.index, env.ctx, tcls, tname, TInitCell
  );

  push(env, TInitCell);
}

void in(ISS& env, const bc::IncDecL& op) {
  auto loc = locAsCell(env, op.loc1);
  auto newT = typeIncDec(op.subop2, loc);
  auto const pre = isPre(op.subop2);

  // If it's a non-numeric string, this may cause it to exceed the max length.
  if (!locCouldBeUninit(env, op.loc1) &&
      !loc.couldBe(BStr)) {
    nothrow(env);
  }

  if (!pre) push(env, std::move(loc));
  setLoc(env, op.loc1, newT);
  if (pre)  push(env, std::move(newT));
}

void in(ISS& env, const bc::IncDecG&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::IncDecS& op) {
  auto const tcls  = takeClsRefSlot(env, op.slot);
  auto const tname = popC(env);
  auto const vname = tv(tname);
  auto const self  = selfCls(env);

  if (!self || tcls.couldBe(*self)) {
    if (vname && vname->m_type == KindOfPersistentString) {
      mergeSelfProp(env, vname->m_data.pstr, TInitCell);
    } else {
      loseNonRefSelfPropTypes(env);
    }
  }

  env.collect.publicSPropMutations.merge(
    env.index, env.ctx, tcls, tname, TInitCell
  );

  push(env, TInitCell);
}

void in(ISS& env, const bc::UnsetL& op) {
  if (locRaw(env, op.loc1).subtypeOf(TUninit)) {
    return reduce(env);
  }
  if (any(env.collect.opts & CollectionOpts::Speculating)) {
    nothrow(env);
  } else {
    effect_free(env);
  }
  setLocRaw(env, op.loc1, TUninit);
}

void in(ISS& env, const bc::UnsetG& /*op*/) {
  auto const t1 = popC(env);
  if (!t1.couldBe(BObj | BRes)) nothrow(env);
}

template<class FCallWithFCA>
bool fcallOptimizeChecks(
  ISS& env,
  const FCallArgs& fca,
  const res::Func& func,
  FCallWithFCA fcallWithFCA,
  const ActRec* legacyAR = nullptr
) {
  if (fca.enforceReffiness()) {
    bool match = true;
    for (auto i = 0; i < fca.numArgs; ++i) {
      auto const kind = env.index.lookup_param_prep(env.ctx, func, i);
      if (legacyAR && legacyAR->foldable && kind != PrepKind::Val) {
        fpiNotFoldable(env);
        fpiPop(env);
        discard(env, fca.numArgsInclUnpack());
        for (auto j = uint32_t{0}; j < fca.numRets; ++j) push(env, TBottom);
        return true;
      }

      if (kind == PrepKind::Unknown) {
        match = false;
        break;
      }

      if (kind != (fca.byRef(i) ? PrepKind::Ref : PrepKind::Val)) {
        // Reffiness mismatch
        auto const exCls = makeStaticString("InvalidArgumentException");
        auto const err = makeStaticString(formatParamRefMismatch(
          func.name()->data(), i, !fca.byRef(i)));

        reduce(
          env,
          bc::NewObjD { exCls },
          bc::Dup {},
          bc::NullUninit {},
          bc::NullUninit {},
          bc::String { err },
          bc::FCallCtor { FCallArgs(1), staticEmptyString() },
          bc::PopC {},
          bc::LockObj {},
          bc::Throw {}
        );
        return true;
      }
    }

    if (match) {
      // Optimize away the runtime reffiness check.
      reduce(env, fcallWithFCA(FCallArgs(
        fca.flags, fca.numArgs, fca.numRets, nullptr, fca.asyncEagerTarget,
        fca.constructNoConst)));
      return true;
    }
  }

  // Infer whether the callee supports async eager return.
  if (fca.asyncEagerTarget != NoBlockId &&
      !fca.supportsAsyncEagerReturn()) {
    auto const status = env.index.supports_async_eager_return(func);
    if (status) {
      auto newFCA = fca;
      if (*status) {
        // Callee supports async eager return.
        newFCA.flags = static_cast<FCallArgs::Flags>(
          newFCA.flags | FCallArgs::SupportsAsyncEagerReturn);
      } else {
        // Callee doesn't support async eager return.
        newFCA.asyncEagerTarget = NoBlockId;
      }
      reduce(env, fcallWithFCA(std::move(newFCA)));
      return true;
    }
  }

  return false;
}

bool fcallTryFold(
  ISS& env,
  const FCallArgs& fca,
  const res::Func& func,
  Type context,
  bool maybeDynamic,
  uint32_t numExtraInputs,
  ClsRefSlotId clsRefSlot,
  const ActRec* legacyAR = nullptr
) {
  auto const foldableFunc = func.exactFunc();
  if (!foldableFunc) {
    assertx(!legacyAR || !legacyAR->foldable);
    return false;
  }

  if (legacyAR) {
    if (!legacyAR->foldable) return false;
  } else if (!canFold(env, foldableFunc, fca.numArgs, context, maybeDynamic)) {
    return false;
  }

  auto tried_lookup = false;
  if (options.ConstantFoldBuiltins &&
      !fca.hasUnpack() &&
      fca.numRets == 1) {
    auto ty = [&] () {
      if (foldableFunc->attrs & AttrBuiltin &&
          foldableFunc->attrs & AttrIsFoldable) {
        auto ret = const_fold(env, fca.numArgs, *foldableFunc);
        return ret ? *ret : TBottom;
      }
      CompactVector<Type> args(fca.numArgs);
      for (auto i = uint32_t{0}; i < fca.numArgs; ++i) {
        auto const& arg = topT(env, i);
        auto const argNum = fca.numArgs - i - 1;
        auto const isScalar = is_scalar(arg);
        if (!isScalar &&
            (env.index.func_depends_on_arg(foldableFunc, argNum) ||
             !arg.subtypeOf(BInitCell))) {
          return TBottom;
        }
        args[argNum] = isScalar ? scalarize(arg) : arg;
      }

      tried_lookup = true;
      return env.index.lookup_foldable_return_type(
        env.ctx, foldableFunc, context, std::move(args));
    }();

    if (auto v = tv(ty)) {
      BytecodeVec repl;
      if (clsRefSlot != NoClsRefSlotId) {
        repl.push_back(bc::DiscardClsRef { clsRefSlot });
      }
      for (uint32_t i = 0; i < numExtraInputs; ++i) repl.push_back(bc::PopC {});
      for (uint32_t i = 0; i < fca.numArgs; ++i) repl.push_back(bc::PopC {});
      repl.push_back(bc::PopU {});
      repl.push_back(bc::PopU {});
      if (topT(env, fca.numArgs + 2 + numExtraInputs).subtypeOf(TInitCell)) {
        repl.push_back(bc::PopC {});
      } else {
        assertx(topT(env, fca.numArgs + 2 + numExtraInputs).subtypeOf(TUninit));
        repl.push_back(bc::PopU {});
      }
      repl.push_back(gen_constant(*v));
      if (legacyAR) fpiPop(env);
      reduce(env, std::move(repl));
      return true;
    }
  }

  if (legacyAR) {
    assertx(legacyAR->foldable);
    assertx(numExtraInputs == 0);
    assertx(clsRefSlot == NoClsRefSlotId);
    fpiNotFoldable(env);
    fpiPop(env);
    discard(env, fca.numArgsInclUnpack());
    for (auto i = uint32_t{0}; i < fca.numRets; ++i) push(env, TBottom);
    return true;
  }

  if (tried_lookup) {
    env.collect.unfoldableFuncs.emplace(foldableFunc, env.bid);
  }
  return false;
}

Type typeFromWH(Type t) {
  if (!t.couldBe(BObj)) {
    // Exceptions will be thrown if a non-object is awaited.
    return TBottom;
  }

  // Throw away non-obj component.
  t &= TObj;

  // If we aren't even sure this is a wait handle, there's nothing we can
  // infer here.
  if (!is_specialized_wait_handle(t)) {
    return TInitCell;
  }

  return wait_handle_inner(t);
}

void pushCallReturnType(ISS& env, Type&& ty, const FCallArgs& fca) {
  if (ty == TBottom) {
    // The callee function never returns.  It might throw, or loop forever.
    unreachable(env);
  }
  if (fca.numRets != 1) {
    assertx(fca.asyncEagerTarget == NoBlockId);
    for (auto i = uint32_t{0}; i < fca.numRets - 1; ++i) popU(env);
    if (is_specialized_vec(ty)) {
      for (int32_t i = 1; i < fca.numRets; i++) {
        push(env, vec_elem(ty, ival(i)).first);
      }
      push(env, vec_elem(ty, ival(0)).first);
    } else {
      for (int32_t i = 0; i < fca.numRets; i++) push(env, TInitCell);
    }
    return;
  }
  if (fca.asyncEagerTarget != NoBlockId) {
    push(env, typeFromWH(ty));
    assertx(topC(env) != TBottom);
    env.propagate(fca.asyncEagerTarget, &env.state);
    popC(env);
  }
  return push(env, std::move(ty));
}

const StaticString s_defined { "defined" };
const StaticString s_function_exists { "function_exists" };

template<class FCallWithFCA>
void fcallKnownImpl(
  ISS& env,
  const FCallArgs& fca,
  const res::Func& func,
  Type context,
  bool nullsafe,
  uint32_t numExtraInputs,
  ClsRefSlotId clsRefSlot,
  FCallWithFCA fcallWithFCA,
  bool legacy = false
) {
  auto returnType = [&] {
    CompactVector<Type> args(fca.numArgs);
    auto const firstArgPos = fca.numArgsInclUnpack() - 1;
    for (auto i = uint32_t{0}; i < fca.numArgs; ++i) {
      args[i] = topCV(env, firstArgPos - i);
    }

    auto ty = fca.hasUnpack()
      ? env.index.lookup_return_type(env.ctx, func)
      : env.index.lookup_return_type(env.ctx, args, context, func);
    if (nullsafe) {
      ty = union_of(std::move(ty), TInitNull);
    }
    return ty;
  }();

  if (fca.asyncEagerTarget != NoBlockId && typeFromWH(returnType) == TBottom) {
    // Kill the async eager target if the function never returns.
    auto newFCA = fca;
    newFCA.asyncEagerTarget = NoBlockId;
    newFCA.flags = static_cast<FCallArgs::Flags>(
      newFCA.flags & ~FCallArgs::SupportsAsyncEagerReturn);
    reduce(env, fcallWithFCA(std::move(newFCA)));
    return;
  }

  if (!fca.hasUnpack() && func.name()->isame(s_function_exists.get())) {
    handle_function_exists(env, fca.numArgs, false);
  }

  if (options.HardConstProp &&
      fca.numArgs == 1 &&
      !fca.hasUnpack() &&
      func.name()->isame(s_defined.get())) {
    // If someone calls defined('foo') they probably want foo to be
    // defined normally; ie not a persistent constant.
    if (auto const v = tv(topCV(env))) {
      if (isStringType(v->m_type) &&
          !env.index.lookup_constant(env.ctx, v->m_data.pstr)) {
        env.collect.cnsMap[v->m_data.pstr].m_type = kDynamicConstant;
      }
    }
  }

  if (clsRefSlot != NoClsRefSlotId) takeClsRefSlot(env, clsRefSlot);
  for (auto i = uint32_t{0}; i < numExtraInputs; ++i) popC(env);
  if (fca.hasUnpack()) popC(env);
  for (auto i = uint32_t{0}; i < fca.numArgs; ++i) popCV(env);
  if (legacy) {
    fpiPop(env);
  } else {
    popU(env);
    popU(env);
    popCU(env);
  }
  pushCallReturnType(env, std::move(returnType), fca);
}

void fcallUnknownImpl(ISS& env, const FCallArgs& fca, bool legacy = false) {
  if (fca.hasUnpack()) popC(env);
  for (auto i = uint32_t{0}; i < fca.numArgs; ++i) popCV(env);
  if (legacy) {
    fpiPop(env);
  } else {
    popU(env);
    popU(env);
    popCU(env);
  }
  if (fca.asyncEagerTarget != NoBlockId) {
    assertx(fca.numRets == 1);
    push(env, TInitCell);
    env.propagate(fca.asyncEagerTarget, &env.state);
    popC(env);
  }
  for (auto i = uint32_t{0}; i < fca.numRets - 1; ++i) popU(env);
  for (auto i = uint32_t{0}; i < fca.numRets; ++i) push(env, TInitCell);
}

namespace {

void fPushFuncDImpl(ISS& env, const res::Func& rfunc, int32_t nArgs,
                    bool has_unpack, bool extra_arg) {
  if (!any(env.collect.opts & CollectionOpts::Speculating)) {
    if (auto const func = rfunc.exactFunc()) {
      if (will_reduce(env) && can_emit_builtin(func, nArgs, has_unpack)) {
        fpiPushNoFold(
          env,
          ActRec { FPIKind::Builtin, TBottom, folly::none, rfunc }
        );
        if (extra_arg) return reduce(env, bc::PopC {});
        return reduce(env);
      }
    }
  }
  if (fpiPush(env, ActRec { FPIKind::Func, TBottom, folly::none, rfunc },
              nArgs, false)) {
    if (extra_arg) return reduce(env, bc::PopC {});
    return reduce(env);
  }
  if (extra_arg) popC(env);
  discardAR(env, nArgs);
}

} // namespace

void in(ISS& env, const bc::FPushFuncD& op) {
  auto const rfunc = env.index.resolve_func(env.ctx, op.str2);
  fPushFuncDImpl(env, rfunc, op.arg1, op.has_unpack, false);
}

void in(ISS& env, const bc::FPushFuncRD& op) {
  auto const tsList = topC(env);
  if (!tsList.couldBe(RuntimeOption::EvalHackArrDVArrs ? BVec : BVArr)) {
    return unreachable(env);
  }
  auto const rfunc = env.index.resolve_func(env.ctx, op.str2);
  if (!rfunc.couldHaveReifiedGenerics()) {
    return reduce(
      env,
      bc::PopC {},
      bc::FPushFuncD { op.arg1, op.str2, op.has_unpack }
    );
  }
  fPushFuncDImpl(env, rfunc, op.arg1, op.has_unpack, true);
}

void in(ISS& env, const bc::FPushFunc& op) {
  auto const t1 = topC(env);
  folly::Optional<res::Func> rfunc;
  // FPushFuncD requires that the names of inout functions be
  // mangled, so skip those for now.
  auto const name = getNameFromType(t1);
  if (name && op.argv.size() == 0) {
    auto const nname = normalizeNS(name);
    // FPushFuncD doesn't support class-method pair strings yet.
    if (isNSNormalized(nname) && notClassMethodPair(nname)) {
      rfunc = env.index.resolve_func(env.ctx, nname);
      // If the function might distinguish being called dynamically from not,
      // don't turn a dynamic call into a static one.
      if (rfunc && !rfunc->mightCareAboutDynCalls() &&
          !rfunc->couldHaveReifiedGenerics()) {
        return reduce(env, bc::PopC {},
                      bc::FPushFuncD { op.arg1, nname, op.has_unpack });
      }
    }
  }
  popC(env);
  discardAR(env, op.arg1);
  if (t1.subtypeOf(BObj)) {
    fpiPushNoFold(env, ActRec { FPIKind::ObjInvoke, t1 });
  } else if (t1.subtypeOf(BArr)) {
    fpiPushNoFold(env, ActRec { FPIKind::CallableArr, TTop });
  } else if (t1.subtypeOf(BStr)) {
    fpiPushNoFold(env, ActRec { FPIKind::Func, TTop, folly::none, rfunc });
  } else {
    fpiPushNoFold(env, ActRec { FPIKind::Unknown, TTop });
  }
}

void in(ISS& env, const bc::ResolveFunc& op) {
  // TODO (T29639296)
  push(env, TFunc);
}

void in(ISS& env, const bc::ResolveObjMethod& op) {
  // TODO (T29639296)
  popC(env);
  popC(env);
  if (RuntimeOption::EvalHackArrDVArrs) {
    push(env, TVec);
  } else {
    push(env, TVArr);
  }
}

void in(ISS& env, const bc::ResolveClsMethod& op) {
  popC(env);
  popC(env);
  push(env, TClsMeth);
}

namespace {

void fcallObjMethodNullsafe(ISS& env, const FCallArgs& fca, bool extraInput) {
  BytecodeVec repl;
  if (extraInput) repl.push_back(bc::PopC {});
  if (fca.hasUnpack()) repl.push_back(bc::PopC {});
  for (uint32_t i = 0; i < fca.numArgs; ++i) {
    if (topC(env, repl.size()).subtypeOf(BRef)) {
      repl.push_back(bc::PopV {});
    } else {
      assertx(topC(env, repl.size()).subtypeOf(BInitCell));
      repl.push_back(bc::PopC {});
    }
  }
  repl.push_back(bc::PopU {});
  repl.push_back(bc::PopU {});
  repl.push_back(bc::PopC {});
  for (uint32_t i = 0; i < fca.numRets - 1; ++i) {
    repl.push_back(bc::PopU {});
  }
  repl.push_back(bc::Null {});

  reduce(env, std::move(repl));
}

template <typename Op, class UpdateBC>
void fcallObjMethodImpl(ISS& env, const Op& op, SString methName, bool dynamic,
                        bool extraInput, UpdateBC updateBC) {
  auto const nullThrows = op.subop3 == ObjMethodOp::NullThrows;
  auto const inputPos = op.fca.numArgsInclUnpack() + (extraInput ? 3 : 2);
  auto const input = topC(env, inputPos);
  auto const location = topStkEquiv(env, inputPos);
  auto const mayCallMethod = input.couldBe(BObj);
  auto const mayUseNullsafe = !nullThrows && input.couldBe(BNull);
  auto const mayThrowNonObj = !input.subtypeOf(nullThrows ? BObj : BOptObj);

  auto const refineLoc = [&] {
    if (location == NoLocalId) return;
    if (!refineLocation(env, location, [&] (Type t) {
      if (nullThrows) return intersection_of(t, TObj);
      if (!t.couldBe(BUninit)) return intersection_of(t, TOptObj);
      if (!t.couldBe(BObj)) return intersection_of(t, TNull);
      return t;
    })) {
      unreachable(env);
    }
  };

  auto const unknown = [&] {
    if (extraInput) popC(env);
    fcallUnknownImpl(env, op.fca);
    refineLoc();
  };

  if (!mayCallMethod && !mayUseNullsafe) {
    // This FCallObjMethodD may only throw, make sure it's not optimized away.
    unknown();
    unreachable(env);
    return;
  }

  if (!mayCallMethod && !mayThrowNonObj) {
    // Null input, this may only return null, so do that.
    return fcallObjMethodNullsafe(env, op.fca, extraInput);
  }

  if (!mayCallMethod) {
    // May only return null, but can't fold as we may still throw.
    assertx(mayUseNullsafe && mayThrowNonObj);
    return unknown();
  }

  auto const ctxTy = intersection_of(input, TObj);
  auto const clsTy = objcls(ctxTy);
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, methName);

  auto const canFold = !mayUseNullsafe && !mayThrowNonObj;
  if (fcallOptimizeChecks(env, op.fca, rfunc, updateBC) ||
      (canFold && fcallTryFold(env, op.fca, rfunc, ctxTy, dynamic,
                               extraInput ? 1 : 0, NoClsRefSlotId))) {
    return;
  }

  if (rfunc.exactFunc() && rfunc.cantBeMagicCall() && op.str2->empty()) {
    return reduce(env, updateBC(op.fca, rfunc.exactFunc()->cls->name));
  }

  fcallKnownImpl(env, op.fca, rfunc, ctxTy, mayUseNullsafe, extraInput ? 1 : 0,
                 NoClsRefSlotId, updateBC);
  refineLoc();
}

} // namespace

void in(ISS& env, const bc::FCallObjMethodD& op) {
  auto const updateBC = [&] (FCallArgs fca, SString clsHint = nullptr) {
    if (!clsHint) clsHint = op.str2;
    return bc::FCallObjMethodD { std::move(fca), clsHint, op.subop3, op.str4 };
  };
  fcallObjMethodImpl(env, op, op.str4, false, false, updateBC);
}

void in(ISS& env, const bc::FCallObjMethodRD& op) {
  auto const tsList = topC(env);
  if (!tsList.couldBe(RuntimeOption::EvalHackArrDVArrs ? BVec : BVArr)) {
    return unreachable(env);
  }

  auto const input = topC(env, op.fca.numArgsInclUnpack() + 3);
  auto const clsTy = objcls(intersection_of(input, TObj));
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, op.str4);
  if (!rfunc.couldHaveReifiedGenerics()) {
    return reduce(
      env,
      bc::PopC {},
      bc::FCallObjMethodD { op.fca, op.str2, op.subop3, op.str4 }
    );
  }

  auto const updateBC = [&] (FCallArgs fca, SString clsHint = nullptr) {
    if (!clsHint) clsHint = op.str2;
    return bc::FCallObjMethodRD { std::move(fca), clsHint, op.subop3, op.str4 };
  };
  fcallObjMethodImpl(env, op, op.str4, false, true, updateBC);
}

void in(ISS& env, const bc::FCallObjMethod& op) {
  auto const methName = getNameFromType(topC(env));
  if (!methName || op.argv.size() != 0) {
    popC(env);
    fcallUnknownImpl(env, op.fca);
    return;
  }

  auto const input = topC(env, op.fca.numArgsInclUnpack() + 3);
  auto const clsTy = objcls(intersection_of(input, TObj));
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, methName);
  if (!rfunc.mightCareAboutDynCalls()) {
    return reduce(
      env,
      bc::PopC {},
      bc::FCallObjMethodD { op.fca, op.str2, op.subop3, methName }
    );
  }

  auto const updateBC = [&] (FCallArgs fca, SString clsHint = nullptr) {
    if (!clsHint) clsHint = op.str2;
    return bc::FCallObjMethod { std::move(fca), clsHint, op.subop3, op.argv };
  };
  fcallObjMethodImpl(env, op, methName, true, true, updateBC);
}

namespace {

template <typename Op>
void fpushClsMethodImpl(ISS& env, const Op& op, Type clsTy, SString methName,
                        bool dynamic, bool extraInput,
                        ClsRefSlotId clsRefSlot) {
  auto const rcls = is_specialized_cls(clsTy)
    ? folly::Optional<res::Class>(dcls_of(clsTy).cls)
    : folly::none;
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, methName);
  auto const ar = ActRec { FPIKind::ClsMeth, clsTy, rcls, rfunc };
  if (fpiPush(env, ar, op.arg1, dynamic)) {
    if (clsRefSlot != NoClsRefSlotId) {
      reduce(env, bc::DiscardClsRef { clsRefSlot });
    }
    if (extraInput) reduce(env, bc::PopC {});
    return reduce(env);
  }

  if (clsRefSlot != NoClsRefSlotId) takeClsRefSlot(env, clsRefSlot);
  if (extraInput) popC(env);
  discardAR(env, op.arg1);
}

} // namespace

void in(ISS& env, const bc::FPushClsMethodD& op) {
  auto const rcls = env.index.resolve_class(env.ctx, op.str3);
  auto const clsTy = rcls ? clsExact(*rcls) : TCls;
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, op.str2);
  if (auto const func = rfunc.exactFunc()) {
    assertx(func->cls != nullptr);
    if (will_reduce(env) &&
        !any(env.collect.opts & CollectionOpts::Speculating) &&
        func->cls->name->same(op.str3) &&
        can_emit_builtin(func, op.arg1, op.has_unpack)) {
      // When we use FCallBuiltin to call a static method, the litstr method
      // name will be a fully qualified cls::fn (e.g. "HH\Map::fromItems").
      //
      // As a result, we can only do this optimization if the name of the
      // builtin function's class matches this op's class name immediate.
      auto const ar = ActRec { FPIKind::Builtin, TBottom, folly::none, rfunc };
      fpiPushNoFold(env, ar);
      return reduce(env);
    }
  }

  fpushClsMethodImpl(env, op, clsTy, op.str2, false, false, NoClsRefSlotId);
}

void in(ISS& env, const bc::FPushClsMethodRD& op) {
  auto const rcls = env.index.resolve_class(env.ctx, op.str3);
  auto const clsTy = rcls ? clsExact(*rcls) : TCls;
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, op.str2);
  if (!rfunc.couldHaveReifiedGenerics()) {
    return reduce(
      env,
      bc::PopC {},
      bc::FPushClsMethodD { op.arg1, op.str2, op.str3, op.has_unpack }
    );
  }

  // Builtins do not support reified generics.
  assertx(!rfunc.exactFunc() || !rfunc.exactFunc()->nativeInfo);
  fpushClsMethodImpl(env, op, clsTy, op.str2, false, true, NoClsRefSlotId);
}

void in(ISS& env, const bc::FPushClsMethod& op) {
  auto const methName = getNameFromType(topC(env));
  if (!methName || op.argv.size() != 0) {
    takeClsRefSlot(env, op.slot);
    popC(env);
    discardAR(env, op.arg1);
    fpiPushNoFold(env, ActRec { FPIKind::ClsMeth, TCls });
    return;
  }

  auto const clsTy = peekClsRefSlot(env, op.slot);
  auto const rfunc = env.index.resolve_method(env.ctx, clsTy, methName);
  if (is_specialized_cls(clsTy) && dcls_of(clsTy).type == DCls::Exact &&
      !rfunc.mightCareAboutDynCalls()) {
    auto const clsName = dcls_of(clsTy).cls.name();
    return reduce(
      env,
      bc::DiscardClsRef { op.slot },
      bc::PopC {},
      bc::FPushClsMethodD { op.arg1, methName, clsName, op.has_unpack }
    );
  }

  fpushClsMethodImpl(env, op, clsTy, methName, true, true, op.slot);
}

namespace {

Type ctxCls(ISS& env) {
  auto const s = selfCls(env);
  return setctx(s ? *s : TCls);
}

Type specialClsRefToCls(ISS& env, SpecialClsRef ref) {
  if (!env.ctx.cls) return TCls;
  auto const op = [&]()-> folly::Optional<Type> {
    switch (ref) {
      case SpecialClsRef::Static: return ctxCls(env);
      case SpecialClsRef::Self:   return selfClsExact(env);
      case SpecialClsRef::Parent: return parentClsExact(env);
    }
    always_assert(false);
  }();
  return op ? *op : TCls;
}

}

void in(ISS& env, const bc::FPushClsMethodS& op) {
  auto const t1  = topC(env);
  auto const cls = specialClsRefToCls(env, op.subop2);
  folly::Optional<res::Func> rfunc;
  auto const name = getNameFromType(t1);
  if (name && op.argv.size() == 0) {
    rfunc = env.index.resolve_method(env.ctx, cls, name);
    if (!rfunc->mightCareAboutDynCalls() &&
        !rfunc->couldHaveReifiedGenerics()) {
      return reduce(
        env,
        bc::PopC {},
        bc::FPushClsMethodSD {
          op.arg1, op.subop2, name, op.has_unpack
        }
      );
    }
  }
  auto const rcls = is_specialized_cls(cls)
    ? folly::Optional<res::Class>{dcls_of(cls).cls}
    : folly::none;
  if (fpiPush(env, ActRec {
                FPIKind::ClsMeth,
                ctxCls(env),
                rcls,
                rfunc
              }, op.arg1, true)) {
    return reduce(env, bc::PopC {});
  }
  popC(env);
  discardAR(env, op.arg1);
}

namespace {

template <typename Op>
void implFPushClsMethodSD(ISS& env, const Op& op, bool isRFlavor) {
  auto const cls = specialClsRefToCls(env, op.subop2);

  folly::Optional<res::Class> rcls;
  auto exactCls = false;
  if (is_specialized_cls(cls)) {
    auto dcls = dcls_of(cls);
    rcls = dcls.cls;
    exactCls = dcls.type == DCls::Exact;
  }

  if (op.subop2 == SpecialClsRef::Static && rcls && exactCls) {
    if (isRFlavor) {
      return reduce(
        env,
        bc::FPushClsMethodRD {
          op.arg1, op.str3, rcls->name(), op.has_unpack
        }
      );
    }
    return reduce(
      env,
      bc::FPushClsMethodD {
        op.arg1, op.str3, rcls->name(), op.has_unpack
      }
    );
  }

  auto const rfun = env.index.resolve_method(env.ctx, cls, op.str3);
  if (isRFlavor && !rfun.couldHaveReifiedGenerics()) {
    return reduce(
      env,
      bc::PopC {},
      bc::FPushClsMethodSD { op.arg1, op.subop2, op.str3, op.has_unpack  }
    );
  }
  if (fpiPush(env, ActRec {
                FPIKind::ClsMeth,
                ctxCls(env),
                rcls,
                rfun
              }, op.arg1, false)) {
    assertx(!isRFlavor);
    return reduce(env);
  }
  if (isRFlavor) popC(env);
  discardAR(env, op.arg1);
}

} // namespace

void in(ISS& env, const bc::FPushClsMethodSD& op) {
  implFPushClsMethodSD(env, op, false);
}

void in(ISS& env, const bc::FPushClsMethodSRD& op) {
  implFPushClsMethodSD(env, op, true);
}

namespace {

void newObjDImpl(ISS& env, const StringData* className, bool rflavor) {
  auto const rcls = env.index.resolve_class(env.ctx, className);
  if (!rcls) {
    if (rflavor) popC(env);
    push(env, TObj);
    return;
  }
  if (rflavor && !rcls->couldHaveReifiedGenerics()) {
    return reduce(env, bc::PopC {}, bc::NewObjD { className });
  }
  auto const isCtx = !rcls->couldBeOverriden() && env.ctx.cls &&
    rcls->same(env.index.resolve_class(env.ctx.cls));
  if (rflavor) popC(env);
  push(env, setctx(objExact(*rcls), isCtx));
}

} // namespace

void in(ISS& env, const bc::NewObjD& op)  { newObjDImpl(env, op.str1, false); }
void in(ISS& env, const bc::NewObjRD& op) { newObjDImpl(env, op.str1, true);  }

void in(ISS& env, const bc::NewObjS& op) {
  auto const cls = specialClsRefToCls(env, op.subop1);
  if (!is_specialized_cls(cls)) {
    push(env, TObj);
    return;
  }

  auto const dcls = dcls_of(cls);
  auto const exact = dcls.type == DCls::Exact;
  if (exact && !dcls.cls.couldHaveReifiedGenerics() &&
      (!dcls.cls.couldBeOverriden() || equivalently_refined(cls, unctx(cls)))) {
    return reduce(env, bc::NewObjD { dcls.cls.name() });
  }

  push(env, toobj(cls));
}

void in(ISS& env, const bc::NewObj& op) {
  auto const cls = peekClsRefSlot(env, op.slot);
  if (!is_specialized_cls(cls) || op.subop2 == HasGenericsOp::MaybeGenerics) {
    takeClsRefSlot(env, op.slot);
    push(env, TObj);
    return;
  }

  auto const dcls = dcls_of(cls);
  auto const exact = dcls.type == DCls::Exact;
  if (exact && !dcls.cls.mightCareAboutDynConstructs() &&
      !dcls.cls.couldHaveReifiedGenerics()) {
    return reduce(
      env,
      bc::DiscardClsRef { op.slot },
      bc::NewObjD { dcls.cls.name(), }
    );
  }

  takeClsRefSlot(env, op.slot);
  push(env, toobj(cls));
}

namespace {

bool objMightHaveConstProps(const Type& t) {
  assertx(t.subtypeOf(BObj));
  assertx(is_specialized_obj(t));
  auto const dobj = dobj_of(t);
  switch (dobj.type) {
    case DObj::Exact:
      return dobj.cls.couldHaveConstProp();
    case DObj::Sub:
      return dobj.cls.derivedCouldHaveConstProp();
  }
  not_reached();
}

}

void in(ISS& env, const bc::FCallCtor& op) {
  auto const obj = topC(env, op.fca.numArgsInclUnpack() + 2);
  assertx(op.fca.numRets == 1);

  if (!is_specialized_obj(obj)) {
    return fcallUnknownImpl(env, op.fca);
  }

  if (!op.fca.constructNoConst && !objMightHaveConstProps(obj)) {
    auto newFca = folly::copy(op.fca);
    newFca.constructNoConst = true;
    return reduce(env, bc::FCallCtor { std::move(newFca), op.str2 });
  }

  auto const dobj = dobj_of(obj);
  auto const exact = dobj.type == DObj::Exact;
  auto const rfunc = env.index.resolve_ctor(env.ctx, dobj.cls, exact);
  if (!rfunc) {
    return fcallUnknownImpl(env, op.fca);
  }

  auto const updateFCA = [&] (FCallArgs&& fca) {
    return bc::FCallCtor { std::move(fca), op.str2 };
  };

  auto const canFold = obj.subtypeOf(BObj);
  if (fcallOptimizeChecks(env, op.fca, *rfunc, updateFCA) ||
      (canFold && fcallTryFold(env, op.fca, *rfunc, obj, false /* dynamic */, 0,
                               NoClsRefSlotId))) {
    return;
  }

  if (rfunc->exactFunc() && op.str2->empty()) {
    // We've found the exact func that will be called, set the hint.
    return reduce(env, bc::FCallCtor { op.fca, rfunc->exactFunc()->cls->name });
  }

  fcallKnownImpl(env, op.fca, *rfunc, obj, false /* nullsafe */, 0,
                 NoClsRefSlotId, updateFCA);
}

void in(ISS& env, const bc::LockObj& op) {
  auto const t = topC(env);
  auto bail = [&]() {
    discard(env, 1);
    return push(env, t);
  };
  if (!t.subtypeOf(BObj)) return bail();
  if (!is_specialized_obj(t) || objMightHaveConstProps(t)) {
    nothrow(env);
    return bail();
  }
  reduce(env);
}

void in(ISS& env, const bc::FCall& op) {
  auto const fca = op.fca;
  auto const ar = fpiTop(env);
  if (ar.func) {
    auto const updateFCA = [&] (FCallArgs&& fca) {
      return bc::FCall { std::move(fca), op.str2, op.str3 };
    };

    if (fcallOptimizeChecks(env, fca, *ar.func, updateFCA, &ar) ||
        fcallTryFold(env, fca, *ar.func, ar.context, false /* unused */, 0,
                     NoClsRefSlotId, &ar)) {
      return;
    }

    switch (ar.kind) {
    case FPIKind::Unknown:
    case FPIKind::CallableArr:
    case FPIKind::ObjInvoke:
      not_reached();
    case FPIKind::Func:
      assertx(op.str2->empty());
      if (ar.func->name() != op.str3) {
        // We've found a more precise type for the call, so update it
        return reduce(env, bc::FCall {
          fca, staticEmptyString(), ar.func->name() });
      }
      fcallKnownImpl(env, fca, *ar.func, ar.context, false /* nullsafe */, 0,
                     NoClsRefSlotId, updateFCA, true /* legacy */);
      return;
    case FPIKind::Builtin:
      assertx(fca.numRets == 1);
      finish_builtin(env, ar.func->exactFunc(), fca.numArgs, fca.hasUnpack());
      fpiPop(env);
      return;
    case FPIKind::ClsMeth:
      assertx(op.str2->empty() == op.str3->empty());
      if (ar.cls.hasValue() && ar.func->cantBeMagicCall() &&
          (ar.cls->name() != op.str2 || ar.func->name() != op.str3)) {
        // We've found a more precise type for the call, so update it
        return reduce(env, bc::FCall {
          fca, ar.cls->name(), ar.func->name() });
      }
      // If we didn't return a reduce above, we still can compute a
      // partially-known FCall effect with our res::Func.
      fcallKnownImpl(env, fca, *ar.func, ar.context, false /* nullsafe */, 0,
                     NoClsRefSlotId, updateFCA, true /* legacy */);
      return;
    }
  }

  fcallUnknownImpl(env, fca, /* legacy */ true);
}

namespace {

void iterInitImpl(ISS& env, IterId iter, LocalId valueLoc,
                  BlockId target, const Type& base, LocalId baseLoc,
                  bool needsPop) {
  auto ity = iter_types(base);

  auto const fallthrough = [&] {
    setIter(env, iter, LiveIter { ity, baseLoc, NoLocalId, env.bid });
    // Do this after setting the iterator, in case it clobbers the base local
    // equivalency.
    setLoc(env, valueLoc, std::move(ity.value));
  };

  assert(iterIsDead(env, iter));

  if (!ity.mayThrowOnInit) {
    if (ity.count == IterTypes::Count::Empty && will_reduce(env)) {
      if (needsPop) {
        reduce(env, bc::PopC{});
      } else {
        reduce(env);
      }
      return jmp_setdest(env, target);
    }
    nothrow(env);
  }

  if (needsPop) {
    popC(env);
  }

  switch (ity.count) {
    case IterTypes::Count::Empty:
      mayReadLocal(env, valueLoc);
      jmp_setdest(env, target);
      return;
    case IterTypes::Count::Single:
    case IterTypes::Count::NonEmpty:
      fallthrough();
      return jmp_nevertaken(env);
    case IterTypes::Count::ZeroOrOne:
    case IterTypes::Count::Any:
      // Take the branch before setting locals if the iter is already
      // empty, but after popping.  Similar for the other IterInits
      // below.
      env.propagate(target, &env.state);
      fallthrough();
      return;
  }
  always_assert(false);
}

void iterInitKImpl(ISS& env, IterId iter, LocalId valueLoc, LocalId keyLoc,
                   BlockId target, const Type& base, LocalId baseLoc,
                   bool needsPop) {
  auto ity = iter_types(base);

  auto const fallthrough = [&]{
    setIter(env, iter, LiveIter { ity, baseLoc, NoLocalId, env.bid });
    // Do this after setting the iterator, in case it clobbers the base local
    // equivalency.
    setLoc(env, valueLoc, std::move(ity.value));
    setLoc(env, keyLoc, std::move(ity.key));
    if (!locCouldBeRef(env, keyLoc)) setIterKey(env, iter, keyLoc);
  };

  assert(iterIsDead(env, iter));

  if (!ity.mayThrowOnInit) {
    if (ity.count == IterTypes::Count::Empty && will_reduce(env)) {
      if (needsPop) {
        reduce(env, bc::PopC{});
      } else {
        reduce(env);
      }
      return jmp_setdest(env, target);
    }
    nothrow(env);
  }

  if (needsPop) {
    popC(env);
  }

  switch (ity.count) {
    case IterTypes::Count::Empty:
      mayReadLocal(env, valueLoc);
      mayReadLocal(env, keyLoc);
      return jmp_setdest(env, target);
    case IterTypes::Count::Single:
    case IterTypes::Count::NonEmpty:
      fallthrough();
      return jmp_nevertaken(env);
    case IterTypes::Count::ZeroOrOne:
    case IterTypes::Count::Any:
      env.propagate(target, &env.state);
      fallthrough();
      return;
  }

  always_assert(false);
}

void iterNextImpl(ISS& env,
                  IterId iter, LocalId valueLoc, BlockId target,
                  LocalId baseLoc) {
  auto const curLoc = peekLocRaw(env, valueLoc);
  auto noThrow = false;
  auto const noTaken = match<bool>(
    env.state.iters[iter],
    [&] (DeadIter)           {
      always_assert(false && "IterNext on dead iter");
      return false;
    },
    [&] (const LiveIter& ti) {
      if (!ti.types.mayThrowOnNext) noThrow = true;
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
      switch (ti.types.count) {
        case IterTypes::Count::Single:
        case IterTypes::Count::ZeroOrOne:
          return true;
        case IterTypes::Count::NonEmpty:
        case IterTypes::Count::Any:
          setLoc(env, valueLoc, ti.types.value);
          return false;
        case IterTypes::Count::Empty:
          always_assert(false);
      }
      not_reached();
    }
  );

  if (noTaken && noThrow && will_reduce(env)) {
    if (baseLoc != NoLocalId) {
      return reduce(env, bc::LIterFree { iter, baseLoc });
    }
    return reduce(env, bc::IterFree { iter });
  }

  mayReadLocal(env, valueLoc);
  mayReadLocal(env, baseLoc);

  if (noThrow) nothrow(env);

  if (noTaken) {
    jmp_nevertaken(env);
    freeIter(env, iter);
    return;
  }

  env.propagate(target, &env.state);

  freeIter(env, iter);
  setLocRaw(env, valueLoc, curLoc);
}

void iterNextKImpl(ISS& env, IterId iter, LocalId valueLoc,
                   LocalId keyLoc, BlockId target, LocalId baseLoc) {
  auto const curValue = peekLocRaw(env, valueLoc);
  auto const curKey = peekLocRaw(env, keyLoc);
  auto noThrow = false;
  auto const noTaken = match<bool>(
    env.state.iters[iter],
    [&] (DeadIter)           {
      always_assert(false && "IterNextK on dead iter");
      return false;
    },
    [&] (const LiveIter& ti) {
      if (!ti.types.mayThrowOnNext) noThrow = true;
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
      switch (ti.types.count) {
        case IterTypes::Count::Single:
        case IterTypes::Count::ZeroOrOne:
          return true;
        case IterTypes::Count::NonEmpty:
        case IterTypes::Count::Any:
          setLoc(env, valueLoc, ti.types.value);
          setLoc(env, keyLoc, ti.types.key);
          if (!locCouldBeRef(env, keyLoc)) setIterKey(env, iter, keyLoc);
          return false;
        case IterTypes::Count::Empty:
          always_assert(false);
      }
      not_reached();
    }
  );

  if (noTaken && noThrow && will_reduce(env)) {
    if (baseLoc != NoLocalId) {
      return reduce(env, bc::LIterFree { iter, baseLoc });
    }
    return reduce(env, bc::IterFree { iter });
  }

  mayReadLocal(env, valueLoc);
  mayReadLocal(env, keyLoc);
  mayReadLocal(env, baseLoc);

  if (noThrow) nothrow(env);

  if (noTaken) {
    jmp_nevertaken(env);
    freeIter(env, iter);
    return;
  }

  env.propagate(target, &env.state);

  freeIter(env, iter);
  setLocRaw(env, valueLoc, curValue);
  setLocRaw(env, keyLoc, curKey);
}

}

void in(ISS& env, const bc::IterInit& op) {
  auto base = topC(env);
  iterInitImpl(
    env,
    op.iter1,
    op.loc3,
    op.target2,
    std::move(base),
    topStkLocal(env),
    true
  );
}

void in(ISS& env, const bc::LIterInit& op) {
  iterInitImpl(
    env,
    op.iter1,
    op.loc4,
    op.target3,
    locAsCell(env, op.loc2),
    op.loc2,
    false
  );
}

void in(ISS& env, const bc::IterInitK& op) {
  auto base = topC(env);
  iterInitKImpl(
    env,
    op.iter1,
    op.loc3,
    op.loc4,
    op.target2,
    std::move(base),
    topStkLocal(env),
    true
  );
}

void in(ISS& env, const bc::LIterInitK& op) {
  iterInitKImpl(
    env,
    op.iter1,
    op.loc4,
    op.loc5,
    op.target3,
    locAsCell(env, op.loc2),
    op.loc2,
    false
  );
}

void in(ISS& env, const bc::IterNext& op) {
  iterNextImpl(env, op.iter1, op.loc3, op.target2, NoLocalId);
}

void in(ISS& env, const bc::LIterNext& op) {
  iterNextImpl(env, op.iter1, op.loc4, op.target3, op.loc2);
}

void in(ISS& env, const bc::IterNextK& op) {
  iterNextKImpl(env, op.iter1, op.loc3, op.loc4, op.target2, NoLocalId);
}

void in(ISS& env, const bc::LIterNextK& op) {
  iterNextKImpl(env, op.iter1, op.loc4, op.loc5, op.target3, op.loc2);
}

void in(ISS& env, const bc::IterFree& op) {
  // IterFree is used for weak iterators too, so we can't assert !iterIsDead.
  auto const isNop = match<bool>(
    env.state.iters[op.iter1],
    []  (DeadIter) {
      return true;
    },
    [&] (const LiveIter& ti) {
      if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
      return false;
    }
  );

  if (isNop && will_reduce(env)) return reduce(env);

  nothrow(env);
  freeIter(env, op.iter1);
}

void in(ISS& env, const bc::LIterFree& op) {
  nothrow(env);
  mayReadLocal(env, op.loc2);
  freeIter(env, op.iter1);
}

void in(ISS& env, const bc::IterBreak& op) {
  nothrow(env);

  for (auto const& it : op.iterTab) {
    if (it.kind == KindOfIter || it.kind == KindOfLIter) {
      match<void>(
        env.state.iters[it.id],
        []  (DeadIter) {},
        [&] (const LiveIter& ti) {
          if (ti.baseLocal != NoLocalId) hasInvariantIterBase(env);
        }
      );
    }
    if (it.kind == KindOfLIter) mayReadLocal(env, it.local);
    freeIter(env, it.id);
  }

  env.propagate(op.target1, &env.state);
}

/*
 * Any include/require (or eval) op kills all locals, and private properties.
 */
void inclOpImpl(ISS& env) {
  popC(env);
  killLocals(env);
  killThisProps(env);
  killSelfProps(env);
  mayUseVV(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::Incl&)      { inclOpImpl(env); }
void in(ISS& env, const bc::InclOnce&)  { inclOpImpl(env); }
void in(ISS& env, const bc::Req&)       { inclOpImpl(env); }
void in(ISS& env, const bc::ReqOnce&)   { inclOpImpl(env); }
void in(ISS& env, const bc::ReqDoc&)    { inclOpImpl(env); }
void in(ISS& env, const bc::Eval&)      { inclOpImpl(env); }

void in(ISS& /*env*/, const bc::DefCls&) {}
void in(ISS& /*env*/, const bc::DefRecord&) {}
void in(ISS& /*env*/, const bc::DefClsNop&) {}
void in(ISS& env, const bc::AliasCls&) {
  popC(env);
  push(env, TBool);
}

void in(ISS& env, const bc::DefCns& op) {
  auto const t = popC(env);
  if (options.HardConstProp) {
    auto const v = tv(t);
    auto const val = v && tvAsCVarRef(&*v).isAllowedAsConstantValue() ?
      *v : make_tv<KindOfUninit>();
    auto const res = env.collect.cnsMap.emplace(op.str1, val);
    if (!res.second) {
      if (res.first->second.m_type == kReadOnlyConstant) {
        // we only saw a read of this constant
        res.first->second = val;
      } else {
        // more than one definition in this function
        res.first->second.m_type = kDynamicConstant;
      }
    }
  }
  push(env, TBool);
}

void in(ISS& /*env*/, const bc::DefTypeAlias&) {}

void in(ISS& env, const bc::This&) {
  if (thisAvailable(env)) {
    return reduce(env, bc::BareThis { BareThisOp::NeverNull });
  }
  auto const ty = thisTypeNonNull(env);
  push(env, ty, StackThisId);
  setThisAvailable(env);
  if (ty.subtypeOf(BBottom)) unreachable(env);
}

void in(ISS& env, const bc::LateBoundCls& op) {
  if (env.ctx.cls) effect_free(env);
  auto const ty = selfCls(env);
  putClsRefSlot(env, op.slot, setctx(ty ? *ty : TCls));
}

void in(ISS& env, const bc::CheckThis&) {
  if (thisAvailable(env)) {
    return reduce(env);
  }
  setThisAvailable(env);
}

void in(ISS& env, const bc::BareThis& op) {
  if (thisAvailable(env)) {
    if (op.subop1 != BareThisOp::NeverNull) {
      return reduce(env, bc::BareThis { BareThisOp::NeverNull });
    }
  }

  auto const ty = thisType(env);
  switch (op.subop1) {
    case BareThisOp::Notice:
      break;
    case BareThisOp::NoNotice:
      effect_free(env);
      break;
    case BareThisOp::NeverNull:
      setThisAvailable(env);
      if (!env.state.unreachable) effect_free(env);
      return push(env, ty, StackThisId);
  }

  push(env, ty, StackThisId);
}

void in(ISS& env, const bc::InitThisLoc& op) {
  if (!is_volatile_local(env.ctx.func, op.loc1)) {
    setLocRaw(env, op.loc1, TCell);
    env.state.thisLoc = op.loc1;
  }
}

/*
 * Amongst other things, we use this to mark units non-persistent.
 */
void in(ISS& env, const bc::OODeclExists& op) {
  auto flag = popC(env);
  auto name = popC(env);
  push(env, [&] {
      if (!name.strictSubtypeOf(TStr)) return TBool;
      auto const v = tv(name);
      if (!v) return TBool;
      auto rcls = env.index.resolve_class(env.ctx, v->m_data.pstr);
      if (!rcls || !rcls->cls()) return TBool;
      auto const mayExist = [&] () -> bool {
        switch (op.subop1) {
          case OODeclExistsOp::Class:
            return !(rcls->cls()->attrs & (AttrInterface | AttrTrait));
          case OODeclExistsOp::Interface:
            return rcls->cls()->attrs & AttrInterface;
          case OODeclExistsOp::Trait:
            return rcls->cls()->attrs & AttrTrait;
        }
        not_reached();
      }();
      auto unit = rcls->cls()->unit;
      auto canConstProp = [&] {
        // Its generally not safe to constprop this, because of
        // autoload. We're safe if its part of systemlib, or a
        // superclass of the current context.
        if (is_systemlib_part(*unit)) return true;
        if (!env.ctx.cls) return false;
        auto thisClass = env.index.resolve_class(env.ctx.cls);
        return thisClass.mustBeSubtypeOf(*rcls);
      };
      if (canConstProp()) {
        constprop(env);
        return mayExist ? TTrue : TFalse;
      }
      if (!any(env.collect.opts & CollectionOpts::Inlining)) {
        unit->persistent.store(false, std::memory_order_relaxed);
      }
      // At this point, if it mayExist, we still don't know that it
      // *does* exist, but if not we know that it either doesn't
      // exist, or it doesn't have the right type.
      return mayExist ? TBool : TFalse;
    } ());
}

namespace {
bool couldBeMocked(const Type& t) {
  if (is_specialized_cls(t)) {
    return dcls_of(t).cls.couldBeMocked();
  } else if (is_specialized_obj(t)) {
    return dobj_of(t).cls.couldBeMocked();
  }
  // In practice this should not occur since this is used mostly on the result
  // of looked up type constraints.
  return true;
}
}

void in(ISS& env, const bc::VerifyParamType& op) {
  IgnoreUsedParams _{env};

  if (env.ctx.func->isMemoizeImpl && !locCouldBeRef(env, op.loc1)) {
    // a MemoizeImpl's params have already been checked by the wrapper
    return reduce(env);
  }

  // Generally we won't know anything about the params, but
  // analyze_func_inline does - and this can help with effect-free analysis
  auto const constraint = env.ctx.func->params[op.loc1].typeConstraint;
  if (env.index.satisfies_constraint(env.ctx,
                                     locAsCell(env, op.loc1),
                                     constraint)) {
    if (!locAsCell(env, op.loc1).couldBe(BFunc | BCls)) {
      return reduce(env);
    }
  }

  /*
   * We assume that if this opcode doesn't throw, the parameter was of the
   * specified type (although it may have been a Ref if the parameter was
   * by reference).
   *
   * The env.setLoc here handles dealing with a parameter that was
   * already known to be a reference.
   *
   * NB: VerifyParamType of a reference parameter can kill any references
   * if it re-enters.
   */
  if (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis()) {
    return;
  }
  if (constraint.hasConstraint() && !constraint.isTypeVar() &&
      !constraint.isTypeConstant()) {
    auto t =
      loosen_dvarrayness(env.index.lookup_constraint(env.ctx, constraint));
    if (constraint.isThis() && couldBeMocked(t)) {
      t = unctx(std::move(t));
    }
    if (t.subtypeOf(BBottom)) unreachable(env);
    FTRACE(2, "     {} ({})\n", constraint.fullName(), show(t));
    setLoc(env, op.loc1, std::move(t));
  }
}

void in(ISS& env, const bc::VerifyParamTypeTS& op) {
  auto const a = topC(env);
  auto const requiredTSType = RuntimeOption::EvalHackArrDVArrs ? BDict : BDArr;
  if (!a.couldBe(requiredTSType)) {
    unreachable(env);
    popC(env);
    return;
  }
  auto const constraint = env.ctx.func->params[op.loc1].typeConstraint;
  // TODO(T31677864): We are being extremely pessimistic here, relax it
  if (!env.ctx.func->isReified &&
      (!env.ctx.cls || !env.ctx.cls->hasReifiedGenerics) &&
      !env.index.could_have_reified_type(constraint)) {
    return reduce(env, bc::PopC {}, bc::VerifyParamType { op.loc1 });
  }

  if (auto const inputTS = tv(a)) {
    if (!isValidTSType(*inputTS, false)) {
      unreachable(env);
      popC(env);
      return;
    }
    auto const resolvedTS =
      resolveTSStatically(env, inputTS->m_data.parr, env.ctx.cls, true);
    if (resolvedTS && resolvedTS != inputTS->m_data.parr) {
      reduce(env, bc::PopC {});
      RuntimeOption::EvalHackArrDVArrs ? reduce(env, bc::Dict { resolvedTS })
                                       : reduce(env, bc::Array { resolvedTS });
      reduce(env, bc::VerifyParamTypeTS { op.loc1 });
      return;
    }
  }
  popC(env);
}

void verifyRetImpl(ISS& env, const TypeConstraint& constraint,
                   bool reduce_this, bool ts_flavor) {
  // If it is the ts flavor, then second thing on the stack, otherwise first
  auto stackT = topC(env, (int)ts_flavor);
  auto const stackEquiv = topStkEquiv(env, (int)ts_flavor);

  // If there is no return type constraint, or if the return type
  // constraint is a typevar, or if the top of stack is the same or a
  // subtype of the type constraint, then this is a no-op, unless
  // reified types could be involved.
  if (env.index.satisfies_constraint(env.ctx, stackT, constraint)) {
    if (ts_flavor) {
      // we wouldn't get here if reified types were definitely not
      // involved, so just bail.
      popC(env);
      popC(env);
      push(env, std::move(stackT), stackEquiv);
      return;
    }
    return reduce(env);
  }

  // For CheckReturnTypeHints >= 3 AND the constraint is not soft.
  // We can safely assume that either VerifyRetTypeC will
  // throw or it will produce a value whose type is compatible with the
  // return type constraint.
  auto tcT = remove_uninit(
    loosen_dvarrayness(env.index.lookup_constraint(env.ctx, constraint)));

  // If tcT could be an interface or trait, we upcast it to TObj/TOptObj.
  // Why?  Because we want uphold the invariant that we only refine return
  // types and never widen them, and if we allow tcT to be an interface then
  // it's possible for violations of this invariant to arise.  For an example,
  // see "hphp/test/slow/hhbbc/return-type-opt-bug.php".
  // Note: It's safe to use TObj/TOptObj because lookup_constraint() only
  // returns classes or interfaces or traits (it never returns something that
  // could be an enum or type alias) and it never returns anything that could
  // be a "magic" interface that supports non-objects.  (For traits the return
  // typehint will always throw at run time, so it's safe to use TObj/TOptObj.)
  if (is_specialized_obj(tcT) && dobj_of(tcT).cls.couldBeInterfaceOrTrait()) {
    tcT = is_opt(tcT) ? TOptObj : TObj;
  }

  // In some circumstances, verifyRetType can modify the type. If it
  // does that we can't reduce even when we know it succeeds.
  auto dont_reduce = false;
  if (!constraint.isSoft()) {
    // VerifyRetType will convert a TFunc to a TStr implicitly
    // (and possibly warn)
    if (tcT.subtypeOf(TStr) && stackT.couldBe(BFunc | BCls)) {
      stackT |= TStr;
      dont_reduce = true;
    }

    // VerifyRetType will convert TClsMeth to TVec/TVArr/TArr implicitly
    if (stackT.couldBe(BClsMeth)) {
      if (tcT.couldBe(BVec)) {
        stackT |= TVec;
        dont_reduce = true;
      }
      if (tcT.couldBe(BVArr)) {
        stackT |= TVArr;
        dont_reduce = true;
      }
      if (tcT.couldBe(TArr)) {
        stackT |= TArr;
        dont_reduce = true;
      }
    }
  }

  // If CheckReturnTypeHints < 3 OR if the constraint is soft,
  // then there are no optimizations we can safely do here, so
  // just leave the top of stack as is.
  if (RuntimeOption::EvalCheckReturnTypeHints < 3 || constraint.isSoft() ||
      (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis())) {
    if (ts_flavor) popC(env);
    popC(env);
    push(env, std::move(stackT), stackEquiv);
    return;
  }

  // In cases where we have a `this` hint where stackT is an TOptObj known to
  // be this, we can replace the check with a non null check.  These cases are
  // likely from a BareThis that could return Null.  Since the runtime will
  // split these translations, it will rarely in practice return null.
  if (reduce_this &&
      !dont_reduce &&
      constraint.isThis() &&
      !constraint.isNullable() &&
      is_opt(stackT) &&
      env.index.satisfies_constraint(env.ctx, unopt(stackT), constraint)) {
    if (ts_flavor) {
      return reduce(env, bc::PopC {}, bc::VerifyRetNonNullC {});
    }
    return reduce(env, bc::VerifyRetNonNullC {});
  }

  auto retT = intersection_of(std::move(tcT), std::move(stackT));
  if (retT.subtypeOf(BBottom)) {
    unreachable(env);
    if (ts_flavor) popC(env); // the type structure
    return;
  }

  if (ts_flavor) popC(env); // the type structure
  popC(env);
  push(env, std::move(retT));
}

void in(ISS& env, const bc::VerifyOutType& op) {
  // We reuse VerifyOutType bytecode for log typehint violations on
  // byref parameters. Do not perform any optimizations for byref parameters,
  // as we do not enforce those.
  if (env.ctx.func->params[op.arg1].byRef) {
    return;
  }
  verifyRetImpl(env, env.ctx.func->params[op.arg1].typeConstraint,
                false, false);
}

void in(ISS& env, const bc::VerifyRetTypeC& /*op*/) {
  verifyRetImpl(env, env.ctx.func->retTypeConstraint, true, false);
}

void in(ISS& env, const bc::VerifyRetTypeTS& /*op*/) {
  auto const a = topC(env);
  auto const requiredTSType = RuntimeOption::EvalHackArrDVArrs ? BDict : BDArr;
  if (!a.couldBe(requiredTSType)) {
    unreachable(env);
    popC(env);
    return;
  }
  auto const constraint = env.ctx.func->retTypeConstraint;
  // TODO(T31677864): We are being extremely pessimistic here, relax it
  if (!env.ctx.func->isReified &&
      (!env.ctx.cls || !env.ctx.cls->hasReifiedGenerics) &&
      !env.index.could_have_reified_type(constraint)) {
    return reduce(env, bc::PopC {}, bc::VerifyRetTypeC {});
  }
  if (auto const inputTS = tv(a)) {
    if (!isValidTSType(*inputTS, false)) {
      unreachable(env);
      popC(env);
      return;
    }
    auto const resolvedTS =
      resolveTSStatically(env, inputTS->m_data.parr, env.ctx.cls, true);
    if (resolvedTS && resolvedTS != inputTS->m_data.parr) {
      reduce(env, bc::PopC {});
      RuntimeOption::EvalHackArrDVArrs ? reduce(env, bc::Dict { resolvedTS })
                                       : reduce(env, bc::Array { resolvedTS });
      reduce(env, bc::VerifyRetTypeTS {});
      return;
    }
  }
  verifyRetImpl(env, constraint, true, true);
}

void in(ISS& env, const bc::VerifyRetNonNullC& /*op*/) {
  auto const constraint = env.ctx.func->retTypeConstraint;
  if (RuntimeOption::EvalCheckReturnTypeHints < 3 || constraint.isSoft()
      || (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis())) {
    return;
  }

  auto stackT = topC(env);

  if (!stackT.couldBe(BInitNull)) {
    reduce(env);
    return;
  }

  if (stackT.subtypeOf(BNull)) return unreachable(env);

  auto const equiv = topStkEquiv(env);

  if (is_opt(stackT)) stackT = unopt(std::move(stackT));

  popC(env);
  push(env, stackT, equiv);
}

void in(ISS& env, const bc::Self& op) {
  auto self = selfClsExact(env);
  putClsRefSlot(env, op.slot, self ? *self : TCls);
}

void in(ISS& env, const bc::Parent& op) {
  auto parent = parentClsExact(env);
  putClsRefSlot(env, op.slot, parent ? *parent : TCls);
}

void in(ISS& env, const bc::CreateCl& op) {
  auto const nargs   = op.arg1;
  auto const clsPair = env.index.resolve_closure_class(env.ctx, op.arg2);

  /*
   * Every closure should have a unique allocation site, but we may see it
   * multiple times in a given round of analyzing this function.  Each time we
   * may have more information about the used variables; the types should only
   * possibly grow.  If it's already there we need to merge the used vars in
   * with what we saw last time.
   */
  if (nargs) {
    CompactVector<Type> usedVars(nargs);
    for (auto i = uint32_t{0}; i < nargs; ++i) {
      usedVars[nargs - i - 1] = unctx(popCU(env));
    }
    merge_closure_use_vars_into(
      env.collect.closureUseTypes,
      clsPair.second,
      std::move(usedVars)
    );
  }

  // Closure classes can be cloned and rescoped at runtime, so it's not safe to
  // assert the exact type of closure objects. The best we can do is assert
  // that it's a subclass of Closure.
  auto const closure = env.index.builtin_class(s_Closure.get());

  return push(env, subObj(closure));
}

void in(ISS& env, const bc::CreateCont& /*op*/) {
  // First resume is always next() which pushes null.
  push(env, TInitNull);
}

void in(ISS& env, const bc::ContEnter&) { popC(env); push(env, TInitCell); }
void in(ISS& env, const bc::ContRaise&) { popC(env); push(env, TInitCell); }

void in(ISS& env, const bc::Yield&) {
  popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::YieldK&) {
  popC(env);
  popC(env);
  push(env, TInitCell);
}

void in(ISS& env, const bc::ContAssignDelegate&) {
  popC(env);
}

void in(ISS& env, const bc::ContEnterDelegate&) {
  popC(env);
}

void in(ISS& env, const bc::YieldFromDelegate& op) {
  push(env, TInitCell);
  env.propagate(op.target2, &env.state);
}

void in(ISS& /*env*/, const bc::ContUnsetDelegate&) {}

void in(ISS& /*env*/, const bc::ContCheck&) {}
void in(ISS& env, const bc::ContValid&)   { push(env, TBool); }
void in(ISS& env, const bc::ContKey&)     { push(env, TInitCell); }
void in(ISS& env, const bc::ContCurrent&) { push(env, TInitCell); }
void in(ISS& env, const bc::ContGetReturn&) { push(env, TInitCell); }

void pushTypeFromWH(ISS& env, Type t) {
  auto inner = typeFromWH(t);
  // The next opcode is unreachable if awaiting a non-object or WaitH<Bottom>.
  if (inner.subtypeOf(BBottom)) unreachable(env);
  push(env, std::move(inner));
}

void in(ISS& env, const bc::WHResult&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& env, const bc::Await&) {
  pushTypeFromWH(env, popC(env));
}

void in(ISS& env, const bc::AwaitAll& op) {
  auto const equiv = equivLocalRange(env, op.locrange);
  if (equiv != op.locrange.first) {
    return reduce(
      env,
      bc::AwaitAll {LocalRange {equiv, op.locrange.count}}
    );
  }

  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    mayReadLocal(env, op.locrange.first + i);
  }

  push(env, TInitNull);
}

namespace {

void idxImpl(ISS& env, bool arraysOnly) {
  auto const def  = popC(env);
  auto const key  = popC(env);
  auto const base = popC(env);

  if (key.subtypeOf(BInitNull)) {
    // A null key, regardless of whether we're ArrayIdx or Idx will always
    // silently return the default value, regardless of the base type.
    constprop(env);
    effect_free(env);
    return push(env, def);
  }

  // Push the returned type and annotate effects appropriately, taking into
  // account if the base might be null. Allowing for a possibly null base lets
  // us capture more cases.
  auto const finish = [&] (const Type& t, bool canThrow) {
    // A null base will raise if we're ArrayIdx. For Idx, it will silently
    // return the default value.
    auto const baseMaybeNull = base.couldBe(BInitNull);
    if (!canThrow && (!arraysOnly || !baseMaybeNull)) {
      constprop(env);
      effect_free(env);
    }
    if (!arraysOnly && baseMaybeNull) return push(env, union_of(t, def));
    if (t.subtypeOf(BBottom)) unreachable(env);
    return push(env, t);
  };

  if (arraysOnly) {
    // If ArrayIdx, we'll raise an error for anything other than array-like and
    // null. This op is only terminal if null isn't possible.
    if (!base.couldBe(BArr | BVec | BDict | BKeyset | BClsMeth)) {
      return finish(key.couldBe(BInitNull) ? def : TBottom, true);
    }
  } else if (
    !base.couldBe(BArr | BVec | BDict | BKeyset | BStr | BObj | BClsMeth)) {
    // Otherwise, any strange bases for Idx will just return the default value
    // without raising.
    return finish(def, false);
  }

  // Helper for Hack arrays. "validKey" is the set key types which can return a
  // value from Idx. "silentKey" is the set of key types which will silently
  // return null (anything else throws). The Hack array elem functions will
  // treat values of "silentKey" as throwing, so we must identify those cases
  // and deal with them.
  auto const hackArr = [&] (std::pair<Type, ThrowMode> elem,
                            const Type& validKey,
                            const Type& silentKey) {
    switch (elem.second) {
      case ThrowMode::None:
      case ThrowMode::MaybeMissingElement:
      case ThrowMode::MissingElement:
        assertx(key.subtypeOf(validKey));
        return finish(elem.first, false);
      case ThrowMode::MaybeBadKey:
        assertx(key.couldBe(validKey));
        if (key.couldBe(silentKey)) elem.first |= def;
        return finish(elem.first, !key.subtypeOf(BOptArrKey));
      case ThrowMode::BadOperation:
        assertx(!key.couldBe(validKey));
        return finish(key.couldBe(silentKey) ? def : TBottom, true);
    }
  };

  if (base.subtypeOrNull(BVec)) {
    // Vecs will throw for any key other than Int, Str, or Null, and will
    // silently return the default value for the latter two.
    if (key.subtypeOrNull(BStr)) return finish(def, false);
    return hackArr(vec_elem(base, key, def), TInt, TOptStr);
  }

  if (base.subtypeOfAny(TOptDict, TOptKeyset)) {
    // Dicts and keysets will throw for any key other than Int, Str, or Null,
    // and will silently return the default value for Null.
    auto const elem = base.subtypeOrNull(BDict)
      ? dict_elem(base, key, def)
      : keyset_elem(base, key, def);
    return hackArr(elem, TArrKey, TInitNull);
  }

  if (base.subtypeOrNull(BArr)) {
    // A possibly null key is more complicated for arrays. array_elem() will
    // transform a null key into an empty string (matching the semantics of
    // array access), but that's not what Idx does. So, attempt to remove
    // nullish from the key first. If we can't, it just means we'll get a more
    // conservative value.
    auto maybeNull = false;
    auto const fixedKey = [&]{
      if (key.couldBe(TInitNull)) {
        maybeNull = true;
        if (is_nullish(key)) return unnullish(key);
      }
      return key;
    }();

    auto elem = array_elem(base, fixedKey, def);
    // If the key was null, Idx will return the default value, so add to the
    // return type.
    if (maybeNull) elem.first |= def;

    switch (elem.second) {
      case ThrowMode::None:
      case ThrowMode::MaybeMissingElement:
      case ThrowMode::MissingElement:
        return finish(elem.first, false);
      case ThrowMode::MaybeBadKey:
        return finish(elem.first, true);
      case ThrowMode::BadOperation:
        always_assert(false);
    }
  }

  if (!arraysOnly && base.subtypeOrNull(BStr)) {
    // Idx on a string always produces a string or the default value (without
    // ever raising).
    return finish(union_of(TStr, def), false);
  }

  // Objects or other unions of possible bases
  push(env, TInitCell);
}

}

void in(ISS& env, const bc::Idx&)      { idxImpl(env, false); }
void in(ISS& env, const bc::ArrayIdx&) { idxImpl(env, true);  }

void in(ISS& env, const bc::CheckProp&) {
  if (env.ctx.cls->attrs & AttrNoOverride) {
    return reduce(env, bc::False {});
  }
  nothrow(env);
  push(env, TBool);
}

void in(ISS& env, const bc::InitProp& op) {
  auto const t = topC(env);
  switch (op.subop2) {
    case InitPropOp::Static:
      mergeSelfProp(env, op.str1, t);
      env.collect.publicSPropMutations.merge(
        env.index, env.ctx, *env.ctx.cls, sval(op.str1), t
      );
      break;
    case InitPropOp::NonStatic:
      mergeThisProp(env, op.str1, t);
      break;
  }

  for (auto& prop : env.ctx.func->cls->properties) {
    if (prop.name != op.str1) continue;

    ITRACE(1, "InitProp: {} = {}\n", op.str1, show(t));

    if (env.index.satisfies_constraint(env.ctx, t, prop.typeConstraint)) {
      prop.attrs |= AttrInitialSatisfiesTC;
    } else {
      badPropInitialValue(env);
      prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
    }

    auto const v = tv(t);
    if (v || !could_contain_objects(t)) {
      prop.attrs = (Attr)(prop.attrs & ~AttrDeepInit);
      if (!v) break;
      prop.val = *v;
      env.index.update_static_prop_init_val(env.ctx.func->cls, op.str1);
      return reduce(env, bc::PopC {});
    }
  }

  popC(env);
}

void in(ISS& env, const bc::Silence& op) {
  nothrow(env);
  switch (op.subop2) {
    case SilenceOp::Start:
      setLoc(env, op.loc1, TInt);
      break;
    case SilenceOp::End:
      break;
  }
}

namespace {

template <typename Op, typename Rebind>
bool memoGetImpl(ISS& env, const Op& op, Rebind&& rebind) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  always_assert(op.locrange.first + op.locrange.count
                <= env.ctx.func->locals.size());

  if (will_reduce(env)) {
    // If we can use an equivalent, earlier range, then use that instead.
    auto const equiv = equivLocalRange(env, op.locrange);
    if (equiv != op.locrange.first) {
      reduce(env, rebind(LocalRange { equiv, op.locrange.count }));
      return true;
    }
  }

  auto retTy = memoizeImplRetType(env);

  // MemoGet can raise if we give a non arr-key local, or if we're in a method
  // and $this isn't available.
  auto allArrKey = true;
  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    allArrKey &= locRaw(env, op.locrange.first + i).subtypeOf(BArrKey);
  }
  if (allArrKey &&
      (!env.ctx.func->cls ||
       (env.ctx.func->attrs & AttrStatic) ||
       thisAvailable(env))) {
    if (will_reduce(env)) {
      if (retTy.first.subtypeOf(BBottom)) {
        reduce(env);
        jmp_setdest(env, op.target1);
        return true;
      }
      // deal with constprop manually; otherwise we will propagate the
      // taken edge and *then* replace the MemoGet with a constant.
      if (retTy.second) {
        if (auto v = tv(retTy.first)) {
          reduce(env, gen_constant(*v));
          return true;
        }
      }
    }
    nothrow(env);
  }

  if (retTy.first == TBottom) {
    jmp_setdest(env, op.target1);
    return true;
  }

  env.propagate(op.target1, &env.state);
  push(env, std::move(retTy.first));
  return false;
}

}

void in(ISS& env, const bc::MemoGet& op) {
  memoGetImpl(
    env, op,
    [&] (const LocalRange& l) { return bc::MemoGet { op.target1, l }; }
  );
}

void in(ISS& env, const bc::MemoGetEager& op) {
  always_assert(env.ctx.func->isAsync && !env.ctx.func->isGenerator);

  auto const reduced = memoGetImpl(
    env, op,
    [&] (const LocalRange& l) {
      return bc::MemoGetEager { op.target1, op.target2, l };
    }
  );
  if (reduced) return;

  env.propagate(op.target2, &env.state);
  auto const t = popC(env);
  push(
    env,
    is_specialized_wait_handle(t) ? wait_handle_inner(t) : TInitCell
  );
}

namespace {

template <typename Op>
void memoSetImpl(ISS& env, const Op& op) {
  always_assert(env.ctx.func->isMemoizeWrapper);
  always_assert(op.locrange.first + op.locrange.count
                <= env.ctx.func->locals.size());

  // If we can use an equivalent, earlier range, then use that instead.
  auto const equiv = equivLocalRange(env, op.locrange);
  if (equiv != op.locrange.first) {
    return reduce(
      env,
      Op { LocalRange { equiv, op.locrange.count } }
    );
  }

  // MemoSet can raise if we give a non arr-key local, or if we're in a method
  // and $this isn't available.
  auto allArrKey = true;
  for (uint32_t i = 0; i < op.locrange.count; ++i) {
    allArrKey &= locRaw(env, op.locrange.first + i).subtypeOf(BArrKey);
  }
  if (allArrKey &&
      (!env.ctx.func->cls ||
       (env.ctx.func->attrs & AttrStatic) ||
       thisAvailable(env))) {
    nothrow(env);
  }
  push(env, popC(env));
}

}

void in(ISS& env, const bc::MemoSet& op) {
  memoSetImpl(env, op);
}

void in(ISS& env, const bc::MemoSetEager& op) {
  always_assert(env.ctx.func->isAsync && !env.ctx.func->isGenerator);
  memoSetImpl(env, op);
}

}

namespace {

//////////////////////////////////////////////////////////////////////

void dispatch(ISS& env, const Bytecode& op) {
#define O(opcode, ...) case Op::opcode: interp_step::in(env, op.opcode); return;
  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

void interpStep(ISS& env, const Bytecode& bc) {
  ITRACE(2, "  {} ({})\n",
         show(env.ctx.func, bc),
         env.unchangedBcs + env.replacedBcs.size());
  Trace::Indent _;

  // If there are throw exit edges, make a copy of the state (except
  // stacks) in case we need to propagate across throw exits (if
  // it's a PEI).
  if (!env.stateBefore && env.blk.throwExit != NoBlockId) {
    env.stateBefore.emplace(with_throwable_only(env.index, env.state));
  }

  env.flags = {};

  default_dispatch(env, bc);

  if (env.flags.reduced) return;

  auto const_prop = [&] {
    if (!options.ConstantProp || !env.flags.canConstProp) return false;

    auto const numPushed   = bc.numPush();
    TinyVector<Cell> cells;

    auto i = size_t{0};
    while (i < numPushed) {
      auto const v = tv(topT(env, i));
      if (!v) return false;
      cells.push_back(*v);
      ++i;
    }

    if (env.flags.wasPEI) {
      ITRACE(2, "   nothrow (due to constprop)\n");
      env.flags.wasPEI = false;
    }
    if (!env.flags.effectFree) {
      ITRACE(2, "   effect_free (due to constprop)\n");
      env.flags.effectFree = true;
    }

    rewind(env, bc);

    auto const numPop = bc.numPop();
    for (auto j = 0; j < numPop; j++) {
      switch (bc.popFlavor(j)) {
        case Flavor::CVU:
          // Note that we only support C's for CVU so far (this only
          // comes up with FCallBuiltin)---we'll fail the verifier if
          // something changes to send V's or U's through here.
          interpStep(env, bc::PopC {});
          break;
        case Flavor::CU:
          // We only support C's for CU right now.
          interpStep(env, bc::PopC {});
          break;
        case Flavor::C:
          interpStep(env, bc::PopC {});
          break;
        case Flavor::V:
          interpStep(env, bc::PopV {});
          break;
        case Flavor::U:  not_reached();
        case Flavor::CV: not_reached();
      }
    }

    auto const slot = visit(bc, ReadClsRefSlotVisitor{});
    if (slot != NoClsRefSlotId) interpStep(env, bc::DiscardClsRef { slot });

    while (i--) {
      push(env, from_cell(cells[i]));
      record(env, gen_constant(cells[i]));
    }
    return true;
  };

  if (const_prop()) {
    return;
  }

  assertx(!env.flags.effectFree || !env.flags.wasPEI);
  if (env.flags.wasPEI) {
    ITRACE(2, "   PEI.\n");
    if (env.stateBefore) {
      env.propagate(env.blk.throwExit, &*env.stateBefore);
    }
  }
  env.stateBefore.clear();

  record(env, bc);
}

void interpOne(ISS& env, const Bytecode& bc) {
  env.srcLoc = bc.srcLoc;
  interpStep(env, bc);
}

BlockId speculate(Interp& interp) {
  auto low_water = interp.state.stack.size();

  interp.collect.opts = interp.collect.opts | CollectionOpts::Speculating;
  SCOPE_EXIT {
    interp.collect.opts = interp.collect.opts - CollectionOpts::Speculating;
  };

  auto failed = false;
  ISS env { interp, [&] (BlockId, const State*) { failed = true; } };

  FTRACE(4, "  Speculate B{}\n", interp.bid);
  for (auto const& bc : interp.blk->hhbcs) {
    assertx(!interp.state.unreachable);
    auto const numPop = bc.numPop() +
      (bc.op == Op::CGetL2 ? 1 :
       bc.op == Op::Dup ? -1 : 0);
    if (interp.state.stack.size() - numPop < low_water) {
      low_water = interp.state.stack.size() - numPop;
    }

    interpOne(env, bc);
    if (failed) {
      env.collect.mInstrState.clear();
      FTRACE(3, "  Bailing from speculate because propagate was called\n");
      return NoBlockId;
    }

    auto const& flags = env.flags;
    if (!flags.effectFree) {
      env.collect.mInstrState.clear();
      FTRACE(3, "  Bailing from speculate because not effect free\n");
      return NoBlockId;
    }

    assertx(!flags.returned);

    if (flags.jmpDest != NoBlockId && interp.state.stack.size() == low_water) {
      FTRACE(2, "  Speculate found target block {}\n", flags.jmpDest);
      return flags.jmpDest;
    }
  }

  if (interp.state.stack.size() != low_water) {
    FTRACE(3,
           "  Bailing from speculate because the speculated block "
           "left items on the stack\n");
    return NoBlockId;
  }

  if (interp.blk->fallthrough == NoBlockId) {
    FTRACE(3,
           "  Bailing from speculate because there was no fallthrough");
    return NoBlockId;
  }

  FTRACE(2, "  Speculate found fallthrough block {}\n",
         interp.blk->fallthrough);

  return interp.blk->fallthrough;
}

BlockId speculateHelper(ISS& env, BlockId orig, bool updateTaken) {
  assertx(orig != NoBlockId);

  if (!will_reduce(env)) return orig;

  auto const last = last_op(env);
  bool endsInControlFlow = last && instrIsNonCallControlFlow(last->op);
  auto target = orig;
  auto pops = 0;

  if (options.RemoveDeadBlocks) {
    State temp{env.state, State::Compact{}};
    while (true) {
      auto const func = env.ctx.func;
      auto const targetBlk = func->blocks[target].get();
      if (!targetBlk->multiPred) break;
      auto const ok = [&] {
        switch (targetBlk->hhbcs.back().op) {
          case Op::JmpZ:
          case Op::JmpNZ:
          case Op::SSwitch:
          case Op::Switch:
          return true;
          default:
          return false;
        }
      }();

      if (!ok) break;

      Interp interp {
        env.index, env.ctx, env.collect, target, targetBlk, temp
      };

      auto const old_size = temp.stack.size();
      auto const new_target = speculate(interp);
      if (new_target == NoBlockId) break;

      const ssize_t delta = old_size - temp.stack.size();
      assertx(delta >= 0);
      if (delta && endsInControlFlow) break;

      pops += delta;
      target = new_target;
      temp.stack.compact();
    }
  }

  if (endsInControlFlow && updateTaken) {
    assertx(!pops);
    auto needsUpdate = target != orig;
    if (!needsUpdate) {
      forEachTakenEdge(
        *last,
        [&] (BlockId bid) {
          if (bid != orig) needsUpdate = true;
        }
      );
    }
    if (needsUpdate) {
      auto& bc = mutate_last_op(env);
      forEachTakenEdge(
        bc,
        [&] (BlockId& bid) {
          bid = bid == orig ? target : NoBlockId;
        }
      );
    }
  }

  while (pops--) {
    interpStep(env, bc::PopC {});
  }

  return target;
}

}

//////////////////////////////////////////////////////////////////////

RunFlags run(Interp& interp, const State& in, PropagateFn propagate) {
  SCOPE_EXIT {
    FTRACE(2, "out {}{}\n",
           state_string(*interp.ctx.func, interp.state, interp.collect),
           property_state_string(interp.collect.props));
  };

  auto env = ISS { interp, propagate };
  auto ret = RunFlags {};
  auto finish = [&] (BlockId fallthrough) {
    ret.updateInfo.fallthrough = fallthrough;
    ret.updateInfo.unchangedBcs = env.unchangedBcs;
    ret.updateInfo.replacedBcs = std::move(env.replacedBcs);
    return ret;
  };

  BytecodeVec retryBcs;
  auto retryOffset = interp.blk->hhbcs.size();
  auto size = retryOffset;
  BlockId retryFallthrough = interp.blk->fallthrough;
  size_t idx = 0;

  while (true) {
    if (idx == size) {
      finish_tracked_elems(env, 0);
      if (!env.reprocess) break;
      FTRACE(2, "  Reprocess mutated block {}\n", interp.bid);
      assertx(env.unchangedBcs < retryOffset || env.replacedBcs.size());
      retryOffset = env.unchangedBcs;
      retryBcs = std::move(env.replacedBcs);
      env.unchangedBcs = 0;
      env.state.copy_from(in);
      env.reprocess = false;
      env.replacedBcs.clear();
      size = retryOffset + retryBcs.size();
      idx = 0;
      continue;
    }

    auto const& bc = idx < retryOffset ?
      interp.blk->hhbcs[idx] : retryBcs[idx - retryOffset];
    ++idx;

    interpOne(env, bc);
    auto const& flags = env.flags;
    if (interp.collect.effectFree && !flags.effectFree) {
      interp.collect.effectFree = false;
      if (any(interp.collect.opts & CollectionOpts::EffectFreeOnly)) {
        env.collect.mInstrState.clear();
        FTRACE(2, "  Bailing because not effect free\n");
        return finish(NoBlockId);
      }
    }

    if (flags.returned) {
      always_assert(idx == size);
      if (env.reprocess) continue;

      always_assert(interp.blk->fallthrough == NoBlockId);
      assertx(!ret.returned);
      FTRACE(2, "  returned {}\n", show(*flags.returned));
      ret.retParam = flags.retParam;
      ret.returned = flags.returned;
      return finish(NoBlockId);
    }

    if (flags.jmpDest != NoBlockId) {
      always_assert(idx == size);
      auto const hasFallthrough = [&] {
        if (flags.jmpDest != interp.blk->fallthrough) {
          FTRACE(2, "  <took branch; no fallthrough>\n");
          auto const last = last_op(env);
          return !last || !instrIsNonCallControlFlow(last->op);
        } else {
          FTRACE(2, "  <branch never taken>\n");
          return true;
        }
      }();
      if (hasFallthrough) retryFallthrough = flags.jmpDest;
      if (env.reprocess) continue;
      finish_tracked_elems(env, 0);
      auto const newDest = speculateHelper(env, flags.jmpDest, true);
      propagate(newDest, &interp.state);
      return finish(hasFallthrough ? newDest : NoBlockId);
    }

    if (interp.state.unreachable) {
      if (env.reprocess) {
        idx = size;
        continue;
      }
      FTRACE(2, "  <bytecode fallthrough is unreachable>\n");
      finish_tracked_elems(env, 0);
      return finish(NoBlockId);
    }
  }

  FTRACE(2, "  <end block>\n");
  if (retryFallthrough != NoBlockId) {
    retryFallthrough = speculateHelper(env, retryFallthrough, false);
    propagate(retryFallthrough, &interp.state);
  }
  return finish(retryFallthrough);
}

StepFlags step(Interp& interp, const Bytecode& op) {
  auto noop    = [] (BlockId, const State*) {};
  ISS env { interp, noop };
  env.analyzeDepth++;
  dispatch(env, op);
  if (env.state.unreachable) {
    env.collect.mInstrState.clear();
  }
  assertx(env.trackedElems.empty());
  return env.flags;
}

void default_dispatch(ISS& env, const Bytecode& op) {
  if (!env.trackedElems.empty()) {
    auto const pops = [&] () -> uint32_t {
      switch (op.op) {
        case Op::AddElemC:
        case Op::AddNewElemC:
          return numPop(op) - 1;
        case Op::Concat:
        case Op::ConcatN:
          return 0;
        default:
          return numPop(op);
      }
    }();

    finish_tracked_elems(env, env.state.stack.size() - pops);
  }
  dispatch(env, op);
  if (instrFlags(op.op) & TF && env.flags.jmpDest == NoBlockId) {
    unreachable(env);
  } else if (env.state.unreachable) {
    env.collect.mInstrState.clear();
  }
}

folly::Optional<Type> thisType(const Index& index, Context ctx) {
  return thisTypeFromContext(index, ctx);
}

//////////////////////////////////////////////////////////////////////

}}
