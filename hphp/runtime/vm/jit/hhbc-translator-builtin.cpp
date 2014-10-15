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
#include "hphp/runtime/vm/jit/hhbc-translator.h"

#include <folly/CpuId.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/hhbc-translator-internal.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

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

SSATmp* HhbcTranslator::optimizedCallIsA() {
  // The last param of is_a has a default argument of false, which makes it
  // behave the same as instanceof (which doesn't allow a string as the tested
  // object). Don't do the conversion if we're not sure this arg is false.
  auto const allowStringType = topType(0);
  if (!(allowStringType <= Type::Bool)
      || !allowStringType.isConst()
      || allowStringType.boolVal()) {
    return nullptr;
  }

  // Unlike InstanceOfD, is_a doesn't support interfaces like Stringish, so e.g.
  // "is_a('x', 'Stringish')" is false even though "'x' instanceof Stringish" is
  // true. So if the first arg is not an object, the return is always false.
  auto const objType = topType(2);
  if (!objType.maybe(Type::Obj)) {
    return cns(false);
  }

  if (objType <= Type::Obj) {
    auto const classnameType = topType(1);
    if (classnameType <= Type::StaticStr && classnameType.isConst()) {
      return emitInstanceOfDImpl(topC(2), top(Type::Str, 1)->strVal());
    }
  }

  // The LHS is a strict superset of Obj; bail.
  return nullptr;
}

SSATmp* HhbcTranslator::optimizedCallCount() {
  auto const mode = top(Type::Int, 0);
  auto const val = topC(1);

  // Bail if we're trying to do a recursive count()
  if (!mode->isConst(0)) return nullptr;

  return gen(Count, makeCatch(), val);
}

SSATmp* HhbcTranslator::optimizedCallIniGet() {
  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  Type argType = topType(0);
  if (!(argType <= Type::StaticStr) || !argType.isConst()) {
      return nullptr;
  }

  // We can only optimize settings that are system wide since user level
  // settings can be overridden during the execution of a request.
  std::string settingName = top(Type::Str, 0)->strVal()->toCppString();
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
    return cns(makeStaticString(value.toString()));
  }
  return nullptr;
}

/*
 * Transforms in_array with a static haystack argument into an AKExists with the
 * haystack flipped.
 */
