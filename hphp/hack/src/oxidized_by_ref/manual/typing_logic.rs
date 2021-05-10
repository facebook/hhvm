// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

use crate::typing_defs::*;

/// The reason why something is expected to have a certain type
/// This has to be defined manually (not in oxidized/gen) because there is a function type in the original
/// definition of Disj
/// @TODO: work out what to do about this!
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
pub enum SubtypeProp<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    Coerce(Ty<'a>, Ty<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena")]
    IsSubtype(InternalType<'a>, InternalType<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Conj(&'a [SubtypeProp<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Disj(&'a [SubtypeProp<'a>]),
}

arena_deserializer::impl_deserialize_in_arena!(SubtypeProp<'arena>);

impl arena_trait::TrivialDrop for SubtypeProp<'_> {}

const DEFAULT: SubtypeProp<'_> = SubtypeProp::Disj(&[]);

impl SubtypeProp<'_> {
    pub const fn default_ref() -> &'static Self {
        &DEFAULT
    }
}

impl Default for &SubtypeProp<'_> {
    fn default() -> Self {
        SubtypeProp::default_ref()
    }
}
