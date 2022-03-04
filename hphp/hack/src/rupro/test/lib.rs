// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use anyhow::Result;
use tempdir::TempDir;

use fbinit::FacebookInit;
use hackrs::{
    cache::NonEvictingCache,
    decl_parser::DeclParser,
    folded_decl_provider::LazyFoldedDeclProvider,
    naming_provider::SqliteNamingTable,
    reason::NReason,
    shallow_decl_provider::{LazyShallowDeclProvider, ShallowDeclCache},
    special_names::SpecialNames,
};
use hh24_test::{create_naming_table, TestRepo};
use pos::RelativePathCtx;
use std::path::PathBuf;
use std::sync::Arc;

mod test_file_missing_error;

struct TestContext {
    #[allow(dead_code)]
    pub root: TestRepo,

    #[allow(dead_code)]
    pub decl_parser: DeclParser,

    #[allow(dead_code)]
    pub shallow_decl_provider: Arc<LazyShallowDeclProvider<NReason>>,

    #[allow(dead_code)]
    pub folded_decl_provider: Arc<LazyFoldedDeclProvider<NReason>>,
}

impl TestContext {
    #[allow(dead_code)]
    fn new(_fb: FacebookInit, files: BTreeMap<&str, &str>) -> Result<Self> {
        let tmpdir = TempDir::new("rupro_test")?;
        let root = TestRepo::new(&files)?;
        let naming_table = tmpdir.path().join("names.sql");
        create_naming_table(&naming_table, &files)?;
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.path().to_path_buf(),
            hhi: PathBuf::new(),
            dummy: PathBuf::new(),
            tmp: PathBuf::new(),
        });
        let decl_parser = DeclParser::new(path_ctx);
        let shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::new(ShallowDeclCache::with_no_eviction()),
            Arc::new(SqliteNamingTable::new(&naming_table).unwrap()),
            decl_parser.clone(),
        ));
        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(NonEvictingCache::new()),
            SpecialNames::new(),
            shallow_decl_provider.clone(),
        ));

        Ok(Self {
            root,
            decl_parser,
            shallow_decl_provider,
            folded_decl_provider,
        })
    }
}
