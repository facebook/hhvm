// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod constraint;

use std::ops::Deref;

pub use constraint::Cstr;
use hcons::Conser;
use hcons::Hc;

use crate::local::Ty;
use crate::local_error::TypingError;
use crate::reason::Reason;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum PropF<R: Reason, A> {
    Atom(Cstr<R>),
    Conj(Vec<A>),
    Disj(TypingError<R>, Vec<A>),
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
    pub fn conjs(ps: Vec<Prop<R>>) -> Self {
        PropF::Conj(ps).inj()
    }

    pub fn conj(self, other: Self) -> Self {
        Self::conjs(vec![self, other])
    }

    pub fn disjs(ps: Vec<Prop<R>>, fail: TypingError<R>) -> Self {
        PropF::Disj(fail, ps).inj()
    }

    pub fn disj(self, other: Self, fail: TypingError<R>) -> Self {
        Self::disjs(vec![self, other], fail)
    }

    pub fn subtype(ty_sub: Ty<R>, ty_sup: Ty<R>) -> Self {
        PropF::Atom(Cstr::subtype(ty_sub, ty_sup)).inj()
    }

    pub fn valid() -> Self {
        PropF::Conj(vec![]).inj()
    }

    pub fn invalid(fail: TypingError<R>) -> Self {
        PropF::Disj(fail, vec![]).inj()
    }

    pub fn is_valid(&self) -> bool {
        match self.deref() {
            PropF::Atom(_) => false,
            PropF::Conj(ps) => ps.iter().all(|p| p.is_valid()),
            PropF::Disj(_, ps) => ps.iter().any(|p| p.is_valid()),
        }
    }

    pub fn is_unsat(&self) -> bool {
        match self.deref() {
            PropF::Atom(_) => false,
            PropF::Conj(ps) => ps.iter().any(|p| p.is_unsat()),
            PropF::Disj(_, ps) => ps.iter().all(|p| p.is_unsat()),
        }
    }
}
