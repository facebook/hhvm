// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::{CeVisibility, DeclTy};
use crate::reason::Reason;
use pos::{Symbol, SymbolMap};

#[derive(Debug, Clone)]
pub struct FoldedElement<R: Reason> {
    // note(sf, 2022-01-28): c.f. `Decl_defs.element`
    pub flags: oxidized_by_ref::typing_defs_flags::ClassEltFlags,
    pub origin: Symbol,
    pub visibility: CeVisibility<R::Pos>,
    pub deprecated: Option<intern::string::BytesId>,
}

#[derive(Debug, Clone)]
pub struct SubstContext<R: Reason> {
    // note(sf, 2022-01-28): c.f. `Decl_defs.subst_context`
    pub subst: SymbolMap<DeclTy<R>>,
    pub class_context: Symbol,
}

#[derive(Debug, Clone)]
pub struct FoldedClass<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_defs.decl_class_type`
    pub name: Symbol,
    pub pos: R::Pos,
    pub substs: SymbolMap<SubstContext<R>>,
    pub ancestors: SymbolMap<DeclTy<R>>,
    pub methods: SymbolMap<FoldedElement<R>>,
    pub static_methods: SymbolMap<FoldedElement<R>>,
}
