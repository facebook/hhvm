// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod naming_table;

#[cfg(test)]
mod test_naming_table;

use std::collections::BTreeMap;
use std::path::PathBuf;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::Ordering;
use std::sync::Arc;

use anyhow::Result;
use datastore::ChangesStore;
use datastore::Store;
use decl_parser::DeclParser;
use file_provider::DiskProvider;
use file_provider::FileProvider;
use folded_decl_provider::FoldedDeclProvider;
use folded_decl_provider::LazyFoldedDeclProvider;
use naming_provider::NamingProvider;
use naming_table::NamingTable;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep_caml_builtins::Int64;
use oxidized::global_options::GlobalOptions;
use oxidized::naming_types;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use pos::RelativePath;
use pos::RelativePathCtx;
use pos::TypeName;
use shallow_decl_provider::LazyShallowDeclProvider;
use shallow_decl_provider::ShallowDeclProvider;
use shallow_decl_provider::ShallowDeclStore;
use shm_store::OcamlShmStore;
use ty::decl;
use ty::decl::folded::FoldedClass;
use ty::decl::shallow::NamedDecl;
use ty::reason::BReason as BR;

pub struct HhServerProviderBackend {
    path_ctx: Arc<RelativePathCtx>,
    opts: GlobalOptions,
    decl_parser: DeclParser<BR>,
    file_store: Arc<ChangesStore<RelativePath, FileType>>,
    file_provider: Arc<FileProviderWithContext>,
    naming_table: Arc<NamingTableWithContext>,
    ctx_is_empty: Arc<AtomicBool>,
    /// Collection of Arcs pointing to the backing stores for the
    /// ShallowDeclStore below, allowing us to invoke push/pop_local_changes.
    shallow_decl_changes_store: Arc<ShallowStoreWithChanges>,
    shallow_decl_store: Arc<ShallowDeclStore<BR>>,
    lazy_shallow_decl_provider: Arc<LazyShallowDeclProvider<BR>>,
    folded_classes_shm: Arc<OcamlShmStore<TypeName, Arc<FoldedClass<BR>>>>,
    folded_classes_store: Arc<ChangesStore<TypeName, Arc<FoldedClass<BR>>>>,
    folded_decl_provider: Arc<LazyFoldedDeclProvider<BR>>,
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct Config {
    pub path_ctx: RelativePathCtx,
    pub opts: GlobalOptions,
    pub db_path: Option<PathBuf>,
}

impl HhServerProviderBackend {
    pub fn new(config: Config) -> Result<Self> {
        let Config {
            path_ctx,
            opts,
            db_path,
        } = config;
        let path_ctx = Arc::new(path_ctx);
        let file_store = Arc::new(ChangesStore::new(Arc::new(OcamlShmStore::new(
            "File",
            shm_store::Evictability::NonEvictable,
            shm_store::Compression::default(),
        ))));
        let ctx_is_empty = Arc::new(AtomicBool::new(true));
        let file_provider = Arc::new(FileProviderWithContext {
            ctx_is_empty: Arc::clone(&ctx_is_empty),
            backend: FileProviderWithChanges {
                delta_and_changes: Arc::clone(&file_store),
                disk: DiskProvider::new(Arc::clone(&path_ctx), None),
            },
        });
        let decl_parser = DeclParser::with_options(Arc::clone(&file_provider) as _, opts.clone());
        let naming_table = Arc::new(NamingTableWithContext {
            ctx_is_empty: Arc::clone(&ctx_is_empty),
            fallback: NamingTable::new(db_path)?,
        });

        let shallow_decl_changes_store = Arc::new(ShallowStoreWithChanges::new());
        let shallow_decl_store = shallow_decl_changes_store.as_shallow_decl_store();

        let lazy_shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::clone(&shallow_decl_store),
            Arc::clone(&naming_table) as _,
            decl_parser.clone(),
        ));

