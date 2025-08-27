// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<824d166fddea3dc0f771cbe559e3e9ad>>
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
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum ValidationErr<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Mismatch(
        &'a (
            &'a oxidized::patt_binding_ty::PattBindingTy,
            &'a oxidized::patt_binding_ty::PattBindingTy,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shadowed(&'a patt_var::PattVar<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Unbound(&'a patt_var::PattVar<'a>),
}
impl<'a> TrivialDrop for ValidationErr<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ValidationErr<'arena>);
