// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! `positioned_parser` produces concrete syntax tree parametrized with
//! `PositionedToken` / `PositionedTrivia`. This is probably what you want
//! to use for most applications.
//!
//! As opposed to [minimal_parser](../minimal_parser/), nodes contain more
//! information (like their offset within the file) making it easier to
//! consume (at the cost of taking more memory and more time to produce due
//! to cost of all additional allocations).
//!
//! The structure should be identical to one produced by `minimal_parser`
//! and it *should* be possible to transform minimal tree to positioned one
//! without reparsing the file, but due to bugs in implementations there
//! are multiple small incompatibilities.
//!
//! [coroutine_parser_leak_tree](../coroutine_parser_leak_tree/) and
//! [positioned_coroutine_parser](../positioned_coroutine_parser/) are the
//! versions of the same parser with some small tweaks necessary for
//! particular applications in `hackc` / `hh_server`.

use parser::NoState;
use parser::lexer::Lexer;
use parser::parser::Parser;
use parser::parser_env::ParserEnv;
use parser::positioned_syntax::PositionedSyntax;
use parser::positioned_token::PositionedToken;
use parser::positioned_trivia::PositionedTrivium;
use parser::smart_constructors_wrappers::WithKind;
use parser::source_text::SourceText;
use parser::syntax_error::SyntaxError;
use parser::token_factory::SimpleTokenFactoryImpl;
use positioned_smart_constructors::*;

pub type SmartConstructors = WithKind<
    PositionedSmartConstructors<PositionedSyntax, SimpleTokenFactoryImpl<PositionedToken>, NoState>,
>;

pub type ScState = NoState;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
) -> (PositionedSyntax, Vec<SyntaxError>, NoState) {
    let sc = WithKind::new(PositionedSmartConstructors::new(
        NoState,
        SimpleTokenFactoryImpl::new(),
    ));
    let mut parser = Parser::new(source, env, sc);
    let root = parser.parse_script();
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}

fn trivia_lexer<'a>(
    source_text: &SourceText<'a>,
    offset: usize,
) -> Lexer<'a, SimpleTokenFactoryImpl<PositionedToken>> {
    Lexer::make_at(source_text, offset, SimpleTokenFactoryImpl::new())
}

pub fn scan_leading_xhp_trivia(
    source_text: &SourceText<'_>,
    offset: usize,
    width: usize,
) -> Vec<PositionedTrivium> {
    trivia_lexer(source_text, offset).scan_leading_xhp_trivia_with_width(width)
}

pub fn scan_trailing_xhp_trivia(
    source_text: &SourceText<'_>,
    offset: usize,
) -> Vec<PositionedTrivium> {
    trivia_lexer(source_text, offset).scan_trailing_xhp_trivia()
}

pub fn scan_leading_php_trivia(
    source_text: &SourceText<'_>,
    offset: usize,
    width: usize,
) -> Vec<PositionedTrivium> {
    trivia_lexer(source_text, offset).scan_leading_php_trivia_with_width(width)
}

pub fn scan_trailing_php_trivia(
    source_text: &SourceText<'_>,
    offset: usize,
) -> Vec<PositionedTrivium> {
    trivia_lexer(source_text, offset).scan_trailing_php_trivia()
}