        let folded_classes_shm = Arc::new(OcamlShmStore::new(
            "Decl_Class",
            shm_store::Evictability::Evictable,
            shm_store::Compression::default(),
        ));
        let folded_classes_store =
            Arc::new(ChangesStore::new(Arc::clone(&folded_classes_shm) as _));

        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(opts.clone()),
            Arc::clone(&folded_classes_store) as _,
            Arc::clone(&lazy_shallow_decl_provider) as _,
        ));

        Ok(Self {
            path_ctx,
            opts,
            file_store,
            file_provider,
            decl_parser,
            folded_decl_provider,
            naming_table,
            ctx_is_empty,
            shallow_decl_changes_store,
            shallow_decl_store,
            lazy_shallow_decl_provider,
            folded_classes_shm,
            folded_classes_store,
        })
    }

    pub fn config(&self) -> Config {
        Config {
            path_ctx: (*self.path_ctx).clone(),
            db_path: self.naming_table.fallback.db_path(),
            opts: self.opts.clone(),
        }
    }

    pub fn opts(&self) -> &GlobalOptions {
        &self.opts
    }

    /// Get our implementation of NamingProvider, which calls into OCaml to
    /// provide results which take IDE buffers into consideration.
    pub fn naming_table_with_context(&self) -> &NamingTableWithContext {
        &self.naming_table
    }

    /// Get the underlying naming table (which includes the SQL database, our
    /// sharedmem cache layer, and the ChangesStore layer used in tests).
    pub fn naming_table(&self) -> &NamingTable {
        &self.naming_table.fallback
    }

    pub fn file_store(&self) -> &dyn Store<RelativePath, FileType> {
        &*self.file_store
    }

    pub fn shallow_decl_provider(&self) -> &dyn ShallowDeclProvider<BR> {
        &*self.lazy_shallow_decl_provider
    }

    /// Decl-parse the given file, dedup duplicate definitions of the same
    /// symbol (within the file, as well as removing losers of naming conflicts
    /// with other files), and add the parsed decls to the shallow decl store.
    pub fn parse_and_cache_decls<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> Result<ParsedFileWithHashes<'a>> {
        let hashed_file = self.decl_parser.parse_impl(path, text, arena);
        self.lazy_shallow_decl_provider.dedup_and_add_decls(
            path,
            (hashed_file.iter()).map(|(name, decl, _)| NamedDecl::from(&(*name, *decl))),
        )?;
        Ok(hashed_file)
    }

    /// Directly add the given decls to the shallow decl store (without removing
    /// php_stdlib decls, deduping, or removing naming conflict losers).
    pub fn add_decls(
        &self,
        decls: &[&(&str, oxidized_by_ref::shallow_decl_defs::Decl<'_>)],
    ) -> Result<()> {
        self.shallow_decl_store
            .add_decls(decls.iter().copied().map(Into::into))
    }

    pub fn push_local_changes(&self) {
        self.file_store.push_local_changes();
        self.naming_table.fallback.push_local_changes();
        self.shallow_decl_changes_store.push_local_changes();
        self.folded_classes_store.push_local_changes();
    }

    pub fn pop_local_changes(&self) {
        self.file_store.pop_local_changes();
        self.naming_table.fallback.pop_local_changes();
        self.shallow_decl_changes_store.pop_local_changes();
        self.folded_classes_store.pop_local_changes();
    }

    pub fn set_ctx_empty(&self, is_empty: bool) {
        self.ctx_is_empty.store(is_empty, Ordering::SeqCst);
    }

    pub fn oldify_defs(&self, names: &file_info::Names) -> Result<()> {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))?;
        self.shallow_decl_changes_store.oldify_defs(names)
    }

    pub fn remove_old_defs(&self, names: &file_info::Names) -> Result<()> {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))?;
        self.shallow_decl_changes_store.remove_old_defs(names)
    }

    pub fn remove_defs(&self, names: &file_info::Names) -> Result<()> {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))?;
        self.shallow_decl_changes_store.remove_defs(names)
    }

    pub fn get_old_defs(
        &self,
        names: &file_info::Names,
    ) -> Result<(
        BTreeMap<pos::TypeName, Option<Arc<decl::ShallowClass<BR>>>>,
        BTreeMap<pos::FunName, Option<Arc<decl::FunDecl<BR>>>>,
        BTreeMap<pos::TypeName, Option<Arc<decl::TypedefDecl<BR>>>>,
        BTreeMap<pos::ConstName, Option<Arc<decl::ConstDecl<BR>>>>,
        BTreeMap<pos::ModuleName, Option<Arc<decl::ModuleDecl<BR>>>>,
    )> {
        self.shallow_decl_changes_store.get_old_defs(names)
    }
}

