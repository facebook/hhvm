// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use im::HashSet;

use super::ty::Ty;
use crate::reason::Reason;

#[derive(Debug, Clone, PartialEq, Eq, Hash, Default)]
pub struct KindFlags {
    enforcable: bool,
    newable: bool,
    require_dynamic: bool,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Kind<R: Reason> {
    lower_bounds: HashSet<Ty<R>>,
    upper_bounds: HashSet<Ty<R>>,
    flags: KindFlags,
}

impl<R: Reason> Kind<R> {
    pub fn size(&self) -> usize {
        self.lower_bounds.len() + self.upper_bounds.len()
    }

    pub fn add_upper_bound(&mut self, ty: Ty<R>) {
        self.upper_bounds.insert(ty);
    }

    pub fn remove_upper_bound(&mut self, ty: &Ty<R>) {
        self.upper_bounds.remove(ty);
    }

    pub fn set_upper_bounds(&mut self, upper_bounds: HashSet<Ty<R>>) {
        self.upper_bounds = upper_bounds;
    }

    pub fn upper_bounds(&self) -> &HashSet<Ty<R>> {
        &self.upper_bounds
    }

    pub fn add_lower_bound(&mut self, ty: Ty<R>) {
        self.lower_bounds.insert(ty);
    }

    pub fn remove_lower_bound(&mut self, ty: &Ty<R>) {
        self.lower_bounds.remove(ty);
    }

    pub fn set_lower_bounds(&mut self, lower_bounds: HashSet<Ty<R>>) {
        self.lower_bounds = lower_bounds;
    }

    pub fn lower_bounds(&self) -> &HashSet<Ty<R>> {
        &self.lower_bounds
    }
}

impl<R: Reason> Default for Kind<R> {
    fn default() -> Self {
        Kind {
            lower_bounds: HashSet::default(),
            upper_bounds: HashSet::default(),
            flags: KindFlags::default(),
        }
    }
}
