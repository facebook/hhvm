// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod naming_table;
mod no_depgraph;

use anyhow::Result;
use file_provider::{FileProvider, PlainFileProvider};
use hackrs::{
    cache::Cache,
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    shallow_decl_provider::{LazyShallowDeclProvider, ShallowDeclCache, ShallowDeclProvider},
};
use naming_table::NamingTable;
use no_depgraph::NoDepGraph;
use oxidized_by_ref::parser_options::ParserOptions;
use pos::RelativePathCtx;
use std::sync::Arc;
use ty::{decl::folded::FoldedClass, reason::BReason};

pub struct ProviderBackend {
    pub path_ctx: Arc<RelativePathCtx>,
    pub file_provider: Arc<dyn FileProvider>,
    pub decl_parser: DeclParser<BReason>,
    pub dependency_graph: Arc<NoDepGraph>,
    pub naming_table: Arc<NamingTable>,
    pub shallow_decl_cache: Arc<ShallowDeclCache<BReason>>,
    pub shallow_decl_provider: Arc<dyn ShallowDeclProvider<BReason>>,
    pub folded_classes_cache: Arc<dyn Cache<pos::TypeName, Arc<FoldedClass<BReason>>>>,
    pub folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
}

impl ProviderBackend {
    pub fn new(path_ctx: RelativePathCtx, popt: &ParserOptions<'_>) -> Result<Self> {
        let path_ctx = Arc::new(path_ctx);
        let file_provider: Arc<dyn FileProvider> =
            Arc::new(PlainFileProvider::new(Arc::clone(&path_ctx)));
        let decl_parser = DeclParser::with_options(Arc::clone(&file_provider), popt);
        let dependency_graph = Arc::new(NoDepGraph::new());
        let naming_table = Arc::new(NamingTable::new());

        let shallow_decl_cache = Arc::new(hackrs_test_utils::cache::make_shallow_decl_cache::<
            BReason,
        >(
            hackrs_test_utils::serde_cache::CacheOpts::Unserialized,
        ));

        let shallow_decl_provider: Arc<dyn ShallowDeclProvider<_>> =
            Arc::new(LazyShallowDeclProvider::new(
                Arc::clone(&shallow_decl_cache),
                Arc::clone(&naming_table) as _,
                decl_parser.clone(),
            ));

        let folded_classes_cache: Arc<dyn Cache<pos::TypeName, Arc<FoldedClass<_>>>> =
            Arc::new(hackrs_test_utils::cache::NonEvictingCache::new());

        let folded_decl_provider: Arc<dyn FoldedDeclProvider<_>> =
            Arc::new(LazyFoldedDeclProvider::new(
                Arc::new(Default::default()), // TODO: remove?
                Arc::clone(&folded_classes_cache),
                Arc::clone(&shallow_decl_provider),
                Arc::clone(&dependency_graph) as _,
            ));

        Ok(Self {
            path_ctx,
            file_provider,
            decl_parser,
            dependency_graph,
            naming_table,
            shallow_decl_cache,
            shallow_decl_provider,
            folded_classes_cache,
            folded_decl_provider,
        })
    }
}