impl rust_provider_backend_api::RustProviderBackend<BR> for HhServerProviderBackend {
    fn file_provider(&self) -> &dyn FileProvider {
        &*self.file_provider
    }

    fn naming_provider(&self) -> &dyn NamingProvider {
        &*self.naming_table
    }

    fn shallow_decl_provider(&self) -> &dyn ShallowDeclProvider<BR> {
        &*self.lazy_shallow_decl_provider
    }

    fn folded_decl_provider(&self) -> &dyn FoldedDeclProvider<BR> {
        &*self.folded_decl_provider
    }

    fn as_any(&self) -> &dyn std::any::Any {
        self
    }
}

#[rustfmt::skip]
impl HhServerProviderBackend {
    /// SAFETY: This method (and all other `get_ocaml_` methods) call into the
    /// OCaml runtime and may trigger a GC. Must be invoked from the main thread
    /// with no concurrent interaction with the OCaml runtime. The returned
    /// `UnsafeOcamlPtr` is unrooted and could be invalidated if the GC is
    /// triggered after this method returns.
    pub unsafe fn get_ocaml_shallow_class(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.classes.has_local_changes() { None }
        else { self.shallow_decl_changes_store.classes_shm.get_ocaml_by_byte_string(name) }
    }
    pub unsafe fn get_ocaml_typedef(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.typedefs.has_local_changes() { None }
        else { self.shallow_decl_changes_store.typedefs_shm.get_ocaml_by_byte_string(name) }
    }
    pub unsafe fn get_ocaml_fun(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.funs.has_local_changes() { None }
        else { self.shallow_decl_changes_store.funs_shm.get_ocaml_by_byte_string(name) }
    }
    pub unsafe fn get_ocaml_const(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.consts.has_local_changes() { None }
        else { self.shallow_decl_changes_store.consts_shm.get_ocaml_by_byte_string(name) }
    }
    pub unsafe fn get_ocaml_module(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.modules.has_local_changes() { None }
        else { self.shallow_decl_changes_store.modules_shm.get_ocaml_by_byte_string(name) }
    }

    pub unsafe fn get_ocaml_folded_class(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if self.folded_classes_store.has_local_changes() { None }
        else { self.folded_classes_shm.get_ocaml_by_byte_string(name) }
    }

    /// Returns `Option<UnsafeOcamlPtr>` where the `UnsafeOcamlPtr` is a value
    /// of OCaml type `FileInfo.pos option`.
    pub unsafe fn get_ocaml_type_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        // If the context is non-empty, fall back to the slow path by returning None.
        // `self.naming_table.fallback` returns None when there are local changes.
        if !self.ctx_is_empty.load(Ordering::SeqCst) { None }
        else { self.naming_table.fallback.get_ocaml_type_pos(name) }
    }
    pub unsafe fn get_ocaml_fun_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if !self.ctx_is_empty.load(Ordering::SeqCst) { None }
        else { self.naming_table.fallback.get_ocaml_fun_pos(name) }
    }
    pub unsafe fn get_ocaml_const_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if !self.ctx_is_empty.load(Ordering::SeqCst) { None }
        else { self.naming_table.fallback.get_ocaml_const_pos(name) }
    }
    pub unsafe fn get_ocaml_module_pos(&self, name: &[u8]) -> Option<UnsafeOcamlPtr> {
        if !self.ctx_is_empty.load(Ordering::SeqCst) { None }
        else { self.naming_table.fallback.get_ocaml_module_pos(name) }
    }
}

#[rustfmt::skip]
struct ShallowStoreWithChanges {
    classes:      Arc<ChangesStore <TypeName, Arc<decl::ShallowClass<BR>>>>,
    classes_shm:  Arc<OcamlShmStore<TypeName, Arc<decl::ShallowClass<BR>>>>,
    typedefs:     Arc<ChangesStore <TypeName, Arc<decl::TypedefDecl<BR>>>>,
    typedefs_shm: Arc<OcamlShmStore<TypeName, Arc<decl::TypedefDecl<BR>>>>,
    funs:         Arc<ChangesStore <pos::FunName, Arc<decl::FunDecl<BR>>>>,
    funs_shm:     Arc<OcamlShmStore<pos::FunName, Arc<decl::FunDecl<BR>>>>,
    consts:       Arc<ChangesStore <pos::ConstName, Arc<decl::ConstDecl<BR>>>>,
    consts_shm:   Arc<OcamlShmStore<pos::ConstName, Arc<decl::ConstDecl<BR>>>>,
    modules:      Arc<ChangesStore <pos::ModuleName, Arc<decl::ModuleDecl<BR>>>>,
    modules_shm:  Arc<OcamlShmStore<pos::ModuleName, Arc<decl::ModuleDecl<BR>>>>,
    store_view: Arc<ShallowDeclStore<BR>>,
}

