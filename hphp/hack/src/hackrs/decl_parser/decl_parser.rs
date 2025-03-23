// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::marker::PhantomData;
use std::sync::Arc;

use file_provider::FileProvider;
use names::FileSummary;
pub use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::direct_decl_parser::ParsedFileWithHashes;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes as ParsedFileWithHashesObr;
use pos::RelativePath;
use ty::decl::shallow;
use ty::decl::shallow::NamedDecl;
use ty::reason::Reason;

#[derive(Debug, Clone)]
pub struct DeclParser<R: Reason> {
    file_provider: Arc<dyn FileProvider>,
    decl_parser_opts: DeclParserOptions,
    // We could make our parse methods generic over `R` instead, but it's
    // usually more convenient for callers (especially tests) to pin the decl
    // parser to a single Reason type.
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(file_provider: Arc<dyn FileProvider>, decl_parser_opts: DeclParserOptions) -> Self {
        Self {
            file_provider,
            decl_parser_opts,
            _phantom: PhantomData,
        }
    }

    pub fn parse(&self, path: RelativePath) -> anyhow::Result<Vec<shallow::NamedDecl<R>>> {
        let text = self.file_provider.get(path)?;
        self.parse_with_text(path, &text)
    }

    pub fn parse_with_text(
        &self,
        path: RelativePath,
        text: &[u8],
    ) -> anyhow::Result<Vec<shallow::NamedDecl<R>>> {
        if self.decl_parser_opts.use_oxidized_by_ref_decls {
            let arena = bumpalo::Bump::new();
            let hashed_file = self.parse_impl_obr(path, text, &arena);
            Ok(hashed_file
                .into_iter()
                .map(|(name, decl, _, _)| NamedDecl::from(&(name, decl)))
                .collect())
        } else {
            let hashed_file = self.parse_impl(path, text);
            Ok(hashed_file
                .into_iter()
                .map(|(name, decl, _, _)| NamedDecl::from((name, decl)))
                .collect())
        }
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> anyhow::Result<(Vec<shallow::NamedDecl<R>>, FileSummary)> {
        let text = self.file_provider.get(path)?;
        if self.decl_parser_opts.use_oxidized_by_ref_decls {
            let arena = bumpalo::Bump::new();
            let hashed_file = self.parse_impl_obr(path, &text, &arena);
            let summary = names::FileSummary::new_obr(&hashed_file);
            let decls = hashed_file
                .into_iter()
                .map(|(name, decl, _, _)| NamedDecl::from(&(name, decl)))
                .collect();
            Ok((decls, summary))
        } else {
            let hashed_file = self.parse_impl(path, &text);
            let summary = names::FileSummary::new(&hashed_file);
            let decls = hashed_file
                .into_iter()
                .map(|(name, decl, _, _)| NamedDecl::from((name, decl)))
                .collect();
            Ok((decls, summary))
        }
    }

    /// Parse and hash decls, removing stdlib decls if that's what parser-options say.
    pub fn parse_impl_obr<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> ParsedFileWithHashesObr<'a> {
        let prefix = path.prefix();
        let opts = &self.decl_parser_opts;
        let deregister_php_stdlib_if_hhi = opts.deregister_php_stdlib;
        let parsed_file =
            direct_decl_parser::parse_decls_for_typechecking_obr(opts, path.into(), text, arena);
        ParsedFileWithHashesObr::new(parsed_file, deregister_php_stdlib_if_hhi, prefix, arena)
    }
    /// Parse and hash decls, removing stdlib decls if that's what parser-options say.
    fn parse_impl(&self, path: RelativePath, text: &[u8]) -> ParsedFileWithHashes {
        let prefix = path.prefix();
        let opts = &self.decl_parser_opts;
        let deregister_php_stdlib_if_hhi = opts.deregister_php_stdlib;
        let parsed_file = direct_decl_parser::parse_decls_for_typechecking(opts, path.into(), text);
        ParsedFileWithHashes::new(parsed_file, deregister_php_stdlib_if_hhi, prefix)
    }
}
