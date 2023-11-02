// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::PathBuf;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::Ordering;
use std::sync::Arc;

use anyhow::Result;
use datastore::ChangesStore;
use datastore::Store;
use decl_parser::DeclParser;
use decl_parser::DeclParserOptions;
use file_provider::DiskProvider;
use file_provider::FileProvider;
use folded_decl_provider::FoldedDeclProvider;
use folded_decl_provider::LazyFoldedDeclProvider;
use naming_provider::NamingProvider;
use naming_table::NamingTable;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use oxidized::file_info::NameType;
use oxidized::file_info::Names;
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
    file_store: Arc<ChangesStore<RelativePath, bstr::BString>>,
    file_provider: Arc<FileProviderWithContext>,
    /// Our implementation of NamingProvider, which calls into OCaml to
    /// provide results which take IDE buffers into consideration.
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
        let decl_parser = DeclParser::new(
            Arc::clone(&file_provider) as _,
            DeclParserOptions::from_parser_options(&opts),
            opts.po_deregister_php_stdlib,
        );
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

    fn config(&self) -> Config {
        Config {
            path_ctx: (*self.path_ctx).clone(),
            db_path: self.naming_table.fallback.db_path(),
            opts: self.opts.clone(),
        }
    }

    /// Get the underlying naming table (which includes the SQL database, our
    /// sharedmem cache layer, and the ChangesStore layer used in tests).
    fn naming_table(&self) -> &NamingTable {
        &self.naming_table.fallback
    }

    fn shallow_decl_provider(&self) -> &dyn ShallowDeclProvider<BR> {
        &*self.lazy_shallow_decl_provider
    }

    fn folded_decl_provider(&self) -> &dyn FoldedDeclProvider<BR> {
        &*self.folded_decl_provider
    }

    /// Decl-parse the given file, dedup duplicate definitions of the same
    /// symbol (within the file, as well as removing losers of naming conflicts
    /// with other files), and add the parsed decls to the shallow decl store.
    fn parse_and_cache_decls<'a>(
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
    fn add_decls(
        &self,
        decls: &[&(&str, oxidized_by_ref::shallow_decl_defs::Decl<'_>)],
    ) -> Result<()> {
        self.shallow_decl_store
            .add_decls(decls.iter().copied().map(Into::into))
    }
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_server_provider_backend_register() {
        rust_provider_backend_ffi_trait::DESERIALIZE.set(|data| {
            let config: Config = bincode::deserialize(data).unwrap();
            Arc::new(HhServerProviderBackend::new(config).unwrap())
        }).unwrap();
    }

    fn hh_server_provider_backend_make(
        root: PathBuf,
        hhi_root: PathBuf,
        tmp: PathBuf,
        opts: GlobalOptions,
    ) -> rust_provider_backend_ffi::Backend {
        let path_ctx = RelativePathCtx {
            root,
            hhi: hhi_root,
            tmp,
            ..Default::default()
        };
        let backend = Arc::new(HhServerProviderBackend::new(Config {
            path_ctx,
            opts,
            db_path: None,
        }).unwrap());
        rust_provider_backend_ffi::BackendWrapper::new(backend)
    }
}

impl rust_provider_backend_ffi::ProviderBackendFfi for HhServerProviderBackend {
    fn file_provider(&self) -> &dyn file_provider::FileProvider {
        &*self.file_provider
    }

    fn naming_provider(&self) -> &dyn naming_provider::NamingProvider {
        &*self.naming_table
    }

    fn serialize(&self) -> Vec<u8> {
        bincode::serialize(&self.config()).unwrap()
    }

    fn push_local_changes(&self) {
        self.file_store.push_local_changes();
        self.naming_table.fallback.push_local_changes();
        self.shallow_decl_changes_store.push_local_changes();
        self.folded_classes_store.push_local_changes();
    }

    fn pop_local_changes(&self) {
        self.file_store.pop_local_changes();
        self.naming_table.fallback.pop_local_changes();
        self.shallow_decl_changes_store.pop_local_changes();
        self.folded_classes_store.pop_local_changes();
    }

    fn set_ctx_empty(&self, is_empty: bool) {
        self.ctx_is_empty.store(is_empty, Ordering::SeqCst);
    }

    fn direct_decl_parse_and_cache<'a>(
        &self,
        path: RelativePath,
        text: UnsafeOcamlPtr,
        arena: &'a bumpalo::Bump,
    ) -> oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = ocamlrep::bytes_from_ocamlrep(text_value).expect("expected string");
        self.parse_and_cache_decls(path, text, arena).unwrap()
    }

    fn add_shallow_decls(&self, decls: &[&(&str, oxidized_by_ref::shallow_decl_defs::Decl<'_>)]) {
        self.add_decls(decls).unwrap();
    }

    fn get_fun(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_fun(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::FunName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::FunDecl<BR>>> =
            self.shallow_decl_provider().get_fun(name).unwrap();
        to_ocaml(&res)
    }

    fn get_shallow_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_shallow_class(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ShallowClass<BR>>> =
            self.shallow_decl_provider().get_class(name).unwrap();
        to_ocaml(&res)
    }

    fn get_typedef(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_typedef(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::TypedefDecl<BR>>> =
            self.folded_decl_provider().get_typedef(name).unwrap();
        to_ocaml(&res)
    }

    fn get_gconst(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_const(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::ConstName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ConstDecl<BR>>> =
            self.shallow_decl_provider().get_const(name).unwrap();
        to_ocaml(&res)
    }

    fn get_module(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_module(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::ModuleName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::ModuleDecl<BR>>> =
            self.shallow_decl_provider().get_module(name).unwrap();
        to_ocaml(&res)
    }

    fn get_folded_class(&self, name: UnsafeOcamlPtr) -> UnsafeOcamlPtr {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_folded_class(name) } {
            return to_ocaml(&opt);
        }
        let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
        let res: Option<Arc<decl::FoldedClass<BR>>> =
            self.folded_decl_provider().get_class(name).unwrap();
        to_ocaml(&res)
    }

    fn declare_folded_class(&self, name: pos::TypeName) {
        self.folded_decl_provider().get_class(name).unwrap();
    }

    fn get_type_pos(&self, name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_type_pos(name) } {
            opt
        } else {
            let name = pos::TypeName::from(std::str::from_utf8(name).unwrap());
            Some(to_ocaml(&self.naming_table.get_type_pos(name).unwrap()))
        }
    }

    fn get_fun_pos(&self, name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_fun_pos(name) } {
            opt
        } else {
            let name = pos::FunName::from(std::str::from_utf8(name).unwrap());
            Some(to_ocaml(&self.naming_table.get_fun_pos(name).unwrap()))
        }
    }

    fn get_const_pos(&self, name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_const_pos(name) } {
            opt
        } else {
            let name = pos::ConstName::from(std::str::from_utf8(name).unwrap());
            Some(to_ocaml(&self.naming_table.get_const_pos(name).unwrap()))
        }
    }

    fn get_module_pos(&self, name: UnsafeOcamlPtr) -> Option<UnsafeOcamlPtr> {
        // SAFETY: We have to make sure not to use this value after calling into
        // the OCaml runtime (e.g. after invoking `self.get_ocaml_*`).
        let name = unsafe { name.as_value().as_byte_string().unwrap() };
        if let opt @ Some(_) = unsafe { self.get_ocaml_module_pos(name) } {
            opt
        } else {
            let name = pos::ModuleName::from(std::str::from_utf8(name).unwrap());
            Some(to_ocaml(&self.naming_table.get_module_pos(name).unwrap()))
        }
    }

    fn oldify_defs(&self, names: &Names) {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))
            .unwrap();
        self.shallow_decl_changes_store.oldify_defs(names).unwrap();
    }

    fn remove_old_defs(&self, names: &Names) {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))
            .unwrap();
        self.shallow_decl_changes_store
            .remove_old_defs(names)
            .unwrap();
    }

    fn remove_defs(&self, names: &Names) {
        self.folded_classes_store
            .remove_batch(&mut names.classes.iter().map(Into::into))
            .unwrap();
        self.shallow_decl_changes_store.remove_defs(names).unwrap();
    }

    fn get_old_defs(
        &self,
        names: &Names,
    ) -> (
        BTreeMap<pos::TypeName, Option<Arc<decl::ShallowClass<BR>>>>,
        BTreeMap<pos::FunName, Option<Arc<decl::FunDecl<BR>>>>,
        BTreeMap<pos::TypeName, Option<Arc<decl::TypedefDecl<BR>>>>,
        BTreeMap<pos::ConstName, Option<Arc<decl::ConstDecl<BR>>>>,
        BTreeMap<pos::ModuleName, Option<Arc<decl::ModuleDecl<BR>>>>,
    ) {
        self.shallow_decl_changes_store.get_old_defs(names).unwrap()
    }

    fn file_provider_get(&self, path: RelativePath) -> Option<bstr::BString> {
        self.file_store.get(path).unwrap()
    }

    fn file_provider_provide_file_for_tests(&self, path: RelativePath, contents: bstr::BString) {
        self.file_store.insert(path, contents).unwrap();
    }

    fn file_provider_remove_batch(&self, paths: BTreeSet<RelativePath>) {
        self.file_store
            .remove_batch(&mut paths.into_iter())
            .unwrap();
    }

    fn naming_types_add(
        &self,
        name: pos::TypeName,
        pos: &(file_info::Pos, naming_types::KindOfType),
    ) {
        self.naming_table().add_type(name, pos).unwrap();
    }

    fn naming_types_remove_batch(&self, names: &[pos::TypeName]) {
        self.naming_table().remove_type_batch(names).unwrap();
    }

    fn naming_funs_add(&self, name: pos::FunName, pos: &file_info::Pos) {
        self.naming_table().add_fun(name, pos).unwrap();
    }

    fn naming_funs_remove_batch(&self, names: &[pos::FunName]) {
        self.naming_table().remove_fun_batch(names).unwrap();
    }

    fn naming_consts_add(&self, name: pos::ConstName, pos: &file_info::Pos) {
        self.naming_table().add_const(name, pos).unwrap();
    }

    fn naming_consts_remove_batch(&self, names: &[pos::ConstName]) {
        self.naming_table().remove_const_batch(names).unwrap();
    }

    fn naming_modules_add(&self, name: pos::ModuleName, pos: &file_info::Pos) {
        self.naming_table().add_module(name, pos).unwrap();
    }

    fn naming_modules_remove_batch(&self, names: &[pos::ModuleName]) {
        self.naming_table().remove_module_batch(names).unwrap();
    }

    fn naming_get_db_path(&self) -> Option<PathBuf> {
        self.naming_table().db_path()
    }

    fn naming_set_db_path(&self, db_path: PathBuf) {
        self.naming_table().set_db_path(db_path).unwrap()
    }

    fn naming_get_filenames_by_hash(
        &self,
        deps: &deps_rust::DepSet,
    ) -> std::collections::BTreeSet<RelativePath> {
        self.naming_table().get_filenames_by_hash(deps).unwrap()
    }

    fn as_any(&self) -> &dyn std::any::Any {
        self
    }
}

