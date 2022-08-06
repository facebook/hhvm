// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;

use anyhow::Result;
use datastore::ChangesStore;
use datastore::DeltaStore;
use datastore::ReadonlyStore;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use naming_provider::NamingProvider;
use ocamlrep::rc::RcOc;
use oxidized::file_info;
use oxidized::file_info::NameType;
use oxidized::naming_types;
use parking_lot::Mutex;
use pos::ConstName;
use pos::FunName;
use pos::ModuleName;
use pos::RelativePath;
use pos::TypeName;
use reverse_naming_table::ReverseNamingTable;
use shm_store::ShmStore;

/// Designed after naming_heap.ml.
pub struct NamingTable {
    types: ReverseNamingTable<TypeName, (Pos, naming_types::KindOfType)>,
    funs: ReverseNamingTable<FunName, Pos>,
    consts: ChangesStore<ToplevelSymbolHash, Pos>,
    modules: ChangesStore<ToplevelSymbolHash, Pos>,
    db: Arc<MaybeNamingDb>,
}

impl NamingTable {
    pub fn new(db_path: Option<PathBuf>) -> Result<Self> {
        let db = Arc::new(MaybeNamingDb(Mutex::new(None)));
        if let Some(db_path) = db_path {
            db.set_db_path(db_path)?;
        }
        Ok(Self {
            types: ReverseNamingTable::new(
                Arc::new(TypeDb(Arc::clone(&db))),
                "TypePos",
                "TypeCanon",
            ),
            funs: ReverseNamingTable::new(Arc::new(FunDb(Arc::clone(&db))), "FunPos", "FunCanon"),
            consts: ChangesStore::new(Arc::new(DeltaStore::new(
                Arc::new(ShmStore::new(
                    "ConstPos",
                    shm_store::Evictability::NonEvictable,
                    shm_store::Compression::None,
                )),
                Arc::new(ConstDb(Arc::clone(&db))),
            ))),
            modules: ChangesStore::new(Arc::new(DeltaStore::new(
                Arc::new(ShmStore::new(
                    "ModulePos",
                    shm_store::Evictability::NonEvictable,
                    shm_store::Compression::None,
                )),
                Arc::new(ModuleDb(Arc::clone(&db))),
            ))),
            db,
        })
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
    ) -> Result<()> {
        self.types
            .insert(name, ((&pos_and_kind.0).into(), pos_and_kind.1))
    }

    pub fn get_type_pos(
        &self,
        name: TypeName,
    ) -> Result<Option<(file_info::Pos, naming_types::KindOfType)>> {
        Ok(self
            .types
            .get_pos(name)?
            .map(|(pos, kind)| (pos.into(), kind)))
    }

    pub fn remove_type_batch(&self, names: &[TypeName]) -> Result<()> {
        self.types.remove_batch(names.iter().copied())
    }

    pub fn get_canon_type_name(&self, name: TypeName) -> Result<Option<TypeName>> {
        self.types.get_canon_name(name)
    }

    pub fn add_fun(&self, name: FunName, pos: &file_info::Pos) -> Result<()> {
        self.funs.insert(name, pos.into())
    }

    pub fn get_fun_pos(&self, name: FunName) -> Result<Option<file_info::Pos>> {
        Ok(self.funs.get_pos(name)?.map(Into::into))
    }

    pub fn remove_fun_batch(&self, names: &[FunName]) -> Result<()> {
        self.funs.remove_batch(names.iter().copied())
    }

    pub fn get_canon_fun_name(&self, name: FunName) -> Result<Option<FunName>> {
        self.funs.get_canon_name(name)
    }

    pub fn add_const(&self, name: ConstName, pos: &file_info::Pos) -> Result<()> {
        self.consts.insert(name.into(), pos.into())
    }

    pub fn get_const_pos(&self, name: ConstName) -> Result<Option<file_info::Pos>> {
        Ok(self.consts.get(name.into())?.map(Into::into))
    }

    pub fn remove_const_batch(&self, names: &[ConstName]) -> Result<()> {
        self.consts
            .remove_batch(&mut names.iter().copied().map(Into::into))
    }

    pub fn add_module(&self, name: ModuleName, pos: &file_info::Pos) -> Result<()> {
        self.modules.insert(name.into(), pos.into())
    }

    pub fn get_module_pos(&self, name: ModuleName) -> Result<Option<file_info::Pos>> {
        Ok(self.modules.get(name.into())?.map(Into::into))
    }

