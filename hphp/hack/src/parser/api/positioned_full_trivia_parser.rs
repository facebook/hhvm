// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::Bump;
use parser::indexed_source_text::IndexedSourceText;
use parser::parser::Parser;
use parser::parser_env::ParserEnv;
use parser::source_text::SourceText;
use parser::syntax_by_ref::arena_state::State;
use parser::syntax_by_ref::positioned_token::PositionedTokenFullTrivia;
use parser::syntax_by_ref::positioned_token::TokenFactoryFullTrivia;
use parser::syntax_by_ref::positioned_value::PositionedValueFullTrivia;
use parser::syntax_by_ref::serialize;
use parser::syntax_by_ref::syntax;
use parser::syntax_error::SyntaxError;
use positioned_smart_constructors::*;
use serde::Serialize;
use serde::Serializer;

pub type Syntax<'a> =
    syntax::Syntax<'a, PositionedTokenFullTrivia<'a>, PositionedValueFullTrivia<'a>>;

pub type SmartConstructors<'a> =
    PositionedSmartConstructors<Syntax<'a>, TokenFactoryFullTrivia<'a>, State<'a>>;

pub fn parse_script<'src, 'arena>(
    arena: &'arena Bump,
    source: &SourceText<'src>,
    env: ParserEnv,
) -> (Syntax<'arena>, Vec<SyntaxError>) {
    let tf = TokenFactoryFullTrivia::new(arena);
    let sc = SmartConstructors::new(State { arena }, tf);
    let mut parser = Parser::new(source, env, sc);
    let root = parser.parse_script();
    let errors = parser.errors();
    (root, errors)
}

pub fn parse_script_to_json<'src, 'arena, S: Serializer>(
    arena: &'arena Bump,
    s: S,
    source: &IndexedSourceText<'src>,
    env: ParserEnv,
) -> Result<S::Ok, S::Error> {
    #[derive(Serialize)]
    struct Output<'a> {
        parse_tree: serialize::WithContext<'a, Syntax<'a>>,
        program_text: &'a str,
        version: &'static str,
    }

    let tf = TokenFactoryFullTrivia::new(arena);
    let sc = SmartConstructors::new(State { arena }, tf);
    let mut parser = Parser::new(source.source_text(), env, sc);
    let root = parser.parse_script();

    let parse_tree = serialize::WithContext(source, &root);
    let output = Output {
        parse_tree,
        program_text: source.source_text().text_as_str(),
        version: full_fidelity_schema_version_number::VERSION,
    };
    output.serialize(s)
}
