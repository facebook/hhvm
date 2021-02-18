// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This is a version of [`positioned_parser`]([../positioned_parser/) which skips
//! method bodies in produced syntax tree. When performing type inference in a file, Hack
//! needs to know types of entities in other files, but the method bodies of those entities are not
//! relevant, while also constituting majority of allocated data. Using this parser for this
//! purpose makes computing declarations much more efficient.
//!
//! The above is a slight lie and in reality function bodies _can_ affect the declaration
//! of a function, for example based on presence of `yield` statement which turns a function into
//! generator. This parser tracks and returns this kind of data in its state.

use bumpalo::Bump;
use decl_mode_smart_constructors::{DeclModeSmartConstructors, State as DeclModeState};
use parser::{
    parser::Parser,
    parser_env::ParserEnv,
    smart_constructors_wrappers::WithKind,
    source_text::SourceText,
    syntax_by_ref::{
        positioned_syntax::PositionedSyntax,
        positioned_token::{PositionedToken, TokenFactory},
        positioned_value::PositionedValue,
    },
    syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub type SmartConstructors<'src, 'arena> = WithKind<
    DeclModeSmartConstructors<
        'src,
        'arena,
        PositionedSyntax<'arena>,
        PositionedToken<'arena>,
        PositionedValue<'arena>,
        TokenFactory<'arena>,
    >,
>;

pub type ScState<'src, 'arena> = DeclModeState<'src, 'arena, PositionedSyntax<'arena>>;

pub fn parse_script<'src, 'arena>(
    arena: &'arena Bump,
    source: &SourceText<'src>,
    env: ParserEnv,
    stack_limit: Option<&'src StackLimit>,
) -> (
    PositionedSyntax<'arena>,
    Vec<SyntaxError>,
    ScState<'src, 'arena>,
) {
    let sc = WithKind::new(DeclModeSmartConstructors::new(
        &source,
        TokenFactory::new(arena),
        arena,
    ));
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
