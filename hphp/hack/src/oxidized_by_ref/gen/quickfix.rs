// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a26c8023d2c275c2478b9b0e6a5e78f6>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
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
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
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
pub enum QfPos<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Qpos(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    QclassishStart(&'a str),
}
impl<'a> TrivialDrop for QfPos<'a> {}
arena_deserializer::impl_deserialize_in_arena!(QfPos<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
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
pub struct Edit<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub QfPos<'a>,
);
impl<'a> TrivialDrop for Edit<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Edit<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
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
pub struct Quickfix<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub title: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub edits: &'a [&'a Edit<'a>],
}
impl<'a> TrivialDrop for Quickfix<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Quickfix<'arena>);