SSATmp* HhbcTranslator::optimizedCallInArray() {
  // We will restrict this optimization to needles that are strings, and
  // haystacks that have only non-numeric string keys. This avoids a bunch of
  // complication around numeric-string array-index semantics.
  if (!(topType(2) <= Type::Str)) {
    return nullptr;
  }

  auto const haystackType = topType(1);
  if (!(haystackType <= Type::StaticArr) || !haystackType.isConst()) {
    // Haystack isn't statically known
    return nullptr;
  }

  auto const haystack = haystackType.arrVal();
  if (haystack->size() == 0) {
    return cns(false);
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

  auto needle = topC(2);
  auto array = flipped.toArray();
  return gen(AKExists, cns(ArrayData::GetScalarArray(array.get())), needle);
}

SSATmp* HhbcTranslator::optimizedCallGetClass(uint32_t numNonDefault) {
  auto const curCls = curClass();
  auto const curName = [&] {
    return curCls != nullptr ? cns(curCls->name()) : nullptr;
  };

  if (numNonDefault == 0) return curName();
  assert(numNonDefault == 1);

  auto const val = topC(0);
  if (val->isA(Type::Null)) return curName();

  if (val->isA(Type::Obj)) return gen(LdClsName, gen(LdObjClass, val));

  return nullptr;
}

SSATmp* HhbcTranslator::optimizedCallGetCalledClass() {
  if (!curClass()) return nullptr;

  auto const ctx = gen(LdCtx, FuncData(curFunc()), m_irb->fp());
  auto const cls = gen(LdClsCtx, ctx);
  return gen(LdClsName, cls);
}

SSATmp* HhbcTranslator::optimizedCallIsObject(SSATmp* src) {
  if (src->isA(Type::Obj) && src->type().isSpecialized()) {
    auto const cls = src->type().getClass();
    if (!m_irb->constrainValue(src, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard
      // any further, use it.
      return cns(cls != SystemLib::s___PHP_Incomplete_ClassClass);
    }
  }

  if (src->type().not(Type::Obj)) {
    return cns(false);
  }

  auto checkClass = [this] (SSATmp* obj) {
    auto cls = gen(LdObjClass, obj);
    auto testCls = SystemLib::s___PHP_Incomplete_ClassClass;
    return gen(ClsNeq, ClsNeqData { testCls }, cls);
  };

  return m_irb->cond(
    0, // references produced
    [&] (Block* taken) {
      auto isObj = gen(IsType, Type::Obj, src);
      gen(JmpZero, taken, isObj);
    },
    [&] { // Next: src is an object
      auto obj = gen(AssertType, Type::Obj, src);
      return checkClass(obj);
    },
    [&] {// Taken: src is not an object
      return cns(false);
    }
  );
}

bool HhbcTranslator::optimizedFCallBuiltin(const Func* func,
                                           uint32_t numArgs,
                                           uint32_t numNonDefault) {
  SSATmp* res = nullptr;
  switch (numArgs) {
    case 0:
      if (func->name()->isame(s_get_called_class.get())) {
        res = optimizedCallGetCalledClass();
      }
      break;
    case 1:
      if (func->name()->isame(s_ini_get.get())) {
        res = optimizedCallIniGet();
      } else if (func->name()->isame(s_get_class.get())) {
        res = optimizedCallGetClass(numNonDefault);
      }
      break;
    case 2:
      if (func->name()->isame(s_count.get())) res = optimizedCallCount();
      break;
    case 3:
      if (func->name()->isame(s_is_a.get())) res = optimizedCallIsA();
      else if (func->name()->isame(s_in_array.get())) {
        res = optimizedCallInArray();
      }
      break;
    default: break;
  }

  if (res == nullptr) return false;

  // Decref and free args
  for (int i = 0; i < numArgs; i++) {
    auto const arg = popR();
    if (i >= numArgs - numNonDefault) {
      gen(DecRef, arg);
    }
  }

  push(res);
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
 *  CheckSideExit FP, SP -> B2
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
template<typename CommonBody, typename SideExitBody, typename TakenBody>
Block* HhbcTranslator::makeParamCoerceExit(CommonBody commonBody,
                                           SideExitBody sideExitBody,
                                           TakenBody takenBody) {
  auto exit = m_irb->makeExit(Block::Hint::Unused);
  auto taken = m_irb->makeExit(Block::Hint::Unused);

  BlockPusher bp(*m_irb, makeMarker(bcOff()), exit);
  gen(BeginCatch);
  commonBody();
  gen(CheckSideExit, taken, m_irb->fp(), m_irb->sp());

  // prepare for regular exception
  {
    BlockPusher bpTaken(*m_irb, makeMarker(bcOff()), taken);

    auto sp = takenBody();
    gen(EndCatch, m_irb->fp(), sp);
  }

  // prepare for side exit
  sideExitBody();

  // Push the side exit return value onto the stack and cleanup the exception
  auto val = gen(LdUnwinderValue, Type::Cell);
  gen(DeleteUnwinderException);

  // Spill the stack
  auto spills = peekSpillValues();
  spills.insert(spills.begin(), val);
  auto stack = emitSpillStack(m_irb->sp(), spills, 0);

  gen(SyncABIRegs, m_irb->fp(), stack);
  gen(EagerSyncVMRegs, m_irb->fp(), stack);
  gen(ReqBindJmp, ReqBindJmpData(nextBcOff()));

  return exit;
}

template<class GetArg>
void HhbcTranslator::emitBuiltinCall(const Func* callee,
                                     uint32_t numArgs,
                                     uint32_t numNonDefault,
                                     SSATmp* paramThis,
                                     bool inlining,
                                     bool wasInliningConstructor,
                                     bool destroyLocals,
                                     GetArg getArg) {
  // Collect the parameter locals---we'll need them later.  Also
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
      switch (*dt) {
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
          paramThroughStack[offset] = false;
          break;
        default:
          ++numParamsThroughStack;
          paramThroughStack[offset] = true;
          break;
      }
    } else {
      ++numParamsThroughStack;
      paramThroughStack[offset] = true;
    }

    paramNeedsConversion[offset] = offset < numNonDefault && dt;
  }

  // For the same reason that we have to IncRef the locals above, we
  // need to grab one on the $this.
  if (paramThis) gen(IncRef, paramThis);

  if (inlining) emitEndInlinedCommon();   /////// leaving inlined function

  /*
   * Everything that needs to be on the stack gets spilled now.
   */
  if (numParamsThroughStack != 0 || !inlining) {
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (paramThroughStack[i]) {
        push(paramSSAs[i]);
      }
    }
    // We're going to do ldStackAddrs on these, so the stack must be
    // materialized:
    spillStack();
    // This marker update is to make sure rbx points to the bottom of
    // our stack when we enter our catch trace.  The catch trace
    // twiddles the VM registers directly on the execution context to
    // make the unwinder understand the situation, however.
    updateMarker();
  }

  auto const decRefForCatch =  [&] {
    // TODO(#4323657): this is generating generic DecRefs at the time
    // of this writing---probably we're not handling the stack chain
    // correctly in a catch block.
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      if (paramThroughStack[i]) {
        popDecRef(Type::Gen);
      } else {
        gen(DecRef, paramSSAs[i]);
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
      emitFPushActRec(cns(callee),
                      paramThis ? paramThis : cns(Type::Nullptr),
                      ActRec::encodeNumArgs(numArgs,
                                            false /* localsDecRefd */,
                                            false /* resumed */,
                                            wasInliningConstructor),
                      nullptr);
    }
    for (auto i = uint32_t{0}; i < numArgs; ++i) {
      // TODO(#4313939): it's not actually necessary to push these
      // nulls.
      push(cns(Type::InitNull));
    }
    auto const stack = spillStack();
    gen(SyncABIRegs, m_irb->fp(), stack);
    gen(EagerSyncVMRegs, m_irb->fp(), stack);
    return stack;
  };


  auto const prepareForSideExit = [&] { if (paramThis) gen(DecRef, paramThis);};
  auto const makeUnusualCatch = [&] {
    return makeCatchImpl([&] {
      decRefForCatch();
      return prepareForCatch();
    });
  };

  auto const makeParamCoerceCatch = [&] {
    return makeParamCoerceExit(decRefForCatch,
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
  args[argIdx++] = m_irb->fp();
  args[argIdx++] = m_irb->sp();
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
                return gen(CoerceCellToInt,
                           CoerceData(callee, i + 1),
                           makeParamCoerceCatch(),
                           src);
              }
              if (t <= Type::Dbl) {
                return gen(CoerceCellToDbl,
                           CoerceData(callee, i + 1),
                           makeParamCoerceCatch(),
                           src);
              }
              always_assert(t <= Type::Bool);
              return gen(CoerceCellToBool,
                         CoerceData(callee, i + 1),
                         makeParamCoerceCatch(),
                         src);
            };

            paramSSAs[i] = [&] {
              if (Type::Null < t) {
                return m_irb->cond(
                  0,
                  [&] (Block* taken) {
                    auto isnull = gen(IsType, Type::Null, oldVal);
                    gen(JmpNZero, taken, isnull);
                  },
                  [&] { // Next: oldVal is non-null
                    t -= Type::Null;
                    return conv(t, oldVal);
                  },
                  [&] { // Taken: oldVal is null
                    return gen(AssertType, Type::Null, oldVal);
                  });
              }
              return conv(t, oldVal);
            }();
          } else {
            paramSSAs[i] = [&] {
              if (t <= Type::Int) {
                return gen(ConvCellToInt, makeUnusualCatch(), oldVal);
              }
              if (t <= Type::Dbl) {
                return gen(ConvCellToDbl, makeUnusualCatch(), oldVal);
              }
              always_assert(t <= Type::Bool);
              return gen(ConvCellToBool, oldVal);
            }();
          }
          gen(DecRef, oldVal);
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
          gen(CoerceStk,
              t,
              CoerceStkData(offset, callee, i + 1),
              makeParamCoerceCatch(),
              m_irb->sp());
        } else {
          gen(CastStk,
              t,
              StackOffset(offset),
              makeUnusualCatch(),
              m_irb->sp());
        }
      }

      args[argIdx++] = ldStackAddr(offset, DataTypeSpecific);
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
    CallBuiltin,
    retType,
    CallBuiltinData { callee, destroyLocals },
    makeUnusualCatch(),
    std::make_pair(cbNumArgs, decayedPtr)
  );

  // Pop the stack params and push the return value.
  if (paramThis) gen(DecRef, paramThis);
  for (auto i = uint32_t{0}; i < numParamsThroughStack; ++i) {
    popDecRef(Type::Gen);
  }
  push(ret);
}

