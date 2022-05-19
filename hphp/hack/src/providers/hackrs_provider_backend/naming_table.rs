// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::cache::{Cache, ChangesCache};
use hh24_types::{ToplevelCanonSymbolHash, ToplevelSymbolHash};
use naming_provider::NamingProvider;
use ocamlrep::rc::RcOc;
use oxidized::{
    file_info::{self, NameType},
    naming_types,
};
use parking_lot::Mutex;
use pos::{ConstName, FunName, ModuleName, RelativePath, TypeName};
use reverse_naming_table::ReverseNamingTable;
use std::path::PathBuf;
use std::sync::Arc;

pub use naming_provider::Result;

/// Designed after naming_heap.ml.
pub struct NamingTable {
    types: ReverseNamingTable<TypeName, (Pos, naming_types::KindOfType), TypeDb>,
    funs: ReverseNamingTable<FunName, Pos, FunDb>,
    consts: ChangesCache<ToplevelSymbolHash, Pos, ConstDb>,
    modules: ChangesCache<ToplevelSymbolHash, Pos, ModuleDb>,
    db: Arc<MaybeNamingDb>,
}

impl NamingTable {
    pub fn new() -> Self {
        let db = Arc::new(MaybeNamingDb(Mutex::new(None)));
        Self {
            types: ReverseNamingTable::new(TypeDb(Arc::clone(&db))),
            funs: ReverseNamingTable::new(FunDb(Arc::clone(&db))),
            consts: ChangesCache::new(ConstDb(Arc::clone(&db))),
            modules: ChangesCache::new(ModuleDb(Arc::clone(&db))),
            db,
        }
    }

    pub fn db_path(&self) -> Option<PathBuf> {
        self.db.db_path()
    }

    pub fn set_db_path(&self, db_path: PathBuf) -> Result<()> {
        Ok(self.db.set_db_path(db_path)?)
    }

    pub fn add_type(
        &self,
        name: TypeName,
        pos_and_kind: &(file_info::Pos, naming_types::KindOfType),
    ) {
        self.types
            .insert(name, ((&pos_and_kind.0).into(), pos_and_kind.1));
    }

    pub fn get_type_pos(
        &self,
        name: TypeName,
    ) -> Result<Option<(file_info::Pos, naming_types::KindOfType)>> {
        Ok(self
            .types
            .get_pos(name)
            .map(|(pos, kind)| (pos.into(), kind)))
    }

    pub fn remove_type_batch(&self, names: &[TypeName]) {
        self.types.remove_batch(names.iter().copied());
    }

    pub fn get_canon_type_name(&self, name: TypeName) -> Result<Option<TypeName>> {
        Ok(self.types.get_canon_name(name))
    }

    pub fn add_fun(&self, name: FunName, pos: &file_info::Pos) {
        self.funs.insert(name, pos.into());
    }

    pub fn get_fun_pos(&self, name: FunName) -> Result<Option<file_info::Pos>> {
        Ok(self.funs.get_pos(name).map(Into::into))
    }

    pub fn remove_fun_batch(&self, names: &[FunName]) {
        self.funs.remove_batch(names.iter().copied());
    }

    pub fn get_canon_fun_name(&self, name: FunName) -> Result<Option<FunName>> {
        Ok(self.funs.get_canon_name(name))
    }

    pub fn add_const(&self, name: ConstName, pos: &file_info::Pos) {
        self.consts.insert(name.into(), pos.into());
    }

    pub fn get_const_pos(&self, name: ConstName) -> Result<Option<file_info::Pos>> {
        Ok(self.consts.get(name.into()).map(Into::into))
    }

    pub fn remove_const_batch(&self, names: &[ConstName]) {
        self.consts
            .remove_batch(names.iter().copied().map(Into::into));
    }

    pub fn add_module(&self, name: ModuleName, pos: &file_info::Pos) {
        self.modules.insert(name.into(), pos.into());
    }

    pub fn get_module_pos(&self, name: ModuleName) -> Result<Option<file_info::Pos>> {
        Ok(self.modules.get(name.into()).map(Into::into))
    }

    pub fn remove_module_batch(&self, names: &[ModuleName]) {
        self.modules
            .remove_batch(names.iter().copied().map(Into::into));
    }

