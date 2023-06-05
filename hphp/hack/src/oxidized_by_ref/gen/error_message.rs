// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cee4932fa44189e75eb0151eec10d5ea>>
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
#[rust_to_ocaml(attr = "deriving (eq, show, yojson)")]
#[repr(C, u8)]
pub enum Elem<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lit(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ty_var")]
    TyVar(&'a patt_var::PattVar<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Name_var")]
    NameVar(&'a patt_var::PattVar<'a>),
}
impl<'a> TrivialDrop for Elem<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Elem<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show, yojson)")]
#[repr(C)]
pub struct ErrorMessage<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub message: &'a [Elem<'a>],
}
impl<'a> TrivialDrop for ErrorMessage<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ErrorMessage<'arena>);
