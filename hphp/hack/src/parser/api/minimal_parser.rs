/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use minimal_smart_constructors::MinimalSmartConstructors;
use parser::{
    lexer::Lexer, minimal_syntax::MinimalSyntax, minimal_token::MinimalToken,
    minimal_trivia::MinimalTrivia, parser::Parser, parser_env::ParserEnv,
    smart_constructors::NoState, smart_constructors_wrappers::WithKind, source_text::SourceText,
    syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub type SmartConstructors = WithKind<MinimalSmartConstructors>;
pub type ScState = NoState;
type MinimalSyntaxParser<'a> = Parser<'a, SmartConstructors, ScState>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (MinimalSyntax, Vec<SyntaxError>, NoState) {
    let mut parser = MinimalSyntaxParser::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}

pub fn parse_header_only<'a>(env: ParserEnv, source: &SourceText<'a>) -> Option<MinimalSyntax> {
    MinimalSyntaxParser::parse_header_only(env, source)
}

fn trivia_lexer<'a>(source_text: SourceText<'a>, offset: usize) -> Lexer<'a, MinimalToken> {
    let is_experimental_mode = false;
    Lexer::make_at(&source_text, is_experimental_mode, offset)
}

pub fn scan_leading_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
    trivia_lexer(source_text, offset).scan_leading_xhp_trivia()
}

pub fn scan_trailing_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
    trivia_lexer(source_text, offset).scan_trailing_xhp_trivia()
}

pub fn scan_leading_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
    trivia_lexer(source_text, offset).scan_leading_php_trivia()
}

pub fn scan_trailing_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
    trivia_lexer(source_text, offset).scan_trailing_php_trivia()
}
