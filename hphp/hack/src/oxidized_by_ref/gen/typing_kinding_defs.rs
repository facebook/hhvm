// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1badc05128ecdc9d9a309f4b3b0b3f45>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;

pub use crate::typing_set as ty_set;
#[allow(unused_imports)]
use crate::*;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
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
pub struct Kind<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower_bounds: &'a TparamBounds<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper_bounds: &'a TparamBounds<'a>,
    pub reified: oxidized::aast::ReifyKind,
    pub enforceable: bool,
    pub newable: bool,
    pub require_dynamic: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub parameters: &'a [&'a NamedKind<'a>],
}
impl<'a> TrivialDrop for Kind<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Kind<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct NamedKind<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Kind<'a>,
);
impl<'a> TrivialDrop for NamedKind<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NamedKind<'arena>);
