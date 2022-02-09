// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::{folded, DeclTy};
use crate::reason::Reason;
use pos::TypeName;

#[derive(Debug)]
pub struct ClassElt<R: Reason> {
    ty: DeclTy<R>,
    #[allow(dead_code)]
    origin: TypeName,
}

impl<R: Reason> ClassElt<R> {
    pub fn new(folded_elt: &folded::FoldedElement<R>, ty: DeclTy<R>) -> Self {
        Self {
            ty,
            origin: folded_elt.origin,
        }
    }

    pub fn ty(&self) -> &DeclTy<R> {
        &self.ty
    }

    pub fn pos(&self) -> &R::Pos {
        self.ty.pos()
    }
}
