// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d81e28b6fd5513eb3ee10128cf3e8c59>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;
pub use typing_kinding_defs::*;

pub use crate::typing_set as ty_set;

pub type TparamName<'a> = str;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

pub type TparamInfo<'a> = typing_kinding_defs::Kind<'a>;

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
pub struct TypeParameterEnv<'a> {
    /// The position indicates where the type parameter was defined.
    /// It may be Pos.none if the type parameter denotes a fresh type variable
    /// (i.e., without a source location that defines it)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: s_map::SMap<'a, (&'a pos_or_decl::PosOrDecl<'a>, &'a TparamInfo<'a>)>,
    pub consistent: bool,
}
impl<'a> TrivialDrop for TypeParameterEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeParameterEnv<'arena>);
