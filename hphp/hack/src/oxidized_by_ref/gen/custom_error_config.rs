// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c9bf242ea153714ce7d72bb11b59dfdf>>
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
#[rust_to_ocaml(attr = "boxed")]
#[repr(C)]
pub struct CustomErrorConfig<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub valid: &'a [custom_error::CustomError<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub invalid: &'a [custom_error::CustomError<'a>],
}
impl<'a> TrivialDrop for CustomErrorConfig<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CustomErrorConfig<'arena>);
