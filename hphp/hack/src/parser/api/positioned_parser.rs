// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! `positioned_parser` produces concrete syntax tree parametrized with `PositionedToken` / `PositionedTrivia`.
//! This is probably what you want to use for most applications.
//!
//! As opposed to [minimal_parser](../minimal_parser/), nodes contain more information (like their offset within the file)
//! making it easier to consume (at the cost of taking more memory and more time to produce due to cost of all
//! additional allocations).
//!
//! The structure should be identical to one produced by `minimal_parser` and it *should* be possible to transform
//! minimal tree to positioned one without reparsing the file, but due to bugs in implementations there
//! are multiple small incompatibilities.
//!
//! [coroutine_parser_leak_tree](../coroutine_parser_leak_tree/) and
//! [positioned_coroutine_parser](../positioned_coroutine_parser/) are the versions of the same parser
//! with some small tweaks necessary for particular applications in `hackc` / `hh_server`.

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
