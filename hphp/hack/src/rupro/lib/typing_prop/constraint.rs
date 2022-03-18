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
use pos::Symbol;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Constraint<R: Reason> {
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

impl<R: Reason> Constraint<R> {
    pub fn has_method(
        name: Symbol,
        ty: Ty<R>,
        class_id: Symbol,
        ty_args: Vec<Ty<R>>, // TODO
    ) -> Self {
        Constraint::HasMethod {
            name,
            ty,
            class_id,
            ty_args,
        }
    }

    pub fn has_prop(name: Symbol, ty: Ty<R>, class_id: Symbol) -> Self {
        Constraint::HasProp { name, ty, class_id }
    }
}
