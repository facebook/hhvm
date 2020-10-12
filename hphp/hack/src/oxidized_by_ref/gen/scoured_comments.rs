// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<50da571c971b45b538d367def9f3adf8>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Fixmes<'a> = i_map::IMap<'a, i_map::IMap<'a, &'a pos::Pos<'a>>>;

#[derive(
    Clone,
    Debug,
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
pub struct ScouredComments<'a> {
    pub comments: &'a [(&'a pos::Pos<'a>, prim_defs::Comment<'a>)],
    pub fixmes: &'a Fixmes<'a>,
    pub misuses: &'a Fixmes<'a>,
    pub error_pos: &'a [&'a pos::Pos<'a>],
}
impl<'a> TrivialDrop for ScouredComments<'a> {}
