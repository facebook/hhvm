// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashMap;
use std::convert::{From, Into};

use crate::decl_defs::DeclTy;
use crate::pos::Symbol;
use crate::reason::Reason;

pub(crate) struct Subst<REASON: Reason>(HashMap<Symbol, DeclTy<REASON>>);

impl<REASON: Reason> From<HashMap<Symbol, DeclTy<REASON>>> for Subst<REASON> {
    fn from(map: HashMap<Symbol, DeclTy<REASON>>) -> Self {
        Self(map)
    }
}

impl<REASON: Reason> Into<HashMap<Symbol, DeclTy<REASON>>> for Subst<REASON> {
    fn into(self) -> HashMap<Symbol, DeclTy<REASON>> {
        self.0
    }
}

impl<REASON: Reason> Subst<REASON> {
    pub(crate) fn new(_tparams: (), _params: &[DeclTy<REASON>]) -> Self {
        Self(HashMap::new())
    }

    pub(crate) fn instantiate(&self, ty: &DeclTy<REASON>) -> DeclTy<REASON> {
        if self.0.is_empty() {
            ty.clone()
        } else {
            unimplemented!()
        }
    }
}
