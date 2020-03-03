// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6cb37057fd8e2c33966ab6e42bcaa5ed>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_continuations as c;
pub use c::map as c_map;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct PerContEntry {
    pub local_types: typing_local_types::TypingLocalTypes,
    pub fake_members: typing_fake_members::TypingFakeMembers,
    pub tpenv: type_parameter_env::TypeParameterEnv,
}

pub type TypingPerContEnv = typing_continuations::map::Map<PerContEntry>;
