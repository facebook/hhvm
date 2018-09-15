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

#include "hphp/runtime/vm/native.h"

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

FuncTable s_systemNativeFuncs;
const FuncTable s_noNativeFuncs; // always empty
ConstantMap s_constant_map;
ClassConstantMapMap s_class_constant_map;

static size_t numGPRegArgs() {
#ifdef __aarch64__
  return 8; // r0-r7
#elif defined(__powerpc64__)
  return 31;
#else // amd64
  return 6; // rdi, rsi, rdx, rcx, r8, r9
#endif
}

// Note: This number should generally not be modified
// as it depends on the CPU's ABI.
// If an update is needed, however, update and run
// make_native-func-caller.php as well
const size_t kNumSIMDRegs = 8;

/////////////////////////////////////////////////////////////////////////////
#include "hphp/runtime/vm/native-func-caller.h"

static void nativeArgHelper(const Func* func, int i,
                            const MaybeDataType& type,
                            TypedValue& arg,
                            int64_t* GP_args, int& GP_count) {
  auto val = arg.m_data.num;
  if (!type) {
    if (func->byRef(i)) {
      if (!isRefType(arg.m_type)) {
        // For OutputArgs, if the param is not a KindOfRef,
        // we give it a nullptr
        val = 0;
      }
    } else {
      GP_args[GP_count++] = val;
      assertx((GP_count + 1) < kMaxBuiltinArgs);
      val = static_cast<data_type_t>(arg.m_type);
    }
  }
  GP_args[GP_count++] = val;
}

/* Shuffle args into two vectors.
 *
 * SIMD_args contains at most 8 elements for the first 8 double args in the
 * call which will end up in xmm0-xmm7 (or v0-v7)
 *
 * GP_args contains all remaining args optionally with padding to ensure the
 * GP regs only contain integer arguments (when there are less than
 * numGPRegArgs INT args)
 */
static void populateArgs(const Func* func,
                         TypedValue* args, const int numArgs,
                         int64_t* GP_args, int& GP_count,
                         double* SIMD_args, int& SIMD_count) {
  auto numGP = numGPRegArgs();
  int64_t tmp[kMaxBuiltinArgs];
  int ntmp = 0;

  for (int i = 0; i < numArgs; ++i) {
    const auto& pi = func->params()[i];
    MaybeDataType type = pi.builtinType;
    if (type == KindOfDouble) {
      if (SIMD_count < kNumSIMDRegs) {
        SIMD_args[SIMD_count++] = args[-i].m_data.dbl;
#if defined(__powerpc64__)
        // According with ABI, the GP index must be incremented after
        // a floating point function argument
        if (GP_count < numGP) GP_args[GP_count++] = 0;
#endif
      } else if (GP_count < numGP) {
        // We have enough double args to hit the stack
        // but we haven't finished filling the GP regs yet.
        // Stack these in tmp (autoboxed to int64_t)
        // until we fill the GP regs, or we run out of args
        // (in which case we'll pad them).
        tmp[ntmp++] = args[-i].m_data.num;
      } else {
        // Additional SIMD args wind up on the stack
        // and can autobox with integer types
        GP_args[GP_count++] = args[-i].m_data.num;
      }
    } else {
      assertx((GP_count + 1) < kMaxBuiltinArgs);
      if (pi.nativeArg) {
        nativeArgHelper(func, i, type, args[-i], GP_args, GP_count);
      } else if (!type) {
        GP_args[GP_count++] = (int64_t)(args - i);
      } else if (isBuiltinByRef(type)) {
        GP_args[GP_count++] = (int64_t)&args[-i].m_data;
      } else {
        GP_args[GP_count++] = args[-i].m_data.num;
      }
      if ((GP_count == numGP) && ntmp) {
        // GP regs are now full, bring tmp back to fill the initial stack
        assertx((GP_count + ntmp) <= kMaxBuiltinArgs);
        memcpy(GP_args + GP_count, tmp, ntmp * sizeof(int64_t));
        GP_count += ntmp;
        ntmp = 0;
      }
    }
  }
  if (ntmp) {
    assertx((GP_count + ntmp) <= kMaxBuiltinArgs);
    // We had more than kNumSIMDRegs doubles,
    // but less than numGPRegArgs INTs.
    // Push out the count and leave garbage behind.
    if (GP_count < numGP) {
      GP_count = numGP;
    }
    memcpy(GP_args + GP_count, tmp, ntmp * sizeof(int64_t));
    GP_count += ntmp;
  }
}

