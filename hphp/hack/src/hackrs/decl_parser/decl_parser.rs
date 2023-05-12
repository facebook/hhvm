// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::marker::PhantomData;
use std::sync::Arc;

use file_provider::FileProvider;
use names::FileSummary;
pub use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use pos::RelativePath;
use ty::decl::shallow;
use ty::decl::shallow::NamedDecl;
use ty::reason::Reason;

#[derive(Debug, Clone)]
pub struct DeclParser<R: Reason> {
    file_provider: Arc<dyn FileProvider>,
    deregister_php_stdlib: bool,
    decl_parser_opts: DeclParserOptions,
    // We could make our parse methods generic over `R` instead, but it's
    // usually more convenient for callers (especially tests) to pin the decl
    // parser to a single Reason type.
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(
        file_provider: Arc<dyn FileProvider>,
        decl_parser_opts: DeclParserOptions,
        deregister_php_stdlib: bool,
    ) -> Self {
        Self {
            file_provider,
            decl_parser_opts,
            deregister_php_stdlib,
            _phantom: PhantomData,
        }
    }

    pub fn parse(&self, path: RelativePath) -> anyhow::Result<Vec<shallow::NamedDecl<R>>> {
        let arena = bumpalo::Bump::new();
        let text = self.file_provider.get(path)?;
        let hashed_file = self.parse_impl(path, &text, &arena);
        Ok(hashed_file
            .into_iter()
            .map(|(name, decl, _)| NamedDecl::from(&(name, decl)))
            .collect())
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> anyhow::Result<(Vec<shallow::NamedDecl<R>>, FileSummary)> {
        let arena = bumpalo::Bump::new();
        let text = self.file_provider.get(path)?;
        let hashed_file = self.parse_impl(path, &text, &arena);
        let summary = names::FileSummary::new(&hashed_file);
        let decls = hashed_file
            .into_iter()
            .map(|(name, decl, _)| NamedDecl::from(&(name, decl)))
            .collect();
        Ok((decls, summary))
    }

    /// Parse and hash decls, removing stdlib decls if that's what parser-options say.
    pub fn parse_impl<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> ParsedFileWithHashes<'a> {
        let prefix = path.prefix();
        let deregister_php_stdlib_if_hhi = self.deregister_php_stdlib;
        let opts = &self.decl_parser_opts;
        let parsed_file =
            direct_decl_parser::parse_decls_for_typechecking(opts, path.into(), text, arena);
        ParsedFileWithHashes::new(parsed_file, deregister_php_stdlib_if_hhi, prefix, arena)
    }
}
