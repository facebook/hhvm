// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::{DeclTy, DeclTy_, Tparam};
use crate::reason::Reason;
use pos::TypeNameMap;

pub(crate) struct Subst<R: Reason>(TypeNameMap<DeclTy<R>>);

impl<R: Reason> From<TypeNameMap<DeclTy<R>>> for Subst<R> {
    fn from(map: TypeNameMap<DeclTy<R>>) -> Self {
        Self(map)
    }
}

impl<R: Reason> From<Subst<R>> for TypeNameMap<DeclTy<R>> {
    fn from(subst: Subst<R>) -> Self {
        subst.0
    }
}

impl<R: Reason> Subst<R> {
    pub(crate) fn new(
        alloc: &Allocator<R>,
        tparams: &[Tparam<R, DeclTy<R>>],
        targs: &[DeclTy<R>],
    ) -> Self {
        // If there are fewer type arguments than type parameters, we'll have
        // emitted an error elsewhere. We bind missing types to `Tany` (rather
        // than `Terr`) here to keep parity with the OCaml implementation, which
        // produces `Tany` because of a now-dead feature called "silent_mode".
        let targs = targs
            .iter()
            .cloned()
            .chain(std::iter::repeat(alloc.decl_ty(R::none(), DeclTy_::DTany)));
        Self(
            tparams
                .iter()
                .map(|tparam| tparam.name.id())
                .zip(targs)
                .collect(),
        )
    }

    pub(crate) fn instantiate(&self, ty: &DeclTy<R>) -> DeclTy<R> {
        if self.0.is_empty() {
            ty.clone()
        } else {
            unimplemented!()
        }
    }
}
