// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::decl_defs::{ShallowClass, ShallowFun};
use crate::pos::Symbol;
use crate::reason::Reason;

pub trait ShallowDeclCache: std::fmt::Debug {
    type R: Reason;

    fn get_shallow_class(&self, name: &Symbol) -> Option<Rc<ShallowClass<Self::R>>>;

    fn put_shallow_class(&self, name: Symbol, cls: Rc<ShallowClass<Self::R>>);

    fn get_shallow_fun(&self, name: &Symbol) -> Option<Rc<ShallowFun<Self::R>>>;

    fn put_shallow_fun(&self, name: Symbol, f: Rc<ShallowFun<Self::R>>);
}

#[derive(Debug)]
pub struct ShallowDeclLocalCache<R: Reason> {
    classes: RefCell<HashMap<Symbol, Rc<ShallowClass<R>>>>,
    funs: RefCell<HashMap<Symbol, Rc<ShallowFun<R>>>>,
}

impl<R: Reason> ShallowDeclLocalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: RefCell::new(HashMap::new()),
            funs: RefCell::new(HashMap::new()),
        }
    }
}

impl<R: Reason> ShallowDeclCache for ShallowDeclLocalCache<R> {
    type R = R;

    fn get_shallow_class(&self, name: &Symbol) -> Option<Rc<ShallowClass<Self::R>>> {
        self.classes.borrow().get(name).cloned()
    }

    fn put_shallow_class(&self, name: Symbol, cls: Rc<ShallowClass<Self::R>>) {
        self.classes.borrow_mut().insert(name, cls);
    }

    fn get_shallow_fun(&self, name: &Symbol) -> Option<Rc<ShallowFun<Self::R>>> {
        self.funs.borrow().get(name).cloned()
    }

    fn put_shallow_fun(&self, name: Symbol, f: Rc<ShallowFun<Self::R>>) {
        self.funs.borrow_mut().insert(name, f);
    }
}
