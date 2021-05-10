// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<28a5c3f6e1749a0ec75e3413301bbdc9>>
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

pub use crate::internal_type_set as i_ty_set;
pub use crate::typing_logic as t_l;
pub use crate::typing_tyvar_occurrences as occ;

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
pub struct TyvarConstraints<'a> {
    /// Does this type variable appear covariantly in the type of the expression?
    pub appears_covariantly: bool,
    /// Does this type variable appear contravariantly in the type of the expression?
    /// If it appears in an invariant position then both will be true; if it doesn't
    /// appear at all then both will be false.
    pub appears_contravariantly: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower_bounds: &'a i_ty_set::ITySet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper_bounds: &'a i_ty_set::ITySet<'a>,
    /// Map associating a type to each type constant id of this variable.
    /// Whenever we localize "T1::T" in a constraint, we add a fresh type variable
    /// indexed by "T" in the type_constants of the type variable representing T1.
    /// This allows to properly check constraints on "T1::T".
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_constants: s_map::SMap<'a, (PosId<'a>, &'a Ty<'a>)>,
}
impl<'a> TrivialDrop for TyvarConstraints<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TyvarConstraints<'arena>);

#[derive(
    Clone,
    Copy,
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
pub enum SolvingInfo<'a> {
    /// when the type variable is bound to a type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TVIType(&'a Ty<'a>),
    /// when the type variable is still unsolved
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TVIConstraints(&'a TyvarConstraints<'a>),
}
impl<'a> TrivialDrop for SolvingInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SolvingInfo<'arena>);

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
pub struct TyvarInfo<'a> {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tyvar_pos: &'a pos::Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub global_reason: Option<&'a reason::Reason<'a>>,
    pub eager_solve_failed: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub solving_info: SolvingInfo<'a>,
}
impl<'a> TrivialDrop for TyvarInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TyvarInfo<'arena>);

pub type Tvenv<'a> = i_map::IMap<'a, &'a TyvarInfo<'a>>;

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
pub struct TypingInferenceEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tvenv: Tvenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tyvars_stack: &'a [(&'a pos::Pos<'a>, &'a [ident::Ident])],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub subtype_prop: &'a t_l::SubtypeProp<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tyvar_occurrences: &'a typing_tyvar_occurrences::TypingTyvarOccurrences<'a>,
    pub allow_solve_globals: bool,
}
impl<'a> TrivialDrop for TypingInferenceEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypingInferenceEnv<'arena>);

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
pub struct GlobalTyvarInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tyvar_reason: &'a reason::Reason<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub solving_info_g: SolvingInfo<'a>,
}
impl<'a> TrivialDrop for GlobalTyvarInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(GlobalTyvarInfo<'arena>);

pub type GlobalTvenv<'a> = i_map::IMap<'a, &'a GlobalTyvarInfo<'a>>;

pub type TGlobal<'a> = GlobalTvenv<'a>;

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
pub struct TGlobalWithPos<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a pos::Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TGlobal<'a>,
);
impl<'a> TrivialDrop for TGlobalWithPos<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TGlobalWithPos<'arena>);
