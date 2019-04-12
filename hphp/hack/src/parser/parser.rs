/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use std::marker::PhantomData;

use crate::declaration_parser::DeclarationParser;
use crate::lexer::Lexer;
use crate::parser_env::ParserEnv;
use crate::parser_trait::{Context, ParserTrait};
use crate::smart_constructors::SmartConstructors;
use crate::source_text::SourceText;
use crate::syntax_error::SyntaxError;

pub struct Parser<'a, S>
where
    S: SmartConstructors,
{
    lexer: Lexer<'a, S::Token>,
    errors: Vec<SyntaxError>,
    env: ParserEnv,
    _phantom: PhantomData<S>,
}

impl<'a, S> Parser<'a, S>
where
    S: SmartConstructors,
{
    pub fn make(source: &'a SourceText<'a>, env: ParserEnv) -> Self {
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
            _phantom: PhantomData,
        }
    }

    fn into_parts(self) -> (Lexer<'a, S::Token>, Vec<SyntaxError>, ParserEnv) {
        (self.lexer, self.errors, self.env)
    }

    pub fn parse_header_only(env: ParserEnv, text: &'a SourceText<'a>) -> Option<S::R> {
        let (lexer, errors, env) = Self::make(text, env).into_parts();
        let mut decl_parser: DeclarationParser<S> =
            DeclarationParser::make(lexer, env, Context::empty(), errors);
        decl_parser.parse_leading_markup_section()
    }

    pub fn parse_script(&mut self) -> S::R {
        let mut decl_parser: DeclarationParser<S> = DeclarationParser::make(
            self.lexer.clone(),
            self.env.clone(),
            Context::empty(),
            vec![],
        );
        let root = decl_parser.parse_script();
        let (_, _, errors) = decl_parser.into_parts();
        self.errors = errors;
        root
    }

    pub fn errors(&self) -> Vec<SyntaxError> {
        let mut res = vec![];
        res.extend_from_slice(self.lexer.errors());
        res.extend(self.errors.clone());
        res
    }
}