/*
 * When we're inlining a NativeImpl opcode, we know this is the only
 * opcode in the callee method body (bytecode invariant).  So in
 * order to make sure we can eliminate the SpillFrame, we do the
 * CallBuiltin instruction after we've left the inlined frame.
 *
 * We may need to pass some arguments to the builtin through the
 * stack (e.g. if it takes const Variant&'s)---these are spilled to
 * the stack after leaving the callee.
 *
 * To make this work, we need to do some weird things in the catch
 * trace.  ;)
 */
void HhbcTranslator::emitNativeImplInlined() {
  auto const callee = curFunc();
  assert(callee->nativeFuncPtr());

  // Figure out if this inlined function was for an FPushCtor.  We'll
  // need this creating the unwind block blow.
  auto const wasInliningConstructor = [&]() -> bool {
    auto const sframe = findSpillFrame(m_irb->sp());
    assert(sframe);
    return sframe->extra<ActRecInfo>()->isFromFPushCtor();
  }();

  bool const instanceMethod = callee->isMethod() &&
                                !(callee->attrs() & AttrStatic);
  // Collect the parameter locals---we'll need them later.  Also
  // determine which ones will need to be passed through the eval
  // stack.
  auto const numArgs = callee->numParams();
  auto const paramThis = instanceMethod ? gen(LdThis, m_irb->fp())
                                        : nullptr;

  emitBuiltinCall(callee,
                  numArgs,
                  numArgs,  /* numNonDefault */
                  paramThis,
                  true,     /* inlining */
                  wasInliningConstructor,
                  builtinFuncDestroysLocals(callee), /* destroyLocals */
                  [&](uint32_t i) {
                    auto ret = ldLoc(i, nullptr, DataTypeSpecific);
                    gen(IncRef, ret);
                    return ret;
                  });
}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitAbs() {
  auto value = popC();

  if (value->isA(Type::Int)) {
    // compute integer absolute value ((src>>63) ^ src) - (src>>63)
    auto t1 = gen(Shr, value, cns(63));
    auto t2 = gen(XorInt, t1, value);
    push(gen(SubInt, t2, t1));
    return;
  }

  if (value->isA(Type::Dbl)) {
    push(gen(AbsDbl, value));
    return;
  }

  if (value->isA(Type::Arr)) {
    gen(DecRef, value);
    push(cns(false));
    return;
  }

  PUNT(Abs);
}

