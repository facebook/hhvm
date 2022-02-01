// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{ShallowClass, ShallowFun};
use crate::reason::Reason;
use dashmap::DashMap;
use pos::{BuildSymbolHasher, Symbol};
use std::sync::Arc;

pub trait ShallowDeclCache: std::fmt::Debug + Send + Sync {
    type Reason: Reason;

    fn get_shallow_class(&self, name: Symbol) -> Option<Arc<ShallowClass<Self::Reason>>>;

    fn put_shallow_class(&self, name: Symbol, cls: Arc<ShallowClass<Self::Reason>>);

    fn get_shallow_fun(&self, name: Symbol) -> Option<Arc<ShallowFun<Self::Reason>>>;

    fn put_shallow_fun(&self, name: Symbol, f: Arc<ShallowFun<Self::Reason>>);
}

#[derive(Debug)]
pub struct ShallowDeclGlobalCache<R: Reason> {
    classes: DashMap<Symbol, Arc<ShallowClass<R>>, BuildSymbolHasher>,
    funs: DashMap<Symbol, Arc<ShallowFun<R>>, BuildSymbolHasher>,
}

impl<R: Reason> ShallowDeclGlobalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: DashMap::default(),
            funs: DashMap::default(),
        }
    }
}

impl<R: Reason> ShallowDeclCache for ShallowDeclGlobalCache<R> {
    type Reason = R;

    fn get_shallow_class(&self, name: Symbol) -> Option<Arc<ShallowClass<Self::Reason>>> {
        self.classes.get(&name).as_ref().map(|x| Arc::clone(x))
    }

    fn put_shallow_class(&self, name: Symbol, cls: Arc<ShallowClass<Self::Reason>>) {
        self.classes.insert(name, cls);
    }

    fn get_shallow_fun(&self, name: Symbol) -> Option<Arc<ShallowFun<Self::Reason>>> {
        self.funs.get(&name).as_ref().map(|x| Arc::clone(x))
    }

    fn put_shallow_fun(&self, name: Symbol, f: Arc<ShallowFun<Self::Reason>>) {
        self.funs.insert(name, f);
    }
}
