// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::Reason;
use crate::typing_defs::{Prim, Ty, Ty_};

use super::Allocator;

impl<R: Reason> Allocator<R> {
    pub fn ty(&self, reason: R, ty: Ty_<R, Ty<R>>) -> Ty<R> {
        Ty::new(reason, R::cons_ty(ty))
    }

    pub fn ty_prim(&self, r: R, prim: Prim) -> Ty<R> {
        self.ty(r, Ty_::Tprim(prim))
    }

    pub fn ty_void(&self, r: R) -> Ty<R> {
        self.ty_prim(r, Prim::Tvoid)
    }

    pub fn tany(&self, r: R) -> Ty<R> {
        self.ty(r, Ty_::Tany)
    }
}
