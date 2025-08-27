// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a5263da8f7e23e12afed29a12239e8f3>>
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

/// How should we indicate to the user that a quickfix is available,
/// additional to the primary error red-squiggle?
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
pub enum HintStyle<Pos> {
    /// Make the quickfix available for the given position, but don't provide
    /// any visual indication.
    HintStyleSilent(Pos),
    /// Use the a non-error/non-warning IDE visual clue that a quickfix is
    /// available for the given position. Example: https://pxl.cl/4x0ZS
    HintStyleHint(Pos),
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum Edits<Pos> {
    /// Make a quickfix when all the information about
    /// edits is already available and does not need to be calculated.
    Eager(Vec<(String, Pos)>),
    /// A quickfix might want to add things to an empty class declaration,
    /// which requires FFP to compute the { position.
    #[rust_to_ocaml(prefix = "classish_end_")]
    #[rust_to_ocaml(name = "Classish_end")]
    ClassishEnd { new_text: String, name: String },
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C)]
pub struct Quickfix<Pos> {
    pub title: String,
    pub edits: Edits<Pos>,
    pub hint_styles: Vec<HintStyle<classish_positions_types::Pos<Pos>>>,
}
