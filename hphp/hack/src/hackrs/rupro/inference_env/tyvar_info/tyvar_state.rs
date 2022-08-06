// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use im::HashSet;
use pos::ToOxidized;
use ty::local::Ty;
use ty::local::Tyvar;
use ty::local::Variance;
use ty::reason::Reason;

use super::tyvar_constraints::TyvarConstraints;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TyvarState<R: Reason> {
    Bound(Ty<R>),
    Constrained(TyvarConstraints<R>),
    Error,
}

impl<R: Reason> Default for TyvarState<R> {
    fn default() -> TyvarState<R> {
        TyvarState::Constrained(TyvarConstraints::default())
    }
}

impl<R: Reason> TyvarState<R> {
    pub fn new(variance: Variance) -> Self {
        Self::Constrained(TyvarConstraints::new(variance))
    }

    pub fn is_bound(&self) -> bool {
        matches!(self, Self::Bound(_))
    }

    pub fn is_error(&self) -> bool {
        matches!(self, Self::Error)
    }

    pub fn binding(&self) -> Option<&Ty<R>> {
        match self {
            Self::Bound(ty) => Some(ty),
            Self::Constrained(_) | Self::Error => None,
        }
    }

    pub fn is_informative(&self) -> bool {
        self.constraints_opt()
            .map_or(true, |cstrs| cstrs.is_informative())
    }

    pub fn variance(&self) -> Option<Variance> {
        self.constraints_opt().map(|cstr| cstr.variance)
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
            Self::Constrained(cstrs) => cstrs.with_appearance(appearing),
            Self::Bound(_) | Self::Error => {}
        }
    }

    #[inline]
    fn constraints_opt(&self) -> Option<&TyvarConstraints<R>> {
        match self {
            Self::Constrained(cstrs) => Some(cstrs),
            Self::Bound(_) | Self::Error => None,
        }
    }

    fn constraints_exn(&self) -> &TyvarConstraints<R> {
        match self {
            Self::Constrained(cstrs) => cstrs,
            Self::Bound(_) => panic!("Already bound"),
            Self::Error => panic!("Error"),
        }
    }

    pub fn upper_bounds(&self) -> Option<HashSet<Ty<R>>> {
        self.constraints_opt().map(|cstr| cstr.upper_bounds.clone())
    }

    pub fn lower_bounds(&self) -> Option<HashSet<Ty<R>>> {
        self.constraints_opt().map(|cstr| cstr.lower_bounds.clone())
    }

    fn constraints(&self) -> TyvarConstraints<R> {
        match &self {
            Self::Constrained(cstrs) => cstrs.clone(),
            Self::Error => TyvarConstraints::default(),
            Self::Bound(ty) => {
                let mut lower_bounds: HashSet<Ty<R>> = Default::default();
                let mut upper_bounds: HashSet<Ty<R>> = Default::default();
                lower_bounds.insert(ty.clone());
                upper_bounds.insert(ty.clone());
                TyvarConstraints {
                    variance: Variance::default(),
                    upper_bounds,
                    lower_bounds,
                }
            }
        }
    }

    pub fn add_upper_bound(&mut self, bound: Ty<R>) {
        match self {
            Self::Error => {}
            Self::Constrained(cstrs) => cstrs.add_upper_bound(bound),
            Self::Bound(_) => {
                panic!(
                    "Attempting to add an upper bound to a type variable that is already solved"
                );
            }
        }
    }

    pub fn add_lower_bound(&mut self, bound: Ty<R>) {
        match self {
            Self::Constrained(cstrs) => cstrs.add_lower_bound(bound),
            Self::Error => {}
            Self::Bound(_) => {
                panic!("Attempting to add a lower bound to a type variable that is already solved");
            }
        }
    }

    pub fn remove_upper_bound(&mut self, bound: &Ty<R>) {
        match self {
            Self::Constrained(cstrs) => cstrs.remove_upper_bound(bound),
            Self::Error => {}
            Self::Bound(_) => {
                panic!(
                    "Attempting to remove an upper bound from a type variable that is already solved"
                );
            }
        }
    }

    pub fn remove_lower_bound(&mut self, bound: &Ty<R>) {
        match self {
            Self::Constrained(cstrs) => cstrs.remove_lower_bound(bound),
            Self::Error => {}
            Self::Bound(_) => {
                panic!(
                    "Attempting to remove a lower bound from a type variable that is already solved"
                );
            }
        }
    }
    pub fn remove_tyvar_upper_bound(&mut self, tvs: &HashSet<Tyvar>) {
        match self {
            Self::Constrained(cstrs) => cstrs.remove_tyvar_upper_bound(tvs),
            Self::Error => {}
            Self::Bound(_) => {
                panic!(
                    "Attempting to remove an upper bound from a type variable that is already solved"
                );
            }
        }
    }

    pub fn remove_tyvar_lower_bound(&mut self, tvs: &HashSet<Tyvar>) {
        match self {
            Self::Constrained(cstrs) => cstrs.remove_tyvar_lower_bound(tvs),
            Self::Error => {}
            Self::Bound(_) => {
                panic!(
                    "Attempting to remove a lower bound from a type variable that is already solved"
                );
            }
        }
    }

    pub fn add_lower_bound_as_union<F>(&mut self, bound: Ty<R>, union: F)
    where
        F: FnOnce(Ty<R>, &HashSet<Ty<R>>) -> HashSet<Ty<R>>,
    {
        match self {
            Self::Constrained(cstrs) => cstrs.add_lower_bound_as_union(bound, union),
            Self::Error => {}
            Self::Bound(_) => {
                panic!("Attempting to add a lower bound to a type variable that is already solved");
            }
        }
    }
}

impl<'a, R: Reason> ToOxidized<'a> for TyvarState<R> {
    type Output = oxidized_by_ref::typing_inference_env::SolvingInfo<'a>;

    fn to_oxidized(&self, bump: &'a bumpalo::Bump) -> Self::Output {
        use oxidized_by_ref::typing_inference_env::SolvingInfo;
        match self {
            TyvarState::Error => todo!(),
            TyvarState::Bound(ty) => SolvingInfo::TVIType(ty.to_oxidized_ref(bump)),
            TyvarState::Constrained(_cstrs) => todo!(),
        }
    }
}
