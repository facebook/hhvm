// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[cfg(test)]
mod test_naming_table;

use std::path::PathBuf;
use std::sync::Arc;

use anyhow::Result;
use datastore::ChangesStore;
use datastore::DeltaStore;
use datastore::ReadonlyStore;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use naming_provider::NamingProvider;
use naming_types::KindOfType;
use ocamlrep::ptr::UnsafeOcamlPtr;
use oxidized::file_info::NameType;
use oxidized::naming_types;
use parking_lot::Mutex;
use pos::ConstName;
use pos::FunName;
use pos::ModuleName;
use pos::RelativePath;
use pos::TypeName;
use reverse_naming_table::ReverseNamingTable;
use shm_store::OcamlShmStore;

/// Designed after naming_heap.ml.
pub struct NamingTable {
    types: ReverseNamingTable<TypeName, (Pos, KindOfType)>,
    funs: ReverseNamingTable<FunName, Pos>,
    consts: ChangesStore<ToplevelSymbolHash, Pos>,
    consts_shm: Arc<OcamlShmStore<ToplevelSymbolHash, Option<Pos>>>,
    modules: ChangesStore<ToplevelSymbolHash, Pos>,
    modules_shm: Arc<OcamlShmStore<ToplevelSymbolHash, Option<Pos>>>,
    db: Arc<MaybeNamingDb>,
}

impl NamingTable {
    pub fn new(db_path: Option<PathBuf>) -> Result<Self> {
        let db = Arc::new(MaybeNamingDb(Mutex::new(None)));
        if let Some(db_path) = db_path {
            db.set_db_path(db_path)?;
        }
        let consts_shm = Arc::new(OcamlShmStore::new(
            "Naming_ConstPos",
            shm_store::Evictability::NonEvictable,
            shm_store::Compression::None,
        ));
        let modules_shm = Arc::new(OcamlShmStore::new(
            "Naming_ModulePos",
            shm_store::Evictability::NonEvictable,
            shm_store::Compression::None,
        ));
        Ok(Self {
            types: ReverseNamingTable::new(
                Arc::new(TypeDb(Arc::clone(&db))),
                "Naming_TypePos",
                "Naming_TypeCanon",
            ),
            funs: ReverseNamingTable::new(
                Arc::new(FunDb(Arc::clone(&db))),
                "Naming_FunPos",
                "Naming_FunCanon",
            ),
            consts: ChangesStore::new(Arc::new(DeltaStore::new(
                Arc::clone(&consts_shm) as _,
                Arc::new(ConstDb(Arc::clone(&db))),
            ))),
            modules: ChangesStore::new(Arc::new(DeltaStore::new(
                Arc::clone(&modules_shm) as _,
                Arc::new(ModuleDb(Arc::clone(&db))),
            ))),
            consts_shm,
            modules_shm,
            db,
        })
    }

    pub fn db_path(&self) -> Option<PathBuf> {
        self.db.db_path()
    }

    pub fn set_db_path(&self, db_path: PathBuf) -> Result<()> {
        self.db.set_db_path(db_path)
    }

    pub fn add_type(
        &self,
        name: TypeName,
        pos_and_kind: &(file_info::Pos, KindOfType),
    ) -> Result<()> {
        self.types
            .insert(name, ((&pos_and_kind.0).into(), pos_and_kind.1))
    }

