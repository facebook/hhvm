// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use direct_decl_smart_constructors::{DirectDeclSmartConstructors, Node, State};
use mode_parser::parse_mode;
use oxidized_by_ref::file_info::Mode;
use parser::parser::Parser;
use parser_core_types::{
    parser_env::ParserEnv, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (Node<'a>, Vec<SyntaxError>, State<'a>, Option<Mode>) {
    let (_, mode) = parse_mode(source);
    let sc = DirectDeclSmartConstructors::new(&source, mode.unwrap_or(Mode::Mpartial), arena);
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state, mode)
}
