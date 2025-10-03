// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::sync::Arc;

use hash::HashMap;

use crate::ConstDecl;
use crate::DeclProvider;
use crate::FunDecl;
use crate::ModuleDecl;
use crate::Result;
use crate::TypeDecl;

/// A DeclProvider that memoizes results of previous queries.
pub struct MemoProvider {
    next: Arc<dyn DeclProvider>,
    types: RefCell<HashMap<String, TypeDecl>>,
    funcs: RefCell<HashMap<String, FunDecl>>,
    consts: RefCell<HashMap<String, ConstDecl>>,
    modules: RefCell<HashMap<String, ModuleDecl>>,
}

impl MemoProvider {
    pub fn new(next: Arc<dyn DeclProvider>) -> Self {
        Self {
            next,
            types: Default::default(),
            funcs: Default::default(),
            consts: Default::default(),
            modules: Default::default(),
        }
    }

    fn fetch_or_insert<T: Clone>(
        table: &RefCell<HashMap<String, T>>,
        symbol: &str,
        mut on_miss: impl FnMut() -> Result<T>,
    ) -> Result<T> {
        use std::collections::hash_map::Entry::*;
        match table.borrow_mut().entry(symbol.into()) {
            Occupied(e) => Ok(e.get().clone()),
            Vacant(e) => {
                let miss = on_miss()?;
                e.insert(miss.clone());
                Ok(miss)
            }
        }
    }
}

impl DeclProvider for MemoProvider {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl> {
        Self::fetch_or_insert(&self.types, symbol, || self.next.type_decl(symbol, depth))
    }

    fn func_decl(&self, symbol: &str) -> Result<FunDecl> {
        Self::fetch_or_insert(&self.funcs, symbol, || self.next.func_decl(symbol))
    }

    fn const_decl(&self, symbol: &str) -> Result<ConstDecl> {
        Self::fetch_or_insert(&self.consts, symbol, || self.next.const_decl(symbol))
    }

    fn module_decl(&self, symbol: &str) -> Result<ModuleDecl> {
        Self::fetch_or_insert(&self.modules, symbol, || self.next.module_decl(symbol))
    }
}

impl std::fmt::Debug for MemoProvider {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.next.fmt(f)
    }
}
