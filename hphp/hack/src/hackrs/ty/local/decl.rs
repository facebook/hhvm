// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::TypeName;

use crate::decl::folded;
use crate::decl::Ty;
use crate::reason::Reason;

#[derive(Debug)]
pub struct ClassElt<R: Reason> {
    ty: Ty<R>,
    #[allow(dead_code)]
    origin: TypeName,
}

impl<R: Reason> ClassElt<R> {
    pub fn new(folded_elt: &folded::FoldedElement, ty: Ty<R>) -> Self {
        Self {
            ty,
            origin: folded_elt.origin,
        }
    }

    pub fn ty(&self) -> &Ty<R> {
        &self.ty
    }

    pub fn pos(&self) -> &R::Pos {
        self.ty.pos()
    }
}
