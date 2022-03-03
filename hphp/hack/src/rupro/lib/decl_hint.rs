// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::{DeclTy, DeclTy_};
use crate::reason::Reason;

pub struct DeclHintEnv<R: Reason> {
    alloc: &'static Allocator<R>,
}

impl<R: Reason> DeclHintEnv<R> {
    pub fn new(alloc: &'static Allocator<R>) -> Self {
        Self { alloc }
    }

    fn mk_hint_decl_ty(&self, pos: &oxidized::pos::Pos, ty: DeclTy_<R>) -> DeclTy<R> {
        let reason = R::hint(self.alloc.pos_from_ast(pos));
        DeclTy::new(reason, ty)
    }

    fn hint_(&self, _pos: &oxidized::pos::Pos, hint: &oxidized::aast_defs::Hint_) -> DeclTy_<R> {
        use oxidized::aast_defs::Hint_ as OH;
        match hint {
            OH::Happly(id, argl) => {
                let id = self.alloc.pos_type_from_ast(id);
                let argl = argl.iter().map(|arg| self.hint(arg)).collect();
                DeclTy_::DTapply(Box::new((id, argl)))
            }
            OH::Hprim(p) => DeclTy_::DTprim(*p),
            h => unimplemented!("hint_: {:?}", h),
        }
    }

    pub fn hint(&self, hint: &oxidized::aast_defs::Hint) -> DeclTy<R> {
        let ty = self.hint_(&hint.0, &hint.1);
        self.mk_hint_decl_ty(&hint.0, ty)
    }
}
