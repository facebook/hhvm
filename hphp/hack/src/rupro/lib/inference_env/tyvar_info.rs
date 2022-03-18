// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use crate::reason::Reason;
use crate::typing_defs::{Ty, Variance};
use im::HashSet;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TyvarConstraints<R: Reason> {
    variance: Variance,
    lower_bounds: HashSet<Ty<R>>,
    upper_bounds: HashSet<Ty<R>>,
}

impl<R: Reason> Default for TyvarConstraints<R> {
    fn default() -> Self {
        TyvarConstraints {
            variance: Variance::default(),
            lower_bounds: HashSet::default(),
            upper_bounds: HashSet::default(),
        }
    }
}

impl<R: Reason> TyvarConstraints<R> {
    pub fn new(
        variance: Variance,
        lower_bounds: HashSet<Ty<R>>,
        upper_bounds: HashSet<Ty<R>>,
    ) -> Self {
        TyvarConstraints {
            variance,
            lower_bounds,
            upper_bounds,
        }
    }

    pub fn append(&mut self, other: Self) {
        self.variance = self.variance.meet(&other.variance);
        self.lower_bounds.extend(other.lower_bounds);
        self.upper_bounds.extend(other.upper_bounds);
    }

    pub fn is_informative(&self) -> bool {
        !self.variance.is_bivariant()
            || !self.lower_bounds.is_empty()
            || !self.upper_bounds.is_empty()
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TyvarState<R: Reason> {
    Bound(Ty<R>),
    Constrained(TyvarConstraints<R>),
}

impl<R: Reason> Default for TyvarState<R> {
    fn default() -> TyvarState<R> {
        TyvarState::Constrained(TyvarConstraints::default())
    }
}

impl<R: Reason> TyvarState<R> {
    pub fn is_bound(&self) -> bool {
        matches!(self, TyvarState::Bound(_))
    }

    pub fn binding(&self) -> Option<&Ty<R>> {
        if let Self::Bound(ty) = self {
            Some(ty)
        } else {
            None
        }
    }

    pub fn is_informative(&self) -> bool {
        match self {
            TyvarState::Bound(_) => true,
            TyvarState::Constrained(cstrs) => cstrs.is_informative(),
        }
    }

    fn constraints_opt(&self) -> Option<&TyvarConstraints<R>> {
        match self {
            TyvarState::Constrained(cstrs) => Some(cstrs),
            TyvarState::Bound(_) => None,
        }
    }

    fn constraints(&self) -> TyvarConstraints<R> {
        match &self {
            TyvarState::Constrained(cstrs) => cstrs.clone(),
            TyvarState::Bound(ty) => {
                let mut lower_bounds: HashSet<Ty<R>> = Default::default();
                let mut upper_bounds: HashSet<Ty<R>> = Default::default();
                lower_bounds.insert(ty.clone());
                upper_bounds.insert(ty.clone());
                TyvarConstraints::new(Default::default(), upper_bounds, lower_bounds)
            }
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TyvarInfo<R: Reason> {
    pos: Option<R::Pos>,
    state: TyvarState<R>,
    early_solve_failed: bool,
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
}
