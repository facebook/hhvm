// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::Bump;
use parser::{
    parser::Parser, parser_env::ParserEnv, source_text::SourceText,
    syntax_by_ref::positioned_syntax::PositionedSyntax, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;
use verify_smart_constructors::{State as VerifyState, VerifySmartConstructors};

pub type SmartConstructors<'a> = VerifySmartConstructors<'a>;

pub fn parse_script<'a>(
    arena: &'a Bump,
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (PositionedSyntax<'a>, Vec<SyntaxError>, VerifyState<'a>) {
    let sc = VerifySmartConstructors::new(arena);
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
