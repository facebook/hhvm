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

use crate::pos::Symbol;
use crate::reason::Reason;
use crate::typing_decl_provider::Class;

pub trait TypingDeclCache: std::fmt::Debug {
    type Reason: Reason;

    fn get_typing_class(&self, name: &Symbol) -> Option<Rc<Class<Self::Reason>>>;

    fn put_typing_class(&self, name: Symbol, cls: Rc<Class<Self::Reason>>);
}

#[derive(Debug)]
pub struct TypingDeclLocalCache<R: Reason> {
    classes: RefCell<HashMap<Symbol, Rc<Class<R>>>>,
}

impl<R: Reason> TypingDeclLocalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: RefCell::new(HashMap::new()),
        }
    }
}

impl<R: Reason> TypingDeclCache for TypingDeclLocalCache<R> {
    type Reason = R;

    fn get_typing_class(&self, name: &Symbol) -> Option<Rc<Class<Self::Reason>>> {
        self.classes.borrow().get(name).cloned()
    }

    fn put_typing_class(&self, name: Symbol, cls: Rc<Class<Self::Reason>>) {
        self.classes.borrow_mut().insert(name, cls);
    }
}
