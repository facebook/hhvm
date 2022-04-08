// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh_config::HhConfig;
use names::FileSummary;
use oxidized_by_ref::decl_parser_options::DeclParserOptions;
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
    #[allow(dead_code)] // 'til first use.
    hh_config: HhConfig,
    opts: Options,
    // We could make our parse methods generic over `R` instead, but it's
    // usually more convenient for callers (especially tests) to pin the decl
    // parser to a single Reason type.
    _phantom: PhantomData<R>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(ctx: Arc<RelativePathCtx>) -> Self {
        let hh_config = HhConfig::from_root(&ctx.root).ok().unwrap_or_default();
        let opts_arena = bumpalo::Bump::new();
        let decl_parser_opts = hh_config.get_decl_parser_options(&opts_arena);
        DeclParser::with_options(ctx, hh_config, &decl_parser_opts)
    }

    pub fn with_options(
        relative_path_ctx: Arc<RelativePathCtx>,
        hh_config: HhConfig,
        opts: &DeclParserOptions<'_>,
    ) -> Self {
        Self {
            relative_path_ctx,
            hh_config,
            opts: Options::from(opts),
            _phantom: PhantomData,
        }
    }

    pub fn parse(&self, path: RelativePath) -> std::io::Result<Vec<shallow::Decl<R>>> {
        let arena = bumpalo::Bump::new();
        let absolute_path = path.to_absolute(&self.relative_path_ctx);
        let text = std::fs::read(&absolute_path)?;
        let parsed_file = self.parse_impl(self.opts.get(), path, &text, &arena);
        Ok(parsed_file.decls.iter().map(Into::into).collect())
    }

    pub fn parse_and_summarize(
        &self,
        path: RelativePath,
    ) -> std::io::Result<(Vec<shallow::Decl<R>>, FileSummary)> {
        let arena = bumpalo::Bump::new();
        let absolute_path = path.to_absolute(&self.relative_path_ctx);
        let text = std::fs::read(&absolute_path)?;
        let parsed_file = self.parse_impl(self.opts.get(), path, &text, &arena);
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
        let mut parsed_file = stack_limit::with_elastic_stack(|stack_limit| {
            direct_decl_parser::parse_decls(opts, path.into(), text, arena, Some(stack_limit))
        })
        .unwrap_or_else(|failure| {
            panic!(
                "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                failure.max_stack_size_tried / stack_limit::KI
            );
        });
        // TODO: The direct decl parser should return decls in the same order as
        // they are declared in the file. At the moment it reverses them.
        // Reverse them again to match the syntactic order.
        parsed_file.decls.rev(arena);
        parsed_file
    }
}
