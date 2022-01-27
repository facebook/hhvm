// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::GlobalAllocator;
use crate::pos::Symbol;

#[derive(Debug)]
pub struct SpecialNames {
    pub typehints: Typehints,
    pub construct: Symbol,
    pub this: Symbol,
}

impl SpecialNames {
    pub fn new(alloc: &GlobalAllocator) -> &'static Self {
        Box::leak(Box::new(Self {
            typehints: Typehints::new(alloc),
            construct: alloc.symbol("__construct"),
            this: alloc.symbol("$this"),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct Typehints {
    pub void: Symbol,
    pub int: Symbol,
}

impl Typehints {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            void: alloc.symbol("void"),
            int: alloc.symbol("int"),
        }
    }
}
