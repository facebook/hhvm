// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};
use serde::{Deserialize, Serialize};

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    EqModuloPos,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
#[serde(bound(deserialize = "T: 'de + arena_deserializer::DeserializeInArena<'de>"))]
pub struct Lazy<T>(#[serde(deserialize_with = "arena_deserializer::arena")] T);

arena_deserializer::impl_deserialize_in_arena!(Lazy<T>);

impl<T> arena_trait::TrivialDrop for Lazy<T> {}

impl<T: ToOcamlRep> ToOcamlRep for Lazy<T> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        unimplemented!()
    }
}

impl<T: FromOcamlRep> FromOcamlRep for Lazy<T> {
    fn from_ocamlrep(_value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        unimplemented!()
    }
}

impl<'a, T: FromOcamlRepIn<'a>> FromOcamlRepIn<'a> for Lazy<T> {
    fn from_ocamlrep_in(
        _value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        unimplemented!()
    }
}
