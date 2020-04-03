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

/////////////////////////////////////////////////////////////////////////////

namespace {

#ifdef __aarch64__
  constexpr size_t kNumGPRegs = 8;
#elif defined(__powerpc64__)
  constexpr size_t kNumGPRegs = 31;
#else
  // amd64 calling convention (also used by x64): rdi, rsi, rdx, rcx, r8, r9
  constexpr size_t kNumGPRegs = 6;
#endif

// Note: This number should generally not be modified
// as it depends on the CPU's ABI.
// If an update is needed, however, update and run
// make_native-func-caller.php as well
constexpr size_t kNumSIMDRegs = 8;

#include "hphp/runtime/vm/native-func-caller.h"

struct Registers {
  // The spilled arguments come right after the GP regs so that we can treat
  // them as a single array of kMaxBuiltinArgs ints after populating them.
  int64_t GP_regs[kNumGPRegs];
  int64_t spilled_args[kMaxBuiltinArgs - kNumGPRegs];
  double SIMD_regs[kNumSIMDRegs];

  int GP_count{0};
  int SIMD_count{0};
  int spilled_count{0};
};

// Push an int argument, spilling to the stack if necessary.
void pushInt(Registers& regs, const int64_t value) {
  if (regs.GP_count < kNumGPRegs) {
    regs.GP_regs[regs.GP_count++] = value;
  } else {
    assertx(regs.spilled_count < kMaxBuiltinArgs - kNumGPRegs);
    regs.spilled_args[regs.spilled_count++] = value;
  }
}

// Push a double argument, spilling to the stack if necessary. We take the
// input as a Value in order to type-pun it as an int when we spill.
void pushDouble(Registers& regs, const Value value) {
  if (regs.SIMD_count < kNumSIMDRegs) {
    regs.SIMD_regs[regs.SIMD_count++] = value.dbl;
#if defined(__powerpc64__)
    // Following ABI, we must increment the GP reg index for each double arg.
    if (regs.GP_count < kNumGPRegs) regs.GP_regs[regs.GP_count++] = 0;
#endif
  } else {
    assertx(regs.spilled_count < kMaxBuiltinArgs - kNumGPRegs);
    regs.spilled_args[regs.spilled_count++] = value.num;
  }
}

// Push a TypedValue argument, spilling to the stack if necessary. We need
// two free GP registers to avoid spilling here. The details of what happens
// when we spill with one free GP register changes between architectures.
void pushTypedValue(Registers& regs, TypedValue tv) {
  auto const dataType = static_cast<data_type_t>(type(tv));
  if (regs.GP_count + 1 < kNumGPRegs) {
    regs.GP_regs[regs.GP_count++] = val(tv).num;
    regs.GP_regs[regs.GP_count++] = dataType;
  } else {
#if defined(__powerpc64__)
    // We don't have room to spill two 64-bit values in PowerPC. If it becomes
    // an issue, we can always up kMaxBuiltinArgs later. (We have room to pass
    // 15 TypedValue arguments on PowerPC already, which should be plenty.)
    always_assert(false);
#else
    assertx(regs.spilled_count + 1 < kMaxBuiltinArgs - kNumGPRegs);
    regs.spilled_args[regs.spilled_count++] = val(tv).num;
    regs.spilled_args[regs.spilled_count++] = dataType;
    // On x86, if we have one free GP register left, we'll use it for the next
    // int argument, but on ARM, we'll just spill all later int arguments.
    #ifdef __aarch64__
      regs.GP_count = kNumGPRegs;
    #endif
#endif
  }
}

// Push a native argument (e.g an ArrayData / TypedValue, not Array / Variant).
void pushNativeArg(Registers& regs, const Func* const func, const int i,
                   MaybeDataType builtinType, TypedValue arg) {
  // If the param type is known, just pass the value.
  if (builtinType) return pushInt(regs, val(arg).num);

  // Pass both the type and value for TypedValue parameters.
  pushTypedValue(regs, arg);
}

// Push each argument, spilling ones we don't have registers for to the stack.
void populateArgs(Registers& regs, const Func* const func,
                  const TypedValue* const args, const int numArgs,
                  bool isFCallBuiltin) {
  auto io = const_cast<TypedValue*>(args + 1);

  // Regular FCalls will have their out parameter locations below the ActRec on
  // the stack, while FCallBuiltin has no ActRec to skip over.
  if (!isFCallBuiltin) io += kNumActRecCells;
  for (auto i = 0; i < numArgs; ++i) {
    auto const& arg = args[-i];
    auto const& pi = func->params()[i];
    auto const type = pi.builtinType;
    if (func->isInOut(i)) {
      if (auto const iv = builtinInValue(func, i)) {
        *io = *iv;
        tvDecRefGen(arg);
      } else {
        *io = arg;
      }

      // Any persistent values may become counted...
      if (isArrayLikeType(io->m_type)) {
        io->m_type = io->m_data.parr->toDataType();
      } else if (isStringType(io->m_type)) {
        io->m_type = KindOfString;
      }

      // Set the input value to null to avoid double freeing it
      const_cast<TypedValue&>(arg).m_type = KindOfNull;

      pushInt(regs, (int64_t)io++);
    } else if (type == KindOfDouble) {
      pushDouble(regs, val(arg));
    } else if (pi.isNativeArg()) {
      pushNativeArg(regs, func, i, type, arg);
    } else if (!type) {
      pushInt(regs, (int64_t)&arg);
    } else if (isBuiltinByRef(type)) {
      pushInt(regs, (int64_t)&val(arg));
    } else {
      pushInt(regs, val(arg).num);
    }
  }
}

}  // namespace

