// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]
use crate::decl_defs::DeclTy;
use crate::pos::Symbol;
use crate::reason::{Reason, ReasonImpl};
use crate::typing_defs::Ty;
use crate::typing_env::TEnv;
use crate::typing_ty_provider::TypingTyProvider;

pub struct TypingReturn;

#[derive(Debug, Clone)]
pub struct TypingReturnInfo<R: Reason> {
    return_type: Ty<R>,
}

impl TypingReturn {
    pub fn make_default_return<R: Reason>(
        env: &TEnv<R>,
        is_method: bool,
        fpos: &R::Pos,
        fname: &Symbol,
    ) -> Ty<R> {
        let reason = R::mk(&|| ReasonImpl::Rwitness(fpos.clone()));
        if is_method && fname == env.ctx.special_names.construct() {
            env.ctx.typing_ty_provider.mk_ty_void(reason)
        } else {
            env.ctx.typing_ty_provider.mk_tany(reason)
        }
    }

    pub fn make_return_type<R: Reason>(
        env: &TEnv<R>,
        localize: &dyn Fn(&TEnv<R>, DeclTy<R>) -> Ty<R>,
        ty: DeclTy<R>,
    ) -> Ty<R> {
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
}

impl<R: Reason> TypingReturnInfo<R> {
    pub fn placeholder(typing_ty_provider: &TypingTyProvider<R>) -> Self {
        // TODO(hrust): Tunion []
        Self {
            return_type: typing_ty_provider.mk_tany(R::mk(&|| ReasonImpl::Rnone)),
        }
    }
}
