// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use aast_parser::{self, AastParser};
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, direct_decl_parser::ParsedFile};
use parser_core_types::indexed_source_text::IndexedSourceText;
use stack_limit::StackLimit;

pub use aast_parser::Result as AastResult;
pub use rust_aast_parser_types::{Env, Result as ParserResult};

pub fn from_text<'a>(
    env: &'a Env,
    indexed_source_text: &'a IndexedSourceText<'a>,
    arena: &'a Bump,
    stack_limit: Option<&StackLimit>,
) -> (AastResult<ParserResult>, ParsedFile<'a>) {
    let source = indexed_source_text.source_text();
    let (language, mode, parser_env) = AastParser::make_parser_env(env, source);
    let opts = arena.alloc(DeclParserOptions::from_oxidized_parser_options(
        arena,
        &env.parser_options,
    ));
    let (cst, decls) =
        cst_and_decl_parser::parse_script(opts, parser_env, source, mode, arena, stack_limit);
    let ast_result = AastParser::from_tree(
        env,
        indexed_source_text,
        stack_limit,
        arena,
        language,
        mode,
        cst,
    );
    (ast_result, decls)
}
