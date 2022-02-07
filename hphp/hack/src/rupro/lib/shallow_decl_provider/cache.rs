// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::Cache;
use crate::decl_defs::shallow::{self, Decl, FunDecl, ShallowClass};
use crate::reason::Reason;
use pos::{Symbol, TypeName};
use std::sync::Arc;

#[derive(Debug)]
pub struct ShallowDeclCache<R: Reason> {
    pub classes: Box<dyn Cache<TypeName, Arc<ShallowClass<R>>>>,
    pub funs: Box<dyn Cache<Symbol, Arc<FunDecl<R>>>>,
}

impl<R: Reason> ShallowDeclCache<R> {
    pub fn with_no_eviction() -> Self {
        use crate::cache::NonEvictingCache;
        Self {
            classes: Box::new(NonEvictingCache::default()),
            funs: Box::new(NonEvictingCache::default()),
        }
    }

    pub fn add_decls(&self, decls: Vec<shallow::Decl<R>>) {
        for decl in decls {
            match decl {
                Decl::Class(name, decl) => self.classes.insert(name, Arc::new(decl)),
                Decl::Fun(name, decl) => self.funs.insert(name, Arc::new(decl)),
                Decl::Typedef(..) => todo!(),
                Decl::Const(..) => todo!(),
            }
        }
    }
}