/////////////////////////////////////////////////////////////////////////////

void callFunc(const Func* const func, const void* const ctx,
              const TypedValue* const args, const int numNonDefault,
              TypedValue& ret, bool isFCallBuiltin) {
  auto const f = func->nativeFuncPtr();
  auto const numArgs = func->numParams();
  auto retType = func->hniReturnType();
  auto regs = Registers{};

  if (ctx) pushInt(regs, (int64_t)ctx);
  populateArgs(regs, func, args, numArgs, isFCallBuiltin);

  // Decide how many int and double arguments we need to call func. Note that
  // spilled arguments come after the GP registers, in line with them. We can
  // spill to the stack without exhausting the GP registers, in two ways:
  //
  //  1. If we exceeed the number of SIMD arguments and spill doubles.
  //
  //  2. If we fill all but 1 GP register and then need to pass a TypedValue
  //     (two registers in size) by value.
  //
  // In these cases, we'll pass garbage in the unused GP registers to force
  // everything in the regs.spilled_args array to go on the stack.
  auto const spilled = regs.spilled_count;
  auto const GP_args = &regs.GP_regs[0];
  auto const GP_count = spilled > 0 ? spilled + kNumGPRegs : regs.GP_count;
  auto const SIMD_args = &regs.SIMD_regs[0];
  auto const SIMD_count = regs.SIMD_count;

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
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfClsMeth:
    case KindOfObject:
    case KindOfResource:
    case KindOfRecord: {
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

void coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func) {
  assertx(func->isBuiltin() && "func is not a builtin");
  assertx(numArgs == func->numParams());

  for (int32_t i = 0; (i < numNonDefault) && (i < numArgs); i++) {
    const Func::ParamInfo& pi = func->params()[i];

    auto tc = pi.typeConstraint;
    auto targetType = pi.builtinType;
    if (tc.isNullable()) {
      if (isNullType(args[-i].m_type)) {
        // No need to coerce when passed a null for a nullable type
        continue;
      }
      // Arg isn't null, so treat it like the underlying type for coersion
      // purposes.  The ABI-passed type will still be mixed/Variant.
      targetType = tc.underlyingDataType();
    }
    if (!targetType) {
      targetType = tc.underlyingDataType();
    }

    // Check if we have the right type, or if its a Variant.
    if (!targetType || equivDataTypes(args[-i].m_type, *targetType)) {
      auto const c = &args[-i];

      if (LIKELY(!RuntimeOption::EvalHackArrCompatTypeHintNotices) ||
          !tc.isArray() ||
          !isArrayType(c->m_type)) {
        continue;
      }

      auto const raise = [&] {
        auto const ad = val(c).parr;
        if (tc.isVArray()) {
          return !ad->isVArray();
        } else if (tc.isDArray()) {
          return !ad->isDArray();
        } else if (tc.isVArrayOrDArray()) {
          return ad->isNotDVArray() ||
                 RuntimeOption::EvalHackArrCompatTypeHintPolymorphism;
        } else {
          return !ad->isNotDVArray();
        }
      }();
      if (raise) {
        raise_hackarr_compat_type_hint_param_notice(
          func,
          c->m_data.parr,
          tc.displayName().c_str(),
          i
        );
      }
      continue;
    }

    if (isFuncType(args[-i].m_type) && isStringType(*targetType)) {
      args[-i].m_data.pstr = const_cast<StringData*>(
        args[-i].m_data.pfunc->name()
      );
      args[-i].m_type = KindOfPersistentString;
      if (RuntimeOption::EvalStringHintNotices) {
        raise_notice(Strings::FUNC_TO_STRING_IMPLICIT);
      }
      continue;
    }
    if (isClassType(args[-i].m_type) && isStringType(*targetType)) {
      args[-i].m_data.pstr = const_cast<StringData*>(
        args[-i].m_data.pclass->name()
      );
      args[-i].m_type = KindOfPersistentString;
      if (RuntimeOption::EvalStringHintNotices) {
        raise_notice(Strings::CLASS_TO_STRING_IMPLICIT);
      }
      continue;
    }
    if (isClsMethType(args[-i].m_type)) {
      auto raise = [&] {
        if (RuntimeOption::EvalVecHintNotices) {
          raise_clsmeth_compat_type_hint(
            func, tc.displayName(func->cls()), i);
        }
      };
      if (RuntimeOption::EvalHackArrDVArrs) {
        if (isVecType(*targetType)) {
          tvCastToVecInPlace(&args[-i]);
          raise();
          continue;
        }
      } else {
        if (isArrayType(*targetType)) {
          tvCastToVArrayInPlace(&args[-i]);
          raise();
          continue;
        }
      }
    }


    auto msg = param_type_error_message(
      func->name()->data(),
      i+1,
      *targetType,
      args[-i].m_type
    );
    if (RuntimeOption::PHP7_EngineExceptions) {
      SystemLib::throwTypeErrorObject(msg);
    }
    SystemLib::throwRuntimeExceptionObject(msg);
  }
}

#undef CASE
#undef COERCE_OR_CAST

TypedValue* functionWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  TypedValue* args = ((TypedValue*)ar) - 1;

  coerceFCallArgs(args, numArgs, numNonDefault, func);

  TypedValue rv;
  rv.m_type = KindOfUninit;
  callFunc(func, nullptr, args, numNonDefault, rv, false);

  assertx(rv.m_type != KindOfUninit);
  frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  tvCopy(rv, *ar->retSlot());
  ar->retSlot()->m_aux.u_asyncEagerReturnFlag = 0;
  return ar->retSlot();
}

