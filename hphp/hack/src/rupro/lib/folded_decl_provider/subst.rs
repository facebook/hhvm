// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::DeclTy;
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
    pub(crate) fn new(_tparams: (), _params: &[DeclTy<R>]) -> Self {
        Self(Default::default())
    }

    pub(crate) fn instantiate(&self, ty: &DeclTy<R>) -> DeclTy<R> {
        if self.0.is_empty() {
            ty.clone()
        } else {
            unimplemented!()
        }
    }
}
