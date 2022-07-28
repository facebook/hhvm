// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<da2a8f6b9e876d18cf6c39b1370c3714>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Fixmes<'a> = i_map::IMap<'a, i_map::IMap<'a, &'a pos::Pos<'a>>>;

#[derive(
    Clone,
    Debug,
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
#[rust_to_ocaml(prefix = "sc")]
#[repr(C)]
pub struct ScouredComments<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub comments: &'a [(&'a pos::Pos<'a>, prim_defs::Comment<'a>)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fixmes: &'a Fixmes<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub misuses: &'a Fixmes<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub error_pos: &'a [&'a pos::Pos<'a>],
}
impl<'a> TrivialDrop for ScouredComments<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ScouredComments<'arena>);
