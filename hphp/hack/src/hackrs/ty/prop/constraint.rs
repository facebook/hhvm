// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use im::HashSet;
use oxidized::ast_defs::Variance;
use pos::Symbol;
use pos::TypeName;

use crate::local::Ty;
use crate::local::Tyvar;
use crate::reason::Reason;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Cstr<R: Reason> {
    Subtype(Ty<R>, Ty<R>),
    HasMethod {
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        // TODO: oxidized::aast::TypeHint (along with most oxidized::aast types)
        // can't be used in hash-consed values because it contains `Rc`s, which
        // can't be shared across threads (and we share hash-consed values
        // across threads). We'll need to map the oxidized `TypeHint` to our own
        // representation which uses `R::Pos` (probably also `pos::Symbol`,
        // `pos::TypeName`, etc).
        ty_args: Vec<Ty<R>>, // was Vec<TypeHint<R>>
    },
    HasProp {
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
    },
}

impl<R: Reason> Cstr<R> {
    pub fn subtype(ty_sub: Ty<R>, ty_sup: Ty<R>) -> Self {
        Self::Subtype(ty_sub, ty_sup)
    }

    pub fn has_method(
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        ty_args: Vec<Ty<R>>, // TODO
    ) -> Self {
        Self::HasMethod {
            name,
            ty,
            class_id,
            ty_args,
        }
    }

    pub fn has_prop(name: Symbol, ty: Ty<R>, class_id: Symbol) -> Self {
        Self::HasProp { name, ty, class_id }
    }

    pub fn tyvars<F>(&self, get_tparam_variance: &F) -> (HashSet<Tyvar>, HashSet<Tyvar>)
    where
        F: Fn(TypeName) -> Option<Vec<Variance>>,
    {
        match self {
            Cstr::Subtype(ty_sub, ty_sup) => {
                let (pos_sub, neg_sub) = ty_sub.tyvars(get_tparam_variance);
                let (pos_sup, neg_sup) = ty_sup.tyvars(get_tparam_variance);
                (pos_sub.union(pos_sup), neg_sub.union(neg_sup))
            }
            Cstr::HasMethod { ty, .. } | Cstr::HasProp { ty, .. } => ty.tyvars(get_tparam_variance),
        }
    }
}
