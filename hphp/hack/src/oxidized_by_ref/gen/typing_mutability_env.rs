// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<fddc290bd0b8adb95b7d21e9cfefff73>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::local_id::map as l_map;

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum MutType {
    Mutable,
    Borrowed,
    MaybeMutable,
    Immutable,
}
impl TrivialDrop for MutType {}

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
pub struct Mutability<'a>(pub &'a pos::Pos<'a>, pub MutType);
impl<'a> TrivialDrop for Mutability<'a> {}

pub type MutabilityEnv<'a> = local_id::map::Map<'a, &'a Mutability<'a>>;
