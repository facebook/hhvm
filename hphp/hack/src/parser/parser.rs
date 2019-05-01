// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::marker::PhantomData;

use crate::declaration_parser::DeclarationParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::{NodeType, SmartConstructors};
use crate::source_text::SourceText;
use crate::syntax_error::SyntaxError;

pub struct Parser<'a, S, T>
where
    S: SmartConstructors<T>,
    S::R: NodeType,
{
    lexer: Lexer<'a, S::Token>,
    errors: Vec<SyntaxError>,
    env: ParserEnv,
    sc_state: T,
    _phantom: PhantomData<S>,
}

impl<'a, S, T: Clone> Parser<'a, S, T>
where
    S: SmartConstructors<T>,
    S::R: NodeType,
{
    pub fn make(source: &'a SourceText<'a>, env: ParserEnv) -> Self {
        let sc_state = S::initial_state(&env);
        Self {
            lexer: Lexer::make(
                source,
                env.is_experimental_mode,
                env.disable_unsafe_expr,
                env.disable_unsafe_block,
                env.force_hh,
                env.enable_xhp,
            ),
            errors: vec![],
            env,
            sc_state,
            _phantom: PhantomData,
        }
    }

    fn into_parts(self) -> (Lexer<'a, S::Token>, Vec<SyntaxError>, ParserEnv, T) {
        (self.lexer, self.errors, self.env, self.sc_state)
    }

    pub fn parse_header_only(
        env: ParserEnv,
        text: &'a SourceText<'a>,
    ) -> Option<<S::R as NodeType>::R> {
        let (lexer, errors, env, sc_state) = Self::make(text, env).into_parts();
        let mut decl_parser: DeclarationParser<S, T> =
            DeclarationParser::make(lexer, env, Context::empty(), errors, sc_state);
        decl_parser
            .parse_leading_markup_section()
            .map(|r| r.extract())
    }

    pub fn parse_script(&mut self) -> <S::R as NodeType>::R {
        let mut decl_parser: DeclarationParser<S, T> = DeclarationParser::make(
            self.lexer.clone(),
            self.env.clone(),
            Context::empty(),
            vec![],
            self.sc_state.clone(),
        );
        let root = decl_parser.parse_script().extract();
        let (lexer, _context, errors, sc_state) = decl_parser.into_parts();
        self.errors = errors;
        self.sc_state = sc_state;
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

    pub fn sc_state(&self) -> &T {
        &self.sc_state
    }
}
