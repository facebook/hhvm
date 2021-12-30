// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use instruction_sequence::Result;
use oxidized::{ast, ast_defs::ParamKind};

pub fn expect_normal_paramkind(arg: &(ParamKind, ast::Expr)) -> Result<&ast::Expr> {
    match arg {
        (ParamKind::Pnormal, e) => Ok(e),
        (ParamKind::Pinout(pk_pos), _) => Err(emit_fatal::raise_fatal_parse(
            pk_pos,
            "Unexpected `inout` annotation",
        )),
    }
}

pub fn ensure_normal_paramkind(pk: &ParamKind) -> Result<()> {
    match pk {
        ParamKind::Pnormal => Ok(()),
        ParamKind::Pinout(p) => Err(emit_fatal::raise_fatal_parse(
            p,
            "Unexpected `inout` annotation",
        )),
    }
}
