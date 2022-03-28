// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use crate::special_names;
use crate::typing::env::typing_env::TEnv;
use crate::typing::env::typing_return_info::TypingReturnInfo;
use crate::typing::typing_error::Result;
use crate::typing_defs::Ty;
use pos::Symbol;

pub struct TypingReturn;

impl TypingReturn {
    pub fn make_default_return<R: Reason>(is_method: bool, fpos: &R::Pos, fname: Symbol) -> Ty<R> {
        let reason = R::witness(fpos.clone());
        if is_method && fname == special_names::members::__construct.as_symbol() {
            Ty::void(reason)
        } else {
            Ty::any(reason)
        }
    }

    pub fn make_return_type<R: Reason>(
        env: &TEnv<R>,
        localize: impl FnOnce(&TEnv<R>, DeclTy<R>) -> Result<Ty<R>>,
        ty: DeclTy<R>,
    ) -> Result<Ty<R>> {
        // TODO(hrust): async
        localize(env, ty)
    }

    pub fn make_info<R: Reason>(
        _env: &TEnv<R>,
        _ret_pos: R::Pos,
        _fun_kind: &oxidized::ast_defs::FunKind,
        _attributes: &[oxidized::aast::UserAttribute<(), ()>],
        _is_explicit: bool,
        locl_ty: Ty<R>,
        _decl_ty: Option<DeclTy<R>>,
    ) -> TypingReturnInfo<R> {
        // TODO(hrust): everything
        TypingReturnInfo {
            return_type: locl_ty,
        }
    }

    pub fn fun_implicit_return<R: Reason>(_env: &TEnv<R>) {
        rupro_todo_mark!(Dynamic);
        rupro_todo_mark!(Awaitable);
        rupro_todo_mark!(Generators);
        rupro_todo_mark!(InoutParameters);
        rupro_todo_mark!(SubtypeCheck);
        rupro_todo_mark!(MissingError);
    }
}