impl ShallowStoreWithChanges {
    #[rustfmt::skip]
    fn new() -> Self {
        use shm_store::{Compression, Evictability::{Evictable, NonEvictable}};
        let classes_shm =  Arc::new(OcamlShmStore::new("Decl_ShallowClass", NonEvictable, Compression::default()));
        let typedefs_shm = Arc::new(OcamlShmStore::new("Decl_Typedef", Evictable, Compression::default()));
        let funs_shm =     Arc::new(OcamlShmStore::new("Decl_Fun", Evictable, Compression::default()));
        let consts_shm =   Arc::new(OcamlShmStore::new("Decl_GConst", Evictable, Compression::default()));
        let modules_shm =  Arc::new(OcamlShmStore::new("Decl_Module", Evictable, Compression::default()));

        let classes =  Arc::new(ChangesStore::new(Arc::clone(&classes_shm) as _));
        let typedefs = Arc::new(ChangesStore::new(Arc::clone(&typedefs_shm) as _));
        let funs =     Arc::new(ChangesStore::new(Arc::clone(&funs_shm) as _));
        let consts =   Arc::new(ChangesStore::new(Arc::clone(&consts_shm) as _));
        let modules =  Arc::new(ChangesStore::new(Arc::clone(&modules_shm) as _));

        let store_view = Arc::new(ShallowDeclStore::with_no_member_stores(
            Arc::clone(&classes) as _,
            Arc::clone(&typedefs) as _,
            Arc::clone(&funs) as _,
            Arc::clone(&consts) as _,
            Arc::clone(&modules) as _,
        ));
        Self {
            classes,
            typedefs,
            funs,
            consts,
            modules,
            classes_shm,
            typedefs_shm,
            funs_shm,
            consts_shm,
            modules_shm,
            store_view,
        }
    }

    fn push_local_changes(&self) {
        self.classes.push_local_changes();
        self.typedefs.push_local_changes();
        self.funs.push_local_changes();
        self.consts.push_local_changes();
        self.modules.push_local_changes();
    }

    fn pop_local_changes(&self) {
        self.classes.pop_local_changes();
        self.typedefs.pop_local_changes();
        self.funs.pop_local_changes();
        self.consts.pop_local_changes();
        self.modules.pop_local_changes();
    }

    fn as_shallow_decl_store(&self) -> Arc<ShallowDeclStore<BR>> {
        Arc::clone(&self.store_view)
    }

    fn to_old_key<K: AsRef<str> + From<String> + Copy>(key: K) -> K {
        format!("old${}", key.as_ref()).into()
    }

    fn oldify<K, V>(
        &self,
        store: &dyn Store<K, V>,
        names: &mut dyn Iterator<Item = K>,
    ) -> Result<()>
    where
        K: AsRef<str> + From<String> + Copy,
    {
        let mut moves = vec![];
        let mut deletes = vec![];
        for name in names {
            let old_name = Self::to_old_key(name);
            if store.contains_key(name)? {
                moves.push((name, old_name));
            } else if store.contains_key(old_name)? {
                deletes.push(old_name);
            }
        }
        store.move_batch(&mut moves.into_iter())?;
        store.remove_batch(&mut deletes.into_iter())?;
        Ok(())
    }

    fn remove_old<K, V>(
        &self,
        store: &dyn Store<K, V>,
        names: &mut dyn Iterator<Item = K>,
    ) -> Result<()>
    where
        K: AsRef<str> + From<String> + Copy,
    {
        let mut deletes = vec![];
        for name in names {
            let old_name = Self::to_old_key(name);
            if store.contains_key(old_name)? {
                deletes.push(old_name);
            }
        }
        store.remove_batch(&mut deletes.into_iter())
    }

