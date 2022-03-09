// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use anyhow::Result;
use tempdir::TempDir;

use fbinit::FacebookInit;
use hackrs::{
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    naming_provider::{NamingProvider, SqliteNamingTable},
    reason::BReason,
    shallow_decl_provider::{LazyShallowDeclProvider, ShallowDeclProvider},
    special_names::SpecialNames,
    typing_decl_provider::{FoldingTypingDeclProvider, TypingDeclProvider},
};
use hh24_test::TestRepo;
use pos::RelativePathCtx;
use std::{path::PathBuf, rc::Rc, sync::Arc};

mod folded_decl_provider;

struct TestContext {
    #[allow(dead_code)]
    pub root: TestRepo,

    #[allow(dead_code)]
    pub decl_parser: DeclParser<BReason>,

    #[allow(dead_code)]
    pub naming_provider: Arc<dyn NamingProvider>,

    #[allow(dead_code)]
    pub shallow_decl_provider: Arc<dyn ShallowDeclProvider<BReason>>,

    #[allow(dead_code)]
    pub folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,

    #[allow(dead_code)]
    pub typing_decl_provider: Rc<dyn TypingDeclProvider<BReason>>,
}

impl TestContext {
    #[allow(dead_code)]
    fn new(_fb: FacebookInit, files: BTreeMap<&str, &str>) -> Result<Self> {
        let tmpdir = TempDir::new("rupro_test")?;
        let root = TestRepo::new(&files)?;
        let naming_table = tmpdir.path().join("names.sql");
        hh24_test::create_naming_table(&naming_table, &files)?;
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.path().to_path_buf(),
            hhi: PathBuf::new(),
            dummy: PathBuf::new(),
            tmp: PathBuf::new(),
        });
        let decl_parser = DeclParser::new(path_ctx);
        let naming_provider: Arc<dyn NamingProvider> =
            Arc::new(SqliteNamingTable::new(&naming_table).unwrap());
        let shallow_decl_provider: Arc<dyn ShallowDeclProvider<_>> =
            Arc::new(LazyShallowDeclProvider::new(
                Arc::new(hackrs_test_utils::cache::make_non_eviction_shallow_decl_cache()),
                Arc::clone(&naming_provider),
                decl_parser.clone(),
            ));
        let folded_decl_provider: Arc<dyn FoldedDeclProvider<_>> =
            Arc::new(LazyFoldedDeclProvider::new(
                Arc::new(oxidized::global_options::GlobalOptions::default()),
                Arc::new(hackrs_test_utils::cache::NonEvictingCache::new()),
                SpecialNames::new(),
                Arc::clone(&shallow_decl_provider),
            ));
        let typing_decl_provider: Rc<dyn TypingDeclProvider<_>> =
            Rc::new(FoldingTypingDeclProvider::new(
                Box::new(hackrs_test_utils::cache::NonEvictingLocalCache::new()),
                Arc::clone(&folded_decl_provider),
            ));

        Ok(Self {
            root,
            decl_parser,
            naming_provider,
            shallow_decl_provider,
            folded_decl_provider,
            typing_decl_provider,
        })
    }
}
