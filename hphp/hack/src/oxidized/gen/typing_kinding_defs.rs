// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c06edcc010b7eee5192eeb56385bb099>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;

pub use crate::typing_set as ty_set;
#[allow(unused_imports)]
use crate::*;

pub type TparamBounds = ty_set::TySet;

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
    pub reified: aast::ReifyKind,
    pub enforceable: bool,
    pub newable: bool,
    pub require_dynamic: bool,
    pub parameters: Vec<NamedKind>,
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
#[repr(C)]
pub struct NamedKind(pub PosId, pub Kind);
