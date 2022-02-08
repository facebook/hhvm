// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::Cache;
use crate::decl_defs::shallow::{self, ConstDecl, Decl, FunDecl, ShallowClass, TypedefDecl};
use crate::reason::Reason;
use pos::{ConstName, FunName, TypeName};
use std::sync::Arc;

#[derive(Clone, Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Arc<ShallowClass<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

#[derive(Debug)]
pub struct ShallowDeclCache<R: Reason> {
    types: Box<dyn Cache<TypeName, TypeDecl<R>>>,
    funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
    consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
}

impl<R: Reason> ShallowDeclCache<R> {
    pub fn new(
        types: Box<dyn Cache<TypeName, TypeDecl<R>>>,
        funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
        consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
    ) -> Self {
        Self {
            types,
            funs,
            consts,
        }
    }

    pub fn with_no_eviction() -> Self {
        use crate::cache::NonEvictingCache;
        Self::new(
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        )
    }

    pub fn add_decls(&self, decls: Vec<shallow::Decl<R>>) {
        for decl in decls {
            match decl {
                Decl::Class(name, decl) => self.types.insert(name, TypeDecl::Class(Arc::new(decl))),
                Decl::Fun(name, decl) => self.funs.insert(name, Arc::new(decl)),
                Decl::Typedef(name, decl) => {
                    self.types.insert(name, TypeDecl::Typedef(Arc::new(decl)))
                }
                Decl::Const(name, decl) => self.consts.insert(name, Arc::new(decl)),
            }
        }
    }

    pub fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>> {
        self.types.get(name)
    }

    pub fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>> {
        self.funs.get(name)
    }

    pub fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>> {
        self.consts.get(name)
    }

    pub fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.types.get(name).and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        })
    }

    pub fn get_typedef(&self, name: TypeName) -> Option<Arc<TypedefDecl<R>>> {
        self.types.get(name).and_then(|decl| match decl {
            TypeDecl::Typedef(td) => Some(td),
            TypeDecl::Class(..) => None,
        })
    }
}
