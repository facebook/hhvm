// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<f6e4bfe77f60ea75ee68896c49a72955>>
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

pub use crate::typing_reason as reason;

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
pub struct TypingFakeMembers<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub valid: &'a blame_set::BlameSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub invalid: &'a blame_set::BlameSet<'a>,
}
impl<'a> TrivialDrop for TypingFakeMembers<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypingFakeMembers<'arena>);
