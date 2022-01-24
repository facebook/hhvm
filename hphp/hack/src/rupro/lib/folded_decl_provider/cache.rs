// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::decl_defs::FoldedClass;
use crate::pos::Symbol;
use crate::reason::Reason;

pub trait FoldedDeclCache: std::fmt::Debug {
    type Reason: Reason;

    fn get_folded_class(&self, name: &Symbol) -> Option<Rc<FoldedClass<Self::Reason>>>;

    fn put_folded_class(&self, name: Symbol, cls: Rc<FoldedClass<Self::Reason>>);
}

#[derive(Debug)]
pub struct FoldedDeclLocalCache<R: Reason> {
    classes: RefCell<HashMap<Symbol, Rc<FoldedClass<R>>>>,
}

impl<R: Reason> FoldedDeclLocalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: RefCell::new(HashMap::new()),
        }
    }
}

impl<R: Reason> FoldedDeclCache for FoldedDeclLocalCache<R> {
    type Reason = R;

    fn get_folded_class(&self, name: &Symbol) -> Option<Rc<FoldedClass<R>>> {
        self.classes.borrow().get(name).cloned()
    }

    fn put_folded_class(&self, name: Symbol, cls: Rc<FoldedClass<R>>) {
        self.classes.borrow_mut().insert(name, cls);
    }
}