    fn get_old<K, V>(
        &self,
        store: &dyn Store<K, V>,
        names: &mut dyn Iterator<Item = K>,
    ) -> Result<BTreeMap<K, Option<V>>>
    where
        K: AsRef<str> + From<String> + Copy + Ord,
    {
        names
            .map(|name| Ok((name, store.get(Self::to_old_key(name))?)))
            .collect()
    }

    #[rustfmt::skip]
    fn oldify_defs(&self, names: &file_info::Names) -> Result<()> {
        self.oldify(&*self.classes,  &mut names.classes.iter().map(Into::into))?;
        self.oldify(&*self.funs,     &mut names.funs.iter().map(Into::into))?;
        self.oldify(&*self.typedefs, &mut names.types.iter().map(Into::into))?;
        self.oldify(&*self.consts,   &mut names.consts.iter().map(Into::into))?;
        self.oldify(&*self.modules,  &mut names.modules.iter().map(Into::into))?;
        Ok(())
    }

    #[rustfmt::skip]
    fn remove_old_defs(&self, names: &file_info::Names) -> Result<()> {
        self.remove_old(&*self.classes,  &mut names.classes.iter().map(Into::into))?;
        self.remove_old(&*self.funs,     &mut names.funs.iter().map(Into::into))?;
        self.remove_old(&*self.typedefs, &mut names.types.iter().map(Into::into))?;
        self.remove_old(&*self.consts,   &mut names.consts.iter().map(Into::into))?;
        self.remove_old(&*self.modules,  &mut names.modules.iter().map(Into::into))?;
        Ok(())
    }

    #[rustfmt::skip]
    fn remove_defs(&self, names: &file_info::Names) -> Result<()> {
        self.classes .remove_batch(&mut names.classes.iter().map(Into::into))?;
        self.funs    .remove_batch(&mut names.funs.iter().map(Into::into))?;
        self.typedefs.remove_batch(&mut names.types.iter().map(Into::into))?;
        self.consts  .remove_batch(&mut names.consts.iter().map(Into::into))?;
        self.modules .remove_batch(&mut names.modules.iter().map(Into::into))?;
        Ok(())
    }

    #[rustfmt::skip]
    fn get_old_defs(
        &self,
        names: &file_info::Names,
    ) -> Result<(
        BTreeMap<pos::TypeName, Option<Arc<decl::ShallowClass<BR>>>>,
        BTreeMap<pos::FunName, Option<Arc<decl::FunDecl<BR>>>>,
        BTreeMap<pos::TypeName, Option<Arc<decl::TypedefDecl<BR>>>>,
        BTreeMap<pos::ConstName, Option<Arc<decl::ConstDecl<BR>>>>,
        BTreeMap<pos::ModuleName, Option<Arc<decl::ModuleDecl<BR>>>>,
    )> {
        Ok((
            self.get_old(&*self.classes,  &mut names.classes.iter().map(Into::into))?,
            self.get_old(&*self.funs,     &mut names.funs.iter().map(Into::into))?,
            self.get_old(&*self.typedefs, &mut names.types.iter().map(Into::into))?,
            self.get_old(&*self.consts,   &mut names.consts.iter().map(Into::into))?,
            self.get_old(&*self.modules,  &mut names.modules.iter().map(Into::into))?,
        ))
    }
}

/// Invoke the callback registered with the given name (via
/// `Callback.register`).
///
/// # Panics
///
/// Raises a panic if no callback is registered with the given name, the
/// callback raises an exception, or the returned value cannot be converted to
/// type `T`.
///
/// # Safety
///
/// Calls into the OCaml runtime and may trigger a GC, which may invalidate any
/// unrooted ocaml values (e.g., `UnsafeOcamlPtr`, `ocamlrep::Value`).
unsafe fn call_ocaml<T: FromOcamlRep>(callback_name: &'static str, args: &impl ToOcamlRep) -> T {
    let callback = ocaml_runtime::named_value(callback_name).unwrap();
    let args = ocamlrep_ocamlpool::to_ocaml(args);
    let ocaml_result = ocaml_runtime::callback_exn(callback, args).unwrap();
    T::from_ocaml(ocaml_result).unwrap()
}

