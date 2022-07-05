// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

mod tyvar_constraints;
mod tyvar_state;

use im::HashSet;
use pos::Pos;
use pos::ToOxidized;
use ty::local::Ty;
use ty::local::Tyvar;
use ty::local::Variance;
use ty::reason::Reason;
use tyvar_state::TyvarState;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TyvarInfo<R: Reason> {
    pos: R::Pos,
    state: TyvarState<R>,
    early_solve_failed: bool,
}

impl<R: Reason> TyvarInfo<R> {
    pub fn new(variance: Variance, pos: R::Pos) -> Self {
        TyvarInfo {
            pos,
            state: TyvarState::new(variance),
            early_solve_failed: false,
        }
    }

    pub fn is_solved(&self) -> bool {
        self.state.is_bound()
    }

    pub fn is_error(&self) -> bool {
        self.state.is_error()
    }

    pub fn bind(&mut self, pos: R::Pos, ty: Ty<R>) {
        self.pos = pos;
        let ty = if self.early_solve_failed {
            ty.map_reason(|r| R::early_solve_failed(r.pos().clone()))
        } else {
            ty
        };
        self.state = TyvarState::Bound(ty);
    }

    pub fn mark_error(&mut self) {
        self.state = TyvarState::Error;
    }

    pub fn binding(&self) -> Option<&Ty<R>> {
        self.state.binding()
    }

    pub fn upper_bounds(&self) -> Option<HashSet<Ty<R>>> {
        self.state.upper_bounds()
    }

    pub fn lower_bounds(&self) -> Option<HashSet<Ty<R>>> {
        self.state.lower_bounds()
    }

    pub fn add_upper_bound(&mut self, bound: Ty<R>) {
        self.state.add_upper_bound(bound)
    }
    pub fn add_lower_bound(&mut self, bound: Ty<R>) {
        self.state.add_lower_bound(bound)
    }
    pub fn add_lower_bound_as_union<F>(&mut self, bound: Ty<R>, union: F)
    where
        F: FnOnce(Ty<R>, &HashSet<Ty<R>>) -> HashSet<Ty<R>>,
    {
        self.state.add_lower_bound_as_union(bound, union)
    }

    pub fn remove_upper_bound(&mut self, bound: &Ty<R>) {
        self.state.remove_upper_bound(bound)
    }

    pub fn remove_lower_bound(&mut self, bound: &Ty<R>) {
        self.state.remove_lower_bound(bound)
    }

    pub fn remove_tyvar_upper_bound(&mut self, tvs: &HashSet<Tyvar>) {
        self.state.remove_tyvar_upper_bound(tvs)
    }

    pub fn remove_tyvar_lower_bound(&mut self, tvs: &HashSet<Tyvar>) {
        self.state.remove_tyvar_lower_bound(tvs)
    }

    pub fn variance(&self) -> Option<Variance> {
        self.state.variance()
    }

    pub fn appears_covariantly(&self) -> bool {
        self.state.appears_covariantly()
    }

    pub fn appears_contravariantly(&self) -> bool {
        self.state.appears_contravariantly()
    }

    pub fn with_appearance(&mut self, appearing: &Variance) {
        self.state.with_appearance(appearing)
    }
}

impl<R: Reason> Default for TyvarInfo<R> {
    fn default() -> Self {
        TyvarInfo {
            pos: R::Pos::none(),
            state: TyvarState::default(),
            early_solve_failed: false,
        }
    }
}

impl<'a, R: Reason> ToOxidized<'a> for TyvarInfo<R> {
    type Output = oxidized_by_ref::typing_inference_env::TyvarInfo<'a>;

    fn to_oxidized(&self, bump: &'a bumpalo::Bump) -> Self::Output {
        let TyvarInfo {
            pos,
            state,
            early_solve_failed,
        } = self;
        oxidized_by_ref::typing_inference_env::TyvarInfo {
            tyvar_pos: pos.to_oxidized(bump),
            global_reason: None,
            eager_solve_failed: *early_solve_failed,
            solving_info: state.to_oxidized(bump),
        }
    }
}
