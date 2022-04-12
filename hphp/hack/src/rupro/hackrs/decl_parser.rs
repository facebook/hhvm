// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use names::FileSummary;
use oxidized_by_ref::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::parser_options::ParserOptions;
use pos::{RelativePath, RelativePathCtx};
use std::marker::PhantomData;
use std::sync::Arc;
use ty::decl::shallow;
use ty::reason::Reason;

mod options;
use options::Options;

#[derive(Debug, Clone)]
pub struct DeclParser<R: Reason> {
    relative_path_ctx: Arc<RelativePathCtx>,
    opts: Options,
    // We could make our parse methods generic over `R` instead, but it's
    // usually more convenient for callers (especially tests) to pin the decl
    // parser to a single Reason type.
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        Self {
            relative_path_ctx,
            opts: Default::default(),
            _phantom: PhantomData,
        }
    }

    pub fn with_options(relative_path_ctx: Arc<RelativePathCtx>, opts: &ParserOptions<'_>) -> Self {
        Self {
            relative_path_ctx,
            opts: Options::from(opts),
            _phantom: PhantomData,
        }
    }

    pub fn parse(&self, path: RelativePath) -> std::io::Result<Vec<shallow::Decl<R>>> {
        let arena = bumpalo::Bump::new();
        let absolute_path = path.to_absolute(&self.relative_path_ctx);
        let text = std::fs::read(&absolute_path)?;
        let decl_parser_opts = DeclParserOptions::from_parser_options(self.opts.get());
        let parsed_file = self.parse_impl(&decl_parser_opts, path, &text, &arena);
        Ok(parsed_file.decls.iter().map(Into::into).collect())
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> std::io::Result<(Vec<shallow::Decl<R>>, FileSummary)> {
        let arena = bumpalo::Bump::new();
        let absolute_path = path.to_absolute(&self.relative_path_ctx);
        let text = std::fs::read(&absolute_path)?;
        let opts = DeclParserOptions::from(self.opts.get());
        let parsed_file = self.parse_impl(&opts, path, &text, &arena);
        let summary = FileSummary::from_decls(parsed_file);
        Ok((parsed_file.decls.iter().map(Into::into).collect(), summary))
    }

    fn parse_impl<'a>(
        &self,
        opts: &'a DeclParserOptions<'a>,
        path: RelativePath,
        text: &'a [u8],
        arena: &'a bumpalo::Bump,
    ) -> oxidized_by_ref::direct_decl_parser::ParsedFile<'a> {
        let mut parsed_file = direct_decl_parser::parse_decls(opts, path.into(), text, arena);
        // TODO: The direct decl parser should return decls in the same order as
        // they are declared in the file. At the moment it reverses them.
        // Reverse them again to match the syntactic order.
        parsed_file.decls.rev(arena);
        parsed_file
    }
}
