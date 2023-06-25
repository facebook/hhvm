// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a5113df8c1279f500ed10d16affa9f87>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
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
#[repr(C, u8)]
pub enum VersionedPattError<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Error_v1")]
    ErrorV1(&'a patt_error::PattError<'a>),
}
impl<'a> TrivialDrop for VersionedPattError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(VersionedPattError<'arena>);

#[derive(
    Clone,
    Copy,
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
#[repr(C, u8)]
pub enum VersionedErrorMessage<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Message_v1")]
    MessageV1(&'a error_message::ErrorMessage<'a>),
}
impl<'a> TrivialDrop for VersionedErrorMessage<'a> {}
arena_deserializer::impl_deserialize_in_arena!(VersionedErrorMessage<'arena>);

#[derive(
    Clone,
    Copy,
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
#[repr(C)]
pub struct CustomError<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub patt: VersionedPattError<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub error_message: VersionedErrorMessage<'a>,
}
impl<'a> TrivialDrop for CustomError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CustomError<'arena>);