/* A much simpler version of the above specialized for GP-arg-only methods */
static void populateArgsNoDoubles(const Func* func,
                                  TypedValue* args, int numArgs,
                                  int64_t* GP_args, int& GP_count) {
  assertx(numArgs >= 0);
  for (int i = 0; i < numArgs; ++i) {
    auto const& pi = func->params()[i];
    auto dt = pi.builtinType;
    assertx(dt != KindOfDouble);
    if (pi.nativeArg) {
      nativeArgHelper(func, i, dt, args[-i], GP_args, GP_count);
    } else if (!dt) {
      GP_args[GP_count++] = (int64_t)(args - i);
    } else if (isBuiltinByRef(dt)) {
      GP_args[GP_count++] = (int64_t)&(args[-i].m_data);
    } else {
      GP_args[GP_count++] = args[-i].m_data.num;
    }
  }
}

template<bool usesDoubles>
void callFunc(const Func* func, void *ctx,
              TypedValue *args, int32_t numNonDefault,
              TypedValue& ret) {
  int64_t GP_args[kMaxBuiltinArgs];
  double SIMD_args[kNumSIMDRegs];
  int GP_count = 0, SIMD_count = 0;

  auto const numArgs = func->numParams();
  auto retType = func->hniReturnType();

  if (ctx) {
    GP_args[GP_count++] = (int64_t)ctx;
  }

  if (func->takesNumArgs()) {
    GP_args[GP_count++] = (int64_t)numNonDefault;
  }

  if (usesDoubles) {
    populateArgs(func, args, numArgs,
                           GP_args, GP_count, SIMD_args, SIMD_count);
  } else {
    populateArgsNoDoubles(func, args, numArgs, GP_args, GP_count);
  }

  auto const f = func->nativeFuncPtr();

  if (!retType) {
    // A folly::none return signifies Variant.
    if (func->isReturnByValue()) {
      ret = callFuncTVImpl(f, GP_args, GP_count, SIMD_args, SIMD_count);
    } else {
      new (&ret) Variant(callFuncIndirectImpl<Variant>(f, GP_args, GP_count,
                                                       SIMD_args, SIMD_count));
      if (ret.m_type == KindOfUninit) {
        ret.m_type = KindOfNull;
      }
    }
    return;
  }

  ret.m_type = *retType;

  switch (*retType) {
    case KindOfNull:
    case KindOfBoolean:
      ret.m_data.num =
        callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count) & 1;
      return;

    case KindOfFunc:
    case KindOfClass:
    case KindOfInt64:
      ret.m_data.num =
        callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count);
      return;

    case KindOfDouble:
      ret.m_data.dbl =
        callFuncDoubleImpl(f, GP_args, GP_count, SIMD_args, SIMD_count);
      return;

    case KindOfPersistentString:
    case KindOfString:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentShape:
    case KindOfShape:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef: {
      assertx(isBuiltinByRef(ret.m_type));
      if (func->isReturnByValue()) {
        auto val = callFuncInt64Impl(f, GP_args, GP_count, SIMD_args,
                                     SIMD_count);
        ret.m_data.num = val;
      } else {
        using T = req::ptr<StringData>;
        new (&ret.m_data) T(callFuncIndirectImpl<T>(f, GP_args, GP_count,
                                                    SIMD_args, SIMD_count));
      }
      if (ret.m_data.num == 0) {
        ret.m_type = KindOfNull;
      }
      return;
    }

    case KindOfUninit:
      break;
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////////////

