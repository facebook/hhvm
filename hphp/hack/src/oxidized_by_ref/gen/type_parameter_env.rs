// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6198ecd41c5c1de5580fe640f9810923>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;
pub use typing_kinding_defs::*;

pub use crate::typing_set as ty_set;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

pub type TparamInfo<'a> = typing_kinding_defs::Kind<'a>;

#[derive(
    Clone,
    Debug,
    Default,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct TypeParameterEnv<'a> {
    /// The position indicates where the type parameter was defined.
    /// It may be Pos.none if the type parameter denotes a fresh type variable
    /// (i.e., without a source location that defines it)
    pub tparams: s_map::SMap<'a, (&'a pos::Pos<'a>, TparamInfo<'a>)>,
    pub consistent: bool,
}
impl<'a> TrivialDrop for TypeParameterEnv<'a> {}
