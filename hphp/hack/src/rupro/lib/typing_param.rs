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

pub struct TypingParamFlags {
    pub dynamic_mode: bool,
}

impl TypingParam {
    pub fn make_param_local_tys<'a, R: Reason>(
        flags: TypingParamFlags,
        env: &TEnv<R>,
        tys: impl Iterator<Item = (&'a oxidized::aast::FunParam<(), ()>, Option<DeclTy<R>>)>,
    ) -> Result<Vec<(&'a oxidized::aast::FunParam<(), ()>, Ty<R>)>> {
        tys.map(|(param, decl_hint)| {
            assert!(!flags.dynamic_mode);
            let ty = Self::make_param_local_ty(&flags, env, decl_hint, param)?;
            Ok((param, ty))
        })
        .collect()
    }

    fn make_param_local_ty<R: Reason>(
        flags: &TypingParamFlags,
        env: &TEnv<R>,
        decl_hint: Option<DeclTy<R>>,
        param: &oxidized::aast::FunParam<(), ()>,
    ) -> Result<Ty<R>> {
        let r = R::witness(R::Pos::from(&param.pos));
        let ty = match decl_hint {
            None => Ty::any(r),
            Some(ty) => {
                // TODO(hrust): enforceability
                // TODO(hrust): variadic
                Phase::localize_no_subst(env, false, None, ty)?
            }
        };
        // TODO(hrust): integrate check_param_has_hint here
        if !flags.dynamic_mode {
            Self::check_param_has_hint(env, param, &ty);
        }
        Ok(ty)
    }

    fn check_param_has_hint<R: Reason>(
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
