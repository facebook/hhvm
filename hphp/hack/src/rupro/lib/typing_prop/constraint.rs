// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use crate::reason::Reason;
use crate::typing_defs::Ty;
use hcons::{Conser, Hc};
use pos::Symbol;
use std::ops::Deref;

type TypeHint<R> = oxidized::aast::TypeHint<Ty<R>>;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum ConstraintF<R: Reason, A> {
    HasMember {
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        ty_args: Vec<TypeHint<R>>,
    },
    HasProp {
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
    },
    Union(Ty<R>, A),
    Intersection(Ty<R>, A),
}

impl<R: Reason, A> ConstraintF<R, A> {
    pub fn map<B, F>(self, f: F) -> ConstraintF<R, B>
    where
        F: FnOnce(A) -> B,
    {
        use ConstraintF::*;
        match self {
            Union(ty, x) => Union(ty, f(x)),
            Intersection(ty, x) => Intersection(ty, f(x)),
            HasMember {
                name,
                ty,
                class_id,
                ty_args,
            } => HasMember {
                name,
                ty,
                class_id,
                ty_args,
            },
            HasProp { name, ty, class_id } => HasProp { name, ty, class_id },
        }
    }
}

type Unfixed<R> = ConstraintF<R, Constraint<R>>;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Constraint<R: Reason>(R, Hc<ConstraintF<R, Constraint<R>>>);

impl<R: Reason> Deref for Constraint<R> {
    type Target = Unfixed<R>;
    // TODO: this projection should really include the reason annotation but
    // I end up with (&R, &ConstraintF<R,Constraint<R>) rather
    // than &Target === &(R, ConstraintF<...>)
    fn deref(&self) -> &Self::Target {
        Deref::deref(&self.1)
    }
}

impl<R: Reason> Constraint<R> {
    pub fn reason_for(&self) -> &R {
        &self.0
    }

    pub fn union(&mut self, conser: &Conser<Unfixed<R>>, ty: Ty<R>, reason: R) -> Self {
        Constraint(reason, conser.mk(ConstraintF::Union(ty, self.to_owned())))
    }

    pub fn intersection(&mut self, conser: &Conser<Unfixed<R>>, ty: Ty<R>, reason: R) -> Self {
        Constraint(
            reason,
            conser.mk(ConstraintF::Intersection(ty, self.to_owned())),
        )
    }

    pub fn has_member(
        conser: &Conser<Unfixed<R>>,
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        ty_args: Vec<TypeHint<R>>,
        reason: R,
    ) -> Self {
        Constraint(
            reason,
            conser.mk(ConstraintF::HasMember {
                name,
                ty,
                class_id,
                ty_args,
            }),
        )
    }

    pub fn has_prop(
        conser: &Conser<Unfixed<R>>,
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        reason: R,
    ) -> Self {
        Constraint(
            reason,
            conser.mk(ConstraintF::HasProp { name, ty, class_id }),
        )
    }

    pub fn is_union(&self) -> bool {
        matches!(self.deref(), ConstraintF::Union(_, _))
    }

    pub fn is_intersection(&self) -> bool {
        matches!(self.deref(), ConstraintF::Union(_, _))
    }
}
