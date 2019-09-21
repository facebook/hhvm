// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_rust as parser;

use ocamlpool_rust::{
    caml_raise,
    ocamlvalue::*,
    utils::{block_field, str_field},
};
use oxidized::{file_info, global_options::GlobalOptions, relative_path::RelativePath};
use parser::{
    indexed_source_text::IndexedSourceText,
    parser::Parser,
    parser_env::ParserEnv,
    positioned_syntax::PositionedValue,
    positioned_token::PositionedToken,
    smart_constructors::{NoState, WithKind},
    source_text::SourceText,
};
use positioned_parser::positioned_smart_constructors::PositionedSmartConstructors;
use syntax_tree::mode_parser::parse_mode;

use lowerer::{Env as LowererEnv, Lowerer};
use std::default::Default;

type PositionedSyntaxParser<'a> = Parser<'a, WithKind<PositionedSmartConstructors>, NoState>;
struct PositionedSyntaxLowerer {}
impl<'a> Lowerer<'a, PositionedToken, PositionedValue> for PositionedSyntaxLowerer {}

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml_raise!(parse_and_lower_from_text, |ocaml_source_text|, <res>, {
    let ocaml_source_text_value = ocaml_source_text.0;

    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make_with_raw(&relative_path, &content.data(), ocaml_source_text_value);
    let indexed_source_text = IndexedSourceText::new(&source_text);
    let mode = parse_mode(&source_text).unwrap_or(file_info::Mode::Mpartial);

    let env = ParserEnv {
        is_experimental_mode : mode == file_info::Mode::Mexperimental,
        hhvm_compat_mode : false,
        php5_compat_mode : false,
        codegen : false,
        allow_new_attribute_syntax : false,
    };

    let mut parser = PositionedSyntaxParser::make(&source_text, env);
    let script = parser.parse_script(None);
    let parser_options = GlobalOptions::default();
    let mut env = LowererEnv::make(
        false, /* codegen */
        false, /* elaborate namespace */
        false, /* quick mode */
        mode,
        &indexed_source_text,
        &parser_options,
    );

    ocamlpool_enter();
    let r = PositionedSyntaxLowerer::lower(&mut env, &script).ocamlvalue();
    ocamlpool_leave();
    res = ocaml::Value::new(r);
} -> res );
