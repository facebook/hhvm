// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<781bf0bfac068a3ec9d33e27dedbb147>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::local_id::map as l_map;

pub use oxidized::typing_mutability_env::MutType;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Mutability<'a>(
    pub &'a pos::Pos<'a>,
    pub oxidized::typing_mutability_env::MutType,
);
impl<'a> TrivialDrop for Mutability<'a> {}

pub type MutabilityEnv<'a> = local_id::map::Map<'a, Mutability<'a>>;
