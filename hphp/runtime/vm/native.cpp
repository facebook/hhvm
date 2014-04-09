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

class NativeFuncCaller {
 public:
  NativeFuncCaller(const Func* f,
                   TypedValue* args, size_t numArgs,
                   TypedValue* ctx = nullptr,
                   void *retArg = nullptr) : m_func(f) {
    auto numGP = numGPRegArgs();
    int64_t tmp[kMaxBuiltinArgs];
    int ntmp = 0;

    // Prepend by-ref arg and/or context as needed
    if (retArg) {
      m_GP[m_GPcount++] = (int64_t)retArg;
    }
    if (ctx) {
      if (ctx->m_type == KindOfClass) {
        m_GP[m_GPcount++] = (int64_t)ctx->m_data.pcls;
      } else {
        assert(ctx->m_type == KindOfObject);
        m_GP[m_GPcount++] = (int64_t)&ctx->m_data;
      }
    }

    // Shuffle args into two vectors.
    //
    // m_SIMD contains at most 8 elements for the first 8 double args in the
    // call which will end up in xmm0-xmm7 (or v0-v7)
    //
    // m_GP contains all remaining args optionally with padding to ensure the
    // GP regs only contain integer arguments (when there are less than
    // numGPRegArgs INT args)
    for (size_t i = 0; i < numArgs; ++i) {
      auto type = m_func->params()[i].builtinType();
      if (type == KindOfDouble) {
        if (m_SIMDcount < kNumSIMDRegs) {
          m_SIMD[m_SIMDcount++] = args[-i].m_data.dbl;
        } else if (m_GPcount < numGP) {
          // We have enough double args to hit the stack
          // but we haven't finished filling the GP regs yet.
          // Stack these in tmp (autoboxed to int64_t)
          // until we fill the GP regs, or we run out of args
          // (in which case we'll pad them).
          tmp[ntmp++] = args[-i].m_data.num;
        } else {
          // Additional SIMD args wind up on the stack
          // and can autobox with integer types
          m_GP[m_GPcount++] = args[-i].m_data.num;
        }
      } else {
        assert((m_GPcount + 1) < kMaxBuiltinArgs);
        if (type == KindOfUnknown) {
          m_GP[m_GPcount++] = (int64_t)(args - i);
        } else if (IsRefType(type)) {
          m_GP[m_GPcount++] = (int64_t)&args[-i].m_data;
        } else {
          m_GP[m_GPcount++] = args[-i].m_data.num;
        }
        if ((m_GPcount == numGP) && ntmp) {
          // GP regs are now full, bring tmp back to fill the initial stack
          assert((m_GPcount + ntmp) <= kMaxBuiltinArgs);
          memcpy(m_GP + m_GPcount, tmp, ntmp * sizeof(int64_t));
          m_GPcount += ntmp;
          ntmp = 0;
        }
      }
    }
    if (ntmp) {
      assert((m_GPcount + ntmp) <= kMaxBuiltinArgs);
      // We had more than kNumSIMDRegs doubles,
      // but less than numGPRegArgs INTs.
      // Push out the count and leave garbage behind.
      if (m_GPcount < numGP) {
        m_GPcount = numGP;
      }
      memcpy(m_GP + m_GPcount, tmp, ntmp * sizeof(int64_t));
      m_GPcount += ntmp;
    }
  }

  int64_t callInt64();
  double callDouble();

  static bool IsRefType(DataType dt) {
    return (dt != KindOfNull)  && (dt != KindOfBoolean) &&
           (dt != KindOfInt64) && (dt != KindOfDouble);
  }

 private:
  size_t numSIMDargs() const { return m_SIMDcount; }
  size_t numGPargs()   const { return m_GPcount;   }

  double simd(size_t i) const {
    assert(i < m_SIMDcount);
    return m_SIMD[i];
  }
  int64_t gp(size_t i) const {
    assert(i < m_GPcount);
    return m_GP[i];
  }

  double m_SIMD[kNumSIMDRegs];
  int m_SIMDcount{0};
  int64_t m_GP[kMaxBuiltinArgs];
  int m_GPcount{0};
  const Func* m_func;
};

