// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use parser::{
    parser::Parser,
    parser_env::ParserEnv,
    smart_constructors_wrappers::WithKind,
    source_text::SourceText,
    syntax_by_ref::{
        has_arena::HasArena,
        positioned_token::{PositionedToken, TokenFactory},
        positioned_value::PositionedValue,
        syntax,
    },
    syntax_error::SyntaxError,
};
use positioned_smart_constructors::*;
use stack_limit::StackLimit;
use syntax_smart_constructors::StateType;

#[derive(Clone)]
pub struct State<'a> {
    arena: &'a Bump,
}

impl<'a> HasArena<'a> for State<'a> {
    fn get_arena(&self) -> &'a Bump {
        self.arena
    }
}

impl<R> StateType<R> for State<'_> {
    fn next(&mut self, _inputs: &[&R]) {}
}

type Syntax<'a> = syntax::Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>;

type SmartConstructors<'a> = PositionedSmartConstructors<Syntax<'a>, TokenFactory<'a>, State<'a>>;

pub fn parse_script<'src, 'arena>(
    arena: &'arena Bump,
    source: &SourceText<'src>,
    env: ParserEnv,
    stack_limit: Option<&'src StackLimit>,
) -> (Syntax<'arena>, Vec<SyntaxError>, State<'arena>) {
    let tf = TokenFactory { arena };
    let sc = WithKind::new(SmartConstructors::new(State { arena }, tf));
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