TypedValue* methodWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->m_func;
  auto numArgs = func->numParams();
  auto numNonDefault = ar->numArgs();
  bool isStatic = func->isStatic();
  TypedValue* args = ((TypedValue*)ar) - 1;

  coerceFCallArgs(args, numArgs, numNonDefault, func);

  // Prepend a context arg for methods
  // Class when it's being called statically Foo::bar()
  // Object when it's being called on an instance $foo->bar()
  void* ctx;  // ObjectData* or Class*
  if (ar->hasThis()) {
    if (isStatic) {
      throw_instance_method_fatal(func->fullName()->data());
    }
    ctx = ar->getThis();
  } else {
    if (!isStatic) {
      throw_instance_method_fatal(func->fullName()->data());
    }
    ctx = ar->getClass();
  }

  TypedValue rv;
  rv.m_type = KindOfUninit;
  callFunc(func, ctx, args, numNonDefault, rv, false);

  assertx(rv.m_type != KindOfUninit);
  if (isStatic) {
    frame_free_locals_no_this_inl(ar, func->numLocals(), &rv);
  } else {
    frame_free_locals_inl(ar, func->numLocals(), &rv);
  }
  tvCopy(rv, *ar->retSlot());
  ar->retSlot()->m_aux.u_asyncEagerReturnFlag = 0;
  return ar->retSlot();
}

