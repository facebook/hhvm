// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::{syntax::Syntax, syntax_error::SyntaxError, syntax_tree::SyntaxTree};

use oxidized::parser_options::ParserOptions;

pub struct ParserErrors<Token, Value, State> {
    phanotm_t: std::marker::PhantomData<*const Token>,
    phanotm_v: std::marker::PhantomData<*const Value>,
    phanotm_s: std::marker::PhantomData<*const State>,
}

impl<Token, Value, State> ParserErrors<Token, Value, State> {
    pub fn parse_errors(
        _tree: &SyntaxTree<Syntax<Token, Value>, State>,
        _parser_options: ParserOptions,
    ) -> Vec<SyntaxError> {
        vec![]
    }
}
