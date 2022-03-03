// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, DeclTy_};
use crate::reason::Reason;
use std::marker::PhantomData;

pub struct DeclHintEnv<R: Reason> {
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclHintEnv<R> {
    pub fn new() -> Self {
        Self {
            _phantom: PhantomData,
        }
    }

    fn mk_hint_decl_ty(&self, pos: &oxidized::pos::Pos, ty: DeclTy_<R>) -> DeclTy<R> {
        DeclTy::new(R::hint(pos.into()), ty)
    }

    fn hint_(&self, _pos: &oxidized::pos::Pos, hint: &oxidized::aast_defs::Hint_) -> DeclTy_<R> {
        use oxidized::aast_defs::Hint_ as OH;
        match hint {
            OH::Happly(id, argl) => {
                let argl = argl.iter().map(|arg| self.hint(arg)).collect();
                DeclTy_::DTapply(Box::new((id.into(), argl)))
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
