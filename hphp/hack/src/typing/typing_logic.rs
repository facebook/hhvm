// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec;

use crate::typing_defs_core::{InternalType, Ty};

pub enum SubtypePropEnum<'a> {
    Coerce(Ty<'a>, Ty<'a>),
    IsSubtype(InternalType<'a>, InternalType<'a>),
    Conj(Vec<'a, SubtypeProp<'a>>),
    Disj(Vec<'a, SubtypeProp<'a>>),
}
pub type SubtypeProp<'a> = &'a SubtypePropEnum<'a>;

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
}
