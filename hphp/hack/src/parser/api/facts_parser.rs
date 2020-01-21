// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use facts_smart_constructors::{FactsSmartConstructors, HasScriptContent, Node};
use parser::{parser::Parser, WithKind};
use parser_core_types::{
    parser_env::ParserEnv, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

type FactsParser<'a> = Parser<'a, WithKind<FactsSmartConstructors<'a>>, HasScriptContent<'a>>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (Node, Vec<SyntaxError>, HasScriptContent<'a>) {
    let mut parser = FactsParser::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
