// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This is a version of [`positioned_parser`](../positioned_parser/) which parses directly into `OCamlSyntax` - it
//! directly allocates OCaml version of `PositionedSyntaxTree` in OCaml heap. Using it should be equivalent
//! to parsing with `positioned_parser` and calling `OcamlRep::to_ocaml` on resulting value (while bypassing the cost
//! of allocating intermediate Rust structure).
//!
//! The fancy sounding "coroutine" part refers to the fact that the parser tracks (and returns in its state) occurence
//! of coroutine related features in the file (`coroutine`, `susped` keywords, `__PPL` attribute) which is used
//! in codegen pipeline to decide whether file needs additional processing to handle those things.
//!
//! This parser could probably be removed now that majority of performance sensitive users go through
//! fully Rust [`aast_parser`](../aast_parser/) bypassing the OCaml CST completely.

use coroutine_smart_constructors::CoroutineSmartConstructors;
use ocaml_syntax::{ocaml_coroutine_state::OcamlCoroutineState, OcamlSyntax};
use parser::{
    parser::Parser, parser_env::ParserEnv, positioned_syntax::PositionedValue,
    smart_constructors::WithKind, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub type SmartConstructors<'a> = WithKind<
    CoroutineSmartConstructors<
        'a,
        OcamlSyntax<PositionedValue>,
        OcamlCoroutineState<'a, OcamlSyntax<PositionedValue>>,
    >,
>;

pub type ScState<'a> = OcamlCoroutineState<'a, OcamlSyntax<PositionedValue>>;

type CoroutineParser<'a> = Parser<'a, SmartConstructors<'a>, ScState<'a>>;

pub fn parse_script<'a>(
    source: &SourceText<'a>,
    env: ParserEnv,
    stack_limit: Option<&'a StackLimit>,
) -> (OcamlSyntax<PositionedValue>, Vec<SyntaxError>, ScState<'a>) {
    let mut parser = CoroutineParser::make(&source, env);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
