// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e8768d83f520bbb3822cd781ececc68d>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use arena_trait::TrivialDrop;
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
    Copy,
    Debug,
    Deserialize,
    Eq,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show, ord)")]
#[repr(u8)]
pub enum Mode {
    ForTypecheck,
    ForCodegen,
}
impl TrivialDrop for Mode {}
arena_deserializer::impl_deserialize_in_arena!(Mode);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, hash, show, ord)")]
#[rust_to_ocaml(prefix = "ns_")]
#[repr(C)]
pub struct Env<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub ns_uses: s_map::SMap<'a, &'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub class_uses: s_map::SMap<'a, &'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub fun_uses: s_map::SMap<'a, &'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub const_uses: s_map::SMap<'a, &'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Option<&'a str>,
    pub mode: Mode,
    pub disable_xhp_element_mangling: bool,
}
impl<'a> TrivialDrop for Env<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Env<'arena>);
