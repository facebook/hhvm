// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use textual_macros::TextualDecl;

use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// These represent builtins for handling HHVM bytecode instructions. In general
/// the names should match the HHBC name except when they are compound bytecodes
/// (like Cmp with a parameter of Eq becoming CmpEq). Documentation can be found
/// in hphp/doc/bytecode.specification.
#[derive(Copy, Clone, TextualDecl)]
pub(crate) enum Hhbc {
    #[decl(fn hhbc_add(*HackMixed, *HackMixed) -> *HackMixed)]
    Add,
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
    #[decl(fn hhbc_concat(*HackMixed, *HackMixed) -> *HackMixed)]
    Concat,
    #[decl(fn hhbc_div(*HackMixed, *HackMixed) -> *HackMixed)]
    Div,
    #[decl(fn hhbc_exit(*HackMixed) -> noreturn)]
    Exit,
    #[decl(fn hhbc_is_type_int(*HackMixed) -> *HackMixed)]
    IsTypeInt,
    #[decl(fn hhbc_is_type_null(*HackMixed) -> *HackMixed)]
    IsTypeNull,
    #[decl(fn hhbc_is_type_str(*HackMixed) -> *HackMixed)]
    IsTypeStr,
    #[decl(fn hhbc_modulo(*HackMixed, *HackMixed) -> *HackMixed)]
    Modulo,
    #[decl(fn hhbc_mul(*HackMixed, *HackMixed) -> *HackMixed)]
    Mul,
    #[decl(fn hhbc_new_dict() -> *HackMixed)]
    NewDictArray,
    #[decl(fn hhbc_new_keyset_array(...) -> *HackMixed)]
    NewKeysetArray,
    #[decl(fn hhbc_new_obj(*class) -> *HackMixed)]
    NewObj,
    #[decl(fn hhbc_new_vec(...) -> *HackVec)]
    NewVec,
    #[decl(fn hhbc_not(*HackMixed) -> *HackMixed)]
    Not,
    #[decl(fn hhbc_print(*HackMixed) -> *HackMixed)]
    Print,
    #[decl(fn hhbc_sub(*HackMixed, *HackMixed) -> *HackMixed)]
    Sub,
    #[decl(fn hhbc_throw(*HackMixed) -> noreturn)]
    Throw,
    #[decl(fn hhbc_verify_param_type_ts(obj: *HackMixed, ts: *HackMixed) -> void)]
    VerifyParamTypeTS,
}

#[derive(TextualDecl)]
pub(crate) enum Builtin {
    /// Allocate an array with the given number of words (a word is a
    /// pointer-sized value).
    #[decl(fn alloc_words(int) -> *void)]
    AllocWords,
    /// Turns a raw boolean into a HackMixed.
    #[decl(fn hack_bool(int) -> *HackBool)]
    Bool,
    /// Turns a raw float into a Mixed.
    #[decl(fn hack_float(float) -> *HackFloat)]
    Float,
    /// Returns the Class identifier for the given class.
    #[decl(fn hack_get_class(*void) -> *class)]
    GetClass,
    /// Returns the Class identifier for the given class's static class.
    #[decl(fn hack_get_static_class(*void) -> *class)]
    GetStaticClass,
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
    /// Build a dict based on key/value pairs.
    #[decl(fn hack_new_dict(...) -> *HackMixed)]
    NewDict,
    /// Returns a HackMixed containing a `null`.
    #[decl(fn hack_null() -> *HackNull)]
    Null,
    /// Lazily initializes a static singleton.
    #[decl(fn lazy_initialize(*HackMixed) -> void)]
    SilLazyInitialize,
    /// Turns a raw string into a HackMixed.
    #[decl(fn hack_string(string) -> *HackString)]
    String,
    /// Checks that the predicate is true or throws a VerifyType error.
    #[decl(fn hhbc_verify_type_pred(obj: *HackMixed, pred: *HackMixed) -> void)]
    VerifyTypePred,
}

pub(crate) fn call_builtin(
    w: &mut textual::FuncWriter<'_>,
    target: Builtin,
    params: impl textual::VarArgs,
) -> Result<Sid> {
    w.call(&target.to_string(), params)
}

pub(crate) fn expr_builtin(target: Builtin, params: impl textual::VarArgs) -> textual::Expr {
    textual::Expr::call(target.to_string(), params)
}
