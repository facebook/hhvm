// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use im::HashSet;
use ty::{local::Variance, prop::CstrTy, reason::Reason};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TyvarConstraints<R: Reason> {
    pub variance: Variance,
    pub lower_bounds: HashSet<CstrTy<R>>,
    pub upper_bounds: HashSet<CstrTy<R>>,
}

impl<R: Reason> TyvarConstraints<R> {
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

    pub fn appears_covariantly(&self) -> bool {
        self.variance.appears_covariantly()
    }

    pub fn appears_contravariantly(&self) -> bool {
        self.variance.appears_contravariantly()
    }

    pub fn add_upper_bound(&mut self, bound: CstrTy<R>) {
        rupro_todo_mark!(
            UnionsIntersections,
            "Provide a function to simplify the upper bound as an intersection"
        );
        self.upper_bounds.insert(bound);
    }

    pub fn add_lower_bound(&mut self, bound: CstrTy<R>) {
        self.lower_bounds.insert(bound);
    }

    pub fn add_lower_bound_as_union<F>(&mut self, bound: CstrTy<R>, union: F)
    where
        F: FnOnce(CstrTy<R>, &HashSet<CstrTy<R>>) -> HashSet<CstrTy<R>>,
    {
        self.lower_bounds = union(bound, &self.lower_bounds);
    }

    pub fn with_appearance(&mut self, appearing: &Variance) {
        self.variance = self.variance.join(appearing)
    }
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
