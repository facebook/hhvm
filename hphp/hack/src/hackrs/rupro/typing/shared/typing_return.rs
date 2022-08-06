// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use pos::Symbol;
use ty::decl;
use ty::local;
use ty::reason::Reason;

use crate::typing::env::typing_env::TEnv;
use crate::typing::env::typing_return_info::TypingReturnInfo;
use crate::typing::typing_error::Result;

pub struct TypingReturn;

impl TypingReturn {
    pub fn make_default_return<R: Reason>(
        is_method: bool,
        fpos: &R::Pos,
        fname: Symbol,
    ) -> local::Ty<R> {
        let reason = R::witness(fpos.clone());
        if is_method && fname == special_names::members::__construct.as_symbol() {
            local::Ty::void(reason)
        } else {
            local::Ty::any(reason)
        }
    }

    pub fn make_return_type<R: Reason>(
        env: &TEnv<R>,
        localize: impl FnOnce(&TEnv<R>, decl::Ty<R>) -> Result<local::Ty<R>>,
        ty: decl::Ty<R>,
    ) -> Result<local::Ty<R>> {
        rupro_todo_mark!(AwaitableAsync);
        localize(env, ty)
    }

    pub fn make_info<R: Reason>(locl_ty: local::Ty<R>) -> TypingReturnInfo<R> {
        rupro_todo_mark!(Disposable);
        rupro_todo_mark!(Dynamic);
        TypingReturnInfo {
            return_type: locl_ty,
        }
    }

    pub fn fun_implicit_return<R: Reason>(_env: &TEnv<R>) {
        rupro_todo_mark!(Dynamic);
        rupro_todo_mark!(AwaitableAsync);
        rupro_todo_mark!(Generators);
        rupro_todo_mark!(InoutParameters);
        rupro_todo_mark!(SubtypeCheck);
        rupro_todo_mark!(MissingError);
    }
}
