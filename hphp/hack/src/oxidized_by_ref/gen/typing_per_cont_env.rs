// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e5570e04dea2602227ecda0b555b31b8>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_continuations as c;
pub use c::map as c_map;

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
pub struct PerContEntry<'a> {
    pub local_types: &'a typing_local_types::TypingLocalTypes<'a>,
    pub fake_members: &'a typing_fake_members::TypingFakeMembers<'a>,
    pub tpenv: &'a type_parameter_env::TypeParameterEnv<'a>,
}
impl<'a> TrivialDrop for PerContEntry<'a> {}

pub type TypingPerContEnv<'a> = typing_continuations::map::Map<'a, &'a PerContEntry<'a>>;
