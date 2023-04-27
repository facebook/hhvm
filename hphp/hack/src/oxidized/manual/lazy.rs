// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

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
pub struct Lazy<T>(pub T);

impl<T: ToOcamlRep> ToOcamlRep for Lazy<T> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        unimplemented!()
    }
}

impl<T: FromOcamlRep> FromOcamlRep for Lazy<T> {
    fn from_ocamlrep(_value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        unimplemented!()
    }
}
