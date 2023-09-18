// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<06410ba8c19b16532214676430a9fd9b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;
pub use typing_kinding_defs::*;

pub use crate::typing_set as ty_set;
#[allow(unused_imports)]
use crate::*;

pub type TparamName = String;

pub type TparamBounds = ty_set::TySet;

#[rust_to_ocaml(attr = "deriving hash")]
pub type TparamInfo = typing_kinding_defs::Kind;

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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C)]
pub struct TypeParameterEnv {
    /// The position indicates where the type parameter was defined.
    /// It may be Pos.none if the type parameter denotes a fresh type variable
    /// (i.e., without a source location that defines it)
    pub tparams: s_map::SMap<(pos_or_decl::PosOrDecl, TparamInfo)>,
    pub consistent: bool,
}