    pub fn push_local_changes(&self) {
        self.types.push_local_changes();
        self.funs.push_local_changes();
        self.consts.push_local_changes();
        self.modules.push_local_changes();
    }

    pub fn pop_local_changes(&self) {
        self.types.pop_local_changes();
        self.funs.pop_local_changes();
        self.consts.pop_local_changes();
        self.modules.pop_local_changes();
    }

    pub fn get_filenames_by_hash(
        &self,
        hashes: &deps_rust::DepSet,
    ) -> Result<std::collections::BTreeSet<RelativePath>> {
        hashes
            .iter()
            .filter_map(|&hash| self.get_filename_by_hash(hash).transpose())
            .collect()
    }

    fn get_filename_by_hash(&self, hash: deps_rust::Dep) -> Result<Option<RelativePath>> {
        Ok(self
            .db
            .with_db(|db| db.get_path_by_symbol_hash(ToplevelSymbolHash::from_u64(hash.into())))?
            .as_ref()
            .map(Into::into))
    }
}

impl NamingProvider for NamingTable {
    fn get_type_path(&self, name: pos::TypeName) -> Result<Option<RelativePath>> {
        Ok(self
            .get_type_pos(name)?
            .map(|(pos, _kind)| pos.path().into()))
    }
    fn get_fun_path(&self, name: pos::FunName) -> Result<Option<RelativePath>> {
        Ok(self.get_fun_pos(name)?.map(|pos| pos.path().into()))
    }
    fn get_const_path(&self, name: pos::ConstName) -> Result<Option<RelativePath>> {
        Ok(self.get_const_pos(name)?.map(|pos| pos.path().into()))
    }
}

impl std::fmt::Debug for NamingTable {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NamingTable").finish()
    }
}

#[derive(Clone, Debug)]
enum Pos {
    Full(pos::BPos),
    File(NameType, RelativePath),
}

impl From<&file_info::Pos> for Pos {
    fn from(pos: &file_info::Pos) -> Self {
        match pos {
            file_info::Pos::Full(pos) => Self::Full(pos.into()),
            file_info::Pos::File(name_type, path) => Self::File(*name_type, (&**path).into()),
        }
    }
}

impl From<Pos> for file_info::Pos {
    fn from(pos: Pos) -> Self {
        match pos {
            Pos::Full(pos) => Self::Full(pos.into()),
            Pos::File(name_type, path) => Self::File(name_type, RcOc::new(path.into())),
        }
    }
}

struct MaybeNamingDb(Mutex<Option<(names::Names, PathBuf)>>);

impl MaybeNamingDb {
    fn db_path(&self) -> Option<PathBuf> {
        self.0.lock().as_ref().map(|(_, path)| path.clone())
    }

    fn set_db_path(&self, db_path: PathBuf) -> rusqlite::Result<()> {
        let mut lock = self.0.lock();
        *lock = Some((names::Names::from_file(&db_path)?, db_path));
        Ok(())
    }

    fn with_db<T, F>(&self, f: F) -> rusqlite::Result<Option<T>>
    where
        F: FnOnce(&names::Names) -> rusqlite::Result<Option<T>>,
    {
        match &*self.0.lock() {
            Some((db, _)) => f(db),
            None => Ok(None),
        }
    }
}

impl std::fmt::Debug for MaybeNamingDb {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("MaybeNamingDb").finish()
    }
}

#[derive(Clone, Debug)]
struct TypeDb(Arc<MaybeNamingDb>);