// NB: this function interacts with the OCaml runtime (but won't trigger a GC).
fn to_ocaml<T: ToOcamlRep + ?Sized>(value: &T) -> UnsafeOcamlPtr {
    // SAFETY: this module doesn't do any concurrent interaction with the OCaml
    // runtime while invoking this function
    unsafe { UnsafeOcamlPtr::new(ocamlrep_ocamlpool::to_ocaml(value)) }
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
    fn oldify_defs(&self, names: &Names) -> Result<()> {
        self.oldify(&*self.classes,  &mut names.classes.iter().map(Into::into))?;
        self.oldify(&*self.funs,     &mut names.funs.iter().map(Into::into))?;
        self.oldify(&*self.typedefs, &mut names.types.iter().map(Into::into))?;
        self.oldify(&*self.consts,   &mut names.consts.iter().map(Into::into))?;
        self.oldify(&*self.modules,  &mut names.modules.iter().map(Into::into))?;
        Ok(())
    }

    #[rustfmt::skip]
    fn remove_old_defs(&self, names: &Names) -> Result<()> {
        self.remove_old(&*self.classes,  &mut names.classes.iter().map(Into::into))?;
        self.remove_old(&*self.funs,     &mut names.funs.iter().map(Into::into))?;
        self.remove_old(&*self.typedefs, &mut names.types.iter().map(Into::into))?;
        self.remove_old(&*self.consts,   &mut names.consts.iter().map(Into::into))?;
        self.remove_old(&*self.modules,  &mut names.modules.iter().map(Into::into))?;
        Ok(())
    }

    #[rustfmt::skip]
    fn remove_defs(&self, names: &Names) -> Result<()> {
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
        names: &Names,
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

/// Port of `File_provider.ml`.
#[derive(Debug)]
struct FileProviderWithChanges {
    // We could use DeltaStore here if not for the fact that the OCaml
    // implementation of `File_provider.get` does not fall back to disk when the
    // given path isn't present in sharedmem/local_changes (it only does so for
    // `File_provider.get_contents`).
    delta_and_changes: Arc<ChangesStore<RelativePath, bstr::BString>>,
    disk: DiskProvider,
}

impl FileProvider for FileProviderWithChanges {
    fn get(&self, file: RelativePath) -> Result<bstr::BString> {
        match self.delta_and_changes.get(file)? {
            Some(contents) => Ok(contents),
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
        fallback: impl Fn() -> Result<Option<(naming_table::Pos, NameType)>>,
        name: pos::Symbol,
    ) -> Result<Option<(naming_table::Pos, file_info::NameType)>> {
        if self.ctx_is_empty() {
            fallback()
        } else {
            // SAFETY: We must have no unrooted values.
            let ctx_pos_opt = unsafe { call_ocaml(find_symbol_callback_name, &name) };
            let fallback_pos_opt = fallback()?;
            match (ctx_pos_opt, fallback_pos_opt) {
                (None, None) => Ok(None),
                (Some(ctx_pos), None) => Ok(Some(ctx_pos)),
                (None, Some((pos, name_type))) => {
                    if unsafe { call_ocaml("hh_rust_provider_backend_is_pos_in_ctx", &pos) } {
                        // If fallback said it thought the symbol was in ctx, but we definitively
                        // know that it isn't, then the answer is None.
                        Ok(None)
                    } else {
                        Ok(Some((pos, name_type)))
                    }
                }
                (Some((ctx_pos, ctx_name_type)), Some((fallback_pos, fallback_name_type))) => {
                    // The alphabetically first filename wins
                    let ctx_fn = ctx_pos.path();
                    let fallback_fn = fallback_pos.path();
                    if ctx_fn <= fallback_fn {
                        // symbol is either (1) a duplicate in both context and fallback, and context is the winner,
                        // or (2) not a duplicate, and both context and fallback claim it to be defined
                        // in a file that's part of the context, in which case context wins.
                        Ok(Some((ctx_pos, ctx_name_type)))
                    } else {
                        // symbol is a duplicate in both context and fallback, and fallback is the winner
                        Ok(Some((fallback_pos, fallback_name_type)))
                    }
                }
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
                    Ok(pos_opt.map(|(pos, kind)| (pos, kind.into())))
                },
                name.as_symbol(),
            )?
            .map(|(pos, name_type)| (pos, name_type.try_into().unwrap())))
    }

    pub fn get_fun_pos(&self, name: pos::FunName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_fun_in_context",
                || {
                    let pos_opt = self.fallback.get_fun_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos, NameType::Fun)))
                },
                name.as_symbol(),
            )?
            .map(|(pos, _)| pos))
    }

    pub fn get_const_pos(&self, name: pos::ConstName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_const_in_context",
                || {
                    let pos_opt = self.fallback.get_const_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos, NameType::Const)))
                },
                name.as_symbol(),
            )?
            .map(|(pos, _)| pos))
    }

    pub fn get_module_pos(&self, name: pos::ModuleName) -> Result<Option<naming_table::Pos>> {
        Ok(self
            .find_symbol_in_context_with_suppression(
                "hh_rust_provider_backend_find_module_in_context",
                || {
                    let pos_opt = self.fallback.get_module_pos(name)?;
                    Ok(pos_opt.map(|pos| (pos, NameType::Module)))
                },
                name.as_symbol(),
            )?
            .map(|(pos, _)| pos))
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
