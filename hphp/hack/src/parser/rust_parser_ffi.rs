/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#[macro_use]
extern crate ocaml;
pub mod rust_to_ocaml;
use parser_rust as parser;

use parser::minimal_syntax::MinimalValue;
use parser::minimal_token::MinimalToken;
use parser::parser::Parser;
use parser::parser_env::ParserEnv;
use parser::source_text::SourceText;
use parser::syntax_smart_constructors::SyntaxSmartConstructors;
use rust_to_ocaml::{SerializationContext, ToOcaml};

type MinimalSyntaxParser<'a> = Parser<'a, SyntaxSmartConstructors<MinimalToken, MinimalValue>>;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

unsafe fn bool_field(block: &ocaml::Value, field: usize) -> bool {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field)).i32_val() != 0
}

caml!(parse_minimal, |ocaml_source_text, opts|, <l>, {
    let content = ocaml::Str::from(
        ocaml::Value::new(*ocaml::core::mlvalues::field(ocaml_source_text.0, 2))
    );
    let data = content.data();
    let source_text = SourceText::make(&data);

    let is_experimental_mode = bool_field(&opts, 0);
    let enable_stronger_await_binding = bool_field(&opts, 1);
    let disable_unsafe_expr = bool_field(&opts, 2);
    let disable_unsafe_block = bool_field(&opts, 3);
    let force_hh = bool_field(&opts, 4);
    let enable_xhp = bool_field(&opts, 5);
    let hhvm_compat_mode = bool_field(&opts, 6);
    let php5_compat_mode = bool_field(&opts, 7);

    let env = ParserEnv {
        is_experimental_mode,
        enable_stronger_await_binding,
        disable_unsafe_expr,
        disable_unsafe_block,
        force_hh,
        enable_xhp,
        hhvm_compat_mode,
        php5_compat_mode,
    };

    let mut parser = MinimalSyntaxParser::make(&source_text, env);
    let root = parser.parse_script();
    ocamlpool_enter();

    let context = SerializationContext::new();
    let ocaml_root = root.to_ocaml(&context);
    l = ocaml::Value::new(ocaml_root);

    ocamlpool_leave();
} -> l);
