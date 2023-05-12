// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;
use std::sync::Arc;

use datastore::NonEvictingStore;
use decl_parser::DeclParser;
use folded_decl_provider::FoldedDeclProvider;
use folded_decl_provider::LazyFoldedDeclProvider;
use naming_provider::SqliteNamingTable;
use oxidized::parser_options::ParserOptions;
use shallow_decl_provider::EagerShallowDeclProvider;
use shallow_decl_provider::LazyShallowDeclProvider;
use shallow_decl_provider::ShallowDeclProvider;
use shallow_decl_provider::ShallowDeclStore;
use ty::reason::Reason;

use crate::serde_store::StoreOpts;
use crate::SerializingStore;

pub fn make_folded_decl_provider<R: Reason>(
    store_opts: StoreOpts,
    naming_table: Option<&PathBuf>,
    shallow_decl_store: ShallowDeclStore<R>,
    opts: Arc<ParserOptions>,
    decl_parser: DeclParser<R>,
) -> impl FoldedDeclProvider<R> {
    let shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>> =
        if let Some(naming_table_path) = naming_table {
            Arc::new(LazyShallowDeclProvider::new(
                Arc::new(shallow_decl_store),
                Arc::new(SqliteNamingTable::new(naming_table_path).unwrap()),
                decl_parser,
            ))
        } else {
            Arc::new(EagerShallowDeclProvider::new(Arc::new(shallow_decl_store)))
        };

    LazyFoldedDeclProvider::new(
        opts,
        match store_opts {
            StoreOpts::Serialized(compression_type) => {
                Arc::new(SerializingStore::with_compression(compression_type))
            }
            StoreOpts::Unserialized => Arc::new(NonEvictingStore::new()),
        },
        shallow_decl_provider,
    )
}
