// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![cfg(test)]

use std::collections::BTreeMap;
use std::path::PathBuf;
use std::sync::Arc;

use anyhow::Result;
use datastore::NonEvictingStore;
use decl_parser::DeclParser;
use fbinit::FacebookInit;
use folded_decl_provider::FoldedDeclProvider;
use folded_decl_provider::LazyFoldedDeclProvider;
use hackrs_test_utils::serde_store::StoreOpts::Unserialized;
use hackrs_test_utils::store::make_shallow_decl_store;
use hh24_test::TestRepo;
use naming_provider::SqliteNamingTable;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::parser_options::ParserOptions;
use pos::RelativePathCtx;
use shallow_decl_provider::LazyShallowDeclProvider;
use tempfile::TempDir;
use ty::reason::BReason;

mod folded_decl_provider_test;
mod pos_test;

struct TestContext {
    root: TestRepo,
    decl_parser: DeclParser<BReason>,
    folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
}

impl TestContext {
    fn new(_fb: FacebookInit, files: BTreeMap<&str, &str>) -> Result<Self> {
        let root = TestRepo::new(&files)?;

        let tmpdir = TempDir::with_prefix("rupro_test.")?;
        let naming_db = tmpdir.path().join("names.sql");
        hh24_test::create_naming_table(&naming_db, &files)?;
        let naming_provider = Arc::new(SqliteNamingTable::new(&naming_db).unwrap());

        let path_ctx = Arc::new(RelativePathCtx {
            root: root.path().to_path_buf(),
            hhi: PathBuf::new(),
            dummy: PathBuf::new(),
            tmp: tmpdir.path().to_path_buf(),
        });
        let parser_opts = ParserOptions::default();
        let decl_parser = DeclParser::new(
            Arc::new(file_provider::DiskProvider::new(path_ctx, None)),
            DeclParserOptions::from_parser_options(&parser_opts),
            parser_opts.po_deregister_php_stdlib,
        );
        let shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::new(make_shallow_decl_store::<BReason>(Unserialized)),
            naming_provider,
            decl_parser.clone(),
        ));
        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(Default::default()), // TODO: remove?
            Arc::new(NonEvictingStore::new()),
            shallow_decl_provider,
        ));
        Ok(Self {
            root,
            decl_parser,
            folded_decl_provider,
        })
    }
}
