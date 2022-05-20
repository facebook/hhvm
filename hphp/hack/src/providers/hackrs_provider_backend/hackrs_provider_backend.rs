// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod naming_table;

use anyhow::Result;
use datastore::Store;
use depgraph_api::{DepGraph, NoDepGraph};
use file_provider::{FileProvider, PlainFileProvider};
use hackrs::{
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    shallow_decl_provider::{LazyShallowDeclProvider, ShallowDeclProvider, ShallowDeclStore},
};
use naming_table::NamingTable;
use oxidized_by_ref::parser_options::ParserOptions;
use pos::{RelativePathCtx, TypeName};
use std::sync::Arc;
use ty::{decl::folded::FoldedClass, reason::BReason};

pub struct ProviderBackend {
    pub path_ctx: Arc<RelativePathCtx>,
    pub file_provider: Arc<dyn FileProvider>,
    pub decl_parser: DeclParser<BReason>,
    pub dependency_graph: Arc<dyn DepGraph>,
    pub naming_table: Arc<NamingTable>,
    pub shallow_decl_store: Arc<ShallowDeclStore<BReason>>,
    pub shallow_decl_provider: Arc<dyn ShallowDeclProvider<BReason>>,
    pub folded_classes_store: Arc<dyn Store<TypeName, Arc<FoldedClass<BReason>>>>,
    pub folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
}

impl ProviderBackend {
    pub fn new(
        decl_parser: DeclParser<BReason>,
        naming_table: Arc<NamingTable>,
        path_ctx: Arc<RelativePathCtx>,
        dependency_graph: Arc<dyn DepGraph>,
        file_provider: Arc<dyn FileProvider>,
        shallow_decl_store: Arc<ShallowDeclStore<BReason>>,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<BReason>>,
        folded_classes_store: Arc<dyn Store<TypeName, Arc<FoldedClass<BReason>>>>,
        folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
    ) -> Result<Self> {
        Ok(Self {
            path_ctx,
            file_provider,
            decl_parser,
            dependency_graph,
            naming_table,
            shallow_decl_store,
            shallow_decl_provider,
            folded_classes_store,
            folded_decl_provider,
        })
    }

    pub fn for_hh_server(path_ctx: RelativePathCtx, popt: &ParserOptions<'_>) -> Result<Self> {
        let path_ctx = Arc::new(path_ctx);
        let file_provider: Arc<dyn FileProvider> =
            Arc::new(PlainFileProvider::new(Arc::clone(&path_ctx)));
        let decl_parser = DeclParser::with_options(Arc::clone(&file_provider), popt);
        let dependency_graph = Arc::new(NoDepGraph::new());
        let naming_table = Arc::new(NamingTable::new());

        let shallow_decl_store = Arc::new(hackrs_test_utils::store::make_shallow_decl_store::<
            BReason,
        >(
            hackrs_test_utils::serde_store::StoreOpts::Unserialized,
        ));

        let shallow_decl_provider: Arc<dyn ShallowDeclProvider<_>> =
            Arc::new(LazyShallowDeclProvider::new(
                Arc::clone(&shallow_decl_store),
                Arc::clone(&naming_table) as _,
                decl_parser.clone(),
            ));

        let folded_classes_store: Arc<dyn Store<pos::TypeName, Arc<FoldedClass<_>>>> =
            Arc::new(datastore::NonEvictingStore::new());

        let folded_decl_provider: Arc<dyn FoldedDeclProvider<_>> =
            Arc::new(LazyFoldedDeclProvider::new(
                Arc::new(Default::default()), // TODO: remove?
                Arc::clone(&folded_classes_store),
                Arc::clone(&shallow_decl_provider),
                Arc::clone(&dependency_graph) as _,
            ));

        Ok(Self {
            path_ctx,
            file_provider,
            decl_parser,
            dependency_graph,
            naming_table,
            shallow_decl_store,
            shallow_decl_provider,
            folded_classes_store,
            folded_decl_provider,
        })
    }
}
