// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{self, DeclTy, DeclTy_};
use crate::folded_decl_provider::DeclName;
use crate::reason::Reason;
use crate::typing::Result;
use crate::typing_decl_provider::{Class, TypeDecl};
use crate::typing_defs::{
    Exact, ExpandEnv, FunParam, FunType, Ty, Ty_, TypeExpansion, TypeExpansions,
};
use crate::typing_env::TEnv;
use crate::typing_error::ReasonsCallback;
use pos::{Positioned, TypeName};

pub struct Phase;

impl Phase {
    fn localize<R: Reason>(
        env: &TEnv<R>,
        ety_env: &mut ExpandEnv<'_, R>,
        ty: DeclTy<R>,
    ) -> Result<Ty<R>> {
        use DeclTy_::*;
        use Ty_::*;
        let r = ty.reason().clone();
        Ok(match &**ty.node() {
            DTprim(p) => Ty::prim(r, *p),
            DTapply(id_and_args) => {
                let (pos_id, tyl) = &**id_and_args;
                match env
                    .ctx
                    .typing_decl_provider
                    .get_type(DeclName::NoDeclName, pos_id.id())?
                {
                    Some(TypeDecl::Class(cls)) => Self::localize_class_instantiation(
                        env,
                        ety_env,
                        r,
                        pos_id.clone(),
                        tyl,
                        Some(&*cls),
                    )?,
                    _ => Self::localize_class_instantiation(
                        env,
                        ety_env,
                        r,
                        pos_id.clone(),
                        tyl,
                        None,
                    )?,
                }
            }
            DTfun(ft) => {
                let pos = r.pos().clone();
                let ft = Self::localize_ft(env, ety_env, pos, ft)?;
                Ty::new(r, Tfun(ft))
            }
            _ => todo!(),
        })
    }

    fn localize_class_instantiation<R: Reason>(
        env: &TEnv<R>,
        ety_env: &mut ExpandEnv<'_, R>,
        r: R,
        sid: Positioned<TypeName, R::Pos>,
        ty_args: &[DeclTy<R>],
        class_info: Option<&dyn Class<R>>,
    ) -> Result<Ty<R>> {
        use Ty_::*;
        Ok(match class_info {
            None => {
                // Without class info, we don't know the kinds of the arguments.
                // We assume they are non-HK types.
                //
                // TODO(hrust): this is not semantically equivalent to the
                // ocaml code. The ocaml code uses a copy of the ety_env
                let tyl = ety_env.with_expand_visible_newtype(|ety_env| {
                    ty_args
                        .iter()
                        .map(|ty| Self::localize(env, ety_env, ty.clone()))
                        .collect::<Result<Vec<_>>>()
                })?;
                Ty::new(r, Tclass(sid, Exact::Nonexact, tyl))
            }
            Some(_class_info) => {
                // TODO(hrust): enum_type
                // TODO(hrust): tparams
                assert!(ty_args.is_empty());
                Ty::new(r, Tclass(sid, Exact::Nonexact, vec![]))
            }
        })
    }

    fn localize_ft<R: Reason>(
        env: &TEnv<R>,
        ety_env: &mut ExpandEnv<'_, R>,
        _def_pos: R::Pos,
        ft: &decl_defs::ty::FunType<R, DeclTy<R>>,
    ) -> Result<FunType<R>> {
        // TODO(hrust): tparams
        assert!(ft.params.is_empty());
        let params = ft
            .params
            .iter()
            .map(|fp| {
                Ok(FunParam {
                    pos: fp.pos.clone(),
                    name: fp.name,
                    ty: Self::localize_possibly_enforced_ty(env, ety_env, fp.ty.ty.clone())?,
                })
            })
            .collect::<Result<Vec<_>>>()?;
        let ret = Self::localize_possibly_enforced_ty(env, ety_env, ft.ret.ty.clone())?;
        Ok(FunType { params, ret })
    }

    fn localize_possibly_enforced_ty<R: Reason>(
        env: &TEnv<R>,
        ety_env: &mut ExpandEnv<'_, R>,
        ty: DeclTy<R>,
    ) -> Result<Ty<R>> {
        Self::localize(env, ety_env, ty)
    }

    pub fn localize_no_subst<R: Reason>(
        env: &TEnv<R>,
        ignore_errors: bool,
        report_cycle: Option<TypeExpansion<R>>,
        ty: DeclTy<R>,
    ) -> Result<Ty<R>> {
        let ty_clone = ty.clone();
        let on_error = || {
            if ignore_errors {
                ReasonsCallback::ignore()
            } else {
                ReasonsCallback::invalid_type_hint(ty_clone.pos().clone())
            }
        };
        let mut ety_env = ExpandEnv::new(&env.ctx);
        ety_env
            .set_type_expansions(TypeExpansions::with_report_cycle(report_cycle))
            .set_on_error(ReasonsCallback::<R>::new(&on_error));
        Self::localize(env, &mut ety_env, ty)
    }
}