void HhbcTranslator::emitIdx() {
  Type keyType = topC(1, DataTypeGeneric)->type();
  SSATmp* base = topC(2, DataTypeGeneric);
  Type baseType = base->type();

  if (baseType <= Type::Arr &&
      (keyType <= Type::Int || keyType <= Type::Str)) {
    emitArrayIdx();
  } else {
    emitIdxCommon(GenericIdx, makeCatch());
  }
}

// NOTE: #3233688 talks about making an idx fast path for collections and
// that is where this function will be used and make more sense. It's only
// called once now.
void HhbcTranslator::emitIdxCommon(Opcode opc, Block* catchBlock) {
  SSATmp* def = popC(DataTypeSpecific);
  SSATmp* key = popC(DataTypeSpecific);
  SSATmp* arr = popC(DataTypeSpecific);
  push(gen(opc, catchBlock, arr, key, def));
  gen(DecRef, arr);
  gen(DecRef, key);
  gen(DecRef, def);
}


void HhbcTranslator::emitArrayIdx() {
  // These types are just used to decide what to do; once we know what we're
  // actually doing we constrain the values with the popC()s later on in this
  // function.
  auto const keyType = topC(1, DataTypeGeneric)->type();
  auto const arrType = topC(2, DataTypeGeneric)->type();

  if (!(arrType <= Type::Arr)) {
    // raise fatal
    emitInterpOne(Type::Cell, 3);
    return;
  }

  if (keyType <= Type::Null) {
    auto const def = popC(DataTypeGeneric); // def is just pushed back on the
                                            // stack
    auto const key = popC();
    auto const arr = popC();

    // if the key is null it will not be found so just return the default
    push(def);
    gen(DecRef, arr);
    gen(DecRef, key);
    return;
  }
  if (!(keyType <= Type::Int || keyType <= Type::Str)) {
    emitInterpOne(Type::Cell, 3);
    return;
  }

  auto const def = popC(DataTypeGeneric); // a helper will decref it but the
                                          // translated code doesn't care about
                                          // the type
  auto const key = popC();
  auto const arr = popC();

  push(gen(ArrayIdx, arr, key, def));
  gen(DecRef, arr);
  gen(DecRef, key);
  gen(DecRef, def);
}

