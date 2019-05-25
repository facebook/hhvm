// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// TODO(kasper): rustfmt is getting confused in this file because of macro definition,
// and @nolint was the only way I found to disable it

#[macro_use]
extern crate ocaml;
pub mod rust_to_ocaml;
use parser_rust as parser;

use parser_rust::file_mode::parse_mode;
use parser::minimal_parser::MinimalSyntaxParser;
use parser::minimal_token::MinimalToken;
use parser::minimal_trivia::MinimalTrivia;
use parser::lexer::Lexer;
use parser::parser_env::ParserEnv;

use parser::source_text::SourceText;

use rust_to_ocaml::{caml_tuple, to_list, SerializationContext, ToOcaml};

use parser::parser::Parser;
use parser::positioned_smart_constructors::*;
use parser::positioned_syntax::{PositionedSyntax, PositionedValue};
use parser::positioned_token::PositionedToken;
use parser::smart_constructors::{NoState, StateType};
use parser::smart_constructors_wrappers::WithKind;

use std::panic;

type PositionedSyntaxParser<'a> =
    Parser<'a, WithKind<PositionedSmartConstructors>, NoState>;

use parser::coroutine_smart_constructors::CoroutineSmartConstructors;
use parser::coroutine_smart_constructors::State as CoroutineState;

type CoroutineParser<'a> =
    Parser<'a, WithKind<CoroutineSmartConstructors<PositionedSyntax>>,
    <CoroutineState<PositionedSyntax> as StateType<'a, PositionedSyntax>>::T>;

use parser::decl_mode_smart_constructors::DeclModeSmartConstructors;
use parser::decl_mode_smart_constructors::State as DeclModeState;

type DeclModeParser<'a> =
    Parser<'a, WithKind<DeclModeSmartConstructors<PositionedToken, PositionedValue>>,
    <DeclModeState<PositionedSyntax> as StateType<'a, PositionedSyntax>>::T>;


extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

unsafe fn block_field(block: &ocaml::Value, field: usize) -> ocaml::Value {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field))
}

unsafe fn bool_field(block: &ocaml::Value, field: usize) -> bool {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field)).i32_val() != 0
}

unsafe fn str_field(block: &ocaml::Value, field: usize) -> ocaml::Str {
    ocaml::Str::from(ocaml::Value::new(*ocaml::core::mlvalues::field(
        block.0, field,
    )))
}

macro_rules! caml_raise {
    ($name:ident, |$($param:ident),*|, <$($local:ident),*>, $code:block -> $retval:ident) => {
        caml!($name, |$($param),*|, <caml_raise_ret, $($local),*>, {
            let result = panic::catch_unwind(
                || {
                    $code;
                    return $retval;
                }
            );
            match result {
                Ok (value) => {
                    caml_raise_ret = value;
                },
                Err (err) => {
                    let msg: &str;
                    if let Some (str) = err.downcast_ref::<&str>() {
                        msg = str;
                    } else if let Some (string) = err.downcast_ref::<String>() {
                        msg = &string[..];
                    } else {
                        msg = "Unknown panic type, only support string type.";
                    }
                    ocaml::runtime::raise_with_string(
                        &ocaml::named_value("rust exception").unwrap(),
                        msg,
                    );
                },
            };
        } -> caml_raise_ret);
    };
}

macro_rules! parse {
    ($name:ident, $parser:ident) => {
        caml_raise!($name, |ocaml_source_text, opts|, <l>, {
            let relative_path = block_field(&ocaml_source_text, 0);
            let file_path = str_field(&relative_path, 1);
            let content = str_field(&ocaml_source_text, 2);
            let source_text = SourceText::make(&file_path.as_str(), &content.data());

            let is_experimental_mode = bool_field(&opts, 0);
            let disable_unsafe_expr = bool_field(&opts, 1);
            let disable_unsafe_block = bool_field(&opts, 2);
            let force_hh = bool_field(&opts, 3);
            let enable_xhp = bool_field(&opts, 4);
            let hhvm_compat_mode = bool_field(&opts, 5);
            let php5_compat_mode = bool_field(&opts, 6);
            let codegen = bool_field(&opts, 7);

            let env = ParserEnv {
                is_experimental_mode,
                disable_unsafe_expr,
                disable_unsafe_block,
                force_hh,
                enable_xhp,
                hhvm_compat_mode,
                php5_compat_mode,
                codegen,
            };
            let mut parser = $parser::make(&source_text, env);
            let root = parser.parse_script();
            let errors = parser.errors();
            ocamlpool_enter();

            let context = SerializationContext::new(ocaml_source_text.0);
            let ocaml_root = root.to_ocaml(&context);
            let ocaml_errors = to_list(&errors, &context);
            let ocaml_state = parser.sc_state().to_ocaml(&context);

            let res = caml_tuple(&[
                ocaml_state,
                ocaml_root,
                ocaml_errors
            ]);
            l = ocaml::Value::new(res);
            ocamlpool_leave();
        } -> l);
    };
}

parse!(parse_minimal, MinimalSyntaxParser);
parse!(parse_positioned, PositionedSyntaxParser);
parse!(parse_positioned_with_coroutine_sc, CoroutineParser);
parse!(parse_positioned_with_decl_mode_sc, DeclModeParser);

caml_raise!(rust_parse_mode, |ocaml_source_text|, <l>, {
    let relative_path = block_field(&ocaml_source_text, 0);
    let file_path = str_field(&relative_path, 1);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make(&file_path.as_str(), &content.data());

    let mode = parse_mode(&source_text);

    ocamlpool_enter();
    let context = SerializationContext::new(ocaml_source_text.0);
    let ocaml_mode = mode.to_ocaml(&context);
    l = ocaml::Value::new(ocaml_mode);
    ocamlpool_leave();
} -> l);

macro_rules! scan_trivia {
    ($name:ident) => {
        caml_raise!($name, |ocaml_source_text, opts, offset|, <l>, {
            let relative_path = block_field(&ocaml_source_text, 0);
            let file_path = str_field(&relative_path, 1);
            let content = str_field(&ocaml_source_text, 2);
            let source_text = SourceText::make(&file_path.as_str(), &content.data());

            let offset = offset.usize_val();

            let is_experimental_mode = false;
            let force_hh = bool_field(&opts, 0);
            let enable_xhp = bool_field(&opts, 1);
            let disable_unsafe_expr = bool_field(&opts, 2);
            let disable_unsafe_block = bool_field(&opts, 3);

            let mut lexer : Lexer<MinimalToken> = Lexer::make_at(
                &source_text,
                is_experimental_mode,
                disable_unsafe_expr,
                disable_unsafe_block,
                force_hh,
                enable_xhp,
                offset,
            );

            let res : Vec<MinimalTrivia> = lexer.$name();

            ocamlpool_enter();
            let context = SerializationContext::new(ocaml_source_text.0);
            let trivia_list = to_list(&res, &context);
            l = ocaml::Value::new(trivia_list);
            ocamlpool_leave();
        } -> l);
    };
}

scan_trivia!(scan_leading_xhp_trivia);
scan_trivia!(scan_trailing_xhp_trivia);
scan_trivia!(scan_leading_php_trivia);
scan_trivia!(scan_trailing_php_trivia);
