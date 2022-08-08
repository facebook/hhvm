// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3f18d2b0cf082deb128c6f1eea4a639f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
pub use c::map as c_map;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::typing_continuations as c;
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
pub struct PerContEntry<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub local_types: &'a typing_local_types::TypingLocalTypes<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fake_members: &'a typing_fake_members::TypingFakeMembers<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
}
impl<'a> TrivialDrop for PerContEntry<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PerContEntry<'arena>);

pub type TypingPerContEnv<'a> = typing_continuations::map::Map<'a, &'a PerContEntry<'a>>;
