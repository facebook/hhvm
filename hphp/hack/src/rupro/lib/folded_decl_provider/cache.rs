// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::FoldedClass;
use crate::reason::Reason;
use dashmap::DashMap;
use pos::Symbol;
use std::sync::Arc;

pub trait FoldedDeclCache: std::fmt::Debug + Send + Sync {
    type Reason: Reason;

    fn get_folded_class(&self, name: Symbol) -> Option<Arc<FoldedClass<Self::Reason>>>;

    fn put_folded_class(&self, name: Symbol, cls: Arc<FoldedClass<Self::Reason>>);
}

#[derive(Debug)]
pub struct FoldedDeclGlobalCache<R: Reason> {
    classes: DashMap<Symbol, Arc<FoldedClass<R>>>,
}

impl<R: Reason> FoldedDeclGlobalCache<R> {
    pub fn new() -> Self {
        Self {
            classes: DashMap::new(),
        }
    }
}

impl<R: Reason> FoldedDeclCache for FoldedDeclGlobalCache<R> {
    type Reason = R;

    fn get_folded_class(&self, name: Symbol) -> Option<Arc<FoldedClass<R>>> {
        self.classes.get(&name).as_ref().map(|x| Arc::clone(x))
    }

    fn put_folded_class(&self, name: Symbol, cls: Arc<FoldedClass<R>>) {
        self.classes.insert(name, cls);
    }
}
