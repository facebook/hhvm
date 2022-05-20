// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use datastore::NonEvictingStore;
use fbinit::FacebookInit;
use file_provider::PlainFileProvider;
use hackrs::{
    decl_parser::DeclParser, folded_decl_provider::LazyFoldedDeclProvider,
    shallow_decl_provider::LazyShallowDeclProvider,
};
use hackrs_provider_backend::{naming_table::NamingTable, ProviderBackend};
use hackrs_test_utils::{
    registrar::DependencyGraph, serde_store::StoreOpts::Unserialized,
    store::make_shallow_decl_store,
};
use hh24_test::TestRepo;
use pos::RelativePathCtx;
use std::{collections::BTreeMap, path::PathBuf, sync::Arc};
use tempdir::TempDir;
use ty::reason::BReason;

mod dependency_registrar;
mod folded_decl_provider;

struct TestContext {
    #[allow(dead_code)]
    pub root: TestRepo,

    #[allow(dead_code)]
    pub provider_backend: ProviderBackend,
}

impl TestContext {
    #[allow(dead_code)]
    fn new(_fb: FacebookInit, files: BTreeMap<&str, &str>) -> Result<Self> {
        let root = TestRepo::new(&files)?;

        let tmpdir = TempDir::new("rupro_test")?;
        let naming_db = tmpdir.path().join("names.sql");
        hh24_test::create_naming_table(&naming_db, &files)?;
        let naming_table = Arc::new(NamingTable::new());
        naming_table.set_db_path(naming_db)?;

        let path_ctx = Arc::new(RelativePathCtx {
            root: root.path().to_path_buf(),
            hhi: PathBuf::new(),
            dummy: PathBuf::new(),
            tmp: tmpdir.path().to_path_buf(),
        });
        let dependency_graph = Arc::new(DependencyGraph::new());
        let file_provider = Arc::new(PlainFileProvider::new(Arc::clone(&path_ctx)));
        let decl_parser = DeclParser::new(Arc::clone(&file_provider) as _);
        let shallow_decl_store = Arc::new(make_shallow_decl_store::<BReason>(Unserialized));
        let shallow_decl_provider = Arc::new(LazyShallowDeclProvider::new(
            Arc::clone(&shallow_decl_store),
            Arc::clone(&naming_table) as _,
            decl_parser.clone(),
        ));
        let folded_classes_store = Arc::new(NonEvictingStore::new());
        let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
            Arc::new(Default::default()), // TODO: remove?
            Arc::clone(&folded_classes_store) as _,
            Arc::clone(&shallow_decl_provider) as _,
            Arc::clone(&dependency_graph) as _,
        ));

        let provider_backend = ProviderBackend::new(
            decl_parser,
            naming_table,
            path_ctx,
            dependency_graph,
            file_provider,
            shallow_decl_store,
            shallow_decl_provider,
            folded_classes_store,
            folded_decl_provider,
        )?;

        Ok(Self {
            root,
            provider_backend,
        })
    }
}