[[noreturn]] TypedValue* unimplementedWrapper(ActRec* ar) {
  auto func = ar->m_func;
  auto cls = func->cls();
  if (cls) {
    raise_error("Call to unimplemented native method %s::%s()",
                cls->name()->data(), func->name()->data());
  }
  raise_error("Call to unimplemented native function %s()",
              func->name()->data());
}

void getFunctionPointers(const NativeFunctionInfo& info, int nativeAttrs,
                         ArFunction& bif, NativeFunction& nif) {
  nif = info.ptr;
  if (!nif) {
    bif = unimplementedWrapper;
    return;
  }

  auto const isMethod = info.sig.args.size() &&
      ((info.sig.args[0] == NativeSig::Type::This) ||
       (info.sig.args[0] == NativeSig::Type::Class));
  bif = isMethod ? methodWrapper : functionWrapper;
}

//////////////////////////////////////////////////////////////////////////////

const StaticString s_outOnly("__OutOnly");

static MaybeDataType typeForOutParam(TypedValue attr) {

  if (!isArrayLikeType(attr.m_type) || attr.m_data.parr->size() < 1) {
    return {};
  }

  auto const& type = attr.m_data.parr->nvGetVal(attr.m_data.parr->iter_begin());
  if (!isStringType(type.m_type)) return {};

  auto const str = type.m_data.pstr->data();

#define DT(name, ...) if (strcmp(str, "KindOf" #name) == 0) return KindOf##name;
  DATATYPES
#undef DT

  return {};
}

MaybeDataType builtinOutType(
  const TypeConstraint& tc,
  const UserAttributeMap& map
) {
  auto const tcDT = tc.underlyingDataType();

  auto const it = map.find(s_outOnly.get());
  if (it == map.end()) return tcDT;

  auto const dt = typeForOutParam(it->second);
  return dt ? dt : tcDT;
}

static folly::Optional<TypedValue> builtinInValue(
  const Func::ParamInfo& pinfo
) {
  auto& map = pinfo.userAttributes;

  auto const it = map.find(s_outOnly.get());
  if (it == map.end()) return {};

  auto const dt = typeForOutParam(it->second);
  if (!dt) return make_tv<KindOfNull>();

  switch (*dt) {
  case KindOfNull:    return make_tv<KindOfNull>();
  case KindOfBoolean: return make_tv<KindOfBoolean>(false);
  case KindOfInt64:   return make_tv<KindOfInt64>(0);
  case KindOfDouble:  return make_tv<KindOfDouble>(0.0);
  case KindOfPersistentString:
  case KindOfString:  return make_tv<KindOfString>(staticEmptyString());
  case KindOfPersistentVec:
  case KindOfVec:     return make_tv<KindOfVec>(ArrayData::CreateVec());
  case KindOfPersistentDict:
  case KindOfDict:    return make_tv<KindOfDict>(ArrayData::CreateDict());
  case KindOfPersistentKeyset:
  case KindOfKeyset:  return make_tv<KindOfNull>();
  case KindOfPersistentDArray:
  case KindOfDArray:  return make_array_like_tv(ArrayData::CreateDArray());
  case KindOfPersistentVArray:
  case KindOfVArray:  return make_array_like_tv(ArrayData::CreateVArray());
  case KindOfPersistentArray:
  case KindOfArray:   return make_array_like_tv(ArrayData::Create());
  case KindOfUninit:
  case KindOfObject:
  case KindOfResource:
  case KindOfFunc:
  case KindOfClass:
  case KindOfClsMeth:
  case KindOfRecord:  return make_tv<KindOfNull>();
  }

  not_reached();
}

MaybeDataType builtinOutType(const Func* builtin, uint32_t i) {
  auto const& pinfo = builtin->params()[i];
  return builtinOutType(pinfo.typeConstraint, pinfo.userAttributes);
}

folly::Optional<TypedValue> builtinInValue(const Func* builtin, uint32_t i) {
  return builtinInValue(builtin->params()[i]);
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
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:        return ty == T::Array    || ty == T::ArrayArg;
    case KindOfResource:     return ty == T::Resource || ty == T::ResourceArg;
    case KindOfUninit:
    case KindOfNull:         return ty == T::Void;
    case KindOfInt64:        return ty == T::Int64    || ty == T::Int32;
    case KindOfFunc:         return ty == T::Func;
    case KindOfClass:        return ty == T::Class;
    case KindOfClsMeth:      return ty == T::ClsMeth;
    case KindOfRecord:       return false; // TODO (T41031632)
  }
  not_reached();
}

