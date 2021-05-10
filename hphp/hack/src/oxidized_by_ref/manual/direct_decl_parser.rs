// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use arena_collections::List;
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::slab::OwnedSlab;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};
use oxidized::file_info::NameType;

use crate::{
    shallow_decl_defs::{self, Decl},
    typing_defs,
};

#[derive(
    Copy,
    Clone,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Decls<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  List<'a, (&'a str, Decl<'a>)>,
);

impl<'a> TrivialDrop for Decls<'a> {}

impl<'a> Decls<'a> {
    pub const fn empty() -> Self {
        Self(List::empty())
    }

    pub fn get(&self, kind: NameType, symbol: &str) -> Option<Decl<'a>> {
        self.iter().find_map(|(name, decl)| {
            if decl.kind() == kind && name == symbol {
                Some(decl)
            } else {
                None
            }
        })
    }

    pub fn add<A: arena_trait::Arena>(&mut self, name: &'a str, decl: Decl<'a>, arena: &'a A) {
        self.0.push_front((name, decl), arena)
    }

    pub fn rev<A: arena_trait::Arena>(&mut self, arena: &'a A) {
        self.0 = self.0.rev(arena)
    }

    pub fn iter(&self) -> impl Iterator<Item = (&'a str, Decl<'a>)> {
        self.0.iter().copied()
    }

    pub fn classes(
        &self,
    ) -> impl Iterator<Item = (&'a str, &'a shallow_decl_defs::ShallowClass<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Class(decl) => Some((name, decl)),
            _ => None,
        })
    }
    pub fn funs(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::FunElt<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Fun(decl) => Some((name, decl)),
            _ => None,
        })
    }
    pub fn typedefs(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::TypedefType<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Typedef(decl) => Some((name, decl)),
            _ => None,
        })
    }
    pub fn consts(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::ConstDecl<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Const(decl) => Some((name, decl)),
            _ => None,
        })
    }
    pub fn records(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::RecordDefType<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Record(decl) => Some((name, decl)),
            _ => None,
        })
    }

    pub fn types(&self) -> impl Iterator<Item = (&'a str, Decl<'a>)> {
        self.iter().filter(|(_, decl)| match decl.kind() {
            NameType::Class | NameType::Typedef | NameType::RecordDef => true,
            NameType::Fun | NameType::Const => false,
        })
    }
}

impl<'a> IntoIterator for Decls<'a> {
    type Item = (&'a str, Decl<'a>);
    type IntoIter = std::iter::Copied<arena_collections::list::Iter<'a, (&'a str, Decl<'a>)>>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter().copied()
    }
}

impl std::fmt::Debug for Decls<'_> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter()).finish()
    }
}

impl<'a> Decl<'a> {
    pub fn kind(&self) -> NameType {
        match self {
            Decl::Class(..) => NameType::Class,
            Decl::Fun(..) => NameType::Fun,
            Decl::Record(..) => NameType::RecordDef,
            Decl::Typedef(..) => NameType::Typedef,
            Decl::Const(..) => NameType::Const,
        }
    }

    pub fn to_slab(&self) -> OwnedSlab {
        match self {
            Decl::Class(decl) => ocamlrep::slab::to_slab(decl),
            Decl::Fun(decl) => ocamlrep::slab::to_slab(decl),
            Decl::Record(decl) => ocamlrep::slab::to_slab(decl),
            Decl::Typedef(decl) => ocamlrep::slab::to_slab(decl),
            Decl::Const(decl) => ocamlrep::slab::to_slab(decl),
        }
        .expect("Got immediate value, but decls should always be block values")
    }
}
