// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a483d1620fa7dc13e4d1b31b4ce99b6c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::typing_reason as reason;
#[allow(unused_imports)]
use crate::*;

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
#[repr(C)]
pub struct TypingFakeMembers<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub valid: &'a blame_set::BlameSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub invalid: &'a blame_set::BlameSet<'a>,
}
impl<'a> TrivialDrop for TypingFakeMembers<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypingFakeMembers<'arena>);
