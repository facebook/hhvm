// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod verify_smart_constructors_generated;

use parser_core_types::{
    positioned_syntax::PositionedSyntax, positioned_token::PositionedToken,
    syntax_kind::SyntaxKind, token_factory::SimpleTokenFactoryImpl,
};
use syntax_smart_constructors::{StateType, SyntaxSmartConstructors};

use ocaml::core::mlvalues::Value;
use rust_to_ocaml::{to_list, SerializationContext, ToOcaml};

// TODO: This parser is only used by the ffp tests, and should be moved out of
// the parser crate into a separate crate colocated with the tests. This will
// improve build times.

#[derive(Clone)]
pub struct State {
    stack: Vec<SyntaxKind>,
}

impl State {
    pub fn new() -> Self {
        State { stack: vec![] }
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
                panic!(format!(
                    "verification unequal for {} and {}",
                    p.to_string(),
                    a.to_string()
                ));
            }
        });
    }

    pub fn stack(&self) -> &[SyntaxKind] {
        &self.stack
    }
}

impl StateType<PositionedSyntax> for State {
    fn next(&mut self, _inputs: &[&PositionedSyntax]) {}
}

pub use crate::verify_smart_constructors_generated::*;

#[derive(Clone)]
pub struct VerifySmartConstructors {
    pub state: State,
    pub token_factory: SimpleTokenFactoryImpl<PositionedToken>,
}

impl VerifySmartConstructors {
    pub fn new() -> Self {
        Self {
            state: State::new(),
            token_factory: SimpleTokenFactoryImpl::new(),
        }
    }
}

impl SyntaxSmartConstructors<PositionedSyntax, SimpleTokenFactoryImpl<PositionedToken>, State>
    for VerifySmartConstructors
{
}

impl ToOcaml for State {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        to_list(self.stack(), context)
    }
}
