/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
ClassConstantMapMap s_class_constant_map;

/**
 * Native function caller
 * Remaps an array of TypedValue* (such as from an ActRec*)
 * into a call to a native (C++) function using type hinting
 * from HPHP::Func
 */

// Return a function pointer type for calling a builtin with a given
// return value and args.
template<class Ret, class... Args> struct NativeFunction {
  typedef Ret (*type)(Args...);
};

// Recursively pack all parameters up to call a native builtin.
template<class Ret, size_t NArgs, size_t CurArg> struct NativeFuncCaller {
  template<class... Args>
  static Ret call(const Func* func, TypedValue* tvs, Args... args) {
    typedef NativeFuncCaller<Ret,NArgs - 1,CurArg + 1> NextArgT;

    /* In order to avoid a combinatorial explosion of generated
     * function types, we bitwise-cast pointer types to uint64_t when
     * passing into the native function. The double parameter stays
     * put because it has a different ABI than uint64_t.
     */

    const DataType type = func->params()[CurArg].builtinType();
    if (type == KindOfDouble) {
      // pass TV.m_data.dbl by value with C++ calling convention for doubles
      return NextArgT::call(func, tvs - 1, args..., tvs->m_data.dbl);
    }
    uint64_t bitwiseArg /* = void */;
    if (type == KindOfInt64 || type == KindOfBoolean) {
      // pass TV.m_data.num of type int64_t by value
      bitwiseArg = tvs->m_data.num; // rely on implicit cast
    } else if (IS_STRING_TYPE(type) || type == KindOfArray
               || type == KindOfObject || type == KindOfResource) {
      // pass ptr to TV.m_data for String&, Array&, or Object&
      static_assert(sizeof(&tvs->m_data) == sizeof(uint64_t),
                    "This code assumes 64-bit pointers.");
      bitwiseArg = reinterpret_cast<uint64_t>(&tvs->m_data);
    } else {
      // final case is for passing full value as Variant&
      static_assert(sizeof(tvs) == sizeof(uint64_t),
                    "This code assumes 64-bit pointers.");
      bitwiseArg = reinterpret_cast<uint64_t>(tvs);
    }
    // Make the recursive call
    return NextArgT::call(func, tvs - 1, args..., bitwiseArg);
  }
};

template<class Ret, size_t CurArg> struct NativeFuncCaller<Ret,0,CurArg> {
  template<class... Args>
  static Ret call(const Func* f, TypedValue*, Args... args) {
    typedef typename NativeFunction<Ret,Args...>::type FuncType;
    return reinterpret_cast<FuncType>(f->nativeFuncPtr())(args...);
  }
};

#define NFC_CALL(numargs, rettype, func, ...) \
  switch (numargs) { \
    case 0: return NativeFuncCaller<rettype,0,0>::call(func, __VA_ARGS__); \
    case 1: return NativeFuncCaller<rettype,1,0>::call(func, __VA_ARGS__); \
    case 2: return NativeFuncCaller<rettype,2,0>::call(func, __VA_ARGS__); \
    case 3: return NativeFuncCaller<rettype,3,0>::call(func, __VA_ARGS__); \
    case 4: return NativeFuncCaller<rettype,4,0>::call(func, __VA_ARGS__); \
    case 5: return NativeFuncCaller<rettype,5,0>::call(func, __VA_ARGS__); \
    default: assert(false); \
  }

// Caller is expected to always pass a String or Object for ctx (or nullptr)
template<class Ret>
static Ret makeNativeCall(const Func* f, TypedValue* args, size_t numArgs,
                          TypedValue *ctx = nullptr) {
  static_assert(kMaxBuiltinArgs == 5,
                "makeNativeCall needs updates for kMaxBuiltinArgs");
  if (ctx == nullptr) {
    NFC_CALL(numArgs, Ret, f, args);
  } else if (ctx->m_type == KindOfClass) {
    NFC_CALL(numArgs, Ret, f, args, ctx->m_data.pcls);
  } else {
    assert(ctx->m_type == KindOfObject);
    NFC_CALL(numArgs, Ret, f, args, &ctx->m_data);
  }
  not_reached();
}

template<class Ret>
static int64_t makeNativeRefCall(const Func* f, Ret* ret,
                                 TypedValue* args, size_t numArgs,
                                 TypedValue* ctx) {
  static_assert(kMaxBuiltinArgs == 5,
                "makeNativeRefCall needs updates for kMaxBuiltinArgs");
  if (ctx == nullptr) {
    NFC_CALL(numArgs, int64_t, f, args, ret);
  } else if (ctx->m_type == KindOfClass) {
    NFC_CALL(numArgs, int64_t, f, args, ret, ctx->m_data.pcls);
  } else {
    assert(ctx->m_type == KindOfObject);
    NFC_CALL(numArgs, int64_t, f, args, ret, &ctx->m_data);
  }
  not_reached();
}
#undef NFC_CALL

//////////////////////////////////////////////////////////////////////////////