// Defines NativeFuncCaller::callInt64(), NativeFuncCaller::callDouble(),
#include "hphp/runtime/vm/native-func-caller.h"

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

    switch (pi.builtinType()) {
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

void callFunc(const Func* func, TypedValue *ctx,
              TypedValue* args, int32_t numArgs,
              TypedValue &ret) {
  ret.m_type = func->returnType();
  void *retArg = nullptr;
  if (ret.m_type == KindOfUnknown) {
    retArg = &ret;
  } else if (NativeFuncCaller::IsRefType(ret.m_type)) {
    retArg = &ret.m_data;
  }
  NativeFuncCaller caller(func, args, numArgs, ctx, retArg);
  switch (func->returnType()) {
    case KindOfNull:
    case KindOfBoolean:
      ret.m_data.num = caller.callInt64() & 1;
      break;
    case KindOfInt64:
      ret.m_data.num = caller.callInt64();
      break;
    case KindOfDouble:
      ret.m_data.dbl = caller.callDouble();
      break;
    case KindOfString:
    case KindOfStaticString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      caller.callInt64();
      if (ret.m_data.num == 0) {
        ret.m_type = KindOfNull;
      }
      break;
    case KindOfUnknown:
      caller.callInt64();
      if (ret.m_type == KindOfUninit) {
        ret.m_type = KindOfNull;
      }
    break;
  default:
    not_reached();
  }
}

static inline int32_t minNumArgs(ActRec *ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  int32_t num = numArgs;
  const Func::ParamInfoVec& paramInfo = func->params();
  while (num &&
         (paramInfo[num-1].funcletOff() != InvalidAbsoluteOffset)) {
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

static inline bool nativeWrapperCheckArgs(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();

  if (numNonDefault < numArgs) {
    const Func::ParamInfoVec& paramInfo = func->params();
    for (auto i = numNonDefault; i < numArgs; ++i) {
      if (InvalidAbsoluteOffset == paramInfo[i].funcletOff()) {
        // There's at least one non-default param which wasn't passed
        throw_wrong_arguments_nr(getInvokeName(ar)->data(),
              numNonDefault, minNumArgs(ar), numArgs, 1);
        return false;
      }
    }
  } else if (numNonDefault > numArgs) {
    // Too many arguments passed, raise a warning ourselves this time
    throw_wrong_arguments_nr(getInvokeName(ar)->data(),
      numNonDefault, minNumArgs(ar), numArgs, 1);
    return false;
  }
  // Looks good
  return true;
}

TypedValue* functionWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  assert(!func->hasVariadicCaptureParam());
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (LIKELY(numNonDefault == numArgs) ||
      LIKELY(nativeWrapperCheckArgs(ar))) {
    if (coerceFCallArgs(args, numArgs, numNonDefault, func)) {
      callFunc(func, nullptr, args, numArgs, rv);
    }
  }

  assert(rv.m_type != KindOfUninit);
  frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  tvCopy(rv, ar->m_r);
  return &ar->m_r;
}

TypedValue* methodWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  bool isStatic = func->isStatic();
  assert(!func->hasVariadicCaptureParam());
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (LIKELY(numNonDefault == numArgs) ||
      LIKELY(nativeWrapperCheckArgs(ar))) {
    if (coerceFCallArgs(args, numArgs, numNonDefault, func)) {

      // Prepend a context arg for methods
      // KindOfClass when it's being called statically Foo::bar()
      // KindOfObject when it's being called on an instance $foo->bar()
      TypedValue ctx;
      if (ar->hasThis()) {
        if (isStatic) {
          throw_instance_method_fatal(getInvokeName(ar)->data());
        }
        ctx.m_type = KindOfObject;
        ctx.m_data.pobj = ar->getThis();
      } else {
        if (!isStatic) {
          throw_instance_method_fatal(getInvokeName(ar)->data());
        }
        ctx.m_type = KindOfClass;
        ctx.m_data.pcls = const_cast<Class*>(ar->getClass());
      }

      callFunc(func, &ctx, args, numArgs, rv);
    }
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

TypedValue* unimplementedWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto cls = func->cls();
  if (cls) {
    raise_error("Call to unimplemented native method %s::%s()",
                cls->name()->data(), func->name()->data());
    if (func->isStatic()) {
      frame_free_locals_no_this_inl(ar, func->numParams(), nullptr);
    } else {
      frame_free_locals_inl(ar, func->numParams(), nullptr);
    }
  } else {
    raise_error("Call to unimplemented native function %s()",
                func->name()->data());
    frame_free_locals_no_this_inl(ar, func->numParams(), nullptr);
  }
  ar->m_r.m_type = KindOfNull;
  return &ar->m_r;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
