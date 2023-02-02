// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8c47559ed12bbcee19eb2c8b0aa5cb7e>>
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
#[repr(C, u8)]
pub enum NamingPhaseError {
    Naming(naming_error::NamingError),
    #[rust_to_ocaml(name = "Nast_check")]
    NastCheck(nast_check_error::NastCheckError),
    #[rust_to_ocaml(name = "Like_type")]
    LikeType(pos::Pos),
    #[rust_to_ocaml(name = "Unexpected_hint")]
    UnexpectedHint(pos::Pos),
    #[rust_to_ocaml(name = "Malformed_access")]
    MalformedAccess(pos::Pos),
    Supportdyn(pos::Pos),
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
#[repr(C)]
pub struct Agg {
    pub naming: Vec<naming_error::NamingError>,
    pub nast_check: Vec<nast_check_error::NastCheckError>,
    pub like_types: Vec<pos::Pos>,
    pub unexpected_hints: Vec<pos::Pos>,
    pub malformed_accesses: Vec<pos::Pos>,
    pub supportdyns: Vec<pos::Pos>,
}
