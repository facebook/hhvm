// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use parser::{
    lexer::Lexer,
    parser::Parser,
    parser_env::ParserEnv,
    source_text::SourceText,
    syntax_by_ref::{
        arena_state::State,
        positioned_token::{PositionedToken, TokenFactory, TokenFactoryFullTrivia},
        positioned_trivia::PositionedTrivia,
        positioned_value::PositionedValue,
        syntax,
    },
    syntax_error::SyntaxError,
};
use positioned_smart_constructors::*;
use stack_limit::StackLimit;

type Syntax<'a> = syntax::Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>;

pub type SmartConstructors<'a> =
    PositionedSmartConstructors<Syntax<'a>, TokenFactory<'a>, State<'a>>;

pub fn parse_script<'src, 'arena>(
    arena: &'arena Bump,
    source: &SourceText<'src>,
    env: ParserEnv,
    stack_limit: Option<&'src StackLimit>,
) -> (Syntax<'arena>, Vec<SyntaxError>, State<'arena>) {
    let tf = TokenFactory::new(arena);
    let sc = SmartConstructors::new(State { arena }, tf);
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}

pub fn parse_header_only<'src, 'arena>(
    env: ParserEnv,
    arena: &'arena Bump,
    source: &SourceText<'src>,
) -> Option<Syntax<'arena>> {
    let tf = TokenFactory::new(arena);
    let sc = SmartConstructors::new(State { arena }, tf);
    Parser::parse_header_only(env, source, sc)
}

fn trivia_lexer<'a>(
    arena: &'a Bump,
    source_text: &'a SourceText<'a>,
    offset: usize,
) -> Lexer<'a, TokenFactoryFullTrivia<'a>> {
    Lexer::make_at(
        source_text,
        offset,
        TokenFactoryFullTrivia::new(arena),
        false,
    )
}

pub fn scan_leading_xhp_trivia<'a>(
    arena: &'a Bump,
    source_text: &'a SourceText<'a>,
    offset: usize,
    width: usize,
) -> PositionedTrivia<'a> {
    trivia_lexer(arena, &source_text, offset).scan_leading_xhp_trivia_with_width(width)
}

pub fn scan_trailing_xhp_trivia<'a>(
    arena: &'a Bump,
    source_text: &'a SourceText<'a>,
    offset: usize,
    _width: usize,
) -> PositionedTrivia<'a> {
    trivia_lexer(arena, source_text, offset).scan_trailing_xhp_trivia()
}

pub fn scan_leading_php_trivia<'a>(
    arena: &'a Bump,
    source_text: &'a SourceText<'a>,
    offset: usize,
    width: usize,
) -> PositionedTrivia<'a> {
    trivia_lexer(arena, source_text, offset).scan_leading_php_trivia_with_width(width)
}

pub fn scan_trailing_php_trivia<'a>(
    arena: &'a Bump,
    source_text: &'a SourceText<'a>,
    offset: usize,
    _width: usize,
) -> PositionedTrivia<'a> {
    trivia_lexer(arena, source_text, offset).scan_trailing_php_trivia()
}
