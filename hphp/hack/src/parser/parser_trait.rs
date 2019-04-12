/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::SmartConstructors;
use crate::syntax_error::SyntaxError;

// This could be a set of token kinds, but it's part of parser envirnoment that is often cloned,
// so trying to keep it small.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ExpectedTokens {
    Classish,
    Semicolon,
    RightParen,
    Visibility,
}

#[derive(Debug, Clone)]
pub struct Context<T> {
    pub expected: Vec<ExpectedTokens>,
    pub skipped_tokens: Vec<T>,
}

impl<T> Context<T> {
    pub fn empty() -> Self {
        Self {
            expected: vec![],
            skipped_tokens: vec![],
        }
    }
}

pub trait ParserTrait<'a, S>: Clone
where
    S: SmartConstructors,
{
    fn make(
        _: Lexer<'a, S::Token>,
        _: ParserEnv,
        _: Context<S::Token>,
        _: Vec<SyntaxError>,
    ) -> Self;
    fn add_error(&mut self, _: SyntaxError);
    fn into_parts(self) -> (Lexer<'a, S::Token>, Context<S::Token>, Vec<SyntaxError>);
    fn lexer(&self) -> &Lexer<'a, S::Token>;
    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Token>;
    fn continue_from<P: ParserTrait<'a, S>>(&mut self, _: P);

    fn skipped_tokens(&self) -> &[S::Token];
    fn skipped_tokens_mut(&mut self) -> &mut Vec<S::Token>;

    fn context_mut(&mut self) -> &mut Context<S::Token>;
    fn context(&self) -> &Context<S::Token>;
}
