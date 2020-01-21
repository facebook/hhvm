// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This is a version of [`positioned_parser`](../positioned_parser/).
//!
//! The "coroutine" part has the same meaning as in [`positioned_coroutine_parser`](../positioned_coroutine_parser/),
//! and `leak_tree` refers to its ability to leak the Rust syntax tree to OCaml runtime instead of dropping it at
//! the end of FFI call. This was used to pass things around while some parts of parser/lowerer/error checker
//! were still in OCaml while others were already ported to Rust.
//!
//! Now there is only one remaining usage of this feature in `hh_parse`, and this parser could be removed
//! after removing that callsite.

use coroutine_smart_constructors::{CoroutineSmartConstructors, State as CoroutineState};
use parser::{
    parser::Parser, parser_env::ParserEnv, positioned_syntax::PositionedSyntax,
    smart_constructors_wrappers::WithKind, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub type SmartConstructors<'a> = WithKind<
    CoroutineSmartConstructors<'a, PositionedSyntax, CoroutineState<'a, PositionedSyntax>>,
>;

pub type ScState<'a> = CoroutineState<'a, PositionedSyntax>;

type CoroutineParserLeakTree<'a> = Parser<'a, SmartConstructors<'a>, ScState<'a>>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (PositionedSyntax, Vec<SyntaxError>, ScState<'a>) {
    let mut parser = CoroutineParserLeakTree::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
