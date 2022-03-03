// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh24_types::ToplevelSymbolHash;
use parking_lot::Mutex;
use pos::{ConstName, FunName, RelativePath, TypeName};
use std::fmt::{self, Debug};
use std::path::Path;

/// An abstraction over the global symbol table. Should be used by
/// `LazyShallowDeclProvider` only, since folding and typechecking logic should
/// have no need for a `NamingProvider`.
pub trait NamingProvider: Debug + Send + Sync {
    fn get_type_path(&self, name: TypeName) -> Result<Option<RelativePath>>;
    fn get_fun_path(&self, name: FunName) -> Result<Option<RelativePath>>;
    fn get_const_path(&self, name: ConstName) -> Result<Option<RelativePath>>;
}

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("Failed to read SQLite naming table: {0}")]
    Sqlite(#[from] rusqlite::Error),
}

/// A naming table in a SQLite database (with the same database schema as
/// hh_server's SQLite saved states).
pub struct SqliteNamingTable {
    names: Mutex<names::Names>,
}

impl SqliteNamingTable {
    pub fn new(path: impl AsRef<Path>) -> rusqlite::Result<Self> {
        Ok(Self {
            names: Mutex::new(names::Names::from_file(path)?),
        })
    }
}

impl NamingProvider for SqliteNamingTable {
    fn get_type_path(&self, name: TypeName) -> Result<Option<RelativePath>> {
        let path_opt = self
            .names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_type(name.as_str()))?;
        Ok(path_opt.map(|path| RelativePath::new(path.prefix(), path.path())))
    }
    fn get_fun_path(&self, name: FunName) -> Result<Option<RelativePath>> {
        let path_opt = self
            .names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_fun(name.as_str()))?;
        Ok(path_opt.map(|path| RelativePath::new(path.prefix(), path.path())))
    }
    fn get_const_path(&self, name: ConstName) -> Result<Option<RelativePath>> {
        let path_opt = self
            .names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_const(name.as_str()))?;
        Ok(path_opt.map(|path| RelativePath::new(path.prefix(), path.path())))
    }
}

impl Debug for SqliteNamingTable {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "SqliteNamingTable")
    }
}
