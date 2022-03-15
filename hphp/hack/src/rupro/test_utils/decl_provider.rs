// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::{make_non_eviction_shallow_decl_cache, NonEvictingCache};
use crate::registrar::DependencyMap;
use hackrs::{
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    naming_provider::SqliteNamingTable,
    reason::Reason,
    shallow_decl_provider::{
        EagerShallowDeclProvider, LazyShallowDeclProvider, ShallowDeclProvider,
    },
    special_names::SpecialNames,
};
use pos::RelativePath;
use std::path::PathBuf;
use std::sync::Arc;

fn make_shallow_decl_provider<R: Reason>(
    naming_table_path_opt: Option<&PathBuf>,
    decl_parser: &DeclParser<R>,
    filenames: impl Iterator<Item = RelativePath>,
) -> Arc<dyn ShallowDeclProvider<R>> {
    let cache = Arc::new(make_non_eviction_shallow_decl_cache());
    for path in filenames {
        let decls = decl_parser.parse(path).unwrap();
        cache.add_decls(decls);
    }
    if let Some(naming_table_path) = naming_table_path_opt {
        Arc::new(LazyShallowDeclProvider::new(
            cache,
            Arc::new(SqliteNamingTable::new(naming_table_path).unwrap()),
            decl_parser.clone(),
        ))
    } else {
        Arc::new(EagerShallowDeclProvider::new(cache))
    }
}

pub fn make_folded_decl_provider<R: Reason>(
    naming_table_path_opt: Option<&PathBuf>,
    decl_parser: &DeclParser<R>,
    filenames: impl Iterator<Item = RelativePath>,
) -> Arc<dyn FoldedDeclProvider<R>> {
    let shallow_decl_provider =
        make_shallow_decl_provider(naming_table_path_opt, decl_parser, filenames);
    Arc::new(LazyFoldedDeclProvider::new(
        Arc::new(oxidized::global_options::GlobalOptions::default()),
        Arc::new(NonEvictingCache::new()),
        SpecialNames::new(),
        shallow_decl_provider,
        Arc::new(DependencyMap::new()),
    ))
}
