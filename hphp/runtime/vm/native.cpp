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

#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP::Native {

//////////////////////////////////////////////////////////////////////////////

ConstantMap s_constant_map;
ClassConstantMapMap s_class_constant_map;

/////////////////////////////////////////////////////////////////////////////

namespace {

#ifdef __aarch64__
  constexpr size_t kNumGPRegs = 8;
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
  TypedValue spilled_rvals[kMaxBuiltinArgs];

  int GP_count{0};
  int SIMD_count{0};
  int spilled_count{0};
  int spilled_rval_count{0};
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

void pushRval(Registers& regs, tv_rval tv, bool isFCallBuiltin) {
  if (isFCallBuiltin) {
    // tv_rval either points at the stack, or we don't have wide
    // tv_vals. In either case, its actually pointing at a TypedValue
    // already.
    static_assert(TVOFF(m_data) == 0, "");
    assertx((const char*)&val(tv) + TVOFF(m_type) == (const char*)&type(tv));
    return pushInt(regs, (int64_t)&val(tv));
  }
  // Otherwise we need to materialize the TypedValue and push a
  // pointer to it.
  assertx(regs.spilled_rval_count < kMaxBuiltinArgs);
  regs.spilled_rvals[regs.spilled_rval_count++] = *tv;
  pushInt(regs, (int64_t)&regs.spilled_rvals[regs.spilled_rval_count - 1]);
}

// Push a double argument, spilling to the stack if necessary. We take the
// input as a Value in order to type-pun it as an int when we spill.
void pushDouble(Registers& regs, const Value value) {
  if (regs.SIMD_count < kNumSIMDRegs) {
    regs.SIMD_regs[regs.SIMD_count++] = value.dbl;
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
    assertx(regs.spilled_count + 1 < kMaxBuiltinArgs - kNumGPRegs);
    regs.spilled_args[regs.spilled_count++] = val(tv).num;
    regs.spilled_args[regs.spilled_count++] = dataType;
    // On x86, if we have one free GP register left, we'll use it for the next
    // int argument, but on ARM, we'll just spill all later int arguments.
    #ifdef __aarch64__
      regs.GP_count = kNumGPRegs;
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
void populateArgs(Registers& regs,
                  const ActRec* fp,
                  const Func* const func,
                  TypedValue* stk,
                  const int numArgs,
                  bool isFCallBuiltin) {
  // Regular FCalls will have their out parameter locations below the ActRec on
  // the stack, while FCallBuiltin has no ActRec to skip over.
  auto io = isFCallBuiltin
    ? stk + 1
    : reinterpret_cast<TypedValue*>(const_cast<ActRec*>(fp) + 1);

  auto const get = [&] (int idx) {
    return isFCallBuiltin
      ? tv_lval{&stk[-idx]}
      : frame_local(fp, idx);
  };

  for (auto i = 0; i < numArgs; ++i) {
    auto const arg = get(i);
    auto const& pi = func->params()[i];
    auto const type = pi.builtinType();
    if (func->isInOut(i)) {
      if (auto const iv = builtinInValue(func, i)) {
        *io = *iv;
        tvDecRefGen(arg);
      } else {
        *io = *arg;
      }

      // Any persistent values may become counted...
      if (isArrayLikeType(io->m_type)) {
        io->m_type = io->m_data.parr->toDataType();
      } else if (isStringType(io->m_type)) {
        io->m_type = KindOfString;
      }

      // Set the input value to null to avoid double freeing it
      arg.type() = KindOfNull;

      pushInt(regs, (int64_t)io++);
    } else if (pi.isTakenAsTypedValue()) {
      pushTypedValue(regs, *arg);
    } else if (pi.isNativeArg()) {
      pushNativeArg(regs, func, i, type, *arg);
    } else if (pi.isTakenAsVariant() || !type) {
      pushRval(regs, arg, isFCallBuiltin);
    } else if (type == KindOfDouble) {
      pushDouble(regs, val(arg));
    } else if (isBuiltinByRef(type)) {
      pushInt(regs, (int64_t)&val(arg));
    } else {
      pushInt(regs, val(arg).num);
    }
  }
}

}  // namespace

/////////////////////////////////////////////////////////////////////////////

/**
 * Dispatches a call to the native function bound to <func>
 * If <ctx> is not nullptr, it is prepended to <args> when
 * calling.
 */
void callFunc(const Func* const func,
              const ActRec* fp,
              const void* const ctx,
              TypedValue* args,
              TypedValue& ret,
              bool isFCallBuiltin) {
  auto const f = func->nativeFuncPtr();
  auto const numArgs = func->numParams();
  auto retType = func->returnTypeConstraint().asSystemlibType();
  auto regs = Registers{};

  if (ctx) pushInt(regs, (int64_t)ctx);
  populateArgs(regs, fp, func, args, numArgs, isFCallBuiltin);

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
    // A std::nullopt return signifies Variant.
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
    case KindOfLazyClass:
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
    case KindOfClsMeth:
    case KindOfObject:
    case KindOfEnumClassLabel:
    case KindOfResource: {
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

    case KindOfRFunc:
    case KindOfRClsMeth:
    case KindOfUninit:
      break;
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////////////

namespace {

template <typename F>
void coerceFCallArgsImpl(int32_t numArgs, const Func* func, F args) {
  assertx(func->isBuiltin() && "func is not a builtin");
  assertx(numArgs == func->numParams());

  for (int32_t i = 0; i < numArgs; i++) {
    const Func::ParamInfo& pi = func->params()[i];

    auto const tv = args(i);

    auto tc = pi.typeConstraint;
    auto targetType = pi.builtinType();

    if (tc.typeName() && interface_supports_arrlike(tc.typeName())) {
      // If we're dealing with an array-like interface, then there's no need to
      // coerce the input type: it's going to be mixed on the C++ side anyhow.
      continue;
    }

    if (tc.isNullable()) {
      if (tvIsNull(tv)) {
        // No need to coerce when passed a null for a nullable type
        continue;
      }
      // Arg isn't null, so treat it like the underlying type for coersion
      // purposes.  The ABI-passed type will still be mixed/Variant.
      targetType = std::nullopt;
    }
    if (!targetType) {
      // FIXME(T116301380): native builtins don't resolve properly
      targetType = tc.isUnresolved() ? KindOfObject : tc.underlyingDataType();
    }

    auto const raise_type_error = [&]{
      auto const expected_type = [&]{
        if (tc.isVecOrDict()) return "vec_or_dict";
        return getDataTypeString(*targetType).data();
      }();
      auto const msg = param_type_error_message(
        func->name()->data(), i+1, expected_type, *tv);
      if (RuntimeOption::PHP7_EngineExceptions) {
        SystemLib::throwTypeErrorObject(msg);
      }
      SystemLib::throwRuntimeExceptionObject(msg);
    };

    // Check the varray_or_darray and vec_or_dict union types.
    // Precondition: the DataType of the TypedValue is correct.
    auto const check_dvarray = [&]{
      assertx(IMPLIES(targetType, equivDataTypes(type(tv), *targetType)));
      if (tc.isVecOrDict() && !tvIsVec(tv) && !tvIsDict(tv)) {
        raise_type_error();
      }
    };

    // Check if we have the right type, or if its a Variant.
    if (!targetType || equivDataTypes(type(tv), *targetType)) {
      check_dvarray();
      continue;
    }

    if ((tvIsClass(tv) || tvIsLazyClass(tv)) && isStringType(*targetType)) {
      val(tv).pstr = tvIsClass(tv) ?
        const_cast<StringData*>(val(tv).pclass->name()) :
        const_cast<StringData*>(val(tv).plazyclass.name());
      type(tv) = KindOfPersistentString;
      if (RuntimeOption::EvalClassStringHintNotices) {
        auto reason = folly::sformat(
          "argument {} passed to {}()", i+1, func->fullName());
        raise_notice(Strings::CLASS_TO_STRING_IMPLICIT, reason.c_str());
      }
      continue;
    }

    raise_type_error();
  }
}

}

void coerceFCallArgsFromLocals(const ActRec* fp,
                               int32_t numArgs,
                               const Func* func) {
  coerceFCallArgsImpl(
    numArgs, func,
    [&] (int32_t idx) { return frame_local(fp, idx); }
  );
}

#undef CASE
#undef COERCE_OR_CAST

TypedValue* functionWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->func();
  auto numArgs = func->numParams();
  TypedValue* args = ((TypedValue*)ar) - 1;

  coerceFCallArgsFromLocals(ar, numArgs, func);

  TypedValue rv;
  rv.m_type = KindOfUninit;
  callFunc(func, ar, nullptr, args, rv, false);

  assertx(rv.m_type != KindOfUninit);
  frame_free_locals_no_this_inl(
    ar,
    func->numLocals(),
    &rv,
    EventHook::Source::Native
  );
  tvCopy(rv, *ar->retSlot());
  ar->retSlot()->m_aux.u_asyncEagerReturnFlag = 0;
  return ar->retSlot();
}

TypedValue* methodWrapper(ActRec* ar) {
  assertx(ar);
  auto func = ar->func();
  auto numArgs = func->numParams();
  bool isStatic = func->isStatic();
  TypedValue* args = ((TypedValue*)ar) - 1;

  coerceFCallArgsFromLocals(ar, numArgs, func);

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
  callFunc(func, ar, ctx, args, rv, false);

  assertx(rv.m_type != KindOfUninit);
  if (isStatic) {
    frame_free_locals_no_this_inl(
      ar,
      func->numLocals(),
      &rv,
      EventHook::Source::Native
    );
  } else {
    frame_free_locals_inl(
      ar,
      func->numLocals(),
      &rv,
      EventHook::Source::Native
    );
  }
  tvCopy(rv, *ar->retSlot());
  ar->retSlot()->m_aux.u_asyncEagerReturnFlag = 0;
  return ar->retSlot();
}

void getFunctionPointers(const NativeFunctionInfo& info, int nativeAttrs,
                         ArFunction& bif, NativeFunction& nif) {
  nif = info.ptr;
  if (!nif) return;
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
  if (strcmp(str, "varray") == 0) return KindOfVec;
  if (strcmp(str, "darray") == 0) return KindOfDict;

#define DT(name, ...) if (strcmp(str, "KindOf" #name) == 0) return KindOf##name;
  DATATYPES
#undef DT

  return {};
}

MaybeDataType builtinOutType(
  const TypeConstraint& tc,
  const UserAttributeMap& map
) {
  // FIXME(T116301380): native builtins don't resolve properly
  auto const tcDT = tc.isUnresolved() ? KindOfObject : tc.underlyingDataType();

  auto const it = map.find(s_outOnly.get());
  if (it == map.end()) return tcDT;

  auto const dt = typeForOutParam(it->second);
  return dt ? dt : tcDT;
}

static Optional<TypedValue> builtinInValue(
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
  case KindOfUninit:
  case KindOfObject:
  case KindOfResource:
  case KindOfEnumClassLabel:
  case KindOfRFunc:
  case KindOfFunc:
  case KindOfClass:
  case KindOfLazyClass:
  case KindOfClsMeth:
  case KindOfRClsMeth: return make_tv<KindOfNull>();
  }

  not_reached();
}

Optional<TypedValue> builtinInValue(const Func* builtin, uint32_t i) {
  return builtinInValue(builtin->params()[i]);
}

//////////////////////////////////////////////////////////////////////////////

static bool tcCheckNative(const TypeConstraint& tc, const NativeSig::Type ty) {
  using T = NativeSig::Type;

  if (tc.typeName() && interface_supports_arrlike(tc.typeName())) {
    // TODO(T116301380): If native builtins resolved properly, we could probably
    // do something smarter here. As it stands we match on whether the type here
    // is _exactly_ one of the magic interfaces that supports array like things,
    // including Hack Collections.
    return ty == T::Mixed || ty == T::MixedTV;
  }

  if (!tc.hasConstraint() || tc.isNullable() || tc.isCallable() ||
      tc.isArrayKey() || tc.isNumber() || tc.isVecOrDict() ||
      tc.isArrayLike() || tc.isClassname() || tc.isTypeVar() ||
      tc.isUnion()) {
    return ty == T::Mixed || ty == T::MixedTV;
  }

  if (tc.isNothing() || tc.isNoReturn()) { return ty == T::Void; }

  if (tc.isUnresolved()) {
    // FIXME(T116301380): native builtins don't resolve properly
    return ty == T::Object || ty == T::ObjectArg;
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
    case KindOfKeyset:       return ty == T::Array    || ty == T::ArrayArg;
    case KindOfResource:     return ty == T::Resource || ty == T::ResourceArg;
    case KindOfUninit:
    case KindOfNull:         return ty == T::Void;
    case KindOfInt64:        return ty == T::Int64;
    case KindOfRFunc:        return false; // TODO(T66903859)
    case KindOfFunc:         return ty == T::Func;
    case KindOfClass:        return ty == T::Class;
    case KindOfClsMeth:      return ty == T::ClsMeth;
    case KindOfRClsMeth:     // TODO(T67037453)
    case KindOfLazyClass:    return false; // TODO (T68823958)
    case KindOfEnumClassLabel: return false;
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
      case KindOfResource:     return ty == T::ResourceIO;
      case KindOfUninit:
      case KindOfNull:         return false;
      case KindOfInt64:        return ty == T::IntIO;
      case KindOfRFunc:        return false; // TODO(T66903859)
      case KindOfFunc:         return ty == T::FuncIO;
      case KindOfClass:        return ty == T::ClassIO;
      case KindOfClsMeth:      return ty == T::ClsMethIO;
      case KindOfRClsMeth:     // TODO (T67037453)
      case KindOfLazyClass:    return false; // TODO (T68823958)
      case KindOfEnumClassLabel: return false;
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
      tc.isArrayLike() || tc.isClassname() || tc.isTypeVar()) {
    return ty == T::MixedIO;
  }

  // FIXME(T116301380): native builtins don't resolve properly
  if (tc.isUnresolved()) return ty == T::ObjectIO;

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

//////////////////////////////////////////////////////////////////////////////

using ClassExtraDataHandlerMap = std::unordered_map
  <const StringData*, FinishFunc>;

static ClassExtraDataHandlerMap s_classExtraDataHandlerMap;

void registerClassExtraDataHandler(const String& clsName, FinishFunc fn) {
  assertx(s_classExtraDataHandlerMap.find(clsName.get()) ==
    s_classExtraDataHandlerMap.end());
  s_classExtraDataHandlerMap[clsName.get()] = fn;
}

FinishFunc getClassExtraDataHandler(const StringData* clsName) {
  auto it = s_classExtraDataHandlerMap.find(clsName);
  if (it == s_classExtraDataHandlerMap.end()) {
    return nullptr;
  }
  return it->second;
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Native
