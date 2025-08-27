// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9500134f50442b257b5f5fef02be2475>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
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
    Or(&'a [PattString<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    And(&'a [PattString<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Not(&'a PattString<'a>),
}
impl<'a> TrivialDrop for PattString<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattString<'arena>);
