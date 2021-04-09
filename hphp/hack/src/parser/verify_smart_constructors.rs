// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod verify_smart_constructors_generated;

use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use parser_core_types::{
    syntax_by_ref::{
        has_arena::HasArena,
        positioned_syntax::PositionedSyntax,
        positioned_token::{PositionedToken, TokenFactory},
    },
    syntax_kind::SyntaxKind,
};
use syntax_smart_constructors::{StateType, SyntaxSmartConstructors};

use bumpalo::Bump;

// TODO: This parser is only used by the ffp tests, and should be moved out of
// the parser crate into a separate crate colocated with the tests. This will
// improve build times.

#[derive(Clone)]
pub struct State<'a> {
    stack: Vec<SyntaxKind>,
    arena: &'a Bump,
}

impl<'a> State<'a> {
    pub fn new(arena: &'a Bump) -> Self {
        State {
            stack: vec![],
            arena,
        }
    }

    pub fn push(&mut self, kind: SyntaxKind) {
        self.stack.push(kind);
    }

    pub fn verify(&mut self, args: &[SyntaxKind]) {
        if self.stack.len() < args.len() {
            panic!("unexpected stack state");
        }

        let index = self.stack.len() - args.len();
        let params = self.stack.split_off(index);

        params.iter().zip(args.iter()).for_each(|(p, a)| {
            if p != a {
                panic!(
                    "verification unequal for {} and {}",
                    p.to_string(),
                    a.to_string()
                );
            }
        });
    }

    pub fn stack(&self) -> &[SyntaxKind] {
        &self.stack
    }
}

impl<'a> StateType<PositionedSyntax<'a>> for State<'a> {
    fn next(&mut self, _inputs: &[&PositionedSyntax]) {}
}

impl<'a> HasArena<'a> for State<'a> {
    fn get_arena(&self) -> &'a Bump {
        self.arena
    }
}

pub use crate::verify_smart_constructors_generated::*;

#[derive(Clone)]
pub struct VerifySmartConstructors<'a> {
    pub state: State<'a>,
    pub token_factory: TokenFactory<'a>,
}

impl<'a> VerifySmartConstructors<'a> {
    pub fn new(arena: &'a Bump) -> Self {
        Self {
            state: State::new(arena),
            token_factory: TokenFactory::new(arena),
        }
    }
}

impl<'a> SyntaxSmartConstructors<PositionedSyntax<'a>, TokenFactory<'a>, State<'a>>
    for VerifySmartConstructors<'a>
{
}

impl ToOcamlRep for State<'_> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        self.stack().to_ocamlrep(alloc)
    }
}
