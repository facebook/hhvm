// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use hcons::Conser;

use crate::decl_defs::{DeclTy, DeclTy_, FunParam, FunType, Prim};
use crate::pos_provider::PosProvider;
use crate::reason::Reason;

#[derive(Debug)]
pub struct DeclTyProvider<R: Reason> {
    pos_provider: Rc<PosProvider>,
    decl_tys: Conser<DeclTy_<R, DeclTy<R>>>,
}

impl<R: Reason> DeclTyProvider<R> {
    pub fn new(pos_provider: Rc<PosProvider>) -> Self {
        Self {
            pos_provider,

            decl_tys: Conser::new(),
        }
    }

    pub fn get_pos_provider(&self) -> &Rc<PosProvider> {
        &self.pos_provider
    }

    pub fn mk_decl_ty(&self, reason: R, ty: DeclTy_<R, DeclTy<R>>) -> DeclTy<R> {
        DeclTy::new(reason, self.decl_tys.mk(ty))
    }

    fn mk_aast_prim_from_parsed(prim: &oxidized_by_ref::aast_defs::Tprim) -> Prim {
        use oxidized::aast::Tprim::*;
        use oxidized_by_ref::aast_defs::Tprim as AP;
        match prim {
            AP::Tint => Tint,
            AP::Tvoid => Tvoid,
            prim => unimplemented!("mk_aast_prim_from_parsed: {:?}", prim),
        }
    }

    pub fn mk_possibly_enforced_decl_ty_from_parsed(
        &self,
        ty: &oxidized_by_ref::typing_defs_core::PossiblyEnforcedTy<'_>,
    ) -> DeclTy<R> {
        self.mk_decl_ty_from_parsed(ty.type_)
    }

    pub fn mk_decl_fun_param(
        &self,
        fp: &oxidized_by_ref::typing_defs_core::FunParam<'_>,
    ) -> FunParam<R, DeclTy<R>> {
        FunParam {
            fp_pos: self.get_pos_provider().mk_pos_of_ref::<R>(fp.pos),
            fp_name: fp.name.map(|name| self.get_pos_provider().mk_symbol(name)),
            fp_type: self.mk_possibly_enforced_decl_ty_from_parsed(fp.type_),
        }
    }

    pub fn mk_decl_fun_type(
        &self,
        ft: &oxidized_by_ref::typing_defs_core::FunType<'_>,
    ) -> FunType<R, DeclTy<R>> {
        FunType {
            ft_params: ft
                .params
                .iter()
                .map(|fp| self.mk_decl_fun_param(fp))
                .collect(),
            ft_ret: self.mk_possibly_enforced_decl_ty_from_parsed(ft.ret),
        }
    }

    pub fn mk_decl_ty_from_parsed(
        &self,
        ty: &oxidized_by_ref::typing_defs_core::Ty<'_>,
    ) -> DeclTy<R> {
        let reason = self.get_pos_provider().mk_reason(ty.0);

        use oxidized_by_ref::typing_defs_core::Ty_ as OT;
        use DeclTy_::*;
        let ty = match ty.1 {
            OT::Tprim(prim) => DTprim(Self::mk_aast_prim_from_parsed(prim)),
            OT::Tapply(&(pos_id, tys)) => DTapply(
                self.get_pos_provider().mk_pos_id_of_ref::<R>(pos_id),
                tys.iter()
                    .map(|ty| self.mk_decl_ty_from_parsed(ty))
                    .collect(),
            ),
            OT::Tfun(ft) => DTfun(self.mk_decl_fun_type(ft)),
            ty => unimplemented!("mk_decl_ty_from_parsed: {:?}", ty),
        };

        self.mk_decl_ty(reason, ty)
    }
}
