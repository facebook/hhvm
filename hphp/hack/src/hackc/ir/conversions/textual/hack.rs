// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use anyhow::Error;
use ir::instr::Special;
use ir::instr::Textual;
use ir::instr::TextualHackBuiltinParam;
use ir::Instr;
use ir::LocId;
use ir::ValueId;

use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// These represent builtins for handling HHVM bytecode instructions. In general
/// the names should match the HHBC name except when they are compound bytecodes
/// (like Cmp with a parameter of Eq becoming CmpEq). Documentation can be found
/// in hphp/doc/bytecode.specification.
pub(crate) enum Hhbc {
    Add,
    AddO,
    CmpEq,
    CmpGt,
    CmpGte,
    CmpLt,
    CmpLte,
    CmpNSame,
    CmpNeq,
    CmpSame,
    IsTypeInt,
    Modulo,
    Not,
    Print,
    Sub,
    SubO,
    VerifyFailed,
}

pub(crate) enum Builtin {
    /// Build a *HackParams for the given number of parameters. Takes a "this"
    /// value and the number of parameters as the value (so one more total than
    /// the arg).
    ///   ArgPack<0>(*Mixed) -> *HackParams
    ///   ArgPack<1>(*Mixed, *Mixed) -> *HackParams
    ///   ArgPack<2>(*Mixed, *Mixed, *Mixed) -> *HackParams
    ArgPack(usize),
    /// Turns a raw boolean into a Mixed.
    ///   Bool(n: bool) -> *Mixed
    Bool,
    Copy,
    /// Hhbc handlers.  See hphp/doc/bytecode.specification for docs.
    Hhbc(Hhbc),
    /// Turns a raw int into a Mixed.
    ///   Int(n: int) -> *Mixed
    Int,
    /// Returns a Mixed containing a `null`.
    ///   Null() -> *Mixed
    Null,
}

impl Builtin {
    pub(crate) fn into_str(&self) -> Cow<'static, str> {
        match self {
            Builtin::Hhbc(Hhbc::Add) => Cow::Borrowed("hack_add"),
            Builtin::Hhbc(Hhbc::AddO) => Cow::Borrowed("hack_add_o"),
            Builtin::ArgPack(n) => Cow::Owned(format!("arg_pack_{}", n)),
            Builtin::Bool => Cow::Borrowed("hack_bool"),
            Builtin::Hhbc(Hhbc::CmpEq) => Cow::Borrowed("hack_cmp_eq"),
            Builtin::Hhbc(Hhbc::CmpGt) => Cow::Borrowed("hack_cmp_gt"),
            Builtin::Hhbc(Hhbc::CmpGte) => Cow::Borrowed("hack_cmp_gte"),
            Builtin::Hhbc(Hhbc::CmpLt) => Cow::Borrowed("hack_cmp_lt"),
            Builtin::Hhbc(Hhbc::CmpLte) => Cow::Borrowed("hack_cmp_lte"),
            Builtin::Hhbc(Hhbc::CmpNSame) => Cow::Borrowed("hack_cmp_nsame"),
            Builtin::Hhbc(Hhbc::CmpNeq) => Cow::Borrowed("hack_cmp_neq"),
            Builtin::Hhbc(Hhbc::CmpSame) => Cow::Borrowed("hack_cmp_same"),
            Builtin::Copy => Cow::Borrowed("copy"),
            Builtin::Int => Cow::Borrowed("hack_int"),
            Builtin::Hhbc(Hhbc::IsTypeInt) => Cow::Borrowed("hack_is_int"),
            Builtin::Hhbc(Hhbc::Modulo) => Cow::Borrowed("hack_modulo"),
            Builtin::Hhbc(Hhbc::Not) => Cow::Borrowed("hack_not"),
            Builtin::Null => Cow::Borrowed("hack_null"),
            Builtin::Hhbc(Hhbc::Print) => Cow::Borrowed("hack_print"),
            Builtin::Hhbc(Hhbc::Sub) => Cow::Borrowed("hack_sub"),
            Builtin::Hhbc(Hhbc::SubO) => Cow::Borrowed("hack_sub_o"),
            Builtin::Hhbc(Hhbc::VerifyFailed) => Cow::Borrowed("hack_verify_failed"),
        }
    }
}

pub(crate) fn builtin_instr(
    target: Builtin,
    params: Vec<TextualHackBuiltinParam>,
    values: Vec<ValueId>,
    loc: LocId,
) -> Instr {
    let target = target.into_str();
    Instr::Special(Special::Textual(Textual::HackBuiltin {
        target,
        params: params.into_boxed_slice(),
        values: values.into_boxed_slice(),
        loc,
    }))
}

pub(crate) fn call_builtin(
    w: &mut textual::FuncWriter<'_>,
    target: Builtin,
    params: impl textual::VarArgs,
) -> Result<Sid> {
    w.call(&target.into_str(), params)
}
