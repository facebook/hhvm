// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<77846c0085eaafe9e81c6c8acddc32cc>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct PosId<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a pos::Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a str,
);
impl<'a> TrivialDrop for PosId<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PosId<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct Package<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub uses: &'a [&'a PosId<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub includes: &'a [&'a PosId<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub soft_includes: &'a [&'a PosId<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub allow_directories: &'a [&'a PosId<'a>],
}
impl<'a> TrivialDrop for Package<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Package<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(u8)]
pub enum PackageRelationship {
    Unrelated,
    Includes,
    #[rust_to_ocaml(name = "Soft_includes")]
    SoftIncludes,
    Equal,
}
impl TrivialDrop for PackageRelationship {}
arena_deserializer::impl_deserialize_in_arena!(PackageRelationship);
