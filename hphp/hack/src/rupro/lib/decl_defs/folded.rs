// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused)]
use std::collections::HashMap;

use crate::decl_defs::{CeVisibility, DeclTy};
use crate::pos::Symbol;
use crate::reason::Reason;

#[derive(Debug, Clone)]
pub struct FoldedElement<R: Reason> {
    pub elt_origin: Symbol,
    pub elt_visibility: CeVisibility<R>,
}

#[derive(Debug, Clone)]
pub struct SubstContext<R: Reason> {
    pub sc_subst: HashMap<Symbol, DeclTy<R>>,
    pub sc_class_context: Symbol,
}

#[derive(Debug, Clone)]
pub struct FoldedClass<R: Reason> {
    pub dc_name: Symbol,
    pub dc_pos: R::Pos,
    pub dc_substs: HashMap<Symbol, SubstContext<R>>,
    pub dc_ancestors: HashMap<Symbol, DeclTy<R>>,
    pub dc_methods: HashMap<Symbol, FoldedElement<R>>,
}