    pub fn get_type_pos(&self, name: TypeName) -> Result<Option<(Pos, KindOfType)>> {
        self.types.get_pos(name)
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

    pub fn get_fun_pos(&self, name: FunName) -> Result<Option<Pos>> {
        self.funs.get_pos(name)
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

    pub fn get_const_pos(&self, name: ConstName) -> Result<Option<Pos>> {
        self.consts.get(name.into())
    }

    pub fn remove_const_batch(&self, names: &[ConstName]) -> Result<()> {
        self.consts
            .remove_batch(&mut names.iter().copied().map(Into::into))
    }

    pub fn add_module(&self, name: ModuleName, pos: &file_info::Pos) -> Result<()> {
        self.modules.insert(name.into(), pos.into())
    }

    pub fn get_module_pos(&self, name: ModuleName) -> Result<Option<Pos>> {
        self.modules.get(name.into())
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
        let hash = ToplevelSymbolHash::from_u64(hash.into());
        if let Some((pos, _kind)) = self.types.get_pos_by_hash(hash)? {
            return Ok(Some(pos.path()));
        } else if let Some(pos) = self.funs.get_pos_by_hash(hash)? {
            return Ok(Some(pos.path()));
        } else if let Some(pos) = self.consts.get(hash)? {
            return Ok(Some(pos.path()));
        };
        Ok(self
            .db
            .with_db(|db| db.get_path_by_symbol_hash(hash))?
            .as_ref()
            .map(Into::into))
    }
}

impl NamingTable {
    /// Returns `Option<UnsafeOcamlPtr>` where the `UnsafeOcamlPtr` is a value
    /// of OCaml type `FileInfo.pos option`.
    ///
    /// SAFETY: This method (and all other `get_ocaml_` methods) call into the
    /// OCaml runtime and may trigger a GC. Must be invoked from the main thread
    /// with no concurrent interaction with the OCaml runtime. The returned
    /// `UnsafeOcamlPtr` is unrooted and could be invalidated if the GC is
    /// triggered after this method returns.
    pub unsafe fn get_ocaml_type_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        self.types
            .get_ocaml_pos_by_hash(ToplevelSymbolHash::from_byte_string(
                // NameType::Class and NameType::Typedef are handled the same here
                file_info::NameType::Class,
                name,
            ))
            // The heap has values of type Option<Pos>, and they've already been
            // converted to an OCaml value here. Map `Some(ocaml_none)` to
            // `None` so that the caller doesn't need to inspect the value.
            .filter(|ptr| ptr.is_block())
    }
    pub unsafe fn get_ocaml_fun_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        self.funs
            .get_ocaml_pos_by_hash(ToplevelSymbolHash::from_byte_string(
                file_info::NameType::Fun,
                name,
            ))
            .filter(|ptr| ptr.is_block())
    }
    pub unsafe fn get_ocaml_const_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.consts.has_local_changes() {
            None
        } else {
            self.consts_shm
                .get_ocaml(ToplevelSymbolHash::from_byte_string(
                    file_info::NameType::Const,
                    name,
                ))
                .filter(|ptr| ptr.is_block())
        }
    }
    pub unsafe fn get_ocaml_module_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.modules.has_local_changes() {
            None
        } else {
            self.modules_shm
                .get_ocaml(ToplevelSymbolHash::from_byte_string(
                    file_info::NameType::Module,
                    name,
                ))
                .filter(|ptr| ptr.is_block())
        }
    }
}

impl NamingProvider for NamingTable {
    fn get_type_path_and_kind(
        &self,
        name: pos::TypeName,
    ) -> Result<Option<(RelativePath, KindOfType)>> {
        Ok(self
            .get_type_pos(name)?
            .map(|(pos, kind)| (pos.path(), kind)))
    }
    fn get_fun_path(&self, name: pos::FunName) -> Result<Option<RelativePath>> {
        Ok(self.get_fun_pos(name)?.map(|pos| pos.path()))
    }
    fn get_const_path(&self, name: pos::ConstName) -> Result<Option<RelativePath>> {
        Ok(self.get_const_pos(name)?.map(|pos| pos.path()))
    }
    fn get_module_path(&self, name: pos::ModuleName) -> Result<Option<RelativePath>> {
        Ok(self.get_module_pos(name)?.map(|pos| pos.path()))
    }
    fn get_canon_type_name(&self, name: TypeName) -> Result<Option<TypeName>> {
        self.get_canon_type_name(name)
    }
    fn get_canon_fun_name(&self, name: FunName) -> Result<Option<FunName>> {
        self.get_canon_fun_name(name)
    }
}

impl std::fmt::Debug for NamingTable {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NamingTable").finish()
    }
}

#[derive(Clone, Debug, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
#[derive(ocamlrep::ToOcamlRep, ocamlrep::FromOcamlRep)]
pub enum Pos {
    Full(pos::BPos),
    File(NameType, RelativePath),
}

impl Pos {
    pub fn path(&self) -> RelativePath {
        match self {
            Self::Full(pos) => pos.file(),
            &Self::File(_, path) => path,
        }
    }
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
            Pos::File(name_type, path) => Self::File(name_type, Arc::new(path.into())),
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
        *lock = Some((names::Names::from_file_readonly(&db_path)?, db_path));
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

impl ReadonlyStore<ToplevelSymbolHash, (Pos, KindOfType)> for TypeDb {
    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<(Pos, KindOfType)>> {
        self.0.with_db(|db| {
            Ok(db.get_filename(key)?.and_then(|(path, name_type)| {
                let kind = KindOfType::try_from(name_type).ok()?;
                Some((Pos::File(name_type, (&path).into()), kind))
            }))
        })
    }
}

impl ReadonlyStore<ToplevelCanonSymbolHash, TypeName> for TypeDb {
    fn contains_key(&self, key: ToplevelCanonSymbolHash) -> Result<bool> {
        Ok(self
            .0
            .with_db(|db| db.get_type_name_case_insensitive(key))?
            .is_some())
    }

