// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<29ebb52552a49507074c051594b3003d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
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
#[repr(C)]
pub struct DeclParserOptions<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub auto_namespace_map: &'a [(&'a str, &'a str)],
    pub disable_xhp_element_mangling: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub everything_sdt: bool,
    pub global_inference: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub gi_reinfer_types: &'a [&'a str],
}
impl<'a> TrivialDrop for DeclParserOptions<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeclParserOptions<'arena>);
