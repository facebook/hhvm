// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;
use std::rc::Rc;

#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct Ident(u64);

impl From<u64> for Ident {
    fn from(x: u64) -> Self {
        Self(x)
    }
}

#[derive(Debug, Clone)]
pub struct IdentGen {
    next: Rc<RefCell<u64>>,
}

impl IdentGen {
    pub fn new() -> Self {
        Self {
            next: Rc::new(RefCell::new(1)),
        }
    }

    pub fn make(&self) -> Ident {
        let mut r = self.next.borrow_mut();
        let v = *r;
        *r += 1;
        Ident(v)
    }
}
