// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use hash::HashSet;
use strum::EnumIter;
use textual_macros::TextualDecl;

use crate::mangle::FunctionName;
use crate::textual;
use crate::textual::Sid;
use crate::textual::TextualFile;

type Result<T = (), E = Error> = std::result::Result<T, E>;

// TODO: Where should we define the builtin type hierarchy?
//
// class HackMixed { }
//
// class HackArray extends HackMixed { }
// class HackBool extends HackMixed { }
// class HackDict extends HackArray { }
// class HackFloat extends HackMixed { }
// class HackInt extends HackMixed { }
// class HackKeyset extends HackArray { }
// class HackString extends HackMixed { }
// class HackVec extends HackArray { }
//
// type HackArraykey = HackInt | HackString;
// type HackNum = HackInt | HackFloat;

/// These represent builtin constants used by Hack.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, Ord, PartialOrd)]
#[derive(TextualDecl, EnumIter)]
#[derive(Default)]
pub(crate) enum HackConst {
    // +inf
    #[decl(fn hack_const_inf() -> float)]
    #[default]
    Inf,
    // nan
    #[decl(fn hack_const_nan() -> float)]
    NaN,
    // -inf
    #[decl(fn hack_const_neginf() -> float)]
    NegInf,
}

/// These represent builtins for handling HHVM bytecode instructions. In general
/// the names should match the HHBC name except when they are compound bytecodes
/// (like Cmp with a parameter of Eq becoming CmpEq). Documentation can be found
/// in hphp/doc/bytecode.specification.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, Ord, PartialOrd)]
#[derive(TextualDecl, EnumIter)]
#[derive(Default)]
pub(crate) enum Hhbc {
    #[decl(fn hhbc_add(*HackMixed, *HackMixed) -> *HackMixed)]
    #[default]
    Add,
    #[decl(fn hhbc_add_elem_c(*HackMixed, *HackMixed, *HackMixed) -> *HackMixed)]
    AddElemC,
    #[decl(fn hhbc_add_new_elem_c(*HackMixed, *HackMixed) -> *HackMixed)]
    AddNewElemC,
    #[decl(fn hhbc_await(*HackMixed) -> *HackMixed)]
    Await,
    #[decl(fn hhbc_await_all(...) -> *HackMixed)]
    AwaitAll,
    #[decl(fn hhbc_bit_and(*HackMixed, *HackMixed) -> *HackMixed)]
    BitAnd,
    #[decl(fn hhbc_bit_or(*HackMixed, *HackMixed) -> *HackMixed)]
    BitOr,
    #[decl(fn hhbc_bit_xor(*HackMixed, *HackMixed) -> *HackMixed)]
    BitXor,
    #[decl(fn hhbc_cast_bool(*HackMixed) -> *HackMixed)]
    CastBool,
    #[decl(fn hhbc_cast_dict(*HackMixed) -> *HackMixed)]
    CastDict,
    #[decl(fn hhbc_cast_double(*HackMixed) -> *HackMixed)]
    CastDouble,
    #[decl(fn hhbc_cast_int(*HackMixed) -> *HackMixed)]
    CastInt,
    #[decl(fn hhbc_cast_keyset(*HackMixed) -> *HackMixed)]
    CastKeyset,
    #[decl(fn hhbc_cast_string(*HackMixed) -> *HackString)]
    CastString,
    #[decl(fn hhbc_cast_vec(*HackMixed) -> *HackMixed)]
    CastVec,
    #[decl(fn hhbc_chain_faults(*HackMixed, *HackMixed) -> *HackMixed)]
    ChainFaults,
    #[decl(fn hhbc_class_get_c(*HackMixed) -> void)]
    CheckClsRGSoft,
    #[decl(fn hhbc_check_this(*HackMixed) -> void)]
    CheckThis,
    #[decl(fn hhbc_class_get_c(*HackMixed) -> *HackMixed)]
    ClassGetC,
    #[decl(fn hhbc_class_has_reified_generics(*HackMixed) -> *HackMixed)]
    ClassHasReifiedGenerics,
    #[decl(fn hhbc_cls_cns(*HackMixed, *HackString) -> *HackMixed)]
    ClsCns,
    #[decl(fn hhbc_cmp_eq(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpEq,
    #[decl(fn hhbc_cmp_gt(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpGt,
    #[decl(fn hhbc_cmp_gte(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpGte,
    #[decl(fn hhbc_cmp_lt(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpLt,
    #[decl(fn hhbc_cmp_lte(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpLte,
    #[decl(fn hhbc_cmp_nsame(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpNSame,
    #[decl(fn hhbc_cmp_neq(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpNeq,
    #[decl(fn hhbc_cmp_same(*HackMixed, *HackMixed) -> *HackMixed)]
    CmpSame,
    // ColFromArray variations for different collection types
    #[decl(fn hhbc_col_from_array_imm_map(*HackMixed) -> *HackMixed)]
    ColFromArrayImmMap,
    #[decl(fn hhbc_col_from_array_imm_set(*HackMixed) -> *HackMixed)]
    ColFromArrayImmSet,
    #[decl(fn hhbc_col_from_array_imm_vector(*HackMixed) -> *HackMixed)]
    ColFromArrayImmVector,
    #[decl(fn hhbc_col_from_array_map(*HackMixed) -> *HackMixed)]
    ColFromArrayMap,
    #[decl(fn hhbc_col_from_array_pair(*HackMixed) -> *HackMixed)]
    ColFromArrayPair,
    #[decl(fn hhbc_col_from_array_set(*HackMixed) -> *HackMixed)]
    ColFromArraySet,
    #[decl(fn hhbc_col_from_array_vector(*HackMixed) -> *HackMixed)]
    ColFromArrayVector,
    // ColFromArray end
    #[decl(fn hhbc_combine_and_resolve_type_struct(...) -> *HackMixed)]
    CombineAndResolveTypeStruct,
    #[decl(fn hhbc_concat(*HackMixed, *HackMixed) -> *HackMixed)]
    Concat,
    #[decl(fn hhbc_concat(...) -> *HackMixed)]
    ConcatN,
    #[decl(fn hhbc_div(*HackMixed, *HackMixed) -> *HackMixed)]
    Div,
    #[decl(fn hhbc_exit(*HackMixed) -> noreturn)]
    Exit,
    #[decl(fn hhbc_fatal(*HackMixed) -> noreturn)]
    Fatal,
    #[decl(fn hhbc_get_cls_rg_prop(*HackMixed) -> *HackMixed)]
    GetClsRGProp,
    #[decl(fn hhbc_get_memo_key_l(*HackMixed) -> *HackMixed)]
    GetMemoKeyL,
    #[decl(fn hhbc_has_reified_parent(*HackMixed) -> *HackMixed)]
    HasReifiedParent,
    #[decl(fn hhbc_idx(*HackMixed, *HackMixed) -> *HackMixed)]
    Idx,
    #[decl(fn hhbc_is_late_bound_cls(*HackMixed) -> *HackMixed)]
    IsLateBoundCls,
    #[decl(fn hhbc_is_type_arr_like(*HackMixed) -> *HackMixed)]
    IsTypeArrLike,
    #[decl(fn hhbc_is_type_bool(*HackMixed) -> *HackMixed)]
    IsTypeBool,
    #[decl(fn hhbc_is_type_class(*HackMixed) -> *HackMixed)]
    IsTypeClass,
    #[decl(fn hhbc_is_type_cls_meth(*HackMixed) -> *HackMixed)]
    IsTypeClsMeth,
    #[decl(fn hhbc_is_type_dbl(*HackMixed) -> *HackMixed)]
    IsTypeDbl,
    #[decl(fn hhbc_is_type_dict(*HackMixed) -> *HackMixed)]
    IsTypeDict,
    #[decl(fn hhbc_is_type_func(*HackMixed) -> *HackMixed)]
    IsTypeFunc,
    #[decl(fn hhbc_is_type_int(*HackMixed) -> *HackMixed)]
    IsTypeInt,
    #[decl(fn hhbc_is_type_keyset(*HackMixed) -> *HackMixed)]
    IsTypeKeyset,
    #[decl(fn hhbc_is_type_legacy_arr_like(*HackMixed) -> *HackMixed)]
    IsTypeLegacyArrLike,
    #[decl(fn hhbc_is_type_null(*HackMixed) -> *HackMixed)]
    IsTypeNull,
    #[decl(fn hhbc_is_type_obj(*HackMixed) -> *HackMixed)]
    IsTypeObj,
    #[decl(fn hhbc_is_type_res(*HackMixed) -> *HackMixed)]
    IsTypeRes,
    #[decl(fn hhbc_is_type_scalar(*HackMixed) -> *HackMixed)]
    IsTypeScalar,
    #[decl(fn hhbc_is_type_str(*HackMixed) -> *HackMixed)]
    IsTypeStr,
    #[decl(fn hhbc_is_type_struct_c(*HackMixed, *HackMixed, *HackMixed) -> *HackMixed)]
    IsTypeStructC,
    #[decl(fn hhbc_is_type_vec(*HackMixed) -> *HackMixed)]
    IsTypeVec,
    #[decl(fn hhbc_iter_free(it: *Iterator) -> void)]
    IterFree,
    #[decl(fn hhbc_iter_init(it: **Iterator, key: **HackMixed, var: **HackMixed, container: *HackMixed) -> *HackBool)]
    IterInit,
    #[decl(fn hhbc_iter_next(it: *Iterator, key: **HackMixed, var: **HackMixed) -> *HackBool)]
    IterNext,
    #[decl(fn hhbc_lazy_class_from_class(*HackMixed) -> *HackString)]
    LazyClassFromClass,
    #[decl(fn hhbc_lock_obj(*HackMixed) -> void)]
    LockObj,
    #[decl(fn hhbc_memo_set(...) -> *HackMixed)]
    MemoSet,
    #[decl(fn hhbc_modulo(*HackMixed, *HackMixed) -> *HackMixed)]
    Modulo,
    #[decl(fn hhbc_mul(*HackMixed, *HackMixed) -> *HackMixed)]
    Mul,
    #[decl(fn hhbc_new_col_imm_map() -> *HackMixed)]
    NewColImmMap,
    #[decl(fn hhbc_new_col_imm_set() -> *HackMixed)]
    NewColImmSet,
    #[decl(fn hhbc_new_col_imm_vector() -> *HackMixed)]
    NewColImmVector,
    #[decl(fn hhbc_new_col_map() -> *HackMixed)]
    NewColMap,
    #[decl(fn hhbc_new_col_pair() -> *HackMixed)]
    NewColPair,
    #[decl(fn hhbc_new_col_set() -> *HackMixed)]
    NewColSet,
    #[decl(fn hhbc_new_col_vector() -> *HackMixed)]
    NewColVector,
    #[decl(fn hhbc_new_dict() -> *HackMixed)]
    NewDictArray,
    #[decl(fn hhbc_new_keyset_array(...) -> *HackMixed)]
    NewKeysetArray,
    #[decl(fn hhbc_new_vec(...) -> *HackVec)]
    NewVec,
    #[decl(fn hhbc_not(*HackMixed) -> *HackMixed)]
    Not,
    #[decl(fn hhbc_pow(*HackMixed, *HackMixed) -> *HackMixed)]
    Pow,
    #[decl(fn hhbc_print(*HackMixed) -> *HackMixed)]
    Print,
    #[decl(fn hhbc_record_reified_generic(*HackMixed) -> *HackMixed)]
    RecordReifiedGeneric,
    #[decl(fn hhbc_shl(*HackMixed, *HackMixed) -> *HackMixed)]
    Shl,
    #[decl(fn hhbc_shr(*HackMixed, *HackMixed) -> *HackMixed)]
    Shr,
    #[decl(fn hhbc_sub(*HackMixed, *HackMixed) -> *HackMixed)]
    Sub,
    #[decl(fn hhbc_throw(*HackMixed) -> noreturn)]
    Throw,
    #[decl(fn hhbc_throw_as_type_struct_exception(*HackMixed, *HackMixed) -> *HackMixed)]
    ThrowAsTypeStructException,
    #[decl(fn hhbc_throw_non_exhaustive_switch() -> noreturn)]
    ThrowNonExhaustiveSwitch,
    #[decl(fn hhbc_verify_param_type_ts(obj: *HackMixed, ts: *HackMixed) -> void)]
    VerifyParamTypeTS,
    #[decl(fn hhbc_wh_result(obj: *HackMixed) -> *HackMixed)]
    WHResult,
}

