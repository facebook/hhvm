// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_collections::List;
use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_caml_builtins::Int64;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use oxidized::file_info::NameType;
use serde::Deserialize;
use serde::Serialize;
pub use shallow_decl_defs::Decl;

use crate::file_info;
use crate::shallow_decl_defs;
use crate::typing_defs;

// NB: Must keep in sync with OCaml type Direct_decl_parser.parsed_file
#[derive(Copy, Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[derive(Deserialize, FromOcamlRepIn, NoPosHash, Serialize, ToOcamlRep)]
pub struct ParsedFile<'a> {
    pub mode: Option<file_info::Mode>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a typing_defs::UserAttribute<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decls: Decls<'a>,
    /// True if the FFP detected parse errors while parsing. Other parse errors
    /// are detected in a second pass over the CST, and this field does not
    /// indicate whether errors would be detected in that second pass.
    pub has_first_pass_parse_errors: bool,
}

// NB: Must keep in sync with OCaml type `Direct_decl_parser.parsed_file_with_hashes`
#[derive(ocamlrep_derive::ToOcamlRep)]
pub struct ParsedFileWithHashes<'a> {
    pub mode: Option<file_info::Mode>,
    pub hash: Int64,
    pub decls: Vec<(&'a str, Decl<'a>, Int64)>,
}

impl<'a> From<ParsedFile<'a>> for ParsedFileWithHashes<'a> {
    fn from(parsed_file: ParsedFile<'a>) -> ParsedFileWithHashes<'a> {
        let file_decls_hash = Int64(hh_hash::position_insensitive_hash(&parsed_file.decls) as i64);
        let decls = Vec::from_iter(
            parsed_file
                .decls
                .into_iter()
                .map(|(name, decl)| (name, decl, Int64(hh_hash::hash(&decl) as i64))),
        );
        ParsedFileWithHashes {
            mode: parsed_file.mode,
            hash: file_decls_hash,
            decls,
        }
    }
}

// NB: Must keep in sync with OCaml type Direct_decl_parser.decls
#[derive(Copy, Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[derive(Deserialize, FromOcamlRepIn, NoPosHash, Serialize, ToOcamlRep)]
pub struct Decls<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  List<'a, (&'a str, Decl<'a>)>,
);

arena_deserializer::impl_deserialize_in_arena!(Decls<'arena>);

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
    pub fn types(&self) -> impl Iterator<Item = (&'a str, Decl<'a>)> {
        self.iter().filter(|(_, decl)| match decl {
            Decl::Class(_) | Decl::Typedef(_) => true,
            Decl::Fun(_) | Decl::Const(_) | Decl::Module(_) => false,
        })
    }

    pub fn modules(&self) -> impl Iterator<Item = (&'a str, &'a typing_defs::ModuleDefType<'a>)> {
        self.iter().filter_map(|(name, decl)| match decl {
            Decl::Module(decl) => Some((name, decl)),
            _ => None,
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
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter()).finish()
    }
}
