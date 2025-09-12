// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<61fd233856f880e63ebc5ce9340323c1>>
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum PattNameContext<'a> {
    #[rust_to_ocaml(name = "Any_name_context")]
    AnyNameContext,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "One_of_name_context")]
    OneOfNameContext(&'a [&'a oxidized::name_context::NameContext]),
}
impl<'a> TrivialDrop for PattNameContext<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattNameContext<'arena>);

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
pub enum PattNamingError<'a> {
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Unbound_name")]
    UnboundName {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name_context: PattNameContext<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: patt_name::PattName<'a>,
    },
    #[rust_to_ocaml(name = "Invalid_naming")]
    InvalidNaming {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        errs: &'a [validation_err::ValidationErr<'a>],
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattNamingError<'a>,
    },
}
impl<'a> TrivialDrop for PattNamingError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattNamingError<'arena>);
