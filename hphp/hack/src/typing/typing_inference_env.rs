// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec;

use oxidized::pos::Pos;
use oxidized::{aast, i_map, ident, s_map, typing_tyvar_occurrences};
use typing_defs_rust::{ITySet, PReason, Ty};

pub struct TyvarConstraints<'a> {
    /// Does this type variable appear covariantly in the type of the expression?
    pub appears_covariantly: bool,
    /// Does this type variable appear contravariantly in the type of the expression?
    /// If it appears in an invariant position then both will be true; if it doesn't
    /// appear at all then both will be false.
    pub appears_contravariantly: bool,
    pub lower_bounds: ITySet<'a>,
    pub upper_bounds: ITySet<'a>,
    /// Map associating a type to each type constant id of this variable.
    /// Whenever we localize "T1::T" in a constraint, we add a fresh type variable
    /// indexed by "T" in the type_constants of the type variable representing T1.
    /// This allows to properly check constraints on "T1::T".
    pub type_constants: s_map::SMap<(&'a aast::Sid, Ty<'a>)>,
    /// Map associating PU information to each instance of
    /// C:@E:@#v:@T
    /// when the type variable #v is not resolved yet. We introduce a new type
    /// variable to 'postpone' the checking of this expression until the end,
    /// when #v will be known.
    pub pu_accesses: s_map::SMap<(Ty<'a>, &'a aast::Sid, Ty<'a>, &'a aast::Sid)>,
}

pub enum SolvingInfo<'a> {
    /// when the type variable is bound to a type
    TVIType(Ty<'a>),
    /// when the type variable is still unsolved
    TVIConstraints(TyvarConstraints<'a>),
}

pub struct TyvarInfo<'a> {
    /// Where was the type variable introduced? (e.g. generic method invocation,
    /// new object construction)
    pub tyvar_pos: &'a Pos,
    pub global_reason: Option<PReason<'a>>,
    pub eager_solve_failed: bool,
    pub solving_info: SolvingInfo<'a>,
}

pub type Tvenv<'a> = i_map::IMap<TyvarInfo<'a>>;

pub struct TypingInferenceEnv<'a> {
    pub tvenv: Tvenv<'a>,
    pub tyvars_stack: Vec<'a, (&'a Pos, Vec<'a, ident::Ident>)>,
    pub tyvar_occurrences: typing_tyvar_occurrences::TypingTyvarOccurrences,
    pub allow_solve_globals: bool,
}
