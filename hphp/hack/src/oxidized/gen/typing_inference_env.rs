// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<321c5f4b89276560c45cf3908d42b20f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs::*;

pub use crate::internal_type_set as i_ty_set;
pub use crate::typing_logic as t_l;
pub use crate::typing_tyvar_occurrences as occ;
#[allow(unused_imports)]
use crate::*;

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
pub struct TyvarConstraints {
    /// Does this type variable appear covariantly in the type of the expression?
    pub appears_covariantly: bool,
    /// Does this type variable appear contravariantly in the type of the expression?
    /// If it appears in an invariant position then both will be true; if it doesn't
    /// appear at all then both will be false.
    pub appears_contravariantly: bool,
    pub lower_bounds: i_ty_set::ITySet,
    pub upper_bounds: i_ty_set::ITySet,
    /// Map associating a type to each type constant id of this variable.
    /// Whenever we localize "T1::T" in a constraint, we add a fresh type variable
    /// indexed by "T" in the type_constants of the type variable representing T1.
    /// This allows to properly check constraints on "T1::T".
    pub type_constants: s_map::SMap<(PosId, Ty)>,
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
#[repr(C, u8)]
pub enum SolvingInfo {
    /// when the type variable is bound to a type
    TVIType(Ty),
    /// when the type variable is still unsolved
    TVIConstraints(TyvarConstraints),
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
#[repr(C)]
pub struct TyvarInfo {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    pub tyvar_pos: pos::Pos,
    pub eager_solve_failed: bool,
    pub solving_info: SolvingInfo,
}

pub type Tvenv = tvid::map::Map<TyvarInfo>;

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
pub struct TypingInferenceEnv {
    pub tvenv: Tvenv,
    pub tyvars_stack: Vec<(pos::Pos, Vec<isize>)>,
    pub subtype_prop: t_l::SubtypeProp,
    pub tyvar_occurrences: typing_tyvar_occurrences::TypingTyvarOccurrences,
}
