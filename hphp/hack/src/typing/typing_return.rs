// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::ast;
use typing_ast_rust::typing_subtype;
use typing_ast_rust::Env;
use typing_ast_rust::Genv;
use typing_defs_rust::typing_reason::*;
use typing_defs_rust::{tast, Ty};

pub fn strip_awaitable<'a>(env: &'a Env<'a>, ty: Ty<'a>) -> Ty<'a> {
    ty
}
pub fn implicit_return<'a>(env: &'a Env<'a>, pos: Pos, expected: Ty<'a>, actual: Ty<'a>) {
    sub_type(env, expected, actual);
}