static bool tcCheckNativeIO(
  const Func::ParamInfo& pinfo, const NativeSig::Type ty
) {
  using T = NativeSig::Type;

  auto const checkDT = [&] (DataType dt) -> bool {
    switch (dt) {
      case KindOfDouble:       return ty == T::DoubleIO;
      case KindOfBoolean:      return ty == T::BoolIO;
      case KindOfObject:       return ty == T::ObjectIO;
      case KindOfPersistentString:
      case KindOfString:       return ty == T::StringIO;
      case KindOfPersistentVec:
      case KindOfVec:          return ty == T::ArrayIO;
      case KindOfPersistentDict:
      case KindOfDict:         return ty == T::ArrayIO;
      case KindOfPersistentKeyset:
      case KindOfKeyset:       return ty == T::ArrayIO;
      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray:        return ty == T::ArrayIO;
      case KindOfResource:     return ty == T::ResourceIO;
      case KindOfUninit:
      case KindOfNull:         return false;
      case KindOfInt64:        return ty == T::IntIO;
      case KindOfFunc:         return ty == T::FuncIO;
      case KindOfClass:        return ty == T::ClassIO;
      case KindOfClsMeth:      return ty == T::ClsMethIO;
      case KindOfRecord:       return false; // TODO (T41031632)
    }
    not_reached();
  };

  auto const tv = builtinInValue(pinfo);
  if (tv) {
    if (isNullType(tv->m_type)) return ty == T::MixedIO;
    return checkDT(tv->m_type);
  }

  auto const& tc = pinfo.typeConstraint;
  if (!tc.hasConstraint() || tc.isNullable() || tc.isCallable() ||
      tc.isArrayKey() || tc.isNumber() || tc.isVecOrDict() ||
      tc.isVArrayOrDArray() || tc.isArrayLike()) {
    return ty == T::MixedIO;
  }

  if (!tc.underlyingDataType()) {
    return false;
  }

  return checkDT(*tc.underlyingDataType());
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

static const StaticString
  s_native("__Native"),
  s_actrec("ActRec");

const char* checkTypeFunc(const NativeSig& sig,
                          const TypeConstraint& retType,
                          const FuncEmitter* func) {
  using T = NativeSig::Type;

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

  for (auto const& pInfo : func->params) {
    if (argIt == endIt) return kInvalidArgCountMessage;

    auto const argTy = *argIt++;

    if (pInfo.isVariadic()) {
      if (argTy != T::Array) return kInvalidArgTypeMessage;
      continue;
    }

    if (pInfo.isInOut()) {
      if (!tcCheckNativeIO(pInfo, argTy)) {
        return kInvalidArgTypeMessage;
      }
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
  assertx(name->isStatic());
  DEBUG_ONLY auto it = m_infos.insert(std::make_pair(name, info));
  assertx(it.second || it.first->second == info);
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
  case T::Mixed:      return "mixed";
  case T::MixedTV:    return "mixed";
  case T::This:       return "this";
  case T::Class:      return "class";
  case T::Void:       return "void";
  case T::Func:       return "func";
  case T::ClsMeth:    return "clsmeth";
  case T::IntIO:      return "inout int";
  case T::DoubleIO:   return "inout double";
  case T::BoolIO:     return "inout bool";
  case T::ObjectIO:   return "inout object";
  case T::StringIO:   return "inout string";
  case T::ArrayIO:    return "inout array";
  case T::ResourceIO: return "inout resource";
  case T::FuncIO:     return "inout func";
  case T::ClassIO:    return "inout class";
  case T::ClsMethIO:  return "inout clsmeth";
  case T::MixedIO:    return "inout mixed";
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
  tv.m_data.pcnt = reinterpret_cast<MaybeCountable*>(callback);
  tv.dynamic() = true;
  if (!Unit::defNativeConstantCallback(cnsName, tv)) {
    return false;
  }
  s_constant_map[cnsName] = tv;
  return true;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
