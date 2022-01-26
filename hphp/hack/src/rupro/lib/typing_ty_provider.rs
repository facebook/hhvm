// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::hcons::Conser;
use crate::reason::Reason;
use crate::typing_defs::{Prim, Ty, Ty_};

#[derive(Debug)]
pub struct TypingTyProvider<R: Reason> {
    typing_tys: Conser<Ty_<R, Ty<R>>>,
}

impl<R: Reason> TypingTyProvider<R> {
    pub fn new() -> Self {
        Self {
            typing_tys: Conser::new(),
        }
    }

    pub fn mk_ty(&self, reason: R, ty: Ty_<R, Ty<R>>) -> Ty<R> {
        Ty::new(reason, self.typing_tys.mk(ty))
    }

    pub fn mk_ty_prim(&self, r: R, prim: Prim) -> Ty<R> {
        self.mk_ty(r, Ty_::Tprim(prim))
    }

    pub fn mk_ty_void(&self, r: R) -> Ty<R> {
        self.mk_ty_prim(r, Prim::Tvoid)
    }

    pub fn mk_tany(&self, r: R) -> Ty<R> {
        self.mk_ty(r, Ty_::Tany)
    }
}
