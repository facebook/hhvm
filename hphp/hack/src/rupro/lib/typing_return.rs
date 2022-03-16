// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use crate::typing::Result;
use crate::typing_defs::Ty;
use crate::typing_env::TEnv;
use pos::Symbol;

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
        fname: Symbol,
    ) -> Ty<R> {
        let reason = R::witness(fpos.clone());
        if is_method && fname == env.ctx.special_names.members.__construct {
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

    fn implicit_return<R: Reason>(_env: &TEnv<R>, _pos: R::Pos, _expected: Ty<R>, _actual: Ty<R>) {
        // TODO(hrust): sound dynamic
        // TODO(hrust, mjt): subtype
    }

    pub fn fun_implicit_return<R: Reason>(
        env: &TEnv<R>,
        pos: R::Pos,
        ret: Ty<R>,
        fun_kind: &oxidized::ast_defs::FunKind,
    ) {
        // TODO(hrust): sound dynamic
        use oxidized::ast_defs::FunKind;
        match fun_kind {
            FunKind::FSync => {
                // TODO(hrust): check_inout_return
                let rty = Ty::void(R::no_return(pos.clone()));
                Self::implicit_return(env, pos, rty, ret);
            }
            _ => unimplemented!(),
        }
    }
}

impl<R: Reason> TypingReturnInfo<R> {
    #[allow(unused_variables)]
    pub fn placeholder() -> Self {
        // TODO(hrust): Tunion []
        Self {
            return_type: Ty::any(R::none()),
        }
    }

    pub fn return_type(&self) -> Ty<R> {
        self.return_type.clone()
    }
}
