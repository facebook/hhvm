// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<db4d5a43405094a4b56067a0acd49c06>>
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

pub use crate::typing_set as ty_set;

pub type TparamBounds<'a> = ty_set::TySet<'a>;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct TparamInfo<'a> {
    pub lower_bounds: TparamBounds<'a>,
    pub upper_bounds: TparamBounds<'a>,
    pub reified: oxidized::aast::ReifyKind,
    pub enforceable: bool,
    pub newable: bool,
    /// If this is non-empty, the type parameter is higher-kinded
    pub parameters: &'a [(aast::Sid<'a>, &'a TparamInfo<'a>)],
}
impl<'a> TrivialDrop for TparamInfo<'a> {}

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
    pub tparams: s_map::SMap<'a, TparamInfo<'a>>,
    pub consistent: bool,
}
impl<'a> TrivialDrop for TypeParameterEnv<'a> {}
