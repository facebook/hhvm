// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<83a9e1b0dbaf1f941eb1d02ccd0f5759>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub use crate::internal_type_set as i_ty_set;
pub use crate::typing_logic as t_l;
pub use crate::typing_tyvar_occurrences as occ;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
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
    pub type_constants: s_map::SMap<(aast::Sid, Ty)>,
    /// Map associating PU information to each instance of
    /// C:@E:@#v:@T
    /// when the type variable #v is not resolved yet. We introduce a new type
    /// variable to 'postpone' the checking of this expression until the end,
    /// when #v will be known.
    pub pu_accesses: s_map::SMap<(Ty, aast::Sid, Ty, aast::Sid)>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
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
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TyvarInfo {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    pub tyvar_pos: pos::Pos,
    pub global_reason: Option<reason::Reason>,
    pub eager_solve_failed: bool,
    pub solving_info: SolvingInfo,
}

pub type Tvenv = i_map::IMap<TyvarInfo>;

#[derive(
    Clone,
    Debug,
    Default,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TypingInferenceEnv {
    pub tvenv: Tvenv,
    pub tyvars_stack: Vec<(pos::Pos, Vec<ident::Ident>)>,
    pub subtype_prop: t_l::SubtypeProp,
    pub tyvar_occurrences: typing_tyvar_occurrences::TypingTyvarOccurrences,
    pub allow_solve_globals: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct GlobalTyvarInfo {
    pub tyvar_reason: reason::Reason,
    pub solving_info_g: SolvingInfo,
}

pub type GlobalTvenv = i_map::IMap<GlobalTyvarInfo>;

pub type TGlobal = GlobalTvenv;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TGlobalWithPos(pub pos::Pos, pub TGlobal);