#define COERCE_OR_CAST(kind, warn_kind)                         \
  if (paramCoerceMode) {                                        \
    auto ty = args[-i].m_type;                                  \
    if (!tvCoerceParamTo##kind##InPlace(&args[-i],              \
                                        func->isBuiltin())) {   \
      raise_param_type_warning(                                 \
        func->displayName()->data(),                            \
        i+1,                                                    \
        KindOf##warn_kind,                                      \
        args[-i].m_type                                         \
      );                                                        \
      return false;                                             \
    } else {                                                    \
      if (RuntimeOption::EvalWarnOnCoerceBuiltinParams &&       \
          !equivDataTypes(ty, KindOf##warn_kind) &&             \
          args[-i].m_type != KindOfNull) {                      \
        raise_warning(                                          \
          "Argument %i of type %s was passed to %s, "           \
          "it was coerced to %s",                               \
          i + 1,                                                \
          getDataTypeString(ty).data(),                         \
          func->fullDisplayName()->data(),                      \
          getDataTypeString(KindOf##warn_kind).data()           \
        );                                                      \
      }                                                         \
    }                                                           \
  } else {                                                      \
    tvCastTo##kind##InPlace(&args[-i]);                         \
  }

#define CASE(kind)                                      \
  case KindOf##kind:                                    \
    COERCE_OR_CAST(kind, kind)                          \
    break; /* end of case */

bool coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func) {
  assertx(numArgs == func->numParams());

  bool paramCoerceMode = func->isParamCoerceMode();

  for (int32_t i = 0; (i < numNonDefault) && (i < numArgs); i++) {
    const Func::ParamInfo& pi = func->params()[i];

    auto tc = pi.typeConstraint;
    auto targetType = pi.builtinType;
    if (tc.isNullable() && !func->byRef(i)) {
      if (isNullType(args[-i].m_type)) {
        // No need to coerce when passed a null for a nullable type
        continue;
      }
      // Arg isn't null, so treat it like the underlying type for coersion
      // purposes.  The ABI-passed type will still be mixed/Variant.
      targetType = tc.underlyingDataType();
    }

    // Skip tvCoerceParamTo*() call if we're already the right type, or if its a
    // Variant.
    if (!targetType || equivDataTypes(args[-i].m_type, *targetType)) {
      auto const c = &args[-i];
      auto const raise = [&] {
        if (LIKELY(!RuntimeOption::EvalHackArrCompatTypeHintNotices)) {
          return false;
        }
        if (!tc.isArray()) return false;
        if (!isArrayOrShapeType(c->m_type)) return false;
        if (tc.isVArray()) {
          return !c->m_data.parr->isVArray();
        } else if (tc.isDArray()) {
          return !c->m_data.parr->isDArray();
        } else if (tc.isVArrayOrDArray()) {
          return c->m_data.parr->isNotDVArray();
        } else {
          return !c->m_data.parr->isNotDVArray();
        }
      }();
      if (raise) {
        raise_hackarr_compat_type_hint_param_notice(
          func,
          c->m_data.parr,
          tc.type(),
          i
        );
      }
      continue;
    }

    if (RuntimeOption::PHP7_ScalarTypes && call_uses_strict_types(func)) {
      tc.verifyParam(&args[-i], func, i);
      return true;
    }

    switch (*targetType) {
      CASE(Boolean)
      CASE(Int64)
      CASE(Double)
      CASE(String)
      CASE(Vec)
      CASE(Dict)
      CASE(Keyset)
      CASE(Shape)
      CASE(Array)
      CASE(Resource)

      case KindOfObject: {
        if (pi.hasDefaultValue()) {
          COERCE_OR_CAST(NullableObject, Object);
        } else {
          COERCE_OR_CAST(Object, Object);
        }
        break;
      }

      case KindOfUninit:
      case KindOfNull:
      case KindOfPersistentString:
      case KindOfPersistentVec:
      case KindOfPersistentDict:
      case KindOfPersistentKeyset:
      case KindOfPersistentShape:
      case KindOfPersistentArray:
      case KindOfRef:
      case KindOfFunc:
      case KindOfClass:
        not_reached();
    }
  }

  return true;
}

#undef CASE
#undef COERCE_OR_CAST

static inline int32_t minNumArgs(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numNonVariadicParams();
  int32_t num = numArgs;
  const Func::ParamInfoVec& paramInfo = func->params();
  while (num &&
         (paramInfo[num-1].funcletOff != InvalidAbsoluteOffset)) {
    --num;
  }
  return num;
}

const StringData* getInvokeName(ActRec* ar) {
  if (ar->magicDispatch()) {
    return ar->getInvName();
  }
  return ar->func()->fullDisplayName();
}

