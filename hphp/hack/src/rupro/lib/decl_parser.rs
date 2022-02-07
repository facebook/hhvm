// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::shallow;
use crate::reason::Reason;
use pos::{RelativePath, RelativePathCtx};
use std::sync::Arc;

#[derive(Debug)]
pub struct DeclParser<R: Reason> {
    alloc: &'static Allocator<R>,
    relative_path_ctx: Arc<RelativePathCtx>,
}

impl<R: Reason> DeclParser<R> {
    pub fn new(alloc: &'static Allocator<R>, relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        Self {
            alloc,
            relative_path_ctx,
        }
    }

    pub fn parse(&self, path: RelativePath) -> std::io::Result<Vec<shallow::Decl<R>>> {
        let arena = bumpalo::Bump::new();
        let absolute_path = path.to_absolute(&self.relative_path_ctx);
        let text = std::fs::read(&absolute_path)?;
        let parsed_file = stack_limit::with_elastic_stack(|stack_limit| {
            direct_decl_parser::parse_decls(
                Default::default(),
                path.to_oxidized(),
                &text,
                &arena,
                Some(stack_limit),
            )
        })
        .unwrap_or_else(|failure| {
            panic!(
                "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                failure.max_stack_size_tried / stack_limit::KI
            );
        });
        Ok(self.alloc.decls(parsed_file.decls.iter()))
    }
}
