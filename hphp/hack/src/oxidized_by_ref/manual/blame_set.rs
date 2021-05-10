// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

use crate::{local_id::LocalId, typing_reason::Blame};

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    Serialize,
    ToOcamlRep
)]
pub struct IdWithBlame<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub LocalId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] Blame<'a>,
);
arena_deserializer::impl_deserialize_in_arena!(IdWithBlame<'arena>);

impl arena_trait::TrivialDrop for IdWithBlame<'_> {}

impl PartialEq for IdWithBlame<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.0.eq(&other.0)
    }
}

impl PartialOrd for IdWithBlame<'_> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.0.partial_cmp(&other.0)
    }
}

/// A set of LocalIds which treats blame as metadata
pub type BlameSet<'a> = arena_collections::set::Set<'a, IdWithBlame<'a>>;
