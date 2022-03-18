// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<306c2590559242c188cda9a67fcad814>>
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
#[repr(C, u8)]
pub enum DeclReference<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    GlobalConstant(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Function(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Type(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Module(&'a str),
}
impl<'a> TrivialDrop for DeclReference<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeclReference<'arena>);
