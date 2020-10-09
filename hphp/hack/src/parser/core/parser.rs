// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    declaration_parser::DeclarationParser,
    lexer::Lexer,
    parser_env::ParserEnv,
    parser_trait::{Context, ParserTrait},
    smart_constructors::{NodeType, SmartConstructors},
};
use parser_core_types::{source_text::SourceText, syntax_error::SyntaxError};
use stack_limit::StackLimit;

pub struct Parser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    lexer: Lexer<'a, S::TF>,
    errors: Vec<SyntaxError>,
    env: ParserEnv,
    sc: S,
}

impl<'a, S> Parser<'a, S>
where
    S: SmartConstructors,
    S::R: NodeType,
{
    pub fn new(source: &SourceText<'a>, env: ParserEnv, mut sc: S) -> Self {
        let source = source.clone();
        Self {
            lexer: Lexer::make(&source, sc.token_factory().clone()),
            errors: vec![],
            env,
            sc,
        }
    }

    pub fn into_parts(self) -> (Lexer<'a, S::TF>, Vec<SyntaxError>, ParserEnv, S) {
        (self.lexer, self.errors, self.env, self.sc)
    }

    pub fn parse_header_only(
        env: ParserEnv,
        text: &'a SourceText<'a>,
        sc: S,
    ) -> Option<<S::R as NodeType>::R> {
        let (lexer, errors, env, sc) = Self::new(text, env, sc).into_parts();
        let mut decl_parser: DeclarationParser<S> =
            DeclarationParser::make(lexer, env, Context::empty(None), errors, sc);
        decl_parser
            .parse_leading_markup_section()
            .map(|r| r.extract())
    }

    pub fn parse_script(&mut self, stack_limit: Option<&'a StackLimit>) -> <S::R as NodeType>::R {
        let mut decl_parser: DeclarationParser<S> = DeclarationParser::make(
            self.lexer.clone(),
            self.env.clone(),
            Context::empty(stack_limit),
            vec![],
            self.sc.clone(),
        );
        let root = decl_parser.parse_script().extract();
        let (lexer, _context, errors, sc) = decl_parser.into_parts();
        self.errors = errors;
        self.sc = sc;
        self.lexer = lexer;
        root
    }

    pub fn errors(&self) -> Vec<SyntaxError> {
        let mut res = vec![];
        res.extend_from_slice(self.lexer.errors());
        res.extend(self.errors.clone());
        res.reverse();
        res
    }

    pub fn sc_state(&mut self) -> &S::State {
        self.sc.state_mut()
    }

    pub fn into_sc_state(self) -> S::State {
        self.sc.into_state()
    }
}