    fn get(&self, key: ToplevelCanonSymbolHash) -> Result<Option<TypeName>> {
        self.0
            .with_db(|db| Ok(db.get_type_name_case_insensitive(key)?.map(TypeName::new)))
    }
}

#[derive(Clone, Debug)]
struct FunDb(Arc<MaybeNamingDb>);

impl ReadonlyStore<ToplevelSymbolHash, Pos> for FunDb {
    fn contains_key(&self, key: ToplevelSymbolHash) -> Result<bool> {
        Ok(self
            .0
            .with_db(|db| db.get_path_by_symbol_hash(key))?
            .is_some())
    }

    fn get(&self, key: ToplevelSymbolHash) -> Result<Option<Pos>> {
        self.0.with_db(|db| {
            Ok(db
                .get_path_by_symbol_hash(key)?
                .map(|path| Pos::File(NameType::Fun, (&path).into())))
        })
    }
}

impl ReadonlyStore<ToplevelCanonSymbolHash, FunName> for FunDb {
    fn contains_key(&self, key: ToplevelCanonSymbolHash) -> Result<bool> {
        Ok(self
            .0
            .with_db(|db| db.get_fun_name_case_insensitive(key))?
            .is_some())
    }

    fn get(&self, key: ToplevelCanonSymbolHash) -> Result<Option<FunName>> {
        self.0
            .with_db(|db| Ok(db.get_fun_name_case_insensitive(key)?.map(FunName::new)))
    }
}

#[derive(Clone, Debug)]
struct ConstDb(Arc<MaybeNamingDb>);

impl ReadonlyStore<ToplevelSymbolHash, Pos> for ConstDb {
    fn contains_key(&self, key: ToplevelSymbolHash) -> Result<bool> {
        Ok(self
            .0
            .with_db(|db| db.get_path_by_symbol_hash(key))?
            .is_some())
    }

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
    fn contains_key(&self, key: ToplevelSymbolHash) -> Result<bool> {
        Ok(self
            .0
            .with_db(|db| db.get_path_by_symbol_hash(key))?
            .is_some())
    }

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
    use shm_store::OcamlShmStore;
    use shm_store::ShmStore;

    /// In-memory delta for symbols which support a canon-name lookup API (types
    /// and funs).
    pub struct ReverseNamingTable<K, P> {
        positions: ChangesStore<ToplevelSymbolHash, P>,
        positions_shm: Arc<OcamlShmStore<ToplevelSymbolHash, Option<P>>>,
        canon_names: ChangesStore<ToplevelCanonSymbolHash, K>,
    }

    impl<K, P> ReverseNamingTable<K, P>
    where
        K: Copy + Hash + Eq + Send + Sync + 'static + Serialize + DeserializeOwned,
        K: Into<ToplevelSymbolHash> + Into<ToplevelCanonSymbolHash>,
        P: Clone + Send + Sync + 'static + Serialize + DeserializeOwned,
        P: ocamlrep::ToOcamlRep + ocamlrep::FromOcamlRep,
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
            let positions_shm = Arc::new(OcamlShmStore::new(
                pos_prefix,
                shm_store::Evictability::NonEvictable,
                shm_store::Compression::None,
            ));
            Self {
                positions: ChangesStore::new(Arc::new(DeltaStore::new(
                    Arc::clone(&positions_shm) as _,
                    Arc::clone(&fallback) as _,
                ))),
                positions_shm,
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

        pub fn get_pos_by_hash(&self, name: ToplevelSymbolHash) -> Result<Option<P>> {
            self.positions.get(name)
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

        pub unsafe fn get_ocaml_pos_by_hash(
            &self,
            hash: ToplevelSymbolHash,
        ) -> Option<ocamlrep::ptr::UnsafeOcamlPtr> {
            if self.positions.has_local_changes() {
                None
            } else {
                self.positions_shm.get_ocaml(hash)
            }
        }

        pub fn remove_batch<I: Iterator<Item = K> + Clone>(&self, keys: I) -> Result<()> {
            self.canon_names
                .remove_batch(&mut keys.clone().map(Into::into))?;
            self.positions.remove_batch(&mut keys.map(Into::into))?;
            Ok(())
        }
    }
}
