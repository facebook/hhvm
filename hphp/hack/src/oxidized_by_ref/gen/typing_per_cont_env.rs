// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<91d752c3c08f0698fcdf33facf654d27>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_continuations as c;
pub use c::map as c_map;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PerContEntry<'a> {
    pub local_types: typing_local_types::TypingLocalTypes<'a>,
    pub fake_members: typing_fake_members::TypingFakeMembers<'a>,
    pub tpenv: type_parameter_env::TypeParameterEnv<'a>,
}
impl<'a> TrivialDrop for PerContEntry<'a> {}

pub type TypingPerContEnv<'a> = typing_continuations::map::Map<'a, PerContEntry<'a>>;
