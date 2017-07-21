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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/ext/collections/ext_collections.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

#include <limits>

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////
// ConvToBool

void cgConvIntToBool(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testq{src, src, sf};
  v << setcc{CC_NE, sf, dst};
}

void cgConvDblToBool(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const movtdq_res = v.makeReg();
  auto const sf = v.makeReg();
  v << movtdq{src, movtdq_res};
  // 0.0 stays zero and -0.0 is now 0.0
  v << shlqi{1, movtdq_res, v.makeReg(), sf};
  // lower byte becomes 1 if dst != 0
  v << setcc{CC_NE, sf, dst};
}

void cgConvStrToBool(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmplim{1, src[StringData::sizeOff()], sf};

  unlikelyCond(v, vcold(env), CC_E, sf, dst,
    [&] (Vout& v) {
      // Unlikely case is we end up having to check whether the first byte of
      // the string is equal to '0'.
      auto const dst = v.makeReg();
      auto const sf = v.makeReg();
#ifdef NO_M_DATA
      v << cmpbim{'0', src[sizeof(StringData)], sf};
#else
      auto const sd = v.makeReg();
      v << load{src[StringData::dataOff()], sd};
      v << cmpbim{'0', sd[0], sf};
#endif
      v << setcc{CC_NE, sf, dst};
      return dst;
    },
    [&] (Vout& v) {
      // Common case is we have an empty string or a string with size bigger
      // than one.
      auto const dst = v.makeReg();
      v << setcc{CC_G, sf, dst};
      return dst;
    }
  );
}

void cgConvArrToBool(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmplim{0, src[ArrayData::offsetofSize()], sf};

  unlikelyCond(v, vcold(env), CC_S, sf, dst,
    [&] (Vout& v) {
      auto const vsize = v.makeReg();
      cgCallHelper(v, env, CallSpec::method(&ArrayData::vsize),
                   callDest(vsize), SyncOptions::None,
                   argGroup(env, inst).ssa(0));

      auto const sf = v.makeReg();
      auto const d = v.makeReg();
      v << testl{vsize, vsize, sf};
      v << setcc{CC_NZ, sf, d};
      return d;
    },
    [&] (Vout& v) {
      auto const d = v.makeReg();
      v << setcc{CC_NZ, sf, d};
      return d;
    }
  );
}

void cgConvObjToBool(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testwim{ObjectData::CallToImpl, src[ObjectData::attributeOff()], sf};

  unlikelyCond(v, vcold(env), CC_NZ, sf, dst,
    [&] (Vout& v) {
      auto const sf = v.makeReg();
      v << testwim{
        ObjectData::IsCollection,
        src[ObjectData::attributeOff()],
        sf
      };

      return cond(v, CC_NZ, sf, v.makeReg(),
        [&] (Vout& v) { // src points to native collection
          auto const d = v.makeReg();
          auto const sf = v.makeReg();
          v << cmplim{0, src[collections::FAST_SIZE_OFFSET], sf};
          v << setcc{CC_NE, sf, d}; // true iff size not zero
          return d;
        },
        [&] (Vout& v) { // src is not a native collection
          auto const d = v.makeReg();
          cgCallHelper(v, env,
                       CallSpec::method(&ObjectData::toBoolean),
                       CallDest{DestType::Byte, d},
                       SyncOptions::Sync,
                       argGroup(env, inst).ssa(0));
          return d;
        });
    },
    [&] (Vout& v) { return v.cns(true); }
  );
}

IMPL_OPCODE_CALL(ConvCellToBool);

///////////////////////////////////////////////////////////////////////////////
// ConvToInt

void cgConvBoolToInt(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  vmain(env) << movzbq{src, dst};
}

