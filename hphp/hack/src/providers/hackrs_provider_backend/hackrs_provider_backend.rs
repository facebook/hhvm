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
use naming_table::NamingTable;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use oxidized::parser_options::ParserOptions;
use pos::RelativePath;
use pos::RelativePathCtx;
use pos::TypeName;
use shallow_decl_provider::LazyShallowDeclProvider;
use shallow_decl_provider::ShallowDeclProvider;
use shallow_decl_provider::ShallowDeclStore;
use shm_store::ShmStore;
use ty::decl;
use ty::decl::folded::FoldedClass;
use ty::reason::BReason;

pub struct HhServerProviderBackend {
    path_ctx: Arc<RelativePathCtx>,
    parser_options: ParserOptions,
    decl_parser: DeclParser<BReason>,
    file_store: Arc<ChangesStore<RelativePath, FileType>>,
    file_provider: Arc<FileProviderWithChanges>,
    naming_table: Arc<NamingTable>,
    /// Collection of Arcs pointing to the backing stores for the
    /// ShallowDeclStore below, allowing us to invoke push/pop_local_changes.
    shallow_decl_changes_store: Arc<ShallowStoreWithChanges>,
    shallow_decl_store: Arc<ShallowDeclStore<BReason>>,
    lazy_shallow_decl_provider: Arc<LazyShallowDeclProvider<BReason>>,
    folded_classes_store: Arc<ChangesStore<TypeName, Arc<FoldedClass<BReason>>>>,
    folded_decl_provider: Arc<LazyFoldedDeclProvider<BReason>>,
}

#[derive(serde::Serialize, serde::Deserialize)]
pub struct Config {
    pub path_ctx: RelativePathCtx,
    pub parser_options: ParserOptions,
    pub db_path: Option<PathBuf>,
}

impl HhServerProviderBackend {
    pub fn new(config: Config) -> Result<Self> {
        let Config {
            path_ctx,
            parser_options,
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
            disk: DiskProvider::new(Arc::clone(&path_ctx)),
        });
        let decl_parser =
            DeclParser::with_options(Arc::clone(&file_provider) as _, parser_options.clone());
        let dependency_graph = Arc::new(depgraph_api::NoDepGraph::new());
        let naming_table = Arc::new(NamingTable::new(db_path)?);

        let shallow_decl_changes_store = Arc::new(ShallowStoreWithChanges::new());
        let shallow_decl_store = Arc::new(shallow_decl_changes_store.as_shallow_decl_store());

        let lazy_shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::clone(&shallow_decl_store),
            Arc::clone(&naming_table) as _,
            decl_parser.clone(),
        ));

        let folded_classes_store = Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
            "FoldedClasses",
            shm_store::Evictability::Evictable,
            shm_store::Compression::default(),
        ))));

        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(parser_options.clone()),
            Arc::clone(&folded_classes_store) as _,
            Arc::clone(&lazy_shallow_decl_provider) as _,
            dependency_graph,
        ));

        Ok(Self {
            path_ctx,
            parser_options,
            file_store,
            file_provider,
            decl_parser,
            folded_decl_provider,
            naming_table,
            shallow_decl_changes_store,
            shallow_decl_store,
            lazy_shallow_decl_provider,
            folded_classes_store,
        })
    }

    pub fn config(&self) -> Config {
        Config {
            path_ctx: (*self.path_ctx).clone(),
            db_path: self.naming_table.db_path(),
            parser_options: self.parser_options.clone(),
        }
    }

    pub fn naming_table(&self) -> &NamingTable {
        &self.naming_table
    }

    pub fn file_store(&self) -> &dyn Store<RelativePath, FileType> {
        &*self.file_store
    }

    pub fn file_provider(&self) -> &dyn FileProvider {
        &*self.file_provider
    }

    pub fn shallow_decl_provider(&self) -> &dyn ShallowDeclProvider<BReason> {
        &*self.lazy_shallow_decl_provider
    }

    pub fn folded_decl_provider(&self) -> &dyn FoldedDeclProvider<BReason> {
        &*self.folded_decl_provider
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
        decls: &[(&str, oxidized_by_ref::shallow_decl_defs::Decl<'_>)],
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
}

pub struct ShallowStoreWithChanges {
    classes: Arc<ChangesStore<TypeName, Arc<decl::ShallowClass<BReason>>>>,
    typedefs: Arc<ChangesStore<TypeName, Arc<decl::TypedefDecl<BReason>>>>,
    funs: Arc<ChangesStore<pos::FunName, Arc<decl::FunDecl<BReason>>>>,
    consts: Arc<ChangesStore<pos::ConstName, Arc<decl::ConstDecl<BReason>>>>,
    modules: Arc<ChangesStore<pos::ModuleName, Arc<decl::ModuleDecl<BReason>>>>,
    properties: Arc<ChangesStore<(TypeName, pos::PropName), decl::Ty<BReason>>>,
    static_properties: Arc<ChangesStore<(TypeName, pos::PropName), decl::Ty<BReason>>>,
    methods: Arc<ChangesStore<(TypeName, pos::MethodName), decl::Ty<BReason>>>,
    static_methods: Arc<ChangesStore<(TypeName, pos::MethodName), decl::Ty<BReason>>>,
    constructors: Arc<ChangesStore<TypeName, decl::Ty<BReason>>>,
}

impl ShallowStoreWithChanges {
    pub fn new() -> Self {
        Self {
            classes: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Classes",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            typedefs: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Typedefs",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            funs: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Funs",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            consts: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Consts",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            modules: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Modules",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            properties: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Properties",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            static_properties: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "StaticProperties",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            methods: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Methods",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            static_methods: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "StaticMethods",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
            constructors: Arc::new(ChangesStore::new(Arc::new(ShmStore::new(
                "Constructors",
                shm_store::Evictability::Evictable,
                shm_store::Compression::default(),
            )))),
        }
    }

    pub fn push_local_changes(&self) {
        self.classes.push_local_changes();
        self.typedefs.push_local_changes();
        self.funs.push_local_changes();
        self.consts.push_local_changes();
        self.modules.push_local_changes();
        self.properties.push_local_changes();
        self.static_properties.push_local_changes();
        self.methods.push_local_changes();
        self.static_methods.push_local_changes();
        self.constructors.push_local_changes();
    }

    pub fn pop_local_changes(&self) {
        self.classes.pop_local_changes();
        self.typedefs.pop_local_changes();
        self.funs.pop_local_changes();
        self.consts.pop_local_changes();
        self.modules.pop_local_changes();
        self.properties.pop_local_changes();
        self.static_properties.pop_local_changes();
        self.methods.pop_local_changes();
        self.static_methods.pop_local_changes();
        self.constructors.pop_local_changes();
    }

    pub fn as_shallow_decl_store(&self) -> ShallowDeclStore<BReason> {
        ShallowDeclStore::new(
            Arc::clone(&self.classes) as _,
            Arc::clone(&self.typedefs) as _,
            Arc::clone(&self.funs) as _,
            Arc::clone(&self.consts) as _,
            Arc::clone(&self.modules) as _,
            Arc::clone(&self.properties) as _,
            Arc::clone(&self.static_properties) as _,
            Arc::clone(&self.methods) as _,
            Arc::clone(&self.static_methods) as _,
            Arc::clone(&self.constructors) as _,
        )
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
