// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod constraint;

use crate::local::Ty;
use crate::local_error::TypingError;
use crate::reason::Reason;
pub use constraint::Constraint;
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
    Disj(Option<TypingError<R>>, Vec<A>),
    Subtype(CTy<R>, CTy<R>),
}

impl<R: Reason> PropF<R, Prop<R>> {
    pub fn inj(self) -> Prop<R> {
        Prop(Hc::new(self))
    }
}

impl<R: Reason> hcons::Consable for PropF<R, Prop<R>> {
    #[inline]
    fn conser() -> &'static Conser<PropF<R, Prop<R>>> {
        R::prop_conser()
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
    pub fn valid() -> Self {
        PropF::Conj(vec![]).inj()
    }

    pub fn invalid(fail: Option<TypingError<R>>) -> Self {
        PropF::Disj(fail, vec![]).inj()
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
