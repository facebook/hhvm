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
#include "hphp/runtime/vm/jit/irgen-builtin.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-types.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_is_a("is_a"),
  s_count("count"),
  s_ini_get("ini_get"),
  s_in_array("in_array"),
  s_get_class("get_class"),
  s_get_called_class("get_called_class"),
  s_is_object("is_object");

//////////////////////////////////////////////////////////////////////

SSATmp* optimizedCallIsA(HTS& env) {
  // The last param of is_a has a default argument of false, which makes it
  // behave the same as instanceof (which doesn't allow a string as the tested
  // object). Don't do the conversion if we're not sure this arg is false.
  auto const allowStringType = topType(env, 0);
  if (!(allowStringType <= Type::Bool)
      || !allowStringType.isConst()
      || allowStringType.boolVal()) {
    return nullptr;
  }

  // Unlike InstanceOfD, is_a doesn't support interfaces like Stringish, so e.g.
  // "is_a('x', 'Stringish')" is false even though "'x' instanceof Stringish" is
  // true. So if the first arg is not an object, the return is always false.
  auto const objType = topType(env, 2);
  if (!objType.maybe(Type::Obj)) {
    return cns(env, false);
  }

  if (objType <= Type::Obj) {
    auto const classnameType = topType(env, 1);
    if (classnameType <= Type::StaticStr && classnameType.isConst()) {
      return implInstanceOfD(
        env,
        topC(env, 2),
        top(env, Type::Str, 1)->strVal()
      );
    }
  }

  // The LHS is a strict superset of Obj; bail.
  return nullptr;
}

SSATmp* optimizedCallCount(HTS& env) {
  auto const mode = top(env, Type::Int, 0);
  auto const val = topC(env, 1);

  // Bail if we're trying to do a recursive count()
  if (!mode->isConst(0)) return nullptr;

  return gen(env, Count, makeCatch(env), val);
}

SSATmp* optimizedCallIniGet(HTS& env) {
  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  auto const argType = topType(env, 0);
  if (!(argType <= Type::StaticStr) || !argType.isConst()) {
    return nullptr;
  }

  // We can only optimize settings that are system wide since user level
  // settings can be overridden during the execution of a request.
  auto const settingName = top(env, Type::Str, 0)->strVal()->toCppString();
  IniSetting::Mode mode = IniSetting::PHP_INI_NONE;
  if (!IniSetting::GetMode(settingName, mode) ||
      !(mode & IniSetting::PHP_INI_SYSTEM)) {
    return nullptr;
  }

  Variant value;
  IniSetting::Get(settingName, value);
  // ini_get() is now enhanced to return more than strings
  // Only return a string, get out of here if we are something
  // else like an array
  if (value.isString()) {
    return cns(env, makeStaticString(value.toString()));
  }
  return nullptr;
}

/*
 * Transforms in_array with a static haystack argument into an AKExists with the
 * haystack flipped.
 */
SSATmp* optimizedCallInArray(HTS& env) {
  // We will restrict this optimization to needles that are strings, and
  // haystacks that have only non-numeric string keys. This avoids a bunch of
  // complication around numeric-string array-index semantics.
  if (!(topType(env, 2) <= Type::Str)) {
    return nullptr;
  }

  auto const haystackType = topType(env, 1);
  if (!(haystackType <= Type::StaticArr) || !haystackType.isConst()) {
    // Haystack isn't statically known
    return nullptr;
  }

  auto const haystack = haystackType.arrVal();
  if (haystack->size() == 0) {
    return cns(env, false);
  }

  ArrayInit flipped{haystack->size(), ArrayInit::Map{}};

  for (auto iter = ArrayIter{haystack}; iter; ++iter) {
    auto const& key = iter.secondRef();
    int64_t ignoredInt;
    double ignoredDbl;

    if (!key.isString() ||
        key.asCStrRef().get()
          ->isNumericWithVal(ignoredInt, ignoredDbl, false) != KindOfNull) {
      // Numeric strings will complicate matters because the loose comparisons
      // done with array keys are not quite the same as loose comparisons done
      // by in_array. For example: in_array('0', array('0000')) is true, but
      // doing array('0000' => true)['0'] will say "undefined index". This seems
      // unlikely to affect real-world usage.
      return nullptr;
    }

    flipped.set(key.asCStrRef(), init_null_variant);
  }

  auto needle = topC(env, 2);
  auto array = flipped.toArray();
  return gen(env,
             AKExists,
             cns(env, ArrayData::GetScalarArray(array.get())),
             needle);
}

