// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;

use aast_parser::AastParser;
pub use aast_parser::Result;
use bumpalo::Bump;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use parser_core_types::indexed_source_text::IndexedSourceText;
pub use rust_aast_parser_types::Env;
pub use rust_aast_parser_types::ParserResult;

pub fn from_text<'a>(
    env: &'a Env,
    indexed_source_text: &'a IndexedSourceText<'a>,
    arena: &'a Bump,
) -> (Result<ParserResult>, ParsedFile<'a>) {
    let source = indexed_source_text.source_text();
    let (language, mode, parser_env) = AastParser::make_parser_env(env, source);
    let opts = DeclParserOptions::from_parser_options(&env.parser_options);
    let (cst, decls) = cst_and_decl_parser::parse_script(&opts, parser_env, source, mode, arena);
    let ast_result = AastParser::from_tree(
        env,
        indexed_source_text,
        arena,
        language,
        mode,
        cst,
        HashSet::default(),
    );
    (ast_result, decls)
}
