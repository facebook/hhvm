// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use arena_trait::TrivialDrop;
use bumpalo::Bump;
use typing_collections_rust::{pvec, Vec};

use crate::typing_defs_core::{InternalType, Ty};

#[derive(Debug)]
pub enum SubtypePropEnum<'a> {
    Coerce(Ty<'a>, Ty<'a>),
    IsSubtype(InternalType<'a>, InternalType<'a>),
    Conj(Vec<'a, SubtypeProp<'a>>),
    Disj(Vec<'a, SubtypeProp<'a>>),
}
pub type SubtypeProp<'a> = &'a SubtypePropEnum<'a>;

impl TrivialDrop for SubtypePropEnum<'_> {}

impl<'a> SubtypePropEnum<'a> {
    // Return true if this proposition is always true
    pub fn is_valid(&self) -> bool {
        match self {
            SubtypePropEnum::Conj(ps) => ps.iter().all(|p| p.is_valid()),
            SubtypePropEnum::Disj(ps) => ps.iter().any(|p| p.is_valid()),
            SubtypePropEnum::Coerce(_, _) | SubtypePropEnum::IsSubtype(_, _) => false,
        }
    }
    // Return false if this proposition is always false
    pub fn is_unsat(&self) -> bool {
        match self {
            SubtypePropEnum::Conj(ps) => ps.iter().any(|p| p.is_unsat()),
            SubtypePropEnum::Disj(ps) => ps.iter().all(|p| p.is_unsat()),
            SubtypePropEnum::Coerce(_, _) | SubtypePropEnum::IsSubtype(_, _) => false,
        }
    }

    pub fn conj(&'a self, bump: &'a Bump, other: &'a Self) -> &'a Self {
        use SubtypePropEnum as P;
        if self.is_valid() {
            other
        } else if other.is_valid() {
            self
        } else {
            let prop = match (self, other) {
                (P::Conj(props1), P::Conj(props2)) => P::Conj(props1.append(bump, props2)),
                // Preserve the order to maintain legacy behaviour. If two errors share the
                // same position then the first one to be emitted wins.
                // TODO: consider relaxing this behaviour
                (P::Conj(props), prop) => P::Conj(props.append(bump, &pvec![in bump; prop])),
                (prop, P::Conj(props)) => P::Conj((pvec![in bump; prop]).append(bump, props)),
                _ => P::Conj(pvec![in bump; self, other]),
            };
            bump.alloc(prop)
        }
    }
}
