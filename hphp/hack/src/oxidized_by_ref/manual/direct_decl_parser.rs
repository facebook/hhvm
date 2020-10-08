// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_collections::list::List;
use arena_trait::TrivialDrop;
use ocamlrep::slab::OwnedSlab;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};
use oxidized::file_info::NameType;
use serde::Serialize;

use crate::{shallow_decl_defs, typing_defs};

type SMap<'a, T> = arena_collections::SortedAssocList<'a, &'a str, T>;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Decls<'a> {
    pub classes: SMap<'a, &'a shallow_decl_defs::ShallowClass<'a>>,
    pub funs: SMap<'a, &'a typing_defs::FunElt<'a>>,
    pub typedefs: SMap<'a, &'a typing_defs::TypedefType<'a>>,
    pub consts: SMap<'a, &'a typing_defs::ConstDecl<'a>>,
}
impl<'a> TrivialDrop for Decls<'a> {}
impl<'a> Decls<'a> {
    pub fn get_slab(&self, kind: NameType, symbol: &str) -> Option<OwnedSlab> {
        match kind {
            NameType::Fun => Some(Self::decl_to_slab(self.funs.get(symbol)?)),
            NameType::Class => Some(Self::decl_to_slab(self.classes.get(symbol)?)),
            NameType::RecordDef => unimplemented!(),
            NameType::Typedef => Some(Self::decl_to_slab(self.typedefs.get(symbol)?)),
            NameType::Const => Some(Self::decl_to_slab(self.consts.get(symbol)?)),
        }
    }

    pub fn decl_to_slab(decl: &impl ocamlrep::ToOcamlRep) -> OwnedSlab {
        ocamlrep::slab::to_slab(decl)
            .expect("Got immediate value, but decls should always be block values")
    }
}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    ToOcamlRep
)]
pub struct DeclLists<'a> {
    pub classes: List<'a, (&'a str, &'a shallow_decl_defs::ShallowClass<'a>)>,
    pub funs: List<'a, (&'a str, &'a typing_defs::FunElt<'a>)>,
    pub typedefs: List<'a, (&'a str, &'a typing_defs::TypedefType<'a>)>,
    pub consts: List<'a, (&'a str, &'a typing_defs::ConstDecl<'a>)>,
}
impl<'a> TrivialDrop for DeclLists<'a> {}
