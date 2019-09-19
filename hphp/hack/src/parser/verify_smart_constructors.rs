/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use parser_core_types::{source_text::SourceText, syntax_kind::SyntaxKind};
use parser_rust::{parser_env::ParserEnv, positioned_syntax::PositionedSyntax};
use syntax_smart_constructors::{StateType, SyntaxSmartConstructors};

// TODO: This parser is only used by the ffp tests, and should be moved out of
// the parser crate into a separate crate colocated with the tests. This will
// improve build times.

#[derive(Clone)]
pub struct State {
    stack: Vec<SyntaxKind>,
}

impl State {
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

impl<'src> StateType<'src, PositionedSyntax> for State {
    fn initial(_env0: &ParserEnv, _src: &SourceText<'src>) -> Self {
        State { stack: vec![] }
    }

    fn next(&mut self, _inputs: &[&PositionedSyntax]) {}
}

pub use crate::verify_smart_constructors_generated::*;

#[derive(Clone)]
pub struct VerifySmartConstructors {
    pub state: State,
}

impl<'src> SyntaxSmartConstructors<'src, PositionedSyntax, State> for VerifySmartConstructors {
    fn new(env: &ParserEnv, src: &SourceText<'src>) -> Self {
        Self {
            state: <State as StateType<'src, PositionedSyntax>>::initial(env, src),
        }
    }
}
