// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ad10ee6f88703fde3060d829c7ed207b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
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
pub struct TyvarConstraints<'a> {
    /// Does this type variable appear covariantly in the type of the expression?
    pub appears_covariantly: bool,
    /// Does this type variable appear contravariantly in the type of the expression?
    /// If it appears in an invariant position then both will be true; if it doesn't
    /// appear at all then both will be false.
    pub appears_contravariantly: bool,
    pub lower_bounds: &'a i_ty_set::ITySet<'a>,
    pub upper_bounds: &'a i_ty_set::ITySet<'a>,
    /// Map associating a type to each type constant id of this variable.
    /// Whenever we localize "T1::T" in a constraint, we add a fresh type variable
    /// indexed by "T" in the type_constants of the type variable representing T1.
    /// This allows to properly check constraints on "T1::T".
    pub type_constants: s_map::SMap<'a, (PosId<'a>, &'a Ty<'a>)>,
}
impl<'a> TrivialDrop for TyvarConstraints<'a> {}

#[derive(
    Clone,
    Copy,
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
pub enum SolvingInfo<'a> {
    /// when the type variable is bound to a type
    TVIType(&'a Ty<'a>),
    /// when the type variable is still unsolved
    TVIConstraints(&'a TyvarConstraints<'a>),
}
impl<'a> TrivialDrop for SolvingInfo<'a> {}

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
pub struct TyvarInfo<'a> {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    pub tyvar_pos: &'a pos::Pos<'a>,
    pub global_reason: Option<&'a reason::Reason<'a>>,
    pub eager_solve_failed: bool,
    pub solving_info: SolvingInfo<'a>,
}
impl<'a> TrivialDrop for TyvarInfo<'a> {}

pub type Tvenv<'a> = i_map::IMap<'a, &'a TyvarInfo<'a>>;

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
pub struct TypingInferenceEnv<'a> {
    pub tvenv: Tvenv<'a>,
    pub tyvars_stack: &'a [(&'a pos::Pos<'a>, &'a [ident::Ident])],
    pub subtype_prop: &'a t_l::SubtypeProp<'a>,
    pub tyvar_occurrences: &'a typing_tyvar_occurrences::TypingTyvarOccurrences<'a>,
    pub allow_solve_globals: bool,
}
impl<'a> TrivialDrop for TypingInferenceEnv<'a> {}

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
pub struct GlobalTyvarInfo<'a> {
    pub tyvar_reason: &'a reason::Reason<'a>,
    pub solving_info_g: SolvingInfo<'a>,
}
impl<'a> TrivialDrop for GlobalTyvarInfo<'a> {}

pub type GlobalTvenv<'a> = i_map::IMap<'a, &'a GlobalTyvarInfo<'a>>;

pub type TGlobal<'a> = GlobalTvenv<'a>;

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
pub struct TGlobalWithPos<'a>(pub &'a pos::Pos<'a>, pub &'a TGlobal<'a>);
impl<'a> TrivialDrop for TGlobalWithPos<'a> {}
