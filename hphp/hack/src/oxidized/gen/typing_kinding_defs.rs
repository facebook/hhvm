// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3eb607af1201875dc69a56e302ce39be>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;

pub use crate::typing_set as ty_set;
#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (hash, show)")]
pub type TparamBounds = ty_set::TySet;

/// The kind of a type, used to collect information about type paramters.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Kind {
    pub lower_bounds: TparamBounds,
    pub upper_bounds: TparamBounds,
    /// = Reified if generic parameter is marked `reify`, = Erased otherwise
    pub reified: aast::ReifyKind,
    /// Set if generic parameter has attribute <<__Enforceable>>
    pub enforceable: bool,
    /// Set if generic parameter has attribute <<__Newable>>
    pub newable: bool,
    /// Set if class is marked <<__SupportDynamicType>> and
    /// generic parameter does *not* have attribute <<__NoRequireDynamic>>
    pub require_dynamic: bool,
    pub rank: isize,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving (hash, show)")]
#[repr(C)]
pub struct NamedKind(pub typing_defs::PosId, pub Kind);
