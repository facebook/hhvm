// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_collections::list::List;
use arena_trait::TrivialDrop;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};
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
    pub classes: SMap<'a, shallow_decl_defs::ShallowClass<'a>>,
    pub funs: SMap<'a, typing_defs::FunElt<'a>>,
    pub typedefs: SMap<'a, typing_defs::TypedefType<'a>>,
    pub consts: SMap<'a, typing_defs::ConstDecl<'a>>,
}
impl<'a> TrivialDrop for Decls<'a> {}

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
    pub classes: List<'a, (&'a str, shallow_decl_defs::ShallowClass<'a>)>,
    pub funs: List<'a, (&'a str, typing_defs::FunElt<'a>)>,
    pub typedefs: List<'a, (&'a str, typing_defs::TypedefType<'a>)>,
    pub consts: List<'a, (&'a str, typing_defs::ConstDecl<'a>)>,
}
impl<'a> TrivialDrop for DeclLists<'a> {}
