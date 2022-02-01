// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::Reason;
use crate::typing_decl_provider::Class;
use dashmap::DashMap;
use pos::Symbol;
use std::sync::Arc;

pub trait TypingDeclCache: std::fmt::Debug + Send + Sync {
    type Reason: Reason;

    fn get_typing_class(&self, name: &Symbol) -> Option<Arc<Class<Self::Reason>>>;

    fn put_typing_class(&self, name: Symbol, cls: Arc<Class<Self::Reason>>);
}

#[derive(Debug)]
pub struct TypingDeclGlobalCache<R: Reason> {
    classes: DashMap<Symbol, Arc<Class<R>>>,
}

impl<R: Reason> TypingDeclGlobalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: DashMap::new(),
        }
    }
}

impl<R: Reason> TypingDeclCache for TypingDeclGlobalCache<R> {
    type Reason = R;

    fn get_typing_class(&self, name: &Symbol) -> Option<Arc<Class<Self::Reason>>> {
        self.classes.get(name).as_ref().map(|x| Arc::clone(x))
    }

    fn put_typing_class(&self, name: Symbol, cls: Arc<Class<Self::Reason>>) {
        self.classes.insert(name, cls);
    }
}
