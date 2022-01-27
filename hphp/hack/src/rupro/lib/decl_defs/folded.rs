// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashMap;

use crate::decl_defs::{CeVisibility, DeclTy};
use crate::pos::Symbol;
use crate::reason::Reason;

#[derive(Debug, Clone)]
pub struct FoldedElement<R: Reason> {
    pub origin: Symbol,
    pub visibility: CeVisibility<R>,
}

#[derive(Debug, Clone)]
pub struct SubstContext<R: Reason> {
    pub subst: HashMap<Symbol, DeclTy<R>>,
    pub class_context: Symbol,
}

#[derive(Debug, Clone)]
pub struct FoldedClass<R: Reason> {
    pub name: Symbol,
    pub pos: R::Pos,
    pub substs: HashMap<Symbol, SubstContext<R>>,
    pub ancestors: HashMap<Symbol, DeclTy<R>>,
    pub methods: HashMap<Symbol, FoldedElement<R>>,
}
