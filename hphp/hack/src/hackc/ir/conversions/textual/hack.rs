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
use strum::EnumProperty as _;
use strum_macros::EnumIter;
use strum_macros::EnumProperty;

use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// These represent builtins for handling HHVM bytecode instructions. In general
/// the names should match the HHBC name except when they are compound bytecodes
/// (like Cmp with a parameter of Eq becoming CmpEq). Documentation can be found
/// in hphp/doc/bytecode.specification.
#[derive(Copy, Clone, EnumIter, EnumProperty)]
pub(crate) enum Hhbc {
    #[strum(props(Function = "hhbc_add"))]
    Add,
    #[strum(props(Function = "hhbc_add_o"))]
    AddO,
    #[strum(props(Function = "hhbc_cmp_eq"))]
    CmpEq,
    #[strum(props(Function = "hhbc_cmp_gt"))]
    CmpGt,
    #[strum(props(Function = "hhbc_cmp_gte"))]
    CmpGte,
    #[strum(props(Function = "hhbc_cmp_lt"))]
    CmpLt,
    #[strum(props(Function = "hhbc_cmp_lte"))]
    CmpLte,
    #[strum(props(Function = "hhbc_cmp_nsame"))]
    CmpNSame,
    #[strum(props(Function = "hhbc_cmp_neq"))]
    CmpNeq,
    #[strum(props(Function = "hhbc_cmp_same"))]
    CmpSame,
    #[strum(props(Function = "hhbc_is_type_int"))]
    IsTypeInt,
    #[strum(props(Function = "hhbc_modulo"))]
    Modulo,
    #[strum(props(Function = "hhbc_not"))]
    Not,
    #[strum(props(Function = "hhbc_print"))]
    Print,
    #[strum(props(Function = "hhbc_sub"))]
    Sub,
    #[strum(props(Function = "hhbc_sub_o"))]
    SubO,
    #[strum(props(Function = "hhbc_verify_failed"))]
    VerifyFailed,
}

// Need Default for EnumIter on Builtin
impl std::default::Default for Hhbc {
    fn default() -> Self {
        Hhbc::Add
    }
}

#[derive(EnumIter, EnumProperty)]
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
    #[strum(props(Function = "hack_bool"))]
    Bool,
    /// Hhbc handlers.  See hphp/doc/bytecode.specification for docs.
    Hhbc(Hhbc),
    /// Turns a raw int into a Mixed.
    ///   Int(n: int) -> *Mixed
    #[strum(props(Function = "hack_int"))]
    Int,
    /// Returns a Mixed containing a `null`.
    ///   Null() -> *Mixed
    #[strum(props(Function = "hack_null"))]
    Null,
    /// Turns a string into a Mixed.
    ///   String(s: *string) -> *Mixed
    #[strum(props(Function = "hack_string"))]
    String,
}

impl Builtin {
    pub(crate) fn into_str(&self) -> Cow<'static, str> {
        match self {
            Builtin::Hhbc(hhbc) => Cow::Borrowed(hhbc.get_str("Function").unwrap()),
            Builtin::ArgPack(n) => Cow::Owned(format!("arg_pack_{}", n)),
            _ => Cow::Borrowed(self.get_str("Function").unwrap()),
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
