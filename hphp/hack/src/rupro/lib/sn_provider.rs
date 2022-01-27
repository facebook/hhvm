// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use once_cell::sync::OnceCell;

use crate::alloc::GlobalAllocator;
use crate::pos::Symbol;

#[derive(Debug)]
pub struct SpecialNamesProvider {
    alloc: &'static GlobalAllocator,

    typehints: Typehints,

    construct: OnceCell<Symbol>,
    this: OnceCell<Symbol>,
}

impl SpecialNamesProvider {
    pub fn new(alloc: &'static GlobalAllocator) -> Self {
        Self {
            alloc,
            typehints: Typehints::new(alloc),
            construct: OnceCell::new(),
            this: OnceCell::new(),
        }
    }

    fn get<'a>(&self, cell: &'a OnceCell<Symbol>, name: &str) -> &'a Symbol {
        cell.get_or_init(|| self.alloc.symbol(name))
    }

    pub fn construct(&self) -> &Symbol {
        self.get(&self.construct, &"__construct")
    }

    pub fn this(&self) -> &Symbol {
        self.get(&self.this, &"$this")
    }

    pub fn typehints(&self) -> &Typehints {
        &self.typehints
    }
}

#[derive(Debug, Clone)]
pub struct Typehints {
    alloc: &'static GlobalAllocator,
    void: OnceCell<Symbol>,
    int: OnceCell<Symbol>,
}

impl Typehints {
    fn new(alloc: &'static GlobalAllocator) -> Self {
        Self {
            alloc,
            void: OnceCell::new(),
            int: OnceCell::new(),
        }
    }

    fn get<'a>(&self, cell: &'a OnceCell<Symbol>, name: &str) -> &'a Symbol {
        cell.get_or_init(|| self.alloc.symbol(name))
    }

    pub fn void(&self) -> &Symbol {
        self.get(&self.void, &"void")
    }

    pub fn int(&self) -> &Symbol {
        self.get(&self.int, &"int")
    }
}
