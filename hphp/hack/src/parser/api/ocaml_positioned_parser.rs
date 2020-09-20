// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml_syntax::{OcamlContextState, OcamlSyntax};
use parser::{
    parser::Parser, parser_env::ParserEnv, positioned_syntax::PositionedValue,
    smart_constructors_wrappers::WithKind, source_text::SourceText, syntax_error::SyntaxError,
};
use positioned_smart_constructors::*;
use stack_limit::StackLimit;
use syntax_smart_constructors::StateType;

pub type SmartConstructors<'src> = WithKind<
    PositionedSmartConstructors<'src, OcamlSyntax<PositionedValue>, OcamlContextState<'src>>,
>;

pub type ScState<'src> = OcamlContextState<'src>;

pub fn parse_script<'src>(
    source: &SourceText<'src>,
    env: ParserEnv,
    stack_limit: Option<&'src StackLimit>,
) -> (
    OcamlSyntax<PositionedValue>,
    Vec<SyntaxError>,
    ScState<'src>,
) {
    let sc = WithKind::new(PositionedSmartConstructors::new(
        <OcamlContextState as StateType<OcamlSyntax<PositionedValue>>>::initial(&env, source),
    ));
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state)
}
