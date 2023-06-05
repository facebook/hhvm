// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1b4b237cfe6f928d694855b7f5d6bbe9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use core::*;

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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show, yojson)")]
#[repr(C, u8)]
pub enum PattString {
    Exactly(String),
    #[rust_to_ocaml(name = "Starts_with")]
    StartsWith(String),
    #[rust_to_ocaml(name = "Ends_with")]
    EndsWith(String),
    Contains(String),
    #[rust_to_ocaml(name = "One_of")]
    OneOf(Vec<PattString>),
    And(Vec<PattString>),
    #[rust_to_ocaml(name = "Is_not")]
    IsNot(Box<PattString>),
}