#[derive(Copy, Clone, Eq, PartialEq, Hash, Ord, PartialOrd, Debug)]
#[derive(TextualDecl, EnumIter)]
pub(crate) enum Builtin {
    /// Allocate an array with the given number of words (a word is a
    /// pointer-sized value).
    #[decl(fn alloc_words(int) -> *void)]
    AllocWords,
    /// Turns a raw boolean into a HackMixed.
    #[decl(fn hack_bool(int) -> *HackBool)]
    Bool,
    /// Get the value of a named field from a struct.
    #[decl(fn hack_field_get(base: *HackMixed, name: *HackMixed) -> *HackMixed)]
    FieldGet,
    /// Turns a raw float into a Mixed.
    #[decl(fn hack_float(float) -> *HackFloat)]
    Float,
    /// Returns the Class identifier for the given class.
    #[decl(fn hack_get_class(*void) -> *class)]
    GetClass,
    /// Returns the Class identifier for the given class's static class.
    #[decl(fn hack_get_static_class(*void) -> *class)]
    GetStaticClass,
    /// Get the named superglobal.
    #[decl(fn hack_get_superglobal(name: *HackMixed) -> *HackMixed)]
    GetSuperglobal,
    /// Like HackArrayCowSet but appends the value to the previous array and
    /// returns the copied base array.
    ///
    /// This is equivalent to the sequence:
    ///   a = ensure_unique(a)
    ///   a[b] = ensure_unique(a[b])
    ///   a[b][] = c
    ///
    #[decl(fn hack_array_cow_append(...) -> *HackMixed)]
    HackArrayCowAppend,
    /// n-ary array "set".
    ///
    /// Performs the n-ary array "set" operation but ensures that the arrays
    /// along the way are unique and then updates the final value. Returns the
    /// copied base array.
    ///
    /// This is equivalent to the sequence:
    ///   a = ensure_unique(a)
    ///   a[b] = ensure_unique(a[b])
    ///   a[b][c] = d
    ///
    #[decl(fn hack_array_cow_set(...) -> *HackMixed)]
    HackArrayCowSet,
    /// n-ary array "unset".
    ///
    /// Performs the n-ary array "unset" operation but ensures that the arrays
    /// along the way are unique and then updates the final value. Returns the
    /// copied base array.
    ///
    /// This is equivalent to the sequence:
    ///   a = ensure_unique(a)
    ///   a[b] = ensure_unique(a[b])
    ///   unset a[b][c]
    ///
    #[decl(fn hack_array_cow_unset(...) -> *HackMixed)]
    HackArrayCowUnset,
    /// n-ary array "get"
    ///
    /// Performs the n-ary array "get" operation without any copies and returns
    /// the tail value.
    ///
    /// This is equivalent to `a[b][c]`.
    ///
    #[decl(fn hack_array_get(...) -> *HackMixed)]
    HackArrayGet,
    /// Hack constants.
    #[decl(skip)]
    HackConst(HackConst),
    /// 1-ary prop "get".
    ///
    /// Dynamically fetches the value from the named key of the instance.
    ///
    /// If null_safe is true then a base of null returns null.
    ///
    /// This (when null_safe is false) is equivalent to:
    ///   base.?.{dynamic key}
    ///
    #[decl(fn hack_prop_get(base: *HackMixed, key: *HackMixed, null_safe: int) -> *HackMixed)]
    HackPropGet,
    /// 1-ary prop "set".
    ///
    /// Dynamically stores the value into the named key of the instance.
    ///
    /// If null_safe is true then a base of null is a no-op.
    ///
    /// This (when null_safe is false) is equivalent to:
    ///   base.?.{dynamic key} = value
    ///
    #[decl(fn hack_prop_set(base: *HackMixed, key: *HackMixed, null_safe: int, value: *HackMixed) -> void)]
    HackPropSet,
    /// Hhbc handlers.  See hphp/doc/bytecode.specification for docs.
    #[decl(skip)]
    Hhbc(Hhbc),
    /// Turns a raw int into a HackMixed.
    #[decl(fn hack_int(int) -> *HackInt)]
    Int,
    /// Returns true if the given HackMixed is truthy.
    #[decl(fn hack_is_true(*HackMixed) -> int)]
    IsTrue,
    /// Returns true if the given object is of the named type.
    #[decl(fn hack_is_type(obj: *HackMixed, ty: *HackString) -> *HackMixed)]
    IsType,
    /// Merge the given values into a single hash key and return the memoized
    /// value.
    /// (&global, ops)
    #[decl(fn hack_memo_get(...) -> *HackMixed)]
    MemoGet,
    /// Merge the given values into a single hash key and return true if the
    /// memo key is already set.
    /// (&global, ops)
    #[decl(fn hack_memo_isset(...) -> *HackMixed)]
    MemoIsset,
    /// Build a dict based on key/value pairs.
    #[decl(fn hack_new_dict(...) -> *HackMixed)]
    NewDict,
    #[decl(fn hack_set_static_prop(classname: string, propname: string, value: *HackArray) -> void)]
    SetStaticProp,
    /// Set the named superglobal.
    #[decl(fn hack_get_superglobal(name: *HackMixed, value: *HackMixed) -> void)]
    SetSuperglobal,
    /// Note that this argument is 'readonly'.
    #[decl(fn __sil_readonly(*HackArray) -> *HackArray)]
    SilReadonly,
    /// Note that this argument is a 'splat' (unwrapped array args for a function).
    #[decl(fn __sil_splat(*HackArray) -> *HackArray)]
    SilSplat,
    /// Turns a raw string into a HackMixed.
    #[decl(fn hack_string(string) -> *HackString)]
    String,
    /// Checks that the predicate is true or throws a VerifyType error.
    #[decl(fn hhbc_verify_type_pred(obj: *HackMixed, pred: *HackMixed) -> void)]
    VerifyTypePred,
}

pub(crate) fn call_builtin(
    fb: &mut textual::FuncBuilder<'_, '_>,
    target: Builtin,
    params: impl textual::VarArgs,
) -> Result<Sid> {
    fb.call(&FunctionName::Builtin(target), params)
}

pub(crate) fn expr_builtin(target: Builtin, params: impl textual::VarArgs) -> textual::Expr {
    textual::Expr::call(FunctionName::Builtin(target), params)
}