/// An implementation of `FileProvider` which calls into
/// `Provider_context.get_entries` in order to read from IDE entries before
/// falling back to reading from ChangesStore/sharedmem/disk.
#[derive(Debug)]
pub struct FileProviderWithContext {
    ctx_is_empty: Arc<AtomicBool>,
    backend: FileProviderWithChanges,
}

impl FileProvider for FileProviderWithContext {
    fn get(&self, file: RelativePath) -> Result<bstr::BString> {
        if !self.ctx_is_empty.load(Ordering::SeqCst) {
            // SAFETY: We must have no unrooted values.
            if let Some(s) =
                unsafe { call_ocaml("hh_rust_provider_backend_get_entry_contents", &file) }
            {
                return Ok(s);
            }
        }
        self.backend.get(file)
    }
}

#[derive(Clone, Debug, ToOcamlRep, FromOcamlRep)]
#[derive(serde::Serialize, serde::Deserialize)]
pub enum FileType {
    Disk(bstr::BString),
    Ide(bstr::BString),
}

/// Port of `File_provider.ml`.
#[derive(Debug)]
struct FileProviderWithChanges {
    // We could use DeltaStore here if not for the fact that the OCaml
    // implementation of `File_provider.get` does not fall back to disk when the
    // given path isn't present in sharedmem/local_changes (it only does so for
    // `File_provider.get_contents`).
    delta_and_changes: Arc<ChangesStore<RelativePath, FileType>>,
    disk: DiskProvider,
}

impl FileProvider for FileProviderWithChanges {
    fn get(&self, file: RelativePath) -> Result<bstr::BString> {
        match self.delta_and_changes.get(file)? {
            Some(FileType::Disk(contents)) => Ok(contents),
            Some(FileType::Ide(contents)) => Ok(contents),
            None => match self.disk.read(file) {
                Ok(contents) => Ok(contents),
                Err(e) if e.kind() == std::io::ErrorKind::NotFound => Ok("".into()),
                Err(e) => Err(e.into()),
            },
        }
    }
}

/// An implementation of `NamingProvider` which calls into `Provider_context` to
/// give naming results reflecting the contents of IDE buffers.
#[derive(Debug)]
pub struct NamingTableWithContext {
    ctx_is_empty: Arc<AtomicBool>,
    fallback: NamingTable,
}

impl NamingTableWithContext {
    fn ctx_is_empty(&self) -> bool {
        self.ctx_is_empty.load(Ordering::SeqCst)
    }

    fn find_symbol_in_context_with_suppression(
        &self,
        find_symbol_callback_name: &'static str,
        fallback: impl Fn() -> Result<Option<(RelativePath, (naming_table::Pos, file_info::NameType))>>,
        name: pos::Symbol,
    ) -> Result<Option<(RelativePath, (naming_table::Pos, file_info::NameType))>> {
        if self.ctx_is_empty() {
            fallback()
        } else {
            // SAFETY: We must have no unrooted values.
            match unsafe { call_ocaml(find_symbol_callback_name, &name) } {
                pos_opt @ Some(_) => Ok(pos_opt),
                None => match fallback()? {
                    None => Ok(None),
                    Some((path, (pos, name_type))) => {
                        // If fallback said it thought the symbol was in ctx, but we definitively
                        // know that it isn't, then the answer is None.
                        if unsafe { call_ocaml("hh_rust_provider_backend_is_pos_in_ctx", &pos) } {
                            Ok(None)
                        } else {
                            Ok(Some((path, (pos, name_type))))
                        }
                    }
                },
            }
        }
    }

    pub fn get_type_pos(
        &self,
        name: TypeName,
    ) -> Result<Option<(naming_table::Pos, naming_types::KindOfType)>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_type_in_context",
                || {
                    let pos_opt = self.fallback.get_type_pos(name)?;
                    Ok(pos_opt.map(|(pos, kind)| (pos.path(), (pos, kind.into()))))
                },
                name.as_symbol(),
            )?
            .map(|(_, (pos, name_type))| (pos, name_type.try_into().unwrap())))
    }

    pub fn get_fun_pos(&self, name: pos::FunName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_fun_in_context",
                || {
                    let pos_opt = self.fallback.get_fun_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos.path(), (pos, file_info::NameType::Fun))))
                },
                name.as_symbol(),
            )?
            .map(|(_, (pos, _))| pos))
    }

    pub fn get_const_pos(&self, name: pos::ConstName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_const_in_context",
                || {
                    let pos_opt = self.fallback.get_const_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos.path(), (pos, file_info::NameType::Const))))
                },
                name.as_symbol(),
            )?
            .map(|(_, (pos, _))| pos))
    }

    pub fn get_module_pos(&self, name: pos::ModuleName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_module_in_context",
                || {
                    let pos_opt = self.fallback.get_module_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos.path(), (pos, file_info::NameType::Module))))
                },
                name.as_symbol(),
            )?
            .map(|(_, (pos, _))| pos))
    }
}

