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

pub(crate) enum Builtin {
    Add,
    AddO,
    ArgPack(usize),
    Bool,
    CmpEq,
    CmpGt,
    CmpGte,
    CmpLt,
    CmpLte,
    CmpNSame,
    CmpNeq,
    CmpSame,
    Copy,
    Int,
    IsInt,
    Modulo,
    Not,
    Null,
    Print,
    Sub,
    SubO,
    VerifyFailed,
}

impl Builtin {
    pub(crate) fn into_str(&self) -> Cow<'static, str> {
        match self {
            Builtin::Add => Cow::Borrowed("hack_add"),
            Builtin::AddO => Cow::Borrowed("hack_add_o"),
            Builtin::ArgPack(n) => Cow::Owned(format!("arg_pack_{}", n)),
            Builtin::Bool => Cow::Borrowed("hack_bool"),
            Builtin::CmpEq => Cow::Borrowed("hack_cmp_eq"),
            Builtin::CmpGt => Cow::Borrowed("hack_cmp_gt"),
            Builtin::CmpGte => Cow::Borrowed("hack_cmp_gte"),
            Builtin::CmpLt => Cow::Borrowed("hack_cmp_lt"),
            Builtin::CmpLte => Cow::Borrowed("hack_cmp_lte"),
            Builtin::CmpNSame => Cow::Borrowed("hack_cmp_nsame"),
            Builtin::CmpNeq => Cow::Borrowed("hack_cmp_neq"),
            Builtin::CmpSame => Cow::Borrowed("hack_cmp_same"),
            Builtin::Copy => Cow::Borrowed("copy"),
            Builtin::Int => Cow::Borrowed("hack_int"),
            Builtin::IsInt => Cow::Borrowed("hack_is_int"),
            Builtin::Modulo => Cow::Borrowed("hack_modulo"),
            Builtin::Not => Cow::Borrowed("hack_not"),
            Builtin::Null => Cow::Borrowed("hack_null"),
            Builtin::Print => Cow::Borrowed("hack_print"),
            Builtin::Sub => Cow::Borrowed("hack_sub"),
            Builtin::SubO => Cow::Borrowed("hack_sub_o"),
            Builtin::VerifyFailed => Cow::Borrowed("hack_verify_failed"),
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
