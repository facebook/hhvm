// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;

use hash::HashMap;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;

use crate::DeclProvider;
use crate::Result;
use crate::TypeDecl;

/// A DeclProvider that memoizes results of previous queries.
pub struct MemoProvider<'d> {
    next: &'d dyn DeclProvider,
    types: RefCell<HashMap<String, TypeDecl<'d>>>,
    funcs: RefCell<HashMap<String, &'d FunDecl<'d>>>,
    consts: RefCell<HashMap<String, &'d ConstDecl<'d>>>,
    modules: RefCell<HashMap<String, &'d ModuleDecl<'d>>>,
}

impl<'d> MemoProvider<'d> {
    pub fn new(next: &'d dyn DeclProvider) -> Self {
        Self {
            next,
            types: Default::default(),
            funcs: Default::default(),
            consts: Default::default(),
            modules: Default::default(),
        }
    }

    fn fetch_or_insert<T: Copy>(
        table: &RefCell<HashMap<String, T>>,
        symbol: &str,
        mut on_miss: impl FnMut() -> Result<T>,
    ) -> Result<T> {
        use std::collections::hash_map::Entry::*;
        match table.borrow_mut().entry(symbol.into()) {
            Occupied(e) => Ok(*e.get()),
            Vacant(e) => Ok(*e.insert(on_miss()?)),
        }
    }
}

impl<'d> DeclProvider for MemoProvider<'d> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'d>> {
        Self::fetch_or_insert(&self.types, symbol, || self.next.type_decl(symbol, depth))
    }

    fn func_decl(&self, symbol: &str) -> Result<&'d FunDecl<'d>> {
        Self::fetch_or_insert(&self.funcs, symbol, || self.next.func_decl(symbol))
    }

    fn const_decl(&self, symbol: &str) -> Result<&'d ConstDecl<'d>> {
        Self::fetch_or_insert(&self.consts, symbol, || self.next.const_decl(symbol))
    }

    fn module_decl(&self, symbol: &str) -> Result<&'d ModuleDecl<'d>> {
        Self::fetch_or_insert(&self.modules, symbol, || self.next.module_decl(symbol))
    }
}

impl<'d> std::fmt::Debug for MemoProvider<'d> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.next.fmt(f)
    }
}
