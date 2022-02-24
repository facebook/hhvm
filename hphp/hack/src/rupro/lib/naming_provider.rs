// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::GlobalAllocator;
use hh24_types::ToplevelSymbolHash;
use parking_lot::Mutex;
use pos::{ConstName, FunName, RelativePath, TypeName};
use std::fmt::{self, Debug};
use std::path::Path;

/// An abstraction over the global symbol table. Should be used by
/// `LazyShallowDeclProvider` only, since folding and typechecking logic should
/// have no need for a `NamingProvider`.
pub trait NamingProvider: Debug + Send + Sync {
    fn get_type_path(&self, name: TypeName) -> Option<RelativePath>;
    fn get_fun_path(&self, name: FunName) -> Option<RelativePath>;
    fn get_const_path(&self, name: ConstName) -> Option<RelativePath>;
}

/// A naming table in a SQLite database (with the same database schema as
/// hh_server's SQLite saved states).
pub struct SqliteNamingTable {
    alloc: &'static GlobalAllocator,
    names: Mutex<names::Names>,
}

impl SqliteNamingTable {
    pub fn new(alloc: &'static GlobalAllocator, path: impl AsRef<Path>) -> Self {
        Self {
            alloc,
            names: Mutex::new(names::Names::from_file(path).unwrap()),
        }
    }
}

impl NamingProvider for SqliteNamingTable {
    fn get_type_path(&self, name: TypeName) -> Option<RelativePath> {
        self.names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_type(name.as_str()))
            .unwrap()
            .map(|path| self.alloc.relative_path(path.prefix(), path.path()))
    }
    fn get_fun_path(&self, name: FunName) -> Option<RelativePath> {
        self.names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_fun(name.as_str()))
            .unwrap()
            .map(|path| self.alloc.relative_path(path.prefix(), path.path()))
    }
    fn get_const_path(&self, name: ConstName) -> Option<RelativePath> {
        self.names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_const(name.as_str()))
            .unwrap()
            .map(|path| self.alloc.relative_path(path.prefix(), path.path()))
    }
}

impl Debug for SqliteNamingTable {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "SqliteNamingTable")
    }
}
