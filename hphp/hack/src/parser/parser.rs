// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::declaration_parser::DeclarationParser;
use crate::lexer::Lexer;
use crate::mode_parser::parse_mode;
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::{NodeType, SmartConstructors};
use crate::stack_limit::StackLimit;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_tree::SyntaxTree;

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
    pub fn make(source: &'a SourceText<'a>, env: ParserEnv) -> Self {
        let sc = S::new(&env, source);
        Self {
            lexer: Lexer::make(source, env.is_experimental_mode),
            errors: vec![],
            env,
            sc,
        }
    }

    pub fn make_syntax_tree(
        source: &'a SourceText<'a>,
        env: ParserEnv,
    ) -> SyntaxTree<<S::R as NodeType>::R, S> {
        let mode = parse_mode(&source);
        let mut parser = Parser::make(&source, env);
        let root = parser.parse_script(None);
        let (_, errors, _, state) = parser.into_parts();
        SyntaxTree::create(source, root, errors, mode, state, None)
    }

    fn into_parts(self) -> (Lexer<'a, S::Token>, Vec<SyntaxError>, ParserEnv, S) {
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
}
