// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod naming_table;

#[cfg(test)]
mod test_naming_table;

use std::path::PathBuf;
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
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use oxidized::global_options::GlobalOptions;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::ModuleName;
use pos::PropName;
use pos::RelativePath;
use pos::RelativePathCtx;
use pos::TypeName;
use shallow_decl_provider::LazyShallowDeclProvider;
use shallow_decl_provider::ShallowDeclProvider;
use shallow_decl_provider::ShallowDeclStore;
use shm_store::OcamlShmStore;
use shm_store::ShmStore;
use ty::decl;
use ty::decl::folded::FoldedClass;
use ty::reason::BReason as BR;

pub struct HhServerProviderBackend {
    path_ctx: Arc<RelativePathCtx>,
    opts: GlobalOptions,
    decl_parser: DeclParser<BR>,
    file_store: Arc<ChangesStore<RelativePath, FileType>>,
    file_provider: Arc<FileProviderWithChanges>,
    naming_table: Arc<NamingTable>,
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
        let file_store = Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
            "File",
            shm_store::Evictability::NonEvictable,
            shm_store::Compression::default(),
        ))));
        let file_provider = Arc::new(FileProviderWithChanges {
            delta_and_changes: Arc::clone(&file_store),
            disk: DiskProvider::new(Arc::clone(&path_ctx), None),
        });
        let decl_parser = DeclParser::with_options(Arc::clone(&file_provider) as _, opts.clone());
        let dependency_graph = Arc::new(depgraph_api::NoDepGraph::new());
        let naming_table = Arc::new(NamingTable::new(db_path)?);

        let shallow_decl_changes_store =
            Arc::new(ShallowStoreWithChanges::new(opts.tco_populate_member_heaps));
        let shallow_decl_store = shallow_decl_changes_store.as_shallow_decl_store();

        let lazy_shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::clone(&shallow_decl_store),
            Arc::clone(&naming_table) as _,
            decl_parser.clone(),
        ));

        let folded_classes_shm = Arc::new(OcamlShmStore::new(
            "FoldedClasses",
            shm_store::Evictability::Evictable,
            shm_store::Compression::default(),
        ));
        let folded_classes_store =
            Arc::new(ChangesStore::new(Arc::clone(&folded_classes_shm) as _));

        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(opts.clone()),
            Arc::clone(&folded_classes_store) as _,
            Arc::clone(&lazy_shallow_decl_provider) as _,
            dependency_graph,
        ));

        Ok(Self {
            path_ctx,
            opts,
            file_store,
            file_provider,
            decl_parser,
            folded_decl_provider,
            naming_table,
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
            db_path: self.naming_table.db_path(),
            opts: self.opts.clone(),
        }
    }

    pub fn opts(&self) -> &GlobalOptions {
        &self.opts
    }

    pub fn naming_table(&self) -> &NamingTable {
        &self.naming_table
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
    ) -> Result<oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes<'a>> {
        let mut parsed_file = self.decl_parser.parse_impl(path, text, arena);
        self.lazy_shallow_decl_provider
            .dedup_and_add_decls(path, parsed_file.decls.iter().map(Into::into))?;
        parsed_file.decls.rev(arena); // To match OCaml behavior
        Ok(parsed_file.into())
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
        self.naming_table.push_local_changes();
        self.shallow_decl_changes_store.push_local_changes();
        self.folded_classes_store.push_local_changes();
    }

    pub fn pop_local_changes(&self) {
        self.file_store.pop_local_changes();
        self.naming_table.pop_local_changes();
        self.shallow_decl_changes_store.pop_local_changes();
        self.folded_classes_store.pop_local_changes();
    }

    // ---
    // Deletion support

    pub fn oldify_funs_batch(&self, _names: &[FunName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_funs_batch(&self, names: &[FunName]) -> Result<()> {
        self.shallow_decl_changes_store
            .funs
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_funs_batch(&self, _names: &[FunName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_shallow_classes_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_shallow_classes_batch(&self, names: &[TypeName]) -> Result<()> {
        self.shallow_decl_changes_store
            .classes
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_shallow_classes_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_folded_classes_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_folded_classes_batch(&self, names: &[TypeName]) -> Result<()> {
        self.folded_classes_store
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_folded_classes_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_typedefs_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_typedefs_batch(&self, names: &[TypeName]) -> Result<()> {
        self.shallow_decl_changes_store
            .typedefs
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_typedefs_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_gconsts_batch(&self, _names: &[ConstName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_gconsts_batch(&self, names: &[ConstName]) -> Result<()> {
        self.shallow_decl_changes_store
            .consts
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_gconsts_batch(&self, _names: &[ConstName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_modules_batch(&self, _names: &[ModuleName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_modules_batch(&self, names: &[ModuleName]) -> Result<()> {
        self.shallow_decl_changes_store
            .modules
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_modules_batch(&self, _names: &[ModuleName]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_props_batch(&self, _names: &[(TypeName, PropName)]) -> Result<()> {
        Ok(())
    }
    pub fn remove_props_batch(&self, names: &[(TypeName, PropName)]) -> Result<()> {
        self.shallow_decl_changes_store
            .props
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_props_batch(&self, _names: &[(TypeName, PropName)]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_static_props_batch(&self, _names: &[(TypeName, PropName)]) -> Result<()> {
        Ok(())
    }
    pub fn remove_static_props_batch(&self, names: &[(TypeName, PropName)]) -> Result<()> {
        self.shallow_decl_changes_store
            .static_props
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_static_props_batch(&self, _names: &[(TypeName, PropName)]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_methods_batch(&self, _names: &[(TypeName, MethodName)]) -> Result<()> {
        Ok(())
    }
    pub fn remove_methods_batch(&self, names: &[(TypeName, MethodName)]) -> Result<()> {
        self.shallow_decl_changes_store
            .methods
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_methods_batch(&self, _names: &[(TypeName, MethodName)]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_static_methods_batch(&self, _names: &[(TypeName, MethodName)]) -> Result<()> {
        Ok(())
    }
    pub fn remove_static_methods_batch(&self, names: &[(TypeName, MethodName)]) -> Result<()> {
        self.shallow_decl_changes_store
            .static_methods
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_static_methods_batch(&self, _names: &[(TypeName, MethodName)]) -> Result<()> {
        Ok(())
    }

    pub fn oldify_constructors_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }
    pub fn remove_constructors_batch(&self, names: &[TypeName]) -> Result<()> {
        self.shallow_decl_changes_store
            .constructors
            .remove_batch(&mut names.iter().copied())
    }
    pub fn remove_old_constructors_batch(&self, _names: &[TypeName]) -> Result<()> {
        Ok(())
    }

    //
    // ---
}

impl rust_provider_backend_api::RustProviderBackend<BR> for HhServerProviderBackend {
    fn file_provider(&self) -> &dyn FileProvider {
        &*self.file_provider
    }

    fn naming_provider(&self) -> &dyn NamingProvider {
        &*self.naming_table
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
    pub unsafe fn get_ocaml_shallow_class(&self, name: TypeName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.classes.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.classes_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_typedef(&self, name: TypeName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.typedefs.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.typedefs_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_fun(&self, name: pos::FunName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.funs.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.funs_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_const(&self, name: pos::ConstName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.consts.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.consts_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_module(&self, name: pos::ModuleName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.modules.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.modules_shm.get_ocaml_value(name) }
    }

    pub unsafe fn get_ocaml_folded_class(&self, name: TypeName) -> Option<UnsafeOcamlPtr> {
        if self.folded_classes_store.has_local_change(name) { None }
        else { self.folded_classes_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_property(&self, name: (TypeName, pos::PropName)) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.props.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.props_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_static_property(&self, name: (TypeName, pos::PropName)) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.static_props.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.static_props_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_method(&self, name: (TypeName, pos::MethodName)) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.methods.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.methods_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_static_method(&self, name: (TypeName, pos::MethodName)) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.static_methods.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.static_methods_shm.get_ocaml_value(name) }
    }
    pub unsafe fn get_ocaml_constructor(&self, name: TypeName) -> Option<UnsafeOcamlPtr> {
        if self.shallow_decl_changes_store.constructors.has_local_change(name) { None }
        else { self.shallow_decl_changes_store.constructors_shm.get_ocaml_value(name) }
    }
}

#[rustfmt::skip]
struct ShallowStoreWithChanges {
    classes:            Arc<ChangesStore <TypeName, Arc<decl::ShallowClass<BR>>>>,
    classes_shm:        Arc<OcamlShmStore<TypeName, Arc<decl::ShallowClass<BR>>>>,
    typedefs:           Arc<ChangesStore <TypeName, Arc<decl::TypedefDecl<BR>>>>,
    typedefs_shm:       Arc<OcamlShmStore<TypeName, Arc<decl::TypedefDecl<BR>>>>,
    funs:               Arc<ChangesStore <pos::FunName, Arc<decl::FunDecl<BR>>>>,
    funs_shm:           Arc<OcamlShmStore<pos::FunName, Arc<decl::FunDecl<BR>>>>,
    consts:             Arc<ChangesStore <pos::ConstName, Arc<decl::ConstDecl<BR>>>>,
    consts_shm:         Arc<OcamlShmStore<pos::ConstName, Arc<decl::ConstDecl<BR>>>>,
    modules:            Arc<ChangesStore <pos::ModuleName, Arc<decl::ModuleDecl<BR>>>>,
    modules_shm:        Arc<OcamlShmStore<pos::ModuleName, Arc<decl::ModuleDecl<BR>>>>,
    props:              Arc<ChangesStore <(TypeName, pos::PropName), decl::Ty<BR>>>,
    props_shm:          Arc<OcamlShmStore<(TypeName, pos::PropName), decl::Ty<BR>>>,
    static_props:       Arc<ChangesStore <(TypeName, pos::PropName), decl::Ty<BR>>>,
    static_props_shm:   Arc<OcamlShmStore<(TypeName, pos::PropName), decl::Ty<BR>>>,
    methods:            Arc<ChangesStore <(TypeName, pos::MethodName), decl::Ty<BR>>>,
    methods_shm:        Arc<OcamlShmStore<(TypeName, pos::MethodName), decl::Ty<BR>>>,
    static_methods:     Arc<ChangesStore <(TypeName, pos::MethodName), decl::Ty<BR>>>,
    static_methods_shm: Arc<OcamlShmStore<(TypeName, pos::MethodName), decl::Ty<BR>>>,
    constructors:       Arc<ChangesStore <TypeName, decl::Ty<BR>>>,
    constructors_shm:   Arc<OcamlShmStore<TypeName, decl::Ty<BR>>>,
    store_view: Arc<ShallowDeclStore<BR>>,
}

impl ShallowStoreWithChanges {
    #[rustfmt::skip]
    fn new(populate_member_heaps: bool) -> Self {
        use shm_store::{Compression, Evictability::Evictable};
        let classes_shm =        Arc::new(OcamlShmStore::new("Classes", Evictable, Compression::default()));
        let typedefs_shm =       Arc::new(OcamlShmStore::new("Typedefs", Evictable, Compression::default()));
        let funs_shm =           Arc::new(OcamlShmStore::new("Funs", Evictable, Compression::default()));
        let consts_shm =         Arc::new(OcamlShmStore::new("Consts", Evictable, Compression::default()));
        let modules_shm =        Arc::new(OcamlShmStore::new("Modules", Evictable, Compression::default()));
        let props_shm =          Arc::new(OcamlShmStore::new("Props", Evictable, Compression::default()));
        let static_props_shm =   Arc::new(OcamlShmStore::new("StaticProps", Evictable, Compression::default()));
        let methods_shm =        Arc::new(OcamlShmStore::new("Methods", Evictable, Compression::default()));
        let static_methods_shm = Arc::new(OcamlShmStore::new("StaticMethods", Evictable, Compression::default()));
        let constructors_shm =   Arc::new(OcamlShmStore::new("Constructors", Evictable, Compression::default()));

        let classes =        Arc::new(ChangesStore::new(Arc::clone(&classes_shm) as _));
        let typedefs =       Arc::new(ChangesStore::new(Arc::clone(&typedefs_shm) as _));
        let funs =           Arc::new(ChangesStore::new(Arc::clone(&funs_shm) as _));
        let consts =         Arc::new(ChangesStore::new(Arc::clone(&consts_shm) as _));
        let modules =        Arc::new(ChangesStore::new(Arc::clone(&modules_shm) as _));
        let props =          Arc::new(ChangesStore::new(Arc::clone(&props_shm) as _));
        let static_props =   Arc::new(ChangesStore::new(Arc::clone(&static_props_shm) as _));
        let methods =        Arc::new(ChangesStore::new(Arc::clone(&methods_shm) as _));
        let static_methods = Arc::new(ChangesStore::new(Arc::clone(&static_methods_shm) as _));
        let constructors =   Arc::new(ChangesStore::new(Arc::clone(&constructors_shm) as _));

        let store_view = if populate_member_heaps {
            Arc::new(ShallowDeclStore::new(
                Arc::clone(&classes) as _,
                Arc::clone(&typedefs) as _,
                Arc::clone(&funs) as _,
                Arc::clone(&consts) as _,
                Arc::clone(&modules) as _,
                Arc::clone(&props) as _,
                Arc::clone(&static_props) as _,
                Arc::clone(&methods) as _,
                Arc::clone(&static_methods) as _,
                Arc::clone(&constructors) as _,
            ))
        } else {
            Arc::new(ShallowDeclStore::with_no_member_stores(
                Arc::clone(&classes) as _,
                Arc::clone(&typedefs) as _,
                Arc::clone(&funs) as _,
                Arc::clone(&consts) as _,
                Arc::clone(&modules) as _,
            ))
        };
        Self {
            classes,
            typedefs,
            funs,
            consts,
            modules,
            props,
            static_props,
            methods,
            static_methods,
            constructors,
            classes_shm,
            typedefs_shm,
            funs_shm,
            consts_shm,
            modules_shm,
            props_shm,
            static_props_shm,
            methods_shm,
            static_methods_shm,
            constructors_shm,
            store_view,
        }
    }

    fn push_local_changes(&self) {
        self.classes.push_local_changes();
        self.typedefs.push_local_changes();
        self.funs.push_local_changes();
        self.consts.push_local_changes();
        self.modules.push_local_changes();
        self.props.push_local_changes();
        self.static_props.push_local_changes();
        self.methods.push_local_changes();
        self.static_methods.push_local_changes();
        self.constructors.push_local_changes();
    }

    fn pop_local_changes(&self) {
        self.classes.pop_local_changes();
        self.typedefs.pop_local_changes();
        self.funs.pop_local_changes();
        self.consts.pop_local_changes();
        self.modules.pop_local_changes();
        self.props.pop_local_changes();
        self.static_props.pop_local_changes();
        self.methods.pop_local_changes();
        self.static_methods.pop_local_changes();
        self.constructors.pop_local_changes();
    }

    fn as_shallow_decl_store(&self) -> Arc<ShallowDeclStore<BR>> {
        Arc::clone(&self.store_view)
    }
}

#[derive(Clone, Debug, ToOcamlRep, FromOcamlRep)]
#[derive(serde::Serialize, serde::Deserialize)]
pub enum FileType {
    Disk(bstr::BString),
    Ide(bstr::BString),
}

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