void cgConvDblToInt(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const indef = v.cns(0x8000000000000000L);

  auto const d = v.makeReg();
  auto const sf = v.makeReg();
  v << cvttsd2siq{src, d};
  v << cmpq{indef, d, sf};

  unlikelyCond(
    v, vcold(env), CC_E, sf, dst,
    [&](Vout& v) {
      // result > max signed int or unordered
      auto const sf = v.makeReg();
      v << ucomisd{v.cns(0.0), src, sf};

      return cond(
        v, CC_NB, sf, v.makeReg(), [&](Vout& /*v*/) { return d; },
        [&](Vout& v) {
          // src > 0 (CF = 1 -> less than 0 or unordered)
          return cond(v, CC_P, sf, v.makeReg(),
            [&] (Vout& v) {
              // PF = 1 -> unordered, i.e., we are doing an int cast of NaN.
              // PHP5 didn't formally define this, but observationally returns
              // the truncated value (i.e., what d currently holds).  PHP7
              // formally defines this case to return 0.
              return RuntimeOption::PHP7_IntSemantics ? v.cns(0) : d;
            },
            [&] (Vout& v) {
              constexpr uint64_t ulong_max =
                std::numeric_limits<unsigned long>::max();

              auto const sf = v.makeReg();
              v << ucomisd{v.cns(static_cast<double>(ulong_max)), src, sf};

              return cond(v, CC_B, sf, v.makeReg(),
                [&] (Vout& v) { return v.cns(0); }, // src > ULONG_MAX
                [&] (Vout& v) {
                  // 0 < src <= ULONG_MAX
                  //
                  // We know that LONG_MAX < src <= ULONG_MAX, so therefore:
                  // 0 < src - LONG_MAX <= ULONG_MAX
                  constexpr uint64_t long_max =
                    std::numeric_limits<long>::max();

                  auto const tmp_sub = v.makeReg();
                  auto const tmp_int = v.makeReg();
                  v << subsd{v.cns(static_cast<double>(long_max)),
                             src, tmp_sub};
                  v << cvttsd2siq{tmp_sub, tmp_int};

                  // We want to simulate integer overflow so we take the
                  // resulting integer and flip its sign bit.  (NB: We don't
                  // use orq{} here because it's possible that src == LONG_MAX
                  // in which case cvttsd2siq will yield an indefiniteInteger,
                  // which we would like to make zero.)
                  auto const res = v.makeReg();
                  v << xorq{indef, tmp_int, res, v.makeReg()};
                  return res;
                }
              );
            }
          );
        });
    },
    [&](Vout& /*v*/) { return d; });
}

IMPL_OPCODE_CALL(ConvStrToInt);
IMPL_OPCODE_CALL(ConvObjToInt);
IMPL_OPCODE_CALL(ConvResToInt);
IMPL_OPCODE_CALL(ConvCellToInt);

///////////////////////////////////////////////////////////////////////////////
// ConvToDbl

namespace {

void implConvBoolOrIntToDbl(IRLS& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  assertx(val->isA(TBool) || val->isA(TInt));

  auto const dst = dstLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // cvtsi2sd doesn't modify the high bits of its target, which can cause false
  // dependencies to prevent register renaming from kicking in.  Break the
  // dependency chain by zeroing out the XMM reg.
  auto const src_zext = zeroExtendIfBool(v, val->type(), src);
  v << cvtsi2sd{src_zext, dst};
}

}

void cgConvBoolToDbl(IRLS& env, const IRInstruction* inst) {
  implConvBoolOrIntToDbl(env, inst);
}

void cgConvIntToDbl(IRLS& env, const IRInstruction* inst) {
  implConvBoolOrIntToDbl(env, inst);
}

IMPL_OPCODE_CALL(ConvStrToDbl);
IMPL_OPCODE_CALL(ConvArrToDbl);
IMPL_OPCODE_CALL(ConvObjToDbl);
IMPL_OPCODE_CALL(ConvResToDbl);
IMPL_OPCODE_CALL(ConvCellToDbl);

///////////////////////////////////////////////////////////////////////////////
// ConvToVArray

static ArrayData* convArrToVArrImpl(ArrayData* adIn) {
  assertx(adIn->isPHPArray());
  auto a = adIn->toVArray(adIn->cowCheck());
  assertx(a->isVArray());
  if (a != adIn) decRefArr(adIn);
  return a;
}

static ArrayData* convVecToVArrImpl(ArrayData* adIn) {
  assertx(adIn->isVecArray());
  auto a = PackedArray::ToVArrayVec(adIn, adIn->cowCheck());
  assertx(a->isVArray());
  if (a != adIn) decRefArr(adIn);
  return a;
}

static ArrayData* convDictToVArrImpl(ArrayData* adIn) {
  assertx(adIn->isDict());
  auto a = MixedArray::ToVArrayDict(adIn, adIn->cowCheck());
  assertx(a != adIn);
  assertx(a->isVArray());
  decRefArr(adIn);
  return a;
}

static ArrayData* convKeysetToVArrImpl(ArrayData* adIn) {
  assertx(adIn->isKeyset());
  auto a = SetArray::ToVArray(adIn, adIn->cowCheck());
  assertx(a != adIn);
  assertx(a->isVArray());
  decRefArr(adIn);
  return a;
}

static ArrayData* convObjToVArrImpl(ObjectData* obj) {
  if (obj->isCollection()) {
    auto a = [&]{
      if (auto ad = collections::asArray(obj)) {
        return ArrNR{ad}.asArray().toVArray();
      }
      return collections::toArray(obj).toVArray();
    }();
    assertx(a->isVArray());
    decRefObj(obj);
    return a.detach();
  }

  if (obj->instanceof(SystemLib::s_IteratorClass)) {
    // This assumes that appending to an initially empty array will never
    // promote to mixed.
    auto arr = Array::Create();
    for (ArrayIter iter(obj); iter; ++iter) {
      arr.append(iter.second());
    }
    decRefObj(obj);
    assertx(arr->isVArray());
    return arr.detach();
  }

  SystemLib::throwInvalidOperationExceptionObject(
    "Non-iterable object to varray conversion"
  );
}

