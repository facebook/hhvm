// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::decl_defs::{DeclTy, DeclTy_};
use crate::decl_ty_provider::DeclTyProvider;
use crate::reason::{Reason, ReasonImpl};

pub struct DeclHintEnv<R: Reason> {
    decl_ty_provider: Rc<DeclTyProvider<R>>,
}

impl<R: Reason> DeclHintEnv<R> {
    pub fn new(decl_ty_provider: Rc<DeclTyProvider<R>>) -> Self {
        Self { decl_ty_provider }
    }

    fn mk_hint_decl_ty(&self, pos: &oxidized::pos::Pos, ty: &DeclTy_<R, DeclTy<R>>) -> DeclTy<R> {
        let reason = R::mk(&|| {
            let pos = self.decl_ty_provider.get_pos_provider().mk_pos::<R>(pos);
            ReasonImpl::Rhint(pos)
        });
        self.decl_ty_provider.mk_decl_ty(reason, ty)
    }

    fn hint_(
        &self,
        _pos: &oxidized::pos::Pos,
        hint: &oxidized::aast_defs::Hint_,
    ) -> DeclTy_<R, DeclTy<R>> {
        use oxidized::aast_defs::Hint_ as OH;
        match hint {
            OH::Happly(id, argl) => {
                let id = self.decl_ty_provider.get_pos_provider().mk_pos_id::<R>(id);
                let argl = argl.iter().map(|arg| self.hint(arg)).collect();
                DeclTy_::DTapply(id, argl)
            }
            OH::Hprim(p) => DeclTy_::DTprim(*p),
            h => unimplemented!("hint_: {:?}", h),
        }
    }

    pub fn hint(&self, hint: &oxidized::aast_defs::Hint) -> DeclTy<R> {
        let ty = self.hint_(&hint.0, &hint.1);
        self.mk_hint_decl_ty(&hint.0, &ty)
    }
}
