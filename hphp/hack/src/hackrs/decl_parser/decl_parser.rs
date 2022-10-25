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
use pos::RelativePath;
use ty::decl::shallow;
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
        let parsed_file = self.parse_impl(path, &text, &arena);
        Ok(parsed_file.decls.iter().map(Into::into).collect())
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> anyhow::Result<(Vec<shallow::NamedDecl<R>>, FileSummary)> {
        let arena = bumpalo::Bump::new();
        let text = self.file_provider.get(path)?;
        let parsed_file = self.parse_impl(path, &text, &arena);
        let summary = FileSummary::from_decls(parsed_file);
        Ok((parsed_file.decls.iter().map(Into::into).collect(), summary))
    }

    pub fn parse_impl<'a>(
        &self,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> oxidized_by_ref::direct_decl_parser::ParsedFile<'a> {
        let opts = DeclParserOptions::from_parser_options(&self.opts);
        let mut parsed_file =
            direct_decl_parser::parse_decls_for_typechecking(&opts, path.into(), text, arena);
        // TODO: The direct decl parser should return decls in the same
        // order as they are declared in the file. At the moment it reverses
        // them. Reverse them again to match the syntactic order.
        let deregister_std_lib = path.is_hhi() && self.opts.po_deregister_php_stdlib;
        if deregister_std_lib {
            parsed_file.decls.remove_php_stdlib_decls_and_rev(arena);
        } else {
            parsed_file.decls.rev(arena);
        }
        parsed_file
    }
}