bool nativeWrapperCheckArgs(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numNonVariadicParams();
  auto numNonDefault = ar->numArgs();

  if (numNonDefault < numArgs) {
    const Func::ParamInfoVec& paramInfo = func->params();
    for (auto i = numNonDefault; i < numArgs; ++i) {
      if (InvalidAbsoluteOffset == paramInfo[i].funcletOff) {
        // There's at least one non-default param which wasn't passed
        throw_wrong_arguments_nr(getInvokeName(ar)->data(),
              numNonDefault, minNumArgs(ar), numArgs, 1);
        return false;
      }
    }
  } else if (numNonDefault > numArgs && !func->hasVariadicCaptureParam()) {
    // Too many arguments passed, raise a warning ourselves this time
    throw_wrong_arguments_nr(getInvokeName(ar)->data(),
      numNonDefault, minNumArgs(ar), numArgs, 1);
    return false;
  }
  // Looks good
  return true;
}

template<bool usesDoubles>
TypedValue* functionWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (((numNonDefault == numArgs) ||
       (nativeWrapperCheckArgs(ar))) &&
      (coerceFCallArgs(args, numArgs, numNonDefault, func))) {
    callFunc<usesDoubles>(func, nullptr, args, numNonDefault, rv);
  } else if (func->attrs() & AttrParamCoerceModeFalse) {
    rv.m_type = KindOfBoolean;
    rv.m_data.num = 0;
  }

  assertx(rv.m_type != KindOfUninit);
  frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  tvCopy(rv, *ar->retSlot());
  return ar->retSlot();
}

template<bool usesDoubles>
TypedValue* methodWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  bool isStatic = func->isStatic();
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (((numNonDefault == numArgs) ||
       (nativeWrapperCheckArgs(ar))) &&
      (coerceFCallArgs(args, numArgs, numNonDefault, func))) {
    // Prepend a context arg for methods
    // Class when it's being called statically Foo::bar()
    // Object when it's being called on an instance $foo->bar()
    void* ctx;  // ObjectData* or Class*
    if (ar->hasThis()) {
      if (isStatic) {
        throw_instance_method_fatal(getInvokeName(ar)->data());
      }
      ctx = ar->getThis();
    } else {
      if (!isStatic) {
        throw_instance_method_fatal(getInvokeName(ar)->data());
      }
      ctx = ar->getClass();
    }

    callFunc<usesDoubles>(func, ctx, args, numNonDefault, rv);
  } else if (func->attrs() & AttrParamCoerceModeFalse) {
    rv.m_type = KindOfBoolean;
    rv.m_data.num = 0;
  }

  assertx(rv.m_type != KindOfUninit);
  if (isStatic) {
    frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  } else {
    frame_free_locals_inl(ar, func->numLocals(), &rv);
  }
  tvCopy(rv, *ar->retSlot());
  return ar->retSlot();
}

TypedValue* unimplementedWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto cls = func->cls();
  if (cls) {
    raise_error("Call to unimplemented native method %s::%s()",
                cls->name()->data(), func->name()->data());
    tvWriteNull(*ar->retSlot());
    if (func->isStatic()) {
      frame_free_locals_no_this_inl(ar, func->numParams(), ar->retSlot());
    } else {
      frame_free_locals_inl(ar, func->numParams(), ar->retSlot());
    }
  } else {
    raise_error("Call to unimplemented native function %s()",
                func->displayName()->data());
    tvWriteNull(*ar->retSlot());
    frame_free_locals_no_this_inl(ar, func->numParams(), ar->retSlot());
  }
  return ar->retSlot();
}