    pub fn remove_module_batch(&self, names: &[ModuleName]) -> Result<()> {
        self.modules
            .remove_batch(&mut names.iter().copied().map(Into::into))
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
    fn get_type_path_and_kind(
        &self,
        name: pos::TypeName,
    ) -> Result<Option<(RelativePath, naming_types::KindOfType)>> {
        Ok(self
            .get_type_pos(name)?
            .map(|(pos, kind)| (pos.path().into(), kind)))
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

#[derive(Clone, Debug, serde::Serialize, serde::Deserialize)]
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

    fn set_db_path(&self, db_path: PathBuf) -> Result<()> {
        let mut lock = self.0.lock();
        *lock = Some((names::Names::from_file(&db_path)?, db_path));
        Ok(())
    }

    fn with_db<T, F>(&self, f: F) -> Result<Option<T>>
    where
        F: FnOnce(&names::Names) -> Result<Option<T>>,
    {
        match &*self.0.lock() {
            Some((db, _)) => Ok(f(db)?),
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

impl ReadonlyStore<ToplevelSymbolHash, (Pos, naming_types::KindOfType)> for TypeDb {
    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<(Pos, naming_types::KindOfType)>> {
        self.0.with_db(|db| {
            Ok(db.get_filename(key)?.and_then(|(path, name_type)| {
                let kind = match name_type {
                    NameType::Class => naming_types::KindOfType::TClass,
                    NameType::Typedef => naming_types::KindOfType::TTypedef,
                    _ => return None,
                };
                Some((Pos::File(kind.into(), (&path).into()), kind))
            }))
        })
    }
}

impl ReadonlyStore<ToplevelCanonSymbolHash, TypeName> for TypeDb {
    fn get(&self, key: ToplevelCanonSymbolHash) -> Result<Option<TypeName>> {
        self.0
            .with_db(|db| Ok(db.get_type_name_case_insensitive(key)?.map(TypeName::new)))
    }
}

#[derive(Clone, Debug)]
struct FunDb(Arc<MaybeNamingDb>);

impl ReadonlyStore<ToplevelSymbolHash, Pos> for FunDb {
    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<Pos>> {
        self.0.with_db(|db| {
            Ok(db
                .get_path_by_symbol_hash(key)?
                .map(|path| Pos::File(NameType::Fun, (&path).into())))
        })
    }
}

impl ReadonlyStore<ToplevelCanonSymbolHash, FunName> for FunDb {
    fn get(&self, key: ToplevelCanonSymbolHash) -> Result<Option<FunName>> {
        self.0
            .with_db(|db| Ok(db.get_fun_name_case_insensitive(key)?.map(FunName::new)))
    }
}

#[derive(Clone, Debug)]
struct ConstDb(Arc<MaybeNamingDb>);

impl ReadonlyStore<ToplevelSymbolHash, Pos> for ConstDb {
    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<Pos>> {
        self.0.with_db(|db| {
            Ok(db
                .get_path_by_symbol_hash(key)?
                .map(|path| Pos::File(NameType::Const, (&path).into())))
        })
    }
}

#[derive(Clone, Debug)]
struct ModuleDb(Arc<MaybeNamingDb>);

impl ReadonlyStore<ToplevelSymbolHash, Pos> for ModuleDb {
    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<Pos>> {
        self.0.with_db(|db| {
            Ok(db
                .get_path_by_symbol_hash(key)?
                .map(|path| Pos::File(NameType::Module, (&path).into())))
        })
    }
}

mod reverse_naming_table {
    use std::hash::Hash;
    use std::sync::Arc;

    use anyhow::Result;
    use datastore::ChangesStore;
    use datastore::DeltaStore;
    use datastore::ReadonlyStore;
    use hh24_types::ToplevelCanonSymbolHash;
    use hh24_types::ToplevelSymbolHash;
    use serde::de::DeserializeOwned;
    use serde::Serialize;
    use shm_store::ShmStore;

    /// In-memory delta for symbols which support a canon-name lookup API (types
    /// and funs).
    pub struct ReverseNamingTable<K, P> {
        positions: ChangesStore<ToplevelSymbolHash, P>,
        canon_names: ChangesStore<ToplevelCanonSymbolHash, K>,
    }

    impl<K, P> ReverseNamingTable<K, P>
    where
        K: Copy + Hash + Eq + Send + Sync + 'static + Serialize + DeserializeOwned,
        K: Into<ToplevelSymbolHash> + Into<ToplevelCanonSymbolHash>,
        P: Clone + Send + Sync + 'static + Serialize + DeserializeOwned,
    {
        pub fn new<F>(
            fallback: Arc<F>,
            pos_prefix: &'static str,
            canon_prefix: &'static str,
        ) -> Self
        where
            F: ReadonlyStore<ToplevelSymbolHash, P>
                + ReadonlyStore<ToplevelCanonSymbolHash, K>
                + 'static,
        {
            Self {
                positions: ChangesStore::new(Arc::new(DeltaStore::new(
                    Arc::new(ShmStore::new(
                        pos_prefix,
                        shm_store::Evictability::NonEvictable,
                        shm_store::Compression::None,
                    )),
                    Arc::clone(&fallback) as _,
                ))),
                canon_names: ChangesStore::new(Arc::new(DeltaStore::new(
                    Arc::new(ShmStore::new(
                        canon_prefix,
                        shm_store::Evictability::NonEvictable,
                        shm_store::Compression::None,
                    )),
                    fallback,
                ))),
            }
        }

        pub fn insert(&self, name: K, pos: P) -> Result<()> {
            self.positions.insert(name.into(), pos)?;
            self.canon_names.insert(name.into(), name)?;
            Ok(())
        }

        pub fn get_pos(&self, name: K) -> Result<Option<P>> {
            self.positions.get(name.into())
        }

        pub fn get_canon_name(&self, name: K) -> Result<Option<K>> {
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

        pub fn remove_batch<I: Iterator<Item = K> + Clone>(&self, keys: I) -> Result<()> {
            self.canon_names
                .remove_batch(&mut keys.clone().map(Into::into))?;
            self.positions.remove_batch(&mut keys.map(Into::into))?;
            Ok(())
        }
    }
}
