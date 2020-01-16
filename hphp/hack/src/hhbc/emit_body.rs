// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_imports)]

mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use ast_scope_rust::{Scope, ScopeItem};
use env::{emitter::Emitter, Env};
use hhas_body_rust::HhasBody;
use instruction_sequence_rust::InstrSeq;
use oxidized::{aast, ast as tast, ast_defs, namespace_env, pos::Pos};
use runtime::TypedValue;

extern crate bitflags;

use bitflags::bitflags;

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a> {
    pub immediate_tparams: &'a Vec<tast::Tparam>,
    pub ast_params: &'a Vec<tast::FunParam>,
    pub ret: Option<aast::Hint>,
    pub scope: &'a Scope<'a>,
    pub pos: &'a Pos,
    pub deprecation_info: &'a Option<&'a [TypedValue]>,
    pub doc_comment: Option<&'a str>,
    pub default_dropthrough: Option<InstrSeq>,
    pub flags: Flags,
}
impl Args<'_> {
    pub fn with_default<F, T>(f: F) -> T
    where
        F: FnOnce(Args) -> T,
    {
        let args = Args {
            immediate_tparams: &vec![],
            ast_params: &vec![],
            ret: None,
            scope: &Scope::toplevel(),
            pos: &Pos::make_none(),
            deprecation_info: &None,
            doc_comment: None,
            default_dropthrough: None,
            flags: Flags::empty(),
        };
        f(args)
    }
}

bitflags! {
    pub struct Flags: u8 {
        const SKIP_AWAITABLE = 1 << 1;
        const MEMOIZE = 1 << 2;
        const CLOSURE_BODY = 1 << 3;
        const NATIVE = 1 << 4;
        const RX_BODY = 1 << 5;
        const ASYNC = 1 << 6;
        const DEBUGGER_MODIFY_PROGRAM = 1 << 7;
    }
}

pub fn emit_body(
    _emitter: &mut Emitter,
    _namespace: &namespace_env::Env,
    _return_value: InstrSeq,
    _body: &tast::Program,
    _args: Args,
) -> (HhasBody, bool, bool) {
    unimplemented!("TODO(hrust) finish porting")
}
