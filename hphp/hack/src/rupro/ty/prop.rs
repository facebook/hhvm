// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod constraint;

use crate::local::{Ty, Tyvar};
use crate::local_error::TypingError;
use crate::reason::Reason;
pub use constraint::Cstr;
use hcons::{Conser, Hc};
use im::HashSet;
use oxidized::ast_defs::Variance;
use pos::TypeName;
use std::ops::Deref;

/// TODO[mjt] Consider making 'constraints' 'types' to avoid this
/// or maybe rethink the inference env entirely
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum CstrTy<R: Reason> {
    Locl(Ty<R>),
    Cstr(Cstr<R>),
}

impl<R: Reason> CstrTy<R> {
    pub fn tyvars<F>(&self, get_tparam_variance: &F) -> (HashSet<Tyvar>, HashSet<Tyvar>)
    where
        F: Fn(&TypeName) -> Option<Vec<Variance>>,
    {
        match self {
            CstrTy::Locl(ty) => ty.tyvars(get_tparam_variance),
            CstrTy::Cstr(cstr) => cstr.tyvars(get_tparam_variance),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum PropF<R: Reason, A> {
    Conj(Vec<A>),
    Disj(Option<TypingError<R>>, Vec<A>),
    Subtype(CstrTy<R>, CstrTy<R>),
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

    pub fn disjs(ps: Vec<Prop<R>>, fail: Option<TypingError<R>>) -> Self {
        PropF::Disj(fail, ps).inj()
    }

    pub fn disj(self, other: Self, fail: Option<TypingError<R>>) -> Self {
        Self::disjs(vec![self, other], fail)
    }

    pub fn subtype(cty_sub: CstrTy<R>, cty_sup: CstrTy<R>) -> Self {
        PropF::Subtype(cty_sub, cty_sup).inj()
    }

    pub fn subtype_ty(ty_sub: Ty<R>, ty_sup: Ty<R>) -> Self {
        Self::subtype(CstrTy::Locl(ty_sub), CstrTy::Locl(ty_sup))
    }

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
