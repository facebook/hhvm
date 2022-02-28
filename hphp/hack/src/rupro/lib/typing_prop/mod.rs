// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod constraint;

use crate::reason::Reason;
use crate::typing_defs::Ty;
use crate::typing_error::TypingError;
pub use constraint::{Constraint, ConstraintF};
use hcons::{Conser, Hc};
use std::ops::Deref;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum CTy<R: Reason> {
    Locl(Ty<R>),
    Cstr(Constraint<R>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum PropF<R: Reason, A> {
    Conj(Vec<A>),
    // The i32 is placeholder for error callbacks
    Disj(Option<TypingError<R>>, Vec<A>),
    Subtype(CTy<R>, CTy<R>),
}

impl<R: Reason> PropF<R, Prop<R>> {
    pub fn inj(self, conser: Conser<PropF<R, Prop<R>>>) -> Prop<R> {
        Prop(conser.mk(self))
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Prop<R: Reason>(Hc<PropF<R, Prop<R>>>);

impl<R: Reason> Deref for Prop<R> {
    type Target = PropF<R, Prop<R>>;
    fn deref(&self) -> &Self::Target {
        let Prop(hc_prop_f) = self;
        Deref::deref(hc_prop_f)
    }
}

impl<R: Reason> Prop<R> {
    pub fn valid(conser: Conser<PropF<R, Prop<R>>>) -> Self {
        PropF::Conj(vec![]).inj(conser)
    }

    pub fn invalid(conser: Conser<PropF<R, Prop<R>>>, fail: Option<TypingError<R>>) -> Self {
        PropF::Disj(fail, vec![]).inj(conser)
    }

    pub fn is_valid(&self) -> bool {
        match self.deref() {
            PropF::Subtype(_, _) => false,
            PropF::Conj(ps) => ps.iter().all(|p| p.is_valid()),
            PropF::Disj(_, ps) => ps.iter().any(|p| p.is_valid()),
        }
    }

    pub fn is_unsat(&self) -> bool {
        match self.deref() {
            PropF::Subtype(_, _) => false,
            PropF::Conj(ps) => ps.iter().any(|p| p.is_unsat()),
            PropF::Disj(_, ps) => ps.iter().all(|p| p.is_unsat()),
        }
    }
}
