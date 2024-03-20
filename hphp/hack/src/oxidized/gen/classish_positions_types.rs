// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3a45b5c47cbea16bf6182da0fda15e2c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// We put type definitions that we'll export into a separate module
/// to keep oxidized happy
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum Pos<P> {
    Precomputed(P),
    #[rust_to_ocaml(name = "Classish_end_of_body")]
    ClassishEndOfBody(String),
    #[rust_to_ocaml(name = "Classish_start_of_body")]
    ClassishStartOfBody(String),
}

/// Positional information for a single class
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
#[rust_to_ocaml(prefix = "classish_")]
#[repr(C)]
pub struct ClassishPositions<P> {
    pub start_of_body: P,
    pub end_of_body: P,
}

/// Positional information for a collection of classes
pub type ClassishPositionsTypes<P> = s_map::SMap<ClassishPositions<P>>;
