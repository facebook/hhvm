// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

mod tyvar_constraints;
mod tyvar_state;

use im::HashSet;
use ty::{
    local::{Ty, Variance},
    prop::CstrTy,
    reason::Reason,
};
use tyvar_state::TyvarState;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TyvarInfo<R: Reason> {
    pos: Option<R::Pos>,
    state: TyvarState<R>,
    early_solve_failed: bool,
}

impl<R: Reason> TyvarInfo<R> {
    pub fn is_solved(&self) -> bool {
        matches!(self.state, TyvarState::Bound(_))
    }

    pub fn bind(&mut self, pos: Option<R::Pos>, ty: Ty<R>) {
        self.pos = pos;
        self.state = TyvarState::Bound(ty);
    }

    pub fn binding(&self) -> Option<&Ty<R>> {
        self.state.binding()
    }

    pub fn upper_bounds(&self) -> Option<HashSet<CstrTy<R>>> {
        self.state.upper_bounds()
    }

    pub fn lower_bounds(&self) -> Option<HashSet<CstrTy<R>>> {
        self.state.lower_bounds()
    }

    pub fn add_upper_bound(&mut self, bound: CstrTy<R>) {
        self.state.add_upper_bound(bound)
    }
    pub fn add_lower_bound(&mut self, bound: CstrTy<R>) {
        self.state.add_lower_bound(bound)
    }
    pub fn add_lower_bound_as_union<F>(&mut self, bound: CstrTy<R>, union: F)
    where
        F: FnOnce(CstrTy<R>, &HashSet<CstrTy<R>>) -> HashSet<CstrTy<R>>,
    {
        self.state.add_lower_bound_as_union(bound, union)
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
            pos: None,
            state: TyvarState::default(),
            early_solve_failed: false,
        }
    }
}
