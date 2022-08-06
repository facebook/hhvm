// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::gen::ast_defs::Pstring;

#[derive(
    Copy,
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
pub struct DocComment<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pstring<'a>,
);

arena_deserializer::impl_deserialize_in_arena!(DocComment<'arena>);

impl arena_trait::TrivialDrop for DocComment<'_> {}

impl<'a> DocComment<'a> {
    pub fn new(ps: &'a Pstring<'_>) -> Self {
        Self(ps)
    }
}
