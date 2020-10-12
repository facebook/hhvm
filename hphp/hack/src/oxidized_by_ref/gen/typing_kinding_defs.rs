// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<622a01a9b6d323d5707b0d9bdddd4c6e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use crate::typing_set as ty_set;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

#[derive(
    Clone,
    Debug,
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
pub struct Kind<'a> {
    pub lower_bounds: &'a TparamBounds<'a>,
    pub upper_bounds: &'a TparamBounds<'a>,
    pub reified: oxidized::aast::ReifyKind,
    pub enforceable: bool,
    pub newable: bool,
    pub parameters: &'a [&'a NamedKind<'a>],
}
impl<'a> TrivialDrop for Kind<'a> {}

#[derive(
    Clone,
    Debug,
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
pub struct NamedKind<'a>(pub aast::Sid<'a>, pub &'a Kind<'a>);
impl<'a> TrivialDrop for NamedKind<'a> {}
