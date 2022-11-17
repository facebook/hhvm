// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::marker::PhantomData;
use std::sync::Arc;

use file_provider::FileProvider;
use names::FileSummary;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::parser_options::ParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use pos::RelativePath;
use ty::decl::shallow;
use ty::decl::shallow::NamedDecl;
use ty::reason::Reason;

#[derive(Debug, Clone)]
pub struct DeclParser<R: Reason> {
    file_provider: Arc<dyn FileProvider>,
    pub opts: ParserOptions,
    // We could make our parse methods generic over `R` instead, but it's
    // usually more convenient for callers (especially tests) to pin the decl
    // parser to a single Reason type.
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(file_provider: Arc<dyn FileProvider>) -> Self {
        Self {
            file_provider,
            opts: Default::default(),
            _phantom: PhantomData,
        }
    }

    pub fn with_options(file_provider: Arc<dyn FileProvider>, opts: ParserOptions) -> Self {
        Self {
            file_provider,
            opts,
            _phantom: PhantomData,
        }
    }

    pub fn parse(&self, path: RelativePath) -> anyhow::Result<Vec<shallow::NamedDecl<R>>> {
        let arena = bumpalo::Bump::new();
        let text = self.file_provider.get(path)?;
        let hashed_file = self.parse_impl_fwd(path, &text, &arena);
        Ok((hashed_file.decls.into_iter())
            .map(|(name, decl, _)| NamedDecl::from(&(name, decl)))
            .collect())
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> anyhow::Result<(Vec<shallow::NamedDecl<R>>, FileSummary)> {
        let arena = bumpalo::Bump::new();
        let text = self.file_provider.get(path)?;
        let hashed_file = self.parse_impl_fwd(path, &text, &arena);
        let summary = names::FileSummary::from_fwd_filtered_decls(&hashed_file);
        let decls = (hashed_file.decls.into_iter())
            .map(|(name, decl, _)| NamedDecl::from(&(name, decl)))
            .collect();
        Ok((decls, summary))
    }

    /// parse and hash decls, optionally remove stdlib decls, and restore
    /// decls to file order.
    fn parse_impl_fwd<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> ParsedFileWithHashes<'a> {
        let opts = DeclParserOptions::from_parser_options(&self.opts);
        let parsed_file =
            direct_decl_parser::parse_decls_for_typechecking(&opts, path.into(), text, arena);
        // TODO: The direct decl parser should return decls in the same
        // order as they are declared in the file. At the moment it reverses
        // them. Reverse them again to match the syntactic order.
        let deregister_std_lib = path.is_hhi() && self.opts.po_deregister_php_stdlib;
        let mut hashed_file = ParsedFileWithHashes::from(parsed_file);
        if deregister_std_lib {
            hashed_file.remove_php_stdlib_decls_and_rev(arena);
        } else {
            hashed_file.rev();
        }
        hashed_file
    }

    /// parse and hash decls, then optionally remove stdlib decls.
    /// Decls remain in reversed file order as returned by parser.
    pub fn parse_impl_rev<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> ParsedFileWithHashes<'a> {
        let opts = DeclParserOptions::from_parser_options(&self.opts);
        let parsed_file =
            direct_decl_parser::parse_decls_for_typechecking(&opts, path.into(), text, arena);
        // TODO: The direct decl parser should return decls in the same
        // order as they are declared in the file. At the moment it reverses
        // them. Leave them in reversed order for consumers that expect reversed order.
        let deregister_std_lib = path.is_hhi() && self.opts.po_deregister_php_stdlib;
        let mut hashed_file = ParsedFileWithHashes::from(parsed_file);
        if deregister_std_lib {
            hashed_file.remove_php_stdlib_decls(arena);
        }
        hashed_file
    }
}
