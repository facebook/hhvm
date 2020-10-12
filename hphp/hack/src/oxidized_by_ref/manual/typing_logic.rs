// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Serialize;

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
    Coerce(Ty<'a>, Ty<'a>),
    IsSubtype(InternalType<'a>, InternalType<'a>),
    Conj(&'a [SubtypeProp<'a>]),
    Disj(&'a [SubtypeProp<'a>]),
}

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
