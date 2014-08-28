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

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

BuiltinFunctionMap s_builtinFunctions;
ConstantMap s_constant_map;
ClassConstantMapMap s_class_constant_map;

static size_t numGPRegArgs() {
#ifdef __AARCH64EL__
  return 8; // r0-r7
#else // amd64
  if (UNLIKELY(RuntimeOption::EvalSimulateARM)) {
    return 8;
  }
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

inline bool isRefType(DataType dt) {
  return (dt != KindOfNull)  && (dt != KindOfBoolean) &&
         (dt != KindOfInt64) && (dt != KindOfDouble);
}

inline void* retPtrArg(DataType retType, TypedValue &ret) {
  if (retType == KindOfUnknown) {
    return &ret;
  }
  if (isRefType(retType)) {
    return &ret.m_data;
  }
  return nullptr;
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
template<bool variadic>
static void populateArgs(const Func* func,
                         TypedValue* args, const int numArgs,
                         int64_t* GP_args, int& GP_count,
                         double* SIMD_args, int& SIMD_count) {
  auto numGP = numGPRegArgs();
  int64_t tmp[kMaxBuiltinArgs];
  int ntmp = 0;

  for (size_t i = 0; i < numArgs; ++i) {
    DataType type;
    if (variadic) {
      const auto pi = func->params()[i];
      type = pi.isVariadic() ? KindOfArray : pi.builtinType;
    } else {
      type = func->params()[i].builtinType;
    }
    if (type == KindOfDouble) {
      if (SIMD_count < kNumSIMDRegs) {
        SIMD_args[SIMD_count++] = args[-i].m_data.dbl;
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
      assert((GP_count + 1) < kMaxBuiltinArgs);
      if (type == KindOfUnknown) {
        GP_args[GP_count++] = (int64_t)(args - i);
      } else if (isRefType(type)) {
        GP_args[GP_count++] = (int64_t)&args[-i].m_data;
      } else {
        GP_args[GP_count++] = args[-i].m_data.num;
      }
      if ((GP_count == numGP) && ntmp) {
        // GP regs are now full, bring tmp back to fill the initial stack
        assert((GP_count + ntmp) <= kMaxBuiltinArgs);
        memcpy(GP_args + GP_count, tmp, ntmp * sizeof(int64_t));
        GP_count += ntmp;
        ntmp = 0;
      }
    }
  }
  if (ntmp) {
    assert((GP_count + ntmp) <= kMaxBuiltinArgs);
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
template<bool variadic>
static void populateArgsNoDoubles(const Func* func,
                                  TypedValue* args, int numArgs,
                                  int64_t* GP_args, int& GP_count) {
  if (variadic) --numArgs;
  assert(numArgs >= 0);
  for (int i = 0; i < numArgs; ++i) {
    auto dt = func->params()[i].builtinType;
    assert(dt != KindOfDouble);
    if (dt == KindOfUnknown) {
      GP_args[GP_count++] = (int64_t)(args - i);
    } else if (isRefType(dt)) {
      GP_args[GP_count++] = (int64_t)&(args[-i].m_data);
    } else {
      GP_args[GP_count++] = args[-i].m_data.num;
    }
  }
  if (variadic) {
    GP_args[GP_count++] = (int64_t)&(args[-numArgs].m_data);
  }
}

template<bool usesDoubles, bool variadic>
void callFunc(const Func* func, void *ctx,
              TypedValue *args, TypedValue& ret) {
  assert(variadic == func->hasVariadicCaptureParam());
  int64_t GP_args[kMaxBuiltinArgs];
  double SIMD_args[kNumSIMDRegs];
  int GP_count = 0, SIMD_count = 0;
  const auto numArgs = func->numParams();
  ret.m_type = func->returnType();
  if (auto retArg = retPtrArg(ret.m_type, ret)) {
    GP_args[GP_count++] = (int64_t)retArg;
  }
  if (ctx) {
    GP_args[GP_count++] = (int64_t)ctx;
  }
  if (usesDoubles) {
    populateArgs<variadic>(func, args, numArgs,
                           GP_args, GP_count, SIMD_args, SIMD_count);
  } else {
    populateArgsNoDoubles<variadic>(func, args, numArgs, GP_args, GP_count);
  }

  BuiltinFunction f = func->nativeFuncPtr();
  switch (ret.m_type) {
    case KindOfUnknown:
       callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count);
       if (ret.m_type == KindOfUninit) {
         ret.m_type = KindOfNull;
       }
       return;
    case KindOfNull:
    case KindOfBoolean:
      ret.m_data.num =
        callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count) & 1;
      return;
    case KindOfInt64:
      ret.m_data.num =
        callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count);
      return;
    case KindOfDouble:
      ret.m_data.dbl =
        callFuncDoubleImpl(f, GP_args, GP_count, SIMD_args, SIMD_count);
      return;
    default:
      assert(isRefType(ret.m_type));
      callFuncInt64Impl(f, GP_args, GP_count, SIMD_args, SIMD_count);
      if (ret.m_data.num == 0) {
        ret.m_type = KindOfNull;
      }
      return;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////////////

bool coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func) {
  assert(numArgs == func->numParams());

  bool paramCoerceMode = func->isParamCoerceMode();

  for (int32_t i = 0; (i < numNonDefault) && (i < numArgs); i++) {
    const Func::ParamInfo& pi = func->params()[i];

#define COERCE_OR_CAST(kind, warn_kind)                 \
  if (paramCoerceMode) {                                \
    if (!tvCoerceParamTo##kind##InPlace(&args[-i])) {   \
      raise_param_type_warning(                         \
        func->name()->data(),                           \
        i+1,                                            \
        KindOf##warn_kind,                              \
        args[-i].m_type                                 \
      );                                                \
      return false;                                     \
    }                                                   \
  } else {                                              \
    tvCastTo##kind##InPlace(&args[-i]);                 \
  }

#define CASE(kind)                                      \
  case KindOf##kind:                                    \
    COERCE_OR_CAST(kind, kind)                          \
    break; /* end of case */

    switch (pi.builtinType) {
      CASE(Boolean)
      CASE(Int64)
      CASE(Double)
      CASE(String)
      CASE(Array)
      CASE(Resource)
      case KindOfObject: {
        auto mpi = func->methInfo() ? func->methInfo()->parameters[i] : nullptr;
        if (pi.hasDefaultValue() || (mpi && mpi->valueLen > 0)) {
          COERCE_OR_CAST(NullableObject, Object);
        } else {
          COERCE_OR_CAST(Object, Object);
        }
        break;
      }
      case KindOfUnknown:
        break;
      default:
        not_reached();
    }

#undef CASE
#undef COERCE_OR_CAST

  }
  return true;
}

static inline int32_t minNumArgs(ActRec *ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  int32_t num = numArgs;
  const Func::ParamInfoVec& paramInfo = func->params();
  while (num &&
         (paramInfo[num-1].funcletOff != InvalidAbsoluteOffset)) {
    --num;
  }
  return num;
}

static const StringData* getInvokeName(ActRec *ar) {
  if (ar->hasInvName()) {
    return ar->getInvName();
  }
  auto func = ar->m_func;
  auto cls = func->cls();
  if (!cls) {
    return func->name();
  }
  String clsname(const_cast<StringData*>(cls->name()));
  String funcname(const_cast<StringData*>(func->name()));
  return makeStaticString(clsname + "::" + funcname);
}

template<bool variadic>
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
  } else if (!variadic && (numNonDefault > numArgs)) {
    // Too many arguments passed, raise a warning ourselves this time
    throw_wrong_arguments_nr(getInvokeName(ar)->data(),
      numNonDefault, minNumArgs(ar), numArgs, 1);
    return false;
  }
  // Looks good
  return true;
}

template<bool usesDoubles, bool variadic>
TypedValue* functionWrapper(ActRec* ar) {
  assert(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  assert(variadic == func->hasVariadicCaptureParam());
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (((numNonDefault == numArgs) ||
       (nativeWrapperCheckArgs<variadic>(ar))) &&
      (coerceFCallArgs(args, numArgs, numNonDefault, func))) {
    callFunc<usesDoubles, variadic>(func, nullptr, args, rv);
  } else if (func->attrs() & AttrParamCoerceModeFalse) {
    rv.m_type = KindOfBoolean;
    rv.m_data.num = 0;
  }

  assert(rv.m_type != KindOfUninit);
  frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  tvCopy(rv, ar->m_r);
  return &ar->m_r;
}

template<bool usesDoubles, bool variadic>
TypedValue* methodWrapper(ActRec* ar) {
  assert(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  bool isStatic = func->isStatic();
  assert(variadic == func->hasVariadicCaptureParam());
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (((numNonDefault == numArgs) ||
       (nativeWrapperCheckArgs<variadic>(ar))) &&
      (coerceFCallArgs(args, numArgs, numNonDefault, func))) {
    // Prepend a context arg for methods
    // KindOfClass when it's being called statically Foo::bar()
    // KindOfObject when it's being called on an instance $foo->bar()
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

    callFunc<usesDoubles, variadic>(func, ctx, args, rv);
  } else if (func->attrs() & AttrParamCoerceModeFalse) {
    rv.m_type = KindOfBoolean;
    rv.m_data.num = 0;
  }

  assert(rv.m_type != KindOfUninit);
  if (isStatic) {
    frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  } else {
    frame_free_locals_inl(ar, func->numLocals(), &rv);
  }
  tvCopy(rv, ar->m_r);
  return &ar->m_r;
}

BuiltinFunction getWrapper(bool method, bool usesDoubles, bool variadic) {
  if (method) {
    if ( usesDoubles &&  variadic) return methodWrapper<true,true>;
    if ( usesDoubles && !variadic) return methodWrapper<true,false>;
    if (!usesDoubles &&  variadic) return methodWrapper<false,true>;
    if (!usesDoubles && !variadic) return methodWrapper<false,false>;
  } else {
    if ( usesDoubles &&  variadic) return functionWrapper<true,true>;
    if ( usesDoubles && !variadic) return functionWrapper<true,false>;
    if (!usesDoubles &&  variadic) return functionWrapper<false,true>;
    if (!usesDoubles && !variadic) return functionWrapper<false,false>;
  }
  not_reached();
  return nullptr;
}

TypedValue* unimplementedWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto cls = func->cls();
  ar->m_r.m_type = KindOfNull;
  if (cls) {
    raise_error("Call to unimplemented native method %s::%s()",
                cls->name()->data(), func->name()->data());
    if (func->isStatic()) {
      frame_free_locals_no_this_inl(ar, func->numParams(), &ar->m_r);
    } else {
      frame_free_locals_inl(ar, func->numParams(), &ar->m_r);
    }
  } else {
    raise_error("Call to unimplemented native function %s()",
                func->name()->data());
    frame_free_locals_no_this_inl(ar, func->numParams(), &ar->m_r);
  }
  return &ar->m_r;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