impl Cache<ToplevelSymbolHash, (Pos, naming_types::KindOfType)> for TypeDb {
    fn get(&self, key: ToplevelSymbolHash) -> Option<(Pos, naming_types::KindOfType)> {
        self.0
            .with_db(|db| {
                Ok(db.get_filename(key)?.and_then(|(path, name_type)| {
                    let kind = match name_type {
                        NameType::Class => naming_types::KindOfType::TClass,
                        NameType::Typedef => naming_types::KindOfType::TTypedef,
                        _ => return None,
                    };
                    Some((Pos::File(kind.into(), (&path).into()), kind))
                }))
            })
            .unwrap()
    }
    fn insert(&self, _key: ToplevelSymbolHash, _val: (Pos, naming_types::KindOfType)) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

impl Cache<ToplevelCanonSymbolHash, TypeName> for TypeDb {
    fn get(&self, _key: ToplevelCanonSymbolHash) -> Option<TypeName> {
        self.0.with_db(|_db| todo!()).unwrap()
    }
    fn insert(&self, _key: ToplevelCanonSymbolHash, _val: TypeName) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

#[derive(Clone, Debug)]
struct FunDb(Arc<MaybeNamingDb>);

impl Cache<ToplevelSymbolHash, Pos> for FunDb {
    fn get(&self, key: ToplevelSymbolHash) -> Option<Pos> {
        self.0
            .with_db(|db| {
                Ok(db
                    .get_path_by_symbol_hash(key)?
                    .map(|path| Pos::File(NameType::Fun, (&path).into())))
            })
            .unwrap()
    }
    fn insert(&self, _key: ToplevelSymbolHash, _val: Pos) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

impl Cache<ToplevelCanonSymbolHash, FunName> for FunDb {
    fn get(&self, _key: ToplevelCanonSymbolHash) -> Option<FunName> {
        self.0.with_db(|_db| todo!()).unwrap()
    }
    fn insert(&self, _key: ToplevelCanonSymbolHash, _val: FunName) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

#[derive(Clone, Debug)]
struct ConstDb(Arc<MaybeNamingDb>);

impl Cache<ToplevelSymbolHash, Pos> for ConstDb {
    fn get(&self, key: ToplevelSymbolHash) -> Option<Pos> {
        self.0
            .with_db(|db| {
                Ok(db
                    .get_path_by_symbol_hash(key)?
                    .map(|path| Pos::File(NameType::Const, (&path).into())))
            })
            .unwrap()
    }
    fn insert(&self, _key: ToplevelSymbolHash, _val: Pos) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

#[derive(Clone, Debug)]
struct ModuleDb(Arc<MaybeNamingDb>);

impl Cache<ToplevelSymbolHash, Pos> for ModuleDb {
    fn get(&self, key: ToplevelSymbolHash) -> Option<Pos> {
        self.0
            .with_db(|db| {
                Ok(db
                    .get_path_by_symbol_hash(key)?
                    .map(|path| Pos::File(NameType::Module, (&path).into())))
            })
            .unwrap()
    }

    fn insert(&self, _key: ToplevelSymbolHash, _val: Pos) {
        unreachable!("ChangesCache should not perform inserts on fallback")
    }
}

mod reverse_naming_table {
    use hackrs::cache::{Cache, ChangesCache};
    use hh24_types::{ToplevelCanonSymbolHash, ToplevelSymbolHash};
    use std::hash::Hash;

    /// In-memory delta for symbols which support a canon-name lookup API (types
    /// and funs).
    pub struct ReverseNamingTable<K, P, F> {
        positions: ChangesCache<ToplevelSymbolHash, P, F>,
        canon_names: ChangesCache<ToplevelCanonSymbolHash, K, F>,
    }

    impl<K, P, F> ReverseNamingTable<K, P, F>
    where
        K: Copy + Hash + Eq + Into<ToplevelSymbolHash> + Into<ToplevelCanonSymbolHash>,
        P: Clone,
        F: Clone + Cache<ToplevelSymbolHash, P> + Cache<ToplevelCanonSymbolHash, K>,
    {
        pub fn new(fallback: F) -> Self {
            Self {
                positions: ChangesCache::new(fallback.clone()),
                canon_names: ChangesCache::new(fallback),
            }
        }

        pub fn insert(&self, name: K, pos: P) {
            self.positions.insert(name.into(), pos);
            self.canon_names.insert(name.into(), name);
        }

        pub fn get_pos(&self, name: K) -> Option<P> {
            self.positions.get(name.into())
        }

        pub fn get_canon_name(&self, name: K) -> Option<K> {
            self.canon_names.get(name.into())
        }

        pub fn push_local_changes(&self) {
            self.canon_names.push_local_changes();
            self.positions.push_local_changes();
        }

        pub fn pop_local_changes(&self) {
            self.canon_names.pop_local_changes();
            self.positions.pop_local_changes();
        }

        pub fn remove_batch<I: Iterator<Item = K> + Clone>(&self, keys: I) {
            self.canon_names.remove_batch(keys.clone().map(Into::into));
            self.positions.remove_batch(keys.map(Into::into));
        }
    }
}