namespace {

void convToVArrHelper(IRLS& env, const IRInstruction* inst,
                      CallSpec call, bool sync) {
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(
    vmain(env),
    env,
    call,
    callDest(env, inst),
    sync ? SyncOptions::Sync : SyncOptions::None,
    args
  );
}

}

void cgConvArrToVArr(IRLS& env, const IRInstruction* inst) {
  convToVArrHelper(env, inst, CallSpec::direct(convArrToVArrImpl), false);
}

void cgConvVecToVArr(IRLS& env, const IRInstruction* inst) {
  convToVArrHelper(env, inst, CallSpec::direct(convVecToVArrImpl), false);
}

void cgConvDictToVArr(IRLS& env, const IRInstruction* inst) {
  convToVArrHelper(env, inst, CallSpec::direct(convDictToVArrImpl), false);
}

void cgConvKeysetToVArr(IRLS& env, const IRInstruction* inst) {
  convToVArrHelper(env, inst, CallSpec::direct(convKeysetToVArrImpl), false);
}

void cgConvObjToVArr(IRLS& env, const IRInstruction* inst) {
  convToVArrHelper(env, inst, CallSpec::direct(convObjToVArrImpl), true);
}

///////////////////////////////////////////////////////////////////////////////
// ConvToDArray

static ArrayData* convObjToDArrImpl(ObjectData* obj) {
  if (obj->isCollection()) {
    auto a = [&]{
      if (auto ad = collections::asArray(obj)) {
        return ArrNR{ad}.asArray().toPHPArray();
      }
      return collections::toArray(obj).toPHPArray();
    }();
    assertx(a->isPHPArray());
    decRefObj(obj);
    return a.detach();
  }

  if (obj->instanceof(SystemLib::s_IteratorClass)) {
    auto arr = Array::Create();
    for (ArrayIter iter(obj); iter; ++iter) {
      arr.set(iter.first(), iter.second());
    }
    decRefObj(obj);
    assertx(arr->isPHPArray());
    return arr.detach();
  }

  SystemLib::throwInvalidOperationExceptionObject(
    "Non-iterable object to darray conversion"
  );
}

void cgConvObjToDArr(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(convObjToDArrImpl),
    callDest(env, inst),
    SyncOptions::Sync,
    args
  );
}

///////////////////////////////////////////////////////////////////////////////
// ConvToStr

IMPL_OPCODE_CALL(ConvIntToStr);
IMPL_OPCODE_CALL(ConvDblToStr);
IMPL_OPCODE_CALL(ConvObjToStr);
IMPL_OPCODE_CALL(ConvResToStr);
IMPL_OPCODE_CALL(ConvCellToStr);

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(ConvBoolToArr);
IMPL_OPCODE_CALL(ConvIntToArr);
IMPL_OPCODE_CALL(ConvDblToArr);
IMPL_OPCODE_CALL(ConvStrToArr);
IMPL_OPCODE_CALL(ConvVecToArr);
IMPL_OPCODE_CALL(ConvDictToArr);
IMPL_OPCODE_CALL(ConvKeysetToArr);
IMPL_OPCODE_CALL(ConvObjToArr);
IMPL_OPCODE_CALL(ConvCellToArr);

IMPL_OPCODE_CALL(ConvArrToVec);
IMPL_OPCODE_CALL(ConvDictToVec);
IMPL_OPCODE_CALL(ConvKeysetToVec);
IMPL_OPCODE_CALL(ConvObjToVec);

IMPL_OPCODE_CALL(ConvArrToDict);
IMPL_OPCODE_CALL(ConvVecToDict);
IMPL_OPCODE_CALL(ConvKeysetToDict);
IMPL_OPCODE_CALL(ConvObjToDict);

IMPL_OPCODE_CALL(ConvArrToKeyset);
IMPL_OPCODE_CALL(ConvVecToKeyset);
IMPL_OPCODE_CALL(ConvDictToKeyset);
IMPL_OPCODE_CALL(ConvObjToKeyset);

IMPL_OPCODE_CALL(ConvCellToObj);

///////////////////////////////////////////////////////////////////////////////

static TypedValue strictlyIntegerConvImpl(StringData* str) {
  int64_t n;
  if (str->isStrictlyInteger(n)) {
    return make_tv<KindOfInt64>(n);
  }
  str->incRefCount();
  return make_tv<KindOfString>(str);
}

void cgStrictlyIntegerConv(IRLS& env, const IRInstruction* inst) {
  auto const args = argGroup(env, inst).ssa(0);
  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(strictlyIntegerConvImpl),
    callDestTV(env, inst),
    SyncOptions::None,
    args
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
