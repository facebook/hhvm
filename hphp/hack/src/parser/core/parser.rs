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

pub struct Parser<'a, S, T>
where
    S: SmartConstructors<'a, T>,
    S::R: NodeType,
{
    lexer: Lexer<'a, S::Token>,
    errors: Vec<SyntaxError>,
    env: ParserEnv,
    sc: S,
}

impl<'a, S, T: Clone> Parser<'a, S, T>
where
    S: SmartConstructors<'a, T>,
    S::R: NodeType,
{
    pub fn make(source: &SourceText<'a>, env: ParserEnv) -> Self {
        let source = source.clone();
        let sc = S::new(&env, &source);
        Self {
            lexer: Lexer::make(&source),
            errors: vec![],
            env,
            sc,
        }
    }

    pub fn into_parts(self) -> (Lexer<'a, S::Token>, Vec<SyntaxError>, ParserEnv, S) {
        (self.lexer, self.errors, self.env, self.sc)
    }

    pub fn parse_header_only(
        env: ParserEnv,
        text: &'a SourceText<'a>,
    ) -> Option<<S::R as NodeType>::R> {
        let (lexer, errors, env, sc) = Self::make(text, env).into_parts();
        let mut decl_parser: DeclarationParser<S, T> =
            DeclarationParser::make(lexer, env, Context::empty(None), errors, sc);
        decl_parser
            .parse_leading_markup_section()
            .map(|r| r.extract())
    }

    pub fn parse_script(&mut self, stack_limit: Option<&'a StackLimit>) -> <S::R as NodeType>::R {
        let mut decl_parser: DeclarationParser<S, T> = DeclarationParser::make(
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

    pub fn sc_state(&mut self) -> &T {
        self.sc.state_mut()
    }

    pub fn into_sc_state(self) -> T {
        self.sc.into_state()
    }
}
