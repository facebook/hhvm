// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a584d297c25f4c3cc336525aa04ea1d2>>
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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C, u8)]
pub enum Elem<Pos> {
    Witness(Pos, String),
    #[rust_to_ocaml(name = "Witness_no_pos")]
    WitnessNoPos(String),
    Rule(String),
    Step(String, bool),
    Trans(String),
    Prefix {
        prefix: String,
        sep: String,
    },
    Suffix {
        suffix: String,
        sep: String,
    },
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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C, u8)]
pub enum Explanation<Pos> {
    Derivation(Vec<Elem<Pos>>),
    Debug(String),
    Empty,
}
