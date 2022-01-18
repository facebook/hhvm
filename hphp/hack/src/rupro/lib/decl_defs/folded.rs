// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused)]
use std::collections::HashMap;

use crate::decl_defs::DeclTy;
use crate::pos::Symbol;
use crate::reason::Reason;

#[derive(Debug, Clone)]
pub struct FoldedElement {
    pub elt_origin: Symbol,
}

#[derive(Debug, Clone)]
pub struct SubstContext<REASON: Reason> {
    pub sc_subst: HashMap<Symbol, DeclTy<REASON>>,
    pub sc_class_context: Symbol,
}

#[derive(Debug, Clone)]
pub struct FoldedClass<REASON: Reason> {
    pub dc_name: Symbol,
    pub dc_pos: REASON::P,
    pub dc_substs: HashMap<Symbol, SubstContext<REASON>>,
    pub dc_ancestors: HashMap<Symbol, DeclTy<REASON>>,
    pub dc_methods: HashMap<Symbol, FoldedElement>,
}
