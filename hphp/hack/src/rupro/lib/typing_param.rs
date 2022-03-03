// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use crate::typing::Result;
use crate::typing_defs::Ty;
use crate::typing_env::TEnv;
use crate::typing_error::{Primary, TypingError};
use crate::typing_phase::Phase;

pub struct TypingParam;

impl TypingParam {
    pub fn make_param_local_ty<R: Reason>(
        env: &TEnv<R>,
        decl_hint: Option<DeclTy<R>>,
        param: &oxidized::aast::FunParam<(), ()>,
    ) -> Result<Ty<R>> {
        let r = R::witness(R::Pos::from(&param.pos));
        Ok(match decl_hint {
            None => Ty::any(r),
            Some(ty) => {
                // TODO(hrust): enforceability
                // TODO(hrust): variadic
                Phase::localize_no_subst(env, false, None, ty)?
            }
        })
    }

    pub fn check_param_has_hint<R: Reason>(
        env: &TEnv<R>,
        param: &oxidized::aast::FunParam<(), ()>,
        _ty: &Ty<R>,
    ) {
        // TODO(hrust): variadic+hhi
        let prim_err = match param.type_hint.1 {
            None => Some(Primary::<R>::ExpectingTypeHint(R::Pos::from(&param.pos))),
            Some(_) => None,
        };
        match prim_err {
            None => {}
            Some(prim_err) => env.add_error(TypingError::primary(prim_err)),
        }
    }
}
