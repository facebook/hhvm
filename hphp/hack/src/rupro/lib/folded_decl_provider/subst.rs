// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use pos::Symbol;
use std::collections::HashMap;

pub(crate) struct Subst<R: Reason>(HashMap<Symbol, DeclTy<R>>);

impl<R: Reason> From<HashMap<Symbol, DeclTy<R>>> for Subst<R> {
    fn from(map: HashMap<Symbol, DeclTy<R>>) -> Self {
        Self(map)
    }
}

impl<R: Reason> Into<HashMap<Symbol, DeclTy<R>>> for Subst<R> {
    fn into(self) -> HashMap<Symbol, DeclTy<R>> {
        self.0
    }
}

impl<R: Reason> Subst<R> {
    pub(crate) fn new(_tparams: (), _params: &[DeclTy<R>]) -> Self {
        Self(HashMap::new())
    }

    pub(crate) fn instantiate(&self, ty: &DeclTy<R>) -> DeclTy<R> {
        if self.0.is_empty() {
            ty.clone()
        } else {
            unimplemented!()
        }
    }
}
