// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<76de562a0fd29ee06ec746878fadefdd>>
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
pub enum PattError<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Typing(&'a patt_typing_error::PattTypingError<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Naming(&'a patt_naming_error::PattNamingError<'a>),
}
impl<'a> TrivialDrop for PattError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattError<'arena>);
