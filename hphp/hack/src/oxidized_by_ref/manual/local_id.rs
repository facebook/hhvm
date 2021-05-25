// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct LocalId<'a>(
    isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] &'a str,
);
arena_deserializer::impl_deserialize_in_arena!(LocalId<'arena>);

impl arena_trait::TrivialDrop for LocalId<'_> {}

impl<'a> LocalId<'a> {
    pub fn new_unscoped_in(name: &'a str) -> Self {
        Self(0, name)
    }

    pub fn name(self) -> &'a str {
        self.1
    }
}

impl<'a> From<&'a oxidized::local_id::LocalId> for LocalId<'a> {
    fn from(x: &'a oxidized::local_id::LocalId) -> LocalId<'a> {
        let (a, b) = x;
        LocalId(*a, b.as_str())
    }
}

pub mod map {
    pub type Map<'a, T> = arena_collections::map::Map<'a, super::LocalId<'a>, T>;
}

pub mod set {
    pub type Set<'a> = arena_collections::set::Set<'a, super::LocalId<'a>>;
}
