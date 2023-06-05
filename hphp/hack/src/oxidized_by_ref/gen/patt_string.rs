// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<790758dc9cefc54f9a4c708786729c51>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use core::*;

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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show, yojson)")]
#[repr(C, u8)]
pub enum PattString<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Exactly(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Starts_with")]
    StartsWith(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ends_with")]
    EndsWith(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Contains(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "One_of")]
    OneOf(&'a [PattString<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    And(&'a [PattString<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Is_not")]
    IsNot(&'a PattString<'a>),
}
impl<'a> TrivialDrop for PattString<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattString<'arena>);
