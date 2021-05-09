// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use crate::gen::ast_defs::Pstring;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

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
    pub fn new(ps: &'a Pstring) -> Self {
        Self(ps)
    }
}