bool coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func) {
  assert(numArgs == func->numParams());

  // funcs without a methInfo struct are HNI (with a struct are IDL)
  // All HNI functions have ZPM enabled by default
  bool zendParamMode =
    !func->methInfo() || func->methInfo()->attribute &
    (ClassInfo::ZendParamModeNull | ClassInfo::ZendParamModeFalse);

  for (int32_t i = 0; (i < numNonDefault) && (i < numArgs); i++) {
    const Func::ParamInfo& pi = func->params()[i];

#define CASE(kind)                                      \
  case KindOf##kind:                                    \
    if (zendParamMode) {                                \
      if (!tvCoerceParamTo##kind##InPlace(&args[-i])) { \
        raise_param_type_warning(                       \
          func->name()->data(),                         \
          i+1,                                          \
          KindOf##kind,                                 \
          args[-i].m_type                               \
        );                                              \
        return false;                                   \
      }                                                 \
    } else {                                            \
      tvCastTo##kind##InPlace(&args[-i]);               \
    }                                                   \
    break; /* end of case */

    switch (pi.builtinType()) {
      CASE(Boolean)
      CASE(Int64)
      CASE(Double)
      CASE(String)
      CASE(Array)
      CASE(Object)
      CASE(Resource)
      case KindOfUnknown:
        break;
      default:
        not_reached();
    }

#undef CASE

  }
  return true;
}

void callFunc(const Func* func, TypedValue *ctx,
              TypedValue* args, int32_t numArgs,
              TypedValue &ret) {
  ret.m_type = func->returnType();
  switch (func->returnType()) {
  case KindOfBoolean:
    ret.m_data.num = makeNativeCall<bool>(func, args, numArgs, ctx);
    break;
  case KindOfNull:  /* void return type */
  case KindOfInt64:
    ret.m_data.num = makeNativeCall<int64_t>(func, args, numArgs, ctx);
    break;
  case KindOfDouble:
    ret.m_data.dbl = makeNativeCall<double>(func, args, numArgs, ctx);
    break;
  case KindOfString:
  case KindOfStaticString:
  case KindOfArray:
  case KindOfObject:
  case KindOfResource:
    makeNativeRefCall(func, &ret.m_data, args, numArgs, ctx);
    if (ret.m_data.num == 0) {
      ret.m_type = KindOfNull;
    }
    break;
  case KindOfUnknown:
    makeNativeRefCall(func, &ret, args, numArgs, ctx);
    if (ret.m_type == KindOfUninit) {
      ret.m_type = KindOfNull;
    }
    break;
  default:
    not_reached();
  }
}

static inline int32_t minNumArgs(ActRec *ar) {
  int32_t num = 0;
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  const Func::ParamInfoVec& paramInfo = func->params();
  while ((num < numArgs) &&
         (paramInfo[num].funcletOff() == InvalidAbsoluteOffset)) {
    ++num;
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
  String clsname(cls->name());
  String funcname(func->name());
  return makeStaticString(clsname + "::" + funcname);
}

static inline bool nativeWrapperCheckArgs(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();

  if (numNonDefault < numArgs) {
    const Func::ParamInfoVec& paramInfo = func->params();
    if (InvalidAbsoluteOffset == paramInfo[numNonDefault].funcletOff()) {
      // There's at least one non-default param which wasn't passed
      throw_wrong_arguments_nr(getInvokeName(ar)->data(),
            numNonDefault, minNumArgs(ar), numArgs, 1);
      return false;
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
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (LIKELY(numNonDefault == numArgs) ||
      LIKELY(nativeWrapperCheckArgs(ar))) {
    coerceFCallArgs(args, numArgs, numNonDefault, func);
    callFunc(func, nullptr, args, numArgs, rv);
  }

  frame_free_locals_no_this_inl(ar, func->numLocals());
  assert(rv.m_type != KindOfUninit);
  tvCopy(rv, ar->m_r);
  return &ar->m_r;
}

TypedValue* methodWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  bool isStatic = func->isStatic();
  TypedValue* args = ((TypedValue*)ar) - 1;
  TypedValue rv;
  rv.m_type = KindOfNull;

  if (LIKELY(numNonDefault == numArgs) ||
      LIKELY(nativeWrapperCheckArgs(ar))) {
    coerceFCallArgs(args, numArgs, numNonDefault, func);

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

  if (isStatic) {
    frame_free_locals_no_this_inl(ar, func->numLocals());
  } else {
    frame_free_locals_inl(ar, func->numLocals());
  }
  assert(rv.m_type != KindOfUninit);
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
      frame_free_locals_no_this_inl(ar, func->numParams());
    } else {
      frame_free_locals_inl(ar, func->numParams());
    }
  } else {
    raise_error("Call to unimplemented native function %s()",
                func->name()->data());
    frame_free_locals_no_this_inl(ar, func->numParams());
  }
  ar->m_r.m_type = KindOfNull;
  return &ar->m_r;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