impl NamingProvider for NamingTableWithContext {
    fn get_type_path_and_kind(
        &self,
        name: pos::TypeName,
    ) -> Result<Option<(RelativePath, naming_types::KindOfType)>> {
        Ok(self.get_type_pos(name)?.map(|(pos, k)| (pos.path(), k)))
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
    fn get_canon_type_name(&self, name: pos::TypeName) -> Result<Option<pos::TypeName>> {
        if !self.ctx_is_empty() {
            // SAFETY: We must have no unrooted values.
            if let name_opt @ Some(..) = unsafe {
                call_ocaml(
                    "hh_rust_provider_backend_find_type_canon_name_in_context",
                    &name,
                )
            } {
                return Ok(name_opt);
            }
        }
        self.fallback.get_canon_type_name(name)
    }
    fn get_canon_fun_name(&self, name: pos::FunName) -> Result<Option<pos::FunName>> {
        if !self.ctx_is_empty() {
            // SAFETY: We must have no unrooted values.
            if let name_opt @ Some(..) = unsafe {
                call_ocaml(
                    "hh_rust_provider_backend_find_fun_canon_name_in_context",
                    &name,
                )
            } {
                return Ok(name_opt);
            }
        }
        self.fallback.get_canon_fun_name(name)
    }
}

/// An id contains a pos, name and a optional decl hash. The decl hash is None
/// only in the case when we didn't compute it for performance reasons
pub type Id = (naming_table::Pos, String, Option<Int64>);

pub type HashType = Option<Int64>;

/// A port of `FileInfo.ml`. The record produced by the parsing phase.
#[derive(Clone, Debug, FromOcamlRep, ToOcamlRep)]
#[repr(C)]
pub struct FileInfo {
    pub hash: HashType,
    pub file_mode: Option<oxidized::file_info::Mode>,
    pub funs: Vec<Id>,
    pub classes: Vec<Id>,
    pub typedefs: Vec<Id>,
    pub consts: Vec<Id>,
    pub modules: Vec<Id>,
    /// None if loaded from saved state
    pub comments: Option<()>,
}

impl<'a> From<ParsedFileWithHashes<'a>> for FileInfo {
    /// c.f. OCaml Direct_decl_parser.decls_to_fileinfo

    fn from(file: ParsedFileWithHashes<'a>) -> FileInfo {
        let mut info = FileInfo {
            hash: Some(Int64::from(file.file_decls_hash.as_u64() as i64)),
            file_mode: file.mode,
            funs: vec![],
            classes: vec![],
            typedefs: vec![],
            consts: vec![],
            modules: vec![],
            comments: None,
        };
        let pos = |p: &oxidized_by_ref::pos::Pos<'_>| naming_table::Pos::Full(p.into());
        use oxidized_by_ref::shallow_decl_defs::Decl;
        for &(name, decl, hash) in file.iter() {
            let hash = Int64::from(hash.as_u64() as i64);
            match decl {
                Decl::Class(x) => info.classes.push((pos(x.name.0), name.into(), Some(hash))),
                Decl::Fun(x) => info.funs.push((pos(x.pos), name.into(), Some(hash))),
                Decl::Typedef(x) => info.typedefs.push((pos(x.pos), name.into(), Some(hash))),
                Decl::Const(x) => info.consts.push((pos(x.pos), name.into(), Some(hash))),
                Decl::Module(x) => info.modules.push((pos(x.pos), name.into(), Some(hash))),
            }
        }
        // Match OCaml ordering
        info.classes.reverse();
        info.funs.reverse();
        info.typedefs.reverse();
        info.consts.reverse();
        info.modules.reverse();
        info
    }
}
