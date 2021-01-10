// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! `minimal_parser` produces concrete syntax tree parametrized with `MinimalToken` / `MinimalValue`.
//! _Minimal_ refers to minimal amount of information stored in the tree that is still enough
//! to completely (with _full fidelity_ one might say) describe the structure of the file.
//! This information is syntax / token / trivia kinds and widths of individual nodes.
//!
//! The idea was that the simplicty of this format makes it easy to serialize (but the most successful
//! serialization story so far, `HackAST` chooses `JSON` version of `PositionedSyntax` instead anyway).
//! In practice, the only existing applications of this parser are:
//!
//! * looking up mode of the file (by parsing the header node and looking for `// strict`)
//! * recovering trivia information by re-lexing at given offset. OCaml implementation has a memory saving
//!   optimization where it doesn't store all (rarely used) trivia information in its version of
//!   `PositionedSyntax`, instead calling back to this parser on demand

use minimal_smart_constructors::MinimalSmartConstructors;
use parser::{
    lexer::Lexer, minimal_syntax::MinimalSyntax, minimal_token::MinimalToken,
    minimal_trivia::MinimalTrivium, parser::Parser, parser_env::ParserEnv,
    smart_constructors_wrappers::WithKind, source_text::SourceText,
    token_factory::SimpleTokenFactoryImpl,
};

pub fn parse_header_only<'a>(env: ParserEnv, source: &SourceText<'a>) -> Option<MinimalSyntax> {
    let sc = WithKind::new(MinimalSmartConstructors::new());
    Parser::parse_header_only(env, source, sc)
}

fn trivia_lexer<'a>(
    source_text: &SourceText<'a>,
    offset: usize,
) -> Lexer<'a, SimpleTokenFactoryImpl<MinimalToken>> {
    // TODO: disallow hash style comments by default later
    Lexer::make_at(source_text, offset, SimpleTokenFactoryImpl::new(), false)
}

pub fn scan_leading_xhp_trivia(
    source_text: &SourceText,
    offset: usize,
    width: usize,
) -> Vec<MinimalTrivium> {
    trivia_lexer(&source_text, offset).scan_leading_xhp_trivia_with_width(width)
}

pub fn scan_trailing_xhp_trivia(source_text: &SourceText, offset: usize) -> Vec<MinimalTrivium> {
    trivia_lexer(&source_text, offset).scan_trailing_xhp_trivia()
}

pub fn scan_leading_php_trivia(
    source_text: &SourceText,
    offset: usize,
    width: usize,
) -> Vec<MinimalTrivium> {
    trivia_lexer(&source_text, offset).scan_leading_php_trivia_with_width(width)
}

pub fn scan_trailing_php_trivia(source_text: &SourceText, offset: usize) -> Vec<MinimalTrivium> {
    trivia_lexer(&source_text, offset).scan_trailing_php_trivia()
}
