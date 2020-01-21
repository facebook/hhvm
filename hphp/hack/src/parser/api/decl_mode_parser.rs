// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_mode_smart_constructors::{DeclModeSmartConstructors, State as DeclModeState};
use parser::{
    parser::Parser,
    parser_env::ParserEnv,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    smart_constructors_wrappers::WithKind,
    source_text::SourceText,
    syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub type SmartConstructors<'a> =
    WithKind<DeclModeSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue>>;

pub type ScState<'a> = DeclModeState<'a, PositionedSyntax>;

type DeclModeParser<'a> = Parser<'a, SmartConstructors<'a>, ScState<'a>>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (PositionedSyntax, Vec<SyntaxError>, ScState<'a>) {
    let mut parser = DeclModeParser::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
