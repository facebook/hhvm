// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cdc6851eea591de275fc9c5c31188e0b>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct Summary {
    /// Number of occurrences of a legacy refinement kind
    pub count: isize,
    /// The set of positions (stored as a string), where the refinement occurs.
    /// This field value is depended on the log level:
    /// refinement_counter = 3: Include all positions
    /// refinement_counter = 2: Include all positions, except for class refinements
    /// otherwise: Do not include positions (will be SSet.empty)
    pub positions: s_set::SSet,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct RefinementCounter {
    pub legacy_refinements: s_map::SMap<Summary>,
    pub new_refinements: isize,
    pub total_refinements: isize,
}
