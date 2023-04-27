// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::fmt::Debug;
use std::path::Path;

use anyhow::Result;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use oxidized::file_info::NameType;
use oxidized::naming_types::KindOfType;
use parking_lot::Mutex;
use pos::ConstName;
use pos::FunName;
use pos::ModuleName;
use pos::RelativePath;
use pos::TypeName;

/// An abstraction over the global symbol table. Should be used by
/// `LazyShallowDeclProvider` only, since folding and typechecking logic should
/// have no need for a `NamingProvider`.
pub trait NamingProvider: Debug + Send + Sync {
    fn get_type_path_and_kind(&self, name: TypeName) -> Result<Option<(RelativePath, KindOfType)>>;
    fn get_type_path(&self, name: TypeName) -> Result<Option<RelativePath>> {
        Ok(self.get_type_path_and_kind(name)?.map(|(path, _kind)| path))
    }
    fn get_fun_path(&self, name: FunName) -> Result<Option<RelativePath>>;
    fn get_const_path(&self, name: ConstName) -> Result<Option<RelativePath>>;
    fn get_module_path(&self, name: ModuleName) -> Result<Option<RelativePath>>;

    /// Case-insensitive lookup. Fetch the correct casing according to the
    /// symbol table.
    fn get_canon_type_name(&self, name: TypeName) -> Result<Option<TypeName>>;
    fn get_canon_fun_name(&self, name: FunName) -> Result<Option<FunName>>;
}

/// A naming table in a SQLite database (with the same database schema as
/// hh_server's SQLite saved states).
pub struct SqliteNamingTable {
    names: Mutex<names::Names>,
}

impl SqliteNamingTable {
    pub fn new(path: impl AsRef<Path>) -> anyhow::Result<Self> {
        Ok(Self {
            names: Mutex::new(names::Names::from_file(path)?),
        })
    }
}

impl NamingProvider for SqliteNamingTable {
    fn get_type_path_and_kind(&self, name: TypeName) -> Result<Option<(RelativePath, KindOfType)>> {
        let path_opt = self
            .names
            .lock()
            .get_filename(ToplevelSymbolHash::from_type(name.as_str()))?;
        Ok(path_opt.and_then(|(path, name_type)| {
            let kind = match name_type {
                NameType::Class => KindOfType::TClass,
                NameType::Typedef => KindOfType::TTypedef,
                _ => return None,
            };
            Some((RelativePath::from(&path), kind))
        }))
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
    fn get_module_path(&self, name: ModuleName) -> Result<Option<RelativePath>> {
        let path_opt = self
            .names
            .lock()
            .get_path_by_symbol_hash(ToplevelSymbolHash::from_module(name.as_str()))?;
        Ok(path_opt.map(|path| RelativePath::new(path.prefix(), path.path())))
    }
    fn get_canon_type_name(&self, name: TypeName) -> Result<Option<TypeName>> {
        Ok(self
            .names
            .lock()
            .get_type_name_case_insensitive(ToplevelCanonSymbolHash::from(name))?
            .map(Into::into))
    }
    fn get_canon_fun_name(&self, name: FunName) -> Result<Option<FunName>> {
        Ok(self
            .names
            .lock()
            .get_fun_name_case_insensitive(ToplevelCanonSymbolHash::from(name))?
            .map(Into::into))
    }
}

impl Debug for SqliteNamingTable {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "SqliteNamingTable")
    }
}
