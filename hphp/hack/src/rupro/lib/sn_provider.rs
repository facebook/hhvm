// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use once_cell::sync::OnceCell;

use crate::pos::Symbol;
use crate::pos_provider::PosProvider;

#[derive(Debug)]
pub struct SpecialNamesProvider {
    pos_provider: Rc<PosProvider>,

    typehints: Typehints,

    construct: OnceCell<Symbol>,
    this: OnceCell<Symbol>,
}

impl SpecialNamesProvider {
    pub fn new(pos_provider: Rc<PosProvider>) -> Self {
        let typehints = Typehints::new(Rc::clone(&pos_provider));
        Self {
            pos_provider,
            typehints,
            construct: OnceCell::new(),
            this: OnceCell::new(),
        }
    }

    fn get<'a>(&self, cell: &'a OnceCell<Symbol>, name: &str) -> &'a Symbol {
        cell.get_or_init(|| self.pos_provider.mk_symbol(name))
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
    pos_provider: Rc<PosProvider>,

    void: OnceCell<Symbol>,
    int: OnceCell<Symbol>,
}

impl Typehints {
    fn new(pos_provider: Rc<PosProvider>) -> Self {
        Self {
            pos_provider,

            void: OnceCell::new(),
            int: OnceCell::new(),
        }
    }

    fn get<'a>(&self, cell: &'a OnceCell<Symbol>, name: &str) -> &'a Symbol {
        cell.get_or_init(|| self.pos_provider.mk_symbol(name))
    }

    pub fn void(&self) -> &Symbol {
        self.get(&self.void, &"void")
    }

    pub fn int(&self) -> &Symbol {
        self.get(&self.int, &"int")
    }
}
