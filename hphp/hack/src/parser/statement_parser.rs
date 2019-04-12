/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use std::marker::PhantomData;

use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::SmartConstructors;
use crate::syntax_error::SyntaxError;

pub struct StatementParser<'a, S>
where
    S: SmartConstructors,
{
    lexer: Lexer<'a, S::Token>,
    env: ParserEnv,
    context: Context<S::Token>,
    errors: Vec<SyntaxError>,
    _phantom: PhantomData<S>,
}

impl<'a, S> std::clone::Clone for StatementParser<'a, S>
where
    S: SmartConstructors,
{
    fn clone(&self) -> Self {
        Self {
            lexer: self.lexer.clone(),
            env: self.env.clone(),
            context: self.context.clone(),
            errors: self.errors.clone(),
            _phantom: self._phantom.clone(),
        }
    }
}

impl<'a, S> ParserTrait<'a, S> for StatementParser<'a, S>
where
    S: SmartConstructors,
{
    fn make(
        lexer: Lexer<'a, S::Token>,
        env: ParserEnv,
        context: Context<S::Token>,
        errors: Vec<SyntaxError>,
    ) -> Self {
        Self {
            lexer,
            env,
            context,
            errors,
            _phantom: PhantomData,
        }
    }

    fn into_parts(self) -> (Lexer<'a, S::Token>, Context<S::Token>, Vec<SyntaxError>) {
        (self.lexer, self.context, self.errors)
    }

    fn lexer(&self) -> &Lexer<'a, S::Token> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Token> {
        &mut self.lexer
    }

    fn continue_from<P: ParserTrait<'a, S>>(&mut self, other: P) {
        let (lexer, context, errors) = other.into_parts();
        self.lexer = lexer;
        self.context = context;
        self.errors = errors;
    }

    fn add_error(&mut self, error: SyntaxError) {
        self.errors.push(error)
    }

    fn skipped_tokens_mut(&mut self) -> &mut Vec<S::Token> {
        &mut self.context.skipped_tokens
    }

    fn skipped_tokens(&self) -> &[S::Token] {
        &self.context.skipped_tokens
    }

    fn context_mut(&mut self) -> &mut Context<S::Token> {
        &mut self.context
    }

    fn context(&self) -> &Context<S::Token> {
        &self.context
    }
}

impl<'a, S> StatementParser<'a, S>
where
    S: SmartConstructors,
{
    pub fn parse_statement(&mut self) -> S::R {
        unimplemented!()
    }

    pub fn parse_possible_php_function(&mut self, _toplevel: bool) -> S::R {
        unimplemented!()
    }

    pub fn parse_header(&mut self) -> (S::R, bool) {
        unimplemented!()
    }

    pub fn parse_compound_statement(&mut self) -> S::R {
        unimplemented!()
    }
}