SSATmp* optimizedCallGetClass(HTS& env, uint32_t numNonDefault) {
  auto const curCls = curClass(env);
  auto const curName = [&] {
    return curCls != nullptr ? cns(env, curCls->name()) : nullptr;
  };

  if (numNonDefault == 0) return curName();
  assert(numNonDefault == 1);

  auto const val = topC(env, 0);
  if (val->isA(Type::Null)) return curName();

  if (val->isA(Type::Obj)) {
    return gen(env, LdClsName, gen(env, LdObjClass, val));
  }

  return nullptr;
}

SSATmp* optimizedCallGetCalledClass(HTS& env) {
  if (!curClass(env)) return nullptr;
  auto const ctx = ldCtx(env);
  auto const cls = gen(env, LdClsCtx, ctx);
  return gen(env, LdClsName, cls);
}

//////////////////////////////////////////////////////////////////////

bool optimizedFCallBuiltin(HTS& env,
                           const Func* func,
                           uint32_t numArgs,
                           uint32_t numNonDefault) {
  SSATmp* res = nullptr;
  switch (numArgs) {
  case 0:
    if (func->name()->isame(s_get_called_class.get())) {
      res = optimizedCallGetCalledClass(env);
    }
    break;
  case 1:
    if (func->name()->isame(s_ini_get.get())) {
      res = optimizedCallIniGet(env);
    } else if (func->name()->isame(s_get_class.get())) {
      res = optimizedCallGetClass(env, numNonDefault);
    }
    break;
  case 2:
    if (func->name()->isame(s_count.get())) {
      res = optimizedCallCount(env);
    }
    break;
  case 3:
    if (func->name()->isame(s_is_a.get())) {
      res = optimizedCallIsA(env);
    } else if (func->name()->isame(s_in_array.get())) {
      res = optimizedCallInArray(env);
    }
    break;
  default:
    break;
  }

  if (res == nullptr) return false;

  // Decref and free args
  for (int i = 0; i < numArgs; i++) {
    auto const arg = popR(env);
    if (i >= numArgs - numNonDefault) {
      gen(env, DecRef, arg);
    }
  }

  push(env, res);
  return true;
}

//////////////////////////////////////////////////////////////////////

/*
 * CoerceStk can generate two types of exceptions:
 *
 * TVCoercionException: Indicates parameter coersion failed and designates
 *                      a return value to push onto the stack. These types
 *                      of exceptions will trigger a side exit which will
 *                      push a return value and continue execution at the
 *                      next BC.
 *
 * PHP exceptions: Potentially generated when user code is reentered as part
 *                 of coercion. These exceptions should be treated normally
 *                 and allowed to continue unwinding the stack.
 *
 *
 * The commonBody parameter is a function to be run before checking if we are
 * handling a TVCoercionException or not. It should contain any common cleanup
 * shared between this and regular exceptions.
 *
 * The sideExitBody parameter is a function to be run before pushing the
 * return value. It should pop any parameters off the stack and perform the
 * associated DecRefs if necessary.
 *
 * The takenBody parameter is a function to be run should a non-
 * TVCoercionException be raised from user code. In this case any cleanup
 * necessary should be performed before the undwinder resumes.
 *
 * Three blocks are emitted when this function is called, and they follow this
 * template, B0 is returned:
 *
 * B0:
 *  BeginCatch
 *  <<commonBody>>
 *  UnwindCheckSideExit FP, SP -> B2
 *  -> B1
 * B1:
 *  <<sideExitBody>>
 *  val = LdUnwinderValue<Cell>
 *  DeleteUnwinderException
 *  SP = SpillStack<...>
 *  SyncABIRegs FP, SP
 *  EagerSyncABIRegs FP, SP
 *  ReqBindJmp<nextBcOff>
 * B2:
 *  <<takenBody>>
 *  EndCatch FP, SP
 */
