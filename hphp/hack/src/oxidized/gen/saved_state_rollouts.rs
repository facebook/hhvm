// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6bf34b1048481a6ef1a097e97980512a>>
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
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct SavedStateRollouts {
    pub dummy_one: bool,
    pub dummy_two: bool,
    pub dummy_three: bool,
    pub optimized_member_fanout: bool,
    pub optimized_parent_fanout: bool,
    pub optimized_attribute_fanout: bool,
    pub new_naming_table: bool,
}

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
    #[rust_to_ocaml(name = "Optimized_member_fanout")]
    OptimizedMemberFanout,
    #[rust_to_ocaml(name = "Optimized_parent_fanout")]
    OptimizedParentFanout,
    #[rust_to_ocaml(name = "Optimized_attribute_fanout")]
    OptimizedAttributeFanout,
    #[rust_to_ocaml(name = "New_naming_table")]
    NewNamingTable,
}
impl TrivialDrop for Flag {}
arena_deserializer::impl_deserialize_in_arena!(Flag);

pub type FlagName = String;
