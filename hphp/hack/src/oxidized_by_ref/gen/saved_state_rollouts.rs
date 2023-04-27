// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9707b3a8836da524e3829d0ca9a30ec4>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
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
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "dummy_")]
#[repr(C)]
pub struct SavedStateRollouts {
    pub one: bool,
    pub two: bool,
    pub three: bool,
}
impl TrivialDrop for SavedStateRollouts {}
arena_deserializer::impl_deserialize_in_arena!(SavedStateRollouts);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving show { with_path = false }")]
#[repr(u8)]
pub enum Flag {
    #[rust_to_ocaml(name = "Dummy_one")]
    DummyOne,
    #[rust_to_ocaml(name = "Dummy_two")]
    DummyTwo,
    #[rust_to_ocaml(name = "Dummy_three")]
    DummyThree,
}
impl TrivialDrop for Flag {}
arena_deserializer::impl_deserialize_in_arena!(Flag);

pub type FlagName = str;
