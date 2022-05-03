// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use super::tyvar_constraints::TyvarConstraints;
use im::HashSet;
use ty::{
    local::{Ty, Variance},
    prop::CstrTy,
    reason::Reason,
};

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
        match self {
            Self::Bound(ty) => Some(ty),
            Self::Constrained(_) => None,
        }
    }

    pub fn is_informative(&self) -> bool {
        self.constraints_opt()
            .map_or(true, |cstrs| cstrs.is_informative())
    }

    pub fn appears_covariantly(&self) -> bool {
        self.constraints_opt()
            .map_or(false, |cstrs| cstrs.appears_covariantly())
    }

    pub fn appears_contravariantly(&self) -> bool {
        self.constraints_opt()
            .map_or(false, |cstrs| cstrs.appears_contravariantly())
    }

    pub fn with_appearance(&mut self, appearing: &Variance) {
        match self {
            TyvarState::Constrained(cstrs) => cstrs.with_appearance(appearing),
            TyvarState::Bound(_) => {}
        }
    }

    #[inline]
    fn constraints_opt(&self) -> Option<&TyvarConstraints<R>> {
        match self {
            TyvarState::Constrained(cstrs) => Some(cstrs),
            TyvarState::Bound(_) => None,
        }
    }

    fn constraints_exn(&self) -> &TyvarConstraints<R> {
        match self {
            TyvarState::Constrained(cstrs) => cstrs,
            TyvarState::Bound(_) => panic!("Already bound"),
        }
    }

    pub fn upper_bounds(&self) -> Option<HashSet<CstrTy<R>>> {
        self.constraints_opt().map(|cstr| cstr.upper_bounds.clone())
    }

    pub fn lower_bounds(&self) -> Option<HashSet<CstrTy<R>>> {
        self.constraints_opt().map(|cstr| cstr.lower_bounds.clone())
    }

    fn constraints(&self) -> TyvarConstraints<R> {
        match &self {
            TyvarState::Constrained(cstrs) => cstrs.clone(),
            TyvarState::Bound(ty) => {
                let mut lower_bounds: HashSet<CstrTy<R>> = Default::default();
                let mut upper_bounds: HashSet<CstrTy<R>> = Default::default();
                lower_bounds.insert(CstrTy::Locl(ty.clone()));
                upper_bounds.insert(CstrTy::Locl(ty.clone()));
                TyvarConstraints {
                    variance: Variance::default(),
                    upper_bounds,
                    lower_bounds,
                }
            }
        }
    }

    pub fn add_upper_bound(&mut self, bound: CstrTy<R>) {
        match self {
            TyvarState::Constrained(cstrs) => cstrs.add_upper_bound(bound),
            TyvarState::Bound(_) => {
                panic!(
                    "Attempting to add an upper bound to a type variable that is already solved"
                );
            }
        }
    }

    pub fn add_lower_bound(&mut self, bound: CstrTy<R>) {
        match self {
            TyvarState::Constrained(cstrs) => cstrs.add_lower_bound(bound),
            TyvarState::Bound(_) => {
                panic!("Attempting to add a lower bound to a type variable that is already solved");
            }
        }
    }

    pub fn add_lower_bound_as_union<F>(&mut self, bound: CstrTy<R>, union: F)
    where
        F: FnOnce(CstrTy<R>, &HashSet<CstrTy<R>>) -> HashSet<CstrTy<R>>,
    {
        match self {
            TyvarState::Constrained(cstrs) => cstrs.add_lower_bound_as_union(bound, union),
            TyvarState::Bound(_) => {
                panic!("Attempting to add a lower bound to a type variable that is already solved");
            }
        }
    }
}