void HhbcTranslator::emitSqrt() {
  auto const srcType = topC()->type();
  if (srcType <= Type::Int) {
    auto const src = gen(ConvIntToDbl, popC());
    push(gen(Sqrt, src));
    return;
  }

  if (srcType <= Type::Dbl) {
    auto const src = popC();
    push(gen(Sqrt, src));
    return;
  }

  emitInterpOne(Type::UncountedInit, 1);
}

void HhbcTranslator::emitAKExists() {
  SSATmp* arr = popC();
  SSATmp* key = popC();

  if (!arr->isA(Type::Arr) && !arr->isA(Type::Obj)) {
    PUNT(AKExists_badArray);
  }
  if (!key->isA(Type::Str) && !key->isA(Type::Int) && !key->isA(Type::Null)) {
    PUNT(AKExists_badKey);
  }

  push(gen(AKExists, arr, key));
  gen(DecRef, arr);
  gen(DecRef, key);
}

void HhbcTranslator::emitStrlen() {
  Type inType = topC()->type();

  if (inType <= Type::Str) {
    SSATmp* input = popC();
    if (input->isConst()) {
      // static string; fold its strlen operation
      push(cns(input->strVal()->size()));
    } else {
      push(gen(LdRaw, RawMemData{RawMemData::StrLen}, input));
      gen(DecRef, input);
    }
  } else if (inType <= Type::Null) {
    popC();
    push(cns(0));
  } else if (inType <= Type::Bool) {
    // strlen(true) == 1, strlen(false) == 0.
    push(gen(ConvBoolToInt, popC()));
  } else {
    emitInterpOne(Type::Int | Type::InitNull, 1);
  }
}

void HhbcTranslator::emitFloor() {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Floor);
  }

  auto catchBlock = makeCatch();
  auto val    = popC();
  auto dblVal = gen(ConvCellToDbl, catchBlock, val);
  gen(DecRef, val);
  push(gen(Floor, dblVal));
}

void HhbcTranslator::emitCeil() {
  // need SSE 4.1 support to use roundsd
  if (!folly::CpuId().sse41()) {
    PUNT(Ceil);
  }

  auto catchBlock = makeCatch();
  auto val = popC();
  auto dblVal = gen(ConvCellToDbl, catchBlock, val);
  gen(DecRef, val);
  push(gen(Ceil, dblVal));
}

void HhbcTranslator::emitFCallBuiltin(uint32_t numArgs,
                                      uint32_t numNonDefault,
                                      int32_t funcId,
                                      bool destroyLocals) {
  const NamedEntity* ne = lookupNamedEntityId(funcId);
  const Func* callee = Unit::lookupFunc(ne);

  callee->validate();

  if (optimizedFCallBuiltin(callee, numArgs, numNonDefault)) return;

  emitBuiltinCall(callee,
                  numArgs,
                  numNonDefault,
                  nullptr,  /* no this */
                  false,    /* not inlining */
                  false,    /* not inlining constructor */
                  destroyLocals,
                  [&](uint32_t) { return popC(); });
}

void HhbcTranslator::emitNativeImpl() {
  if (isInlining()) return emitNativeImplInlined();

  gen(NativeImpl, m_irb->fp(), m_irb->sp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

//////////////////////////////////////////////////////////////////////

}}
