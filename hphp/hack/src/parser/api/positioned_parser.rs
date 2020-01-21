// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser::{
    parser::Parser, parser_env::ParserEnv, positioned_syntax::PositionedSyntax,
    smart_constructors_wrappers::WithKind, source_text::SourceText, syntax_error::SyntaxError,
    NoState,
};
use positioned_smart_constructors::*;
use stack_limit::StackLimit;

pub type SmartConstructors = WithKind<PositionedSmartConstructors>;

pub type ScState = NoState;

type PositionedSyntaxParser<'a> = Parser<'a, SmartConstructors, ScState>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (PositionedSyntax, Vec<SyntaxError>, NoState) {
    let mut parser = PositionedSyntaxParser::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
