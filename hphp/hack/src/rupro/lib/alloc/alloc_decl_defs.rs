// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, DeclTy_, FunParam, FunType, Prim, Tparam, UserAttribute};
use crate::reason::Reason;

use super::Allocator;

impl<R: Reason> Allocator<R> {
    pub fn tparam(&self, tparam: &oxidized_by_ref::typing_defs::Tparam<'_>) -> Tparam<R> {
        Tparam {
            variance: tparam.variance,
            name: self.pos_id_from_decl(tparam.name),
            tparams: self.tparams(&tparam.tparams),
            constraints: tparam
                .constraints
                .iter()
                .map(|&(k, ty)| (k, self.decl_ty_from_ast(ty)))
                .collect(),
            reified: tparam.reified,
            user_attributes: tparam
                .user_attributes
                .iter()
                .map(|ua| self.user_attribute(ua))
                .collect(),
        }
    }

    pub fn tparams(&self, tparams: &[&oxidized_by_ref::typing_defs::Tparam<'_>]) -> Vec<Tparam<R>> {
        tparams.iter().map(|tp| self.tparam(tp)).collect()
    }

    pub fn user_attribute(
        &self,
        attr: &oxidized_by_ref::typing_defs::UserAttribute<'_>,
    ) -> UserAttribute<R> {
        UserAttribute {
            name: self.pos_id_from_decl(attr.name),
            classname_params: attr
                .classname_params
                .iter()
                .map(|param| self.symbol(param))
                .collect(),
        }
    }

    pub fn decl_ty(&self, reason: R, ty: DeclTy_<R, DeclTy<R>>) -> DeclTy<R> {
        DeclTy::new(reason, self.decl_tys.mk(ty))
    }

    fn prim_from_ast(prim: &oxidized_by_ref::aast_defs::Tprim) -> Prim {
        use oxidized::aast::Tprim::*;
        use oxidized_by_ref::aast_defs::Tprim as AP;
        match prim {
            AP::Tint => Tint,
            AP::Tvoid => Tvoid,
            prim => unimplemented!("prim_from_ast: {:?}", prim),
        }
    }

    pub fn possibly_enforced_decl_ty_from_ast(
        &self,
        ty: &oxidized_by_ref::typing_defs_core::PossiblyEnforcedTy<'_>,
    ) -> DeclTy<R> {
        self.decl_ty_from_ast(ty.type_)
    }

    pub fn decl_fun_param(
        &self,
        fp: &oxidized_by_ref::typing_defs_core::FunParam<'_>,
    ) -> FunParam<R, DeclTy<R>> {
        FunParam {
            pos: self.pos_from_decl(fp.pos),
            name: fp.name.map(|name| self.symbol(name)),
            ty: self.possibly_enforced_decl_ty_from_ast(fp.type_),
        }
    }

    pub fn decl_fun_type(
        &self,
        ft: &oxidized_by_ref::typing_defs_core::FunType<'_>,
    ) -> FunType<R, DeclTy<R>> {
        FunType {
            params: ft.params.iter().map(|fp| self.decl_fun_param(fp)).collect(),
            ret: self.possibly_enforced_decl_ty_from_ast(ft.ret),
        }
    }

    pub fn decl_ty_from_ast(&self, ty: &oxidized_by_ref::typing_defs_core::Ty<'_>) -> DeclTy<R> {
        use oxidized_by_ref::typing_defs_core::Ty_ as OT;
        use DeclTy_::*;
        let reason = self.reason(ty.0);
        let ty = match ty.1 {
            OT::Tprim(prim) => DTprim(Self::prim_from_ast(prim)),
            OT::Tapply(&(pos_id, tys)) => DTapply(
                self.pos_id_from_decl(pos_id),
                tys.iter().map(|ty| self.decl_ty_from_ast(ty)).collect(),
            ),
            OT::Tfun(ft) => DTfun(self.decl_fun_type(ft)),
            ty => unimplemented!("decl_ty_from_ast: {:?}", ty),
        };

        self.decl_ty(reason, ty)
    }
}
