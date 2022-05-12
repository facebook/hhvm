// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::NonEvictingCache;
use crate::registrar::DependencyGraph;
use crate::serde_cache::CacheOpts;
use crate::SerializingCache;
use hackrs::{
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    shallow_decl_provider::{
        EagerShallowDeclProvider, LazyShallowDeclProvider, ShallowDeclCache, ShallowDeclProvider,
    },
};
use naming_provider::SqliteNamingTable;
use std::path::PathBuf;
use std::sync::Arc;
use ty::reason::Reason;

pub fn make_folded_decl_provider<R: Reason>(
    cache_opts: CacheOpts,
    naming_table: Option<&PathBuf>,
    shallow_decl_cache: ShallowDeclCache<R>,
    decl_parser: DeclParser<R>,
) -> impl FoldedDeclProvider<R> {
    let shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>> =
        if let Some(naming_table_path) = naming_table {
            Arc::new(LazyShallowDeclProvider::new(
                Arc::new(shallow_decl_cache),
                Arc::new(SqliteNamingTable::new(naming_table_path).unwrap()),
                decl_parser,
            ))
        } else {
            Arc::new(EagerShallowDeclProvider::new(Arc::new(shallow_decl_cache)))
        };

    LazyFoldedDeclProvider::new(
        Arc::new(oxidized::global_options::GlobalOptions::default()),
        match cache_opts {
            CacheOpts::Serialized(compression_type) => {
                Arc::new(SerializingCache::with_compression(compression_type))
            }
            CacheOpts::Unserialized => Arc::new(NonEvictingCache::new()),
        },
        shallow_decl_provider,
        Arc::new(DependencyGraph::new()),
    )
}
