// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod syntax_smart_constructors_generated;

use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};
use smart_constructors::NoState;

pub use crate::syntax_smart_constructors_generated::*;

pub trait StateType<'src, R>: Clone {
    fn initial(env: &ParserEnv, source_text: &SourceText<'src>) -> Self;
    fn next(&mut self, inputs: &[&R]);
}

impl<'src, R> StateType<'src, R> for NoState {
    fn initial(_env: &ParserEnv, _: &SourceText<'src>) -> Self {
        NoState {}
    }
    fn next(&mut self, _inputs: &[&R]) {}
}
