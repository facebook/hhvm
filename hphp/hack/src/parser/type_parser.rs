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

pub struct TypeParser<'a, S>
where
    S: SmartConstructors,
{
    lexer: Lexer<'a, S::Token>,
    env: ParserEnv,
    context: Context<S::Token>,
    errors: Vec<SyntaxError>,
    _phantom: PhantomData<S>,
}

impl<'a, S> std::clone::Clone for TypeParser<'a, S>
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

impl<'a, S> ParserTrait<'a, S> for TypeParser<'a, S>
where
    S: SmartConstructors,
{
    fn make(
        mut lexer: Lexer<'a, S::Token>,
        env: ParserEnv,
        context: Context<S::Token>,
        errors: Vec<SyntaxError>,
    ) -> Self {
        lexer.set_in_type(true);
        Self {
            lexer,
            env,
            context,
            errors,
            _phantom: PhantomData,
        }
    }

    fn into_parts(mut self) -> (Lexer<'a, S::Token>, Context<S::Token>, Vec<SyntaxError>) {
        self.lexer.set_in_type(false);
        (self.lexer, self.context, self.errors)
    }

    fn lexer(&self) -> &Lexer<'a, S::Token> {
        &self.lexer
    }

    fn lexer_mut(&mut self) -> &mut Lexer<'a, S::Token> {
        &mut self.lexer
    }

    fn continue_from<P: ParserTrait<'a, S>>(&mut self, other: P) {
        let (mut lexer, context, errors) = other.into_parts();
        lexer.set_in_type(true);
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

impl<'a, S> TypeParser<'a, S>
where
    S: SmartConstructors,
{
    pub fn parse_generic_type_parameter_list(&mut self) -> S::R {
        unimplemented!()
    }

    pub fn parse_type_constraint_opt(&mut self) -> S::R {
        unimplemented!()
    }

    pub fn parse_return_type(&mut self) -> S::R {
        unimplemented!()
    }

    pub fn parse_possible_generic_specifier(&mut self) -> S::R {
        unimplemented!()
    }

    pub fn parse_type_specifier(&mut self, _allow_var: bool) -> S::R {
        unimplemented!()
    }

    pub fn parse_simple_type_or_type_constant(&mut self) -> S::R {
        unimplemented!()
    }
}