template<class CommonBody, class SideExitBody, class TakenBody>
Block* makeParamCoerceExit(HTS& env,
                           CommonBody commonBody,
                           SideExitBody sideExitBody,
                           TakenBody takenBody) {
  auto exit = env.irb->makeExit(Block::Hint::Unused);
  auto taken = env.irb->makeExit(Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
  gen(env, BeginCatch);
  commonBody();
  gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));

  // prepare for regular exception
  {
    BlockPusher bpTaken(*env.irb, makeMarker(env, bcOff(env)), taken);

    auto const stack = takenBody();
    gen(env, EndCatch, fp(env), stack);
  }

  // prepare for side exit
  sideExitBody();

  // Push the side exit return value onto the stack and cleanup the exception
  auto const val = gen(env, LdUnwinderValue, Type::Cell);
  gen(env, DeleteUnwinderException);

  // Spill the stack
  auto spills = peekSpillValues(env);
  spills.insert(spills.begin(), val);
  auto const stack = implSpillStack(env, sp(env), spills, 0);

  gen(env, SyncABIRegs, fp(env), stack);
  gen(env, EagerSyncVMRegs, fp(env), stack);
  gen(env, ReqBindJmp, ReqBindJmpData(nextBcOff(env)));

  return exit;
}

template<class GetArg>
void builtinCall(HTS& env,
                 const Func* callee,
                 uint32_t numArgs,
                 uint32_t numNonDefault,
                 SSATmp* paramThis,
                 bool inlining,
                 bool wasInliningConstructor,
                 GetArg getArg) {
  auto const destroyLocals = builtinFuncDestroysLocals(callee);

  // collect the parameter locals---we'll need them later.  Also
  // determine which ones will need to be passed through the eval
  // stack.
  jit::vector<SSATmp*> paramSSAs(numArgs);
  jit::vector<bool> paramThroughStack(numArgs);
  jit::vector<bool> paramNeedsConversion(numArgs);
  auto numParamsThroughStack = uint32_t{0};
  for (auto i = uint32_t{0}; i < numArgs; ++i) {
    // Fill in paramSSAs in reverse, since they may come from popC's.
    auto const offset = numArgs - i - 1;
    paramSSAs[offset] = getArg(offset);

    auto const& pi = callee->params()[offset];
    auto dt = pi.builtinType;
    auto const& tc = pi.typeConstraint;
    if (tc.isNullable() && !callee->byRef(offset)) {
      dt = tc.underlyingDataType();
      ++numParamsThroughStack;
      paramThroughStack[offset] = true;
    } else if (dt && !callee->byRef(offset)) {
      [&] {
        switch (*dt) {
          case KindOfBoolean:
          case KindOfInt64:
          case KindOfDouble:
            paramThroughStack[offset] = false;
            return;
          case KindOfUninit:
          case KindOfNull:
          case KindOfStaticString:
          case KindOfString:
          case KindOfArray:
          case KindOfObject:
          case KindOfResource:
          case KindOfRef:
            ++numParamsThroughStack;
            paramThroughStack[offset] = true;
            return;
          case KindOfClass:
            break;
        }
        not_reached();
      }();
    } else {
      ++numParamsThroughStack;
      paramThroughStack[offset] = true;
    }

    paramNeedsConversion[offset] = offset < numNonDefault && dt;
  }

  // For the same reason that we have to IncRef the locals above, we
  // need to grab one on the $this.
  if (paramThis) gen(env, IncRef, paramThis);

  if (inlining) endInlinedCommon(env);   /////// leaving inlined function

  /*
   * Everything that needs to be on the stack gets spilled now.
   */
  if (numParamsThroughStack != 0 || !inlining) {
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (paramThroughStack[i]) {
        push(env, paramSSAs[i]);
      }
    }
    // We're going to do ldStackAddrs on these, so the stack must be
    // materialized:
    spillStack(env);
    // This marker update is to make sure rbx points to the bottom of
    // our stack when we enter our catch trace.  The catch trace
    // twiddles the VM registers directly on the execution context to
    // make the unwinder understand the situation, however.
    updateMarker(env);
  }

  auto const decRefForCatch =  [&] {
    // TODO(#4323657): this is generating generic DecRefs at the time
    // of this writing---probably we're not handling the stack chain
    // correctly in a catch block.
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (paramThroughStack[i]) {
        popDecRef(env, Type::Gen);
      } else {
        gen(env, DecRef, paramSSAs[i]);
      }
    }
  };

  /*
   * We have an unusual situation if we raise an exception:
   *
   * The unwinder is going to see our PC as equal to the FCallD for
   * the call to this NativeImpl instruction.  This means the PC will
   * be inside the FPI region for the call, so it'll try to pop an
   * ActRec.
   *
   * Meanwhile, we've just exited the inlined callee (and its frame
   * was hopefully removed by dce), and then pushed any of our
   * by-reference arguments on the eval stack.  So, if we throw, we
   * need to pop anything we pushed, put down a fake ActRec, and then
   * eagerly sync VM regs to represent that stack depth.
   */
  auto const prepareForCatch = [&] {
    if (inlining) {
      fpushActRec(env,
                  cns(env, callee),
                  paramThis ? paramThis : cns(env, Type::Nullptr),
                  ActRec::encodeNumArgs(numArgs,
                                        false /* localsDecRefd */,
                                        false /* resumed */,
                                        wasInliningConstructor),
                  nullptr);
    }
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      // TODO(#4313939): it's not actually necessary to push these
      // nulls.
      push(env, cns(env, Type::InitNull));
    }
    auto const stack = spillStack(env);
    gen(env, SyncABIRegs, fp(env), stack);
    gen(env, EagerSyncVMRegs, fp(env), stack);
    return stack;
  };

  auto const prepareForSideExit = [&] {
    if (paramThis) gen(env, DecRef, paramThis);
  };
  auto const makeUnusualCatch = [&] {
    return makeCatchImpl(
      env,
      [&] {
        decRefForCatch();
        return prepareForCatch();
      }
    );
  };

  auto const makeParamCoerceCatch = [&] {
    return makeParamCoerceExit(env,
                               decRefForCatch,
                               prepareForSideExit,
                               prepareForCatch);
  };

  /*
   * Prepare the actual arguments to the CallBuiltin instruction.  If any of
   * the parameters need type conversions, we need to handle that too.
   */
  auto const cbNumArgs = 2 + numArgs + (paramThis ? 1 : 0);
  SSATmp* args[cbNumArgs];
  auto argIdx = uint32_t{0};
  args[argIdx++] = fp(env);
  args[argIdx++] = sp(env);
  {

    auto stackIdx = uint32_t{0};

    if (paramThis) args[argIdx++] = paramThis;
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      auto pi = callee->params()[i];
      auto dt = pi.builtinType;
      auto const& tc = pi.typeConstraint;

      Type t;
      if (tc.isNullable() && !callee->byRef(i)) {
        dt = tc.underlyingDataType();
        t = Type::Null;
      }
      if (dt) {
        t |= Type(*dt);
      }

      if (!paramThroughStack[i]) {
        if (paramNeedsConversion[i]) {
          auto const oldVal = paramSSAs[i];

          if (callee->isParamCoerceMode()) {
            auto conv = [&](Type t, SSATmp* src) {
              if (t <= Type::Int) {
                return gen(env,
                           CoerceCellToInt,
                           CoerceData(callee, i + 1),
                           makeParamCoerceCatch(),
                           src);
              }
              if (t <= Type::Dbl) {
                return gen(env,
                           CoerceCellToDbl,
                           CoerceData(callee, i + 1),
                           makeParamCoerceCatch(),
                           src);
              }
              always_assert(t <= Type::Bool);
              return gen(env,
                         CoerceCellToBool,
                         CoerceData(callee, i + 1),
                         makeParamCoerceCatch(),
                         src);
            };

            paramSSAs[i] = [&] {
              if (Type::Null < t) {
                return env.irb->cond(
                  0,
                  [&] (Block* taken) {
                    auto isnull = gen(env, IsType, Type::Null, oldVal);
                    gen(env, JmpNZero, taken, isnull);
                  },
                  [&] { // Next: oldVal is non-null
                    t -= Type::Null;
                    return conv(t, oldVal);
                  },
                  [&] { // Taken: oldVal is null
                    return gen(env, AssertType, Type::Null, oldVal);
                  }
                );
              }
              return conv(t, oldVal);
            }();
          } else {
            paramSSAs[i] = [&] {
              if (t <= Type::Int) {
                return gen(env, ConvCellToInt, makeUnusualCatch(), oldVal);
              }
              if (t <= Type::Dbl) {
                return gen(env, ConvCellToDbl, makeUnusualCatch(), oldVal);
              }
              always_assert(t <= Type::Bool);
              return gen(env, ConvCellToBool, oldVal);
            }();
          }
          gen(env, DecRef, oldVal);
        }
        args[argIdx++] = paramSSAs[i];
        continue;
      }

      auto const offset = numParamsThroughStack - stackIdx - 1;
      if (paramNeedsConversion[i]) {
        auto mi = callee->methInfo();
        if (callee->params()[i].builtinType == KindOfObject && mi &&
            mi->parameters[i]->valueLen > 0) {
          t = Type::NullableObj;
        }

        if (callee->isParamCoerceMode()) {
          gen(env,
              CoerceStk,
              t,
              CoerceStkData(offset, callee, i + 1),
              makeParamCoerceCatch(),
              sp(env));
        } else {
          gen(env,
              CastStk,
              t,
              StackOffset(offset),
              makeUnusualCatch(),
              sp(env));
        }
      }

      args[argIdx++] = ldStackAddr(env, offset);
      ++stackIdx;
    }

    assert(stackIdx == numParamsThroughStack);
    assert(argIdx == cbNumArgs);
  }

  // Make the actual call.
  auto const retType = [&] {
    auto const retDT = callee->returnType();
    auto const ret = retDT ? Type(*retDT) : Type::Cell;
    return callee->attrs() & ClassInfo::IsReference ? ret.box() : ret;
  }();
  SSATmp** decayedPtr = &args[0];
  auto const ret = gen(
    env,
    CallBuiltin,
    retType,
    CallBuiltinData { callee, destroyLocals },
    makeUnusualCatch(),
    std::make_pair(cbNumArgs, decayedPtr)
  );

  // Pop the stack params and push the return value.
  if (paramThis) gen(env, DecRef, paramThis);
  for (auto i = uint32_t{0}; i < numParamsThroughStack; ++i) {
    popDecRef(env, Type::Gen);
  }
  push(env, ret);
}