void getFunctionPointers(const NativeFunctionInfo& info, int nativeAttrs,
                         ArFunction& bif, NativeFunction& nif) {
  nif = info.ptr;
  if (!nif) {
    bif = unimplementedWrapper;
    return;
  }
  if (nativeAttrs & AttrActRec) {
    // NativeFunction with the ArFunction signature
    bif = reinterpret_cast<ArFunction>(nif);
    nif = nullptr;
    return;
  }

  bool usesDoubles = false;
  for (auto const argType : info.sig.args) {
    if (argType == NativeSig::Type::Double) {
      usesDoubles = true;
      break;
    }
  }

  bool isMethod = info.sig.args.size() &&
      ((info.sig.args[0] == NativeSig::Type::This) ||
       (info.sig.args[0] == NativeSig::Type::Class));

  if (isMethod) {
    if (usesDoubles) {
      bif = methodWrapper<true>;
    } else {
      bif = methodWrapper<false>;
    }
  } else {
    if (usesDoubles) {
      bif = functionWrapper<true>;
    } else {
      bif = functionWrapper<false>;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

static bool tcCheckNative(const TypeConstraint& tc, const NativeSig::Type ty) {
  using T = NativeSig::Type;

  if (!tc.hasConstraint() || tc.isNullable() || tc.isCallable() ||
      tc.isArrayKey() || tc.isNumber() || tc.isVecOrDict() ||
      tc.isVArrayOrDArray() || tc.isArrayLike()) {
    return ty == T::Mixed || ty == T::MixedTV;
  }

  if (!tc.underlyingDataType()) {
    return false;
  }

  switch (*tc.underlyingDataType()) {
    case KindOfDouble:       return ty == T::Double;
    case KindOfBoolean:      return ty == T::Bool;
    case KindOfObject:       return ty == T::Object   || ty == T::ObjectArg;
    case KindOfPersistentString:
    case KindOfString:       return ty == T::String   || ty == T::StringArg;
    case KindOfPersistentVec:
    case KindOfVec:          return ty == T::Array    || ty == T::ArrayArg;
    case KindOfPersistentDict:
    case KindOfDict:         return ty == T::Array    || ty == T::ArrayArg;
    case KindOfPersistentKeyset:
    case KindOfKeyset:       return ty == T::Array    || ty == T::ArrayArg;
    case KindOfPersistentShape:
    case KindOfShape:        return ty == T::Array    || ty == T::ArrayArg;
    case KindOfPersistentArray:
    case KindOfArray:        return ty == T::Array    || ty == T::ArrayArg;
    case KindOfResource:     return ty == T::Resource || ty == T::ResourceArg;
    case KindOfUninit:
    case KindOfNull:         return ty == T::Void;
    case KindOfRef:          return ty == T::Mixed    || ty == T::OutputArg;
    case KindOfInt64:        return ty == T::Int64    || ty == T::Int32;
    case KindOfFunc:         return ty == T::Func;
    case KindOfClass:        return ty == T::Class;
  }
  not_reached();
}

const char* kInvalidReturnTypeMessage = "Invalid return type detected";
const char* kInvalidArgTypeMessage = "Invalid argument type detected";
const char* kInvalidArgCountMessage = "Invalid argument count detected";
const char* kInvalidNumArgsMessage =
  "\"NumArgs\" builtins must take an int64_t as their first declared argument";
const char* kNeedStaticContextMessage =
  "Static class functions must take a Class* as their first argument";
const char* kNeedObjectContextMessage =
  "Instance methods must take an ObjectData* as their first argument";
const char* kInvalidActRecFuncMessage =
  "Functions declared as ActRec must return a TypedValue* and take an ActRec* "
  "as their sole argument";

static const StaticString
  s_native("__Native"),
  s_actrec("ActRec");

const char* checkTypeFunc(const NativeSig& sig,
                          const TypeConstraint& retType,
                          const FuncEmitter* func) {
  using T = NativeSig::Type;

  auto dummy = HPHP::AttrNone;
  auto nativeAttributes = func->parseNativeAttributes(dummy);

  if (nativeAttributes & Native::AttrActRec) {
    return
      sig.ret == T::ARReturn &&
      sig.args.size() == 1 &&
      sig.args[0] == T::VarArgs
        ? nullptr
        : kInvalidActRecFuncMessage;
  }

  if (!tcCheckNative(retType, sig.ret)) return kInvalidReturnTypeMessage;

  auto argIt = sig.args.begin();
  auto endIt = sig.args.end();
  if (func->pce()) { // called from the verifier so m_cls is not set yet
    if (argIt == endIt) return kInvalidArgCountMessage;
    auto const ctxTy = *argIt++;
    if (func->attrs & HPHP::AttrStatic) {
      if (ctxTy != T::Class) return kNeedStaticContextMessage;
    } else {
      if (ctxTy != T::This) return kNeedObjectContextMessage;
    }
  }

  if (nativeAttributes & Native::AttrTakesNumArgs) {
    if (*argIt++ != T::Int64) return kInvalidNumArgsMessage;
  }

  for (auto const& pInfo : func->params) {
    if (argIt == endIt) return kInvalidArgCountMessage;

    auto const argTy = *argIt++;

    if (pInfo.byRef) {
      if (argTy != T::MixedRef &&
          argTy != T::OutputArg) {
        return kInvalidArgTypeMessage;
      }
      continue;
    }

    if (pInfo.variadic) {
      if (argTy != T::Array) return kInvalidArgTypeMessage;
      continue;
    }

    if (!tcCheckNative(pInfo.typeConstraint, argTy)) {
      return kInvalidArgTypeMessage;
    }
  }

  return argIt == endIt ? nullptr : kInvalidArgCountMessage;
}

String fullName(const StringData* fname, const StringData* cname,
                bool isStatic) {
  return {
    cname == nullptr ? String{const_cast<StringData*>(fname)} :
    (String{const_cast<StringData*>(cname)} +
      (isStatic ? "::" : "->") +
      String{const_cast<StringData*>(fname)})
  };
}

NativeFunctionInfo getNativeFunction(const FuncTable& nativeFuncs,
                                     const StringData* fname,
                                     const StringData* cname,
                                     bool isStatic) {
  auto const name = fullName(fname, cname, isStatic);
  if (auto info = nativeFuncs.get(name.get())) {
    return info;
  }
  return NativeFunctionInfo();
}

NativeFunctionInfo getNativeFunction(const FuncTable& nativeFuncs,
                                     const char* fname,
                                     const char* cname,
                                     bool isStatic) {
  return getNativeFunction(nativeFuncs,
                           makeStaticString(fname),
                           cname ? makeStaticString(cname) : nullptr,
                           isStatic);
}

void registerNativeFunc(Native::FuncTable& nativeFuncs,
                        const StringData* name,
                        const NativeFunctionInfo& info) {
  nativeFuncs.insert(name, info);
}

void FuncTable::insert(const StringData* name,
                       const NativeFunctionInfo& info) {
  assert(name->isStatic());
  DEBUG_ONLY auto it = m_infos.insert(std::make_pair(name, info));
  assert(it.second || it.first->second == info);
}

NativeFunctionInfo FuncTable::get(const StringData* name) const {
  auto const it = m_infos.find(name);
  if (it != m_infos.end()) return it->second;
  return NativeFunctionInfo();
}

void FuncTable::dump() const {
  for (auto e : m_infos) {
    fprintf(stderr, "%s\n", e.first->data());
  }
}

static std::string nativeTypeString(NativeSig::Type ty) {
  using T = NativeSig::Type;
  switch (ty) {
  case T::Int32:
  case T::Int64:      return "int";
  case T::Double:     return "double";
  case T::Bool:       return "bool";
  case T::Object:     return "object";
  case T::String:     return "string";
  case T::Array:      return "array";
  case T::Resource:   return "resource";
  case T::ObjectArg:  return "object";
  case T::StringArg:  return "string";
  case T::ArrayArg:   return "array";
  case T::ResourceArg:return "resource";
  case T::OutputArg:  return "mixed&";
  case T::Mixed:      return "mixed";
  case T::MixedTV:    return "mixed";
  case T::ARReturn:   return "[TypedValue*]";
  case T::MixedRef:   return "mixed&";
  case T::VarArgs:    return "...";
  case T::This:       return "this";
  case T::Class:      return "class";
  case T::Void:       return "void";
  case T::Func:       return "func";
  }
  not_reached();
}

std::string NativeSig::toString(const char* classname,
                                const char* fname) const {
  using T = NativeSig::Type;

  auto str   = folly::to<std::string>(nativeTypeString(ret), " ");
  auto argIt = args.begin();
  auto endIt = args.end();

  if (argIt != endIt) {
    if (classname) str += classname;
    if (*argIt == T::This) {
      str += "->";
      ++argIt;
    } else if (*argIt == T::Class) {
      str += "::";
      ++argIt;
    }
  }
  str += folly::to<std::string>(fname,
                                "(",
                                argIt != endIt ? nativeTypeString(*argIt++)
                                               : "void");

  for (;argIt != endIt; ++argIt) {
    str += folly::to<std::string>(", ", nativeTypeString(*argIt));
  }
  str += ")";

  return str;
}

/////////////////////////////////////////////////////////////////////////////

bool registerConstant(const StringData* cnsName,
                      ConstantCallback callback) {
  TypedValueAux tv;
  tv.m_type = KindOfUninit;
  tv.m_data.pref = reinterpret_cast<RefData*>(callback);
  tv.dynamic() = true;
  if (!Unit::defNativeConstantCallback(cnsName, tv)) {
    return false;
  }
  s_constant_map[cnsName] = tv;
  return true;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
