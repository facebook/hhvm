// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

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
    Hash,
    FromOcamlRep,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum SubtypeProp {
    Coerce(Ty, Ty),
    IsSubtype(InternalType, InternalType),
    Conj(Vec<SubtypeProp>),
    Disj(Vec<SubtypeProp>),
}

impl Default for SubtypeProp {
    fn default() -> Self {
        Self::Disj(vec![])
    }
}