/*
 * When we're inlining a NativeImpl opcode, we know this is the only opcode in
 * the callee method body aside from AssertRATs (bytecode invariant).  So in
 * order to make sure we can eliminate the SpillFrame, we do the CallBuiltin
 * instruction after we've left the inlined frame.
 *
 * We may need to pass some arguments to the builtin through the stack (e.g. if
 * it takes const Variant&'s)---these are spilled to the stack after leaving
 * the callee.
 *
 * To make this work, we need to do some weird things in the catch trace.  ;)
 */
void nativeImplInlined(HTS& env) {
  auto const callee = curFunc(env);
  assert(callee->nativeFuncPtr());

  // Figure out if this inlined function was for an FPushCtor.  We'll
  // need this creating the unwind block blow.
  auto const wasInliningConstructor = [&]() -> bool {
    auto const sframe = findSpillFrame(sp(env));
    assert(sframe);
    return sframe->extra<ActRecInfo>()->isFromFPushCtor();
  }();

  bool const instanceMethod = callee->isMethod() &&
                                !(callee->attrs() & AttrStatic);
  // Collect the parameter locals---we'll need them later.  Also
  // determine which ones will need to be passed through the eval
  // stack.
  auto const numArgs = callee->numParams();
  auto const paramThis = instanceMethod ? ldThis(env) : nullptr;

  builtinCall(env,
              callee,
              numArgs,
              numArgs,  /* numNonDefault */
              paramThis,
              true,     /* inlining */
              wasInliningConstructor,
              [&](uint32_t i) {
                auto ret = ldLoc(env, i, nullptr, DataTypeSpecific);
                gen(env, IncRef, ret);
                return ret;
              });
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

SSATmp* optimizedCallIsObject(HTS& env, SSATmp* src) {
  if (src->isA(Type::Obj) && src->type().isSpecialized()) {
    auto const cls = src->type().getClass();
    if (!env.irb->constrainValue(src, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard
      // any further, use it.
      return cns(env, cls != SystemLib::s___PHP_Incomplete_ClassClass);
    }
  }

  if (src->type().not(Type::Obj)) {
    return cns(env, false);
  }

  auto checkClass = [&] (SSATmp* obj) {
    auto cls = gen(env, LdObjClass, obj);
    auto testCls = SystemLib::s___PHP_Incomplete_ClassClass;
    return gen(env, ClsNeq, ClsNeqData { testCls }, cls);
  };

  return env.irb->cond(
    0, // references produced
    [&] (Block* taken) {
      auto isObj = gen(env, IsType, Type::Obj, src);
      gen(env, JmpZero, taken, isObj);
    },
    [&] { // Next: src is an object
      auto obj = gen(env, AssertType, Type::Obj, src);
      return checkClass(obj);
    },
    [&] { // Taken: src is not an object
      return cns(env, false);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitFCallBuiltin(HTS& env,
                      int32_t numArgs,
                      int32_t numNonDefault,
                      const StringData* funcName) {
  auto const callee = Unit::lookupFunc(funcName);

  if (optimizedFCallBuiltin(env, callee, numArgs, numNonDefault)) return;

  builtinCall(env,
              callee,
              numArgs,
              numNonDefault,
              nullptr,  /* no this */
              false,    /* not inlining */
              false,    /* not inlining constructor */
              [&](uint32_t) { return popC(env); });
}

void emitNativeImpl(HTS& env) {
  if (isInlining(env)) return nativeImplInlined(env);

  gen(env, NativeImpl, fp(env), sp(env));
  auto const stack   = gen(env, RetAdjustStack, fp(env));
  auto const retAddr = gen(env, LdRetAddr, fp(env));
  auto const frame   = gen(env, FreeActRec, fp(env));
  gen(env, RetCtrl, RetCtrlData(false), stack, frame, retAddr);
}

//////////////////////////////////////////////////////////////////////

void emitAbs(HTS& env) {
  auto const value = popC(env);

  if (value->isA(Type::Int)) {
    // compute integer absolute value ((src>>63) ^ src) - (src>>63)
    auto const t1 = gen(env, Shr, value, cns(env, 63));
    auto const t2 = gen(env, XorInt, t1, value);
    push(env, gen(env, SubInt, t2, t1));
    return;
  }

  if (value->isA(Type::Dbl)) {
    push(env, gen(env, AbsDbl, value));
    return;
  }

  if (value->isA(Type::Arr)) {
    gen(env, DecRef, value);
    push(env, cns(env, false));
    return;
  }

  PUNT(Abs);
}

void emitArrayIdx(HTS& env) {
  // These types are just used to decide what to do; once we know what we're
  // actually doing we constrain the values with the popC()s later on in this
  // function.
  auto const keyType = topC(env, 1, DataTypeGeneric)->type();
  auto const arrType = topC(env, 2, DataTypeGeneric)->type();

  if (!(arrType <= Type::Arr)) {
    // raise fatal
    interpOne(env, Type::Cell, 3);
    return;
  }

  if (keyType <= Type::Null) {
    auto const def = popC(env, DataTypeGeneric);
    auto const key = popC(env);
    auto const arr = popC(env);

    // if the key is null it will not be found so just return the default
    push(env, def);
    gen(env, DecRef, arr);
    gen(env, DecRef, key);
    return;
  }
  if (!(keyType <= Type::Int || keyType <= Type::Str)) {
    interpOne(env, Type::Cell, 3);
    return;
  }

  auto const def = popC(env, DataTypeGeneric); // a helper will decref it but
                                               // the translated code doesn't
                                               // care about the type
  auto const key = popC(env);
  auto const arr = popC(env);

  push(env, gen(env, ArrayIdx, arr, key, def));
  gen(env, DecRef, arr);
  gen(env, DecRef, key);
  gen(env, DecRef, def);
}

void emitIdx(HTS& env) {
  auto const keyType  = topC(env, 1, DataTypeGeneric)->type();
  auto const base     = topC(env, 2, DataTypeGeneric);
  auto const baseType = base->type();

  if (baseType <= Type::Arr &&
      (keyType <= Type::Int || keyType <= Type::Str)) {
    emitArrayIdx(env);
    return;
  }

  auto const catchBlock = makeCatch(env);
  auto const def = popC(env, DataTypeSpecific);
  auto const key = popC(env, DataTypeSpecific);
  auto const arr = popC(env, DataTypeSpecific);
  push(env, gen(env, GenericIdx, catchBlock, arr, key, def));
  gen(env, DecRef, arr);
  gen(env, DecRef, key);
  gen(env, DecRef, def);
}

void emitSqrt(HTS& env) {
  auto const srcType = topC(env)->type();
  if (srcType <= Type::Int) {
    auto const src = gen(env, ConvIntToDbl, popC(env));
    push(env, gen(env, Sqrt, src));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = popC(env);
    push(env, gen(env, Sqrt, src));
    return;
  }

  interpOne(env, Type::UncountedInit, 1);
}

void emitAKExists(HTS& env) {
  auto const arr = popC(env);
  auto const key = popC(env);

  if (!arr->isA(Type::Arr) && !arr->isA(Type::Obj)) {
    PUNT(AKExists_badArray);
  }
  if (!key->isA(Type::Str) && !key->isA(Type::Int) && !key->isA(Type::Null)) {
    PUNT(AKExists_badKey);
  }

  push(env, gen(env, AKExists, arr, key));
  gen(env, DecRef, arr);
  gen(env, DecRef, key);
}

void emitStrlen(HTS& env) {
  auto const inType = topC(env)->type();

  if (inType <= Type::Str) {
    auto const input = popC(env);
    if (input->isConst()) {
      // static string; fold its strlen operation
      push(env, cns(env, input->strVal()->size()));
      return;
    }

    push(env, gen(env, LdStrLen, input));
    gen(env, DecRef, input);
    return;
  }

  if (inType <= Type::Null) {
    popC(env);
    push(env, cns(env, 0));
    return;
  }

  if (inType <= Type::Bool) {
    // strlen(true) == 1, strlen(false) == 0.
    push(env, gen(env, ConvBoolToInt, popC(env)));
    return;
  }

  interpOne(env, Type::Int | Type::InitNull, 1);
}

void emitFloor(HTS& env) {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Floor);
  }

  auto const catchBlock = makeCatch(env);
  auto const val = popC(env);
  auto const dblVal = gen(env, ConvCellToDbl, catchBlock, val);
  gen(env, DecRef, val);
  push(env, gen(env, Floor, dblVal));
}

void emitCeil(HTS& env) {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Ceil);
  }

  auto const catchBlock = makeCatch(env);
  auto const val = popC(env);
  auto const dblVal = gen(env, ConvCellToDbl, catchBlock, val);
  gen(env, DecRef, val);
  push(env, gen(env, Ceil, dblVal));
}

void emitSilence(HTS& env, Id localId, SilenceOp subop) {
  // We can't generate direct StLoc and LdLocs in pseudomains (violates an IR
  // invariant).
  if (curFunc(env)->isPseudoMain()) PUNT(PseudoMain-Silence);

  switch (subop) {
  case SilenceOp::Start:
    // We assume that whatever is in the local is dead and doesn't need to be
    // refcounted before being overwritten.
    gen(env, AssertLoc, Type::Uncounted, LocalId(localId), fp(env));
    gen(env, StLoc, LocalId(localId), fp(env), gen(env, ZeroErrorLevel));
    break;
  case SilenceOp::End:
    {
      gen(env, AssertLoc, Type::Int, LocalId(localId), fp(env));
      auto const level = ldLoc(env, localId, makeExit(env), DataTypeGeneric);
      gen(env, RestoreErrorLevel, level);
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}}}

