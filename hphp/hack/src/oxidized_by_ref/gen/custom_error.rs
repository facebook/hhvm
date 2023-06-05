// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2468c379701d6c31cc280b5868087450>>
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
#[repr(C)]
pub struct CustomError<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub patt: patt_error::PattError<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub error_message: error_message::ErrorMessage<'a>,
}
impl<'a> TrivialDrop for CustomError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CustomError<'arena>);
