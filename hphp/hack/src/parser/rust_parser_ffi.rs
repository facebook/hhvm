// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate ocaml;

extern crate libc;

pub mod rust_to_ocaml;
use parser_rust as parser;

use parser::file_mode::parse_mode;
use parser::lexer::Lexer;
use parser::minimal_parser::MinimalSyntaxParser;
use parser::minimal_token::MinimalToken;
use parser::minimal_trivia::MinimalTrivia;
use parser::parser_env::ParserEnv;
use parser::source_text::SourceText;
use parser::stack_limit::StackLimit;

use rust_to_ocaml::{caml_tuple, to_list, SerializationContext, ToOcaml};

use parser::parser::Parser;
use parser::positioned_smart_constructors::*;
use parser::positioned_syntax::{PositionedSyntax, PositionedValue};
use parser::positioned_token::PositionedToken;
use parser::smart_constructors::{NoState, StateType};
use parser::smart_constructors_wrappers::WithKind;

type PositionedSyntaxParser<'a> = Parser<'a, WithKind<PositionedSmartConstructors>, NoState>;

use parser::coroutine_smart_constructors::CoroutineSmartConstructors;
use parser::coroutine_smart_constructors::State as CoroutineState;

type CoroutineParser<'a> = Parser<
    'a,
    WithKind<CoroutineSmartConstructors<PositionedSyntax>>,
    <CoroutineState<PositionedSyntax> as StateType<'a, PositionedSyntax>>::T,
>;

use parser::decl_mode_smart_constructors::DeclModeSmartConstructors;
use parser::decl_mode_smart_constructors::State as DeclModeState;

type DeclModeParser<'a> = Parser<
    'a,
    WithKind<DeclModeSmartConstructors<PositionedToken, PositionedValue>>,
    <DeclModeState<PositionedSyntax> as StateType<'a, PositionedSyntax>>::T,
>;

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
            let result = std::panic::catch_unwind(
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
            let ocaml_source_text_value = ocaml_source_text.0;

            let is_experimental_mode = bool_field(&opts, 0);
            let hhvm_compat_mode = bool_field(&opts, 1);
            let php5_compat_mode = bool_field(&opts, 2);
            let codegen = bool_field(&opts, 3);
            let env = ParserEnv {
                is_experimental_mode,
                hhvm_compat_mode,
                php5_compat_mode,
                codegen,
            };

            // Note: Determining the current thread size cannot be done portably,
            // therefore assume the worst (running on non-main thread with min size, 2MiB)
            const MIB: usize = 1024 * 1024;
            const MAX_STACK_SIZE: usize = 300 * MIB;
            let mut stack_size = 2 * MIB;
            let mut default_stack_size_sufficient = true;
            while stack_size <= MAX_STACK_SIZE {
                // Avoid eagerly wasting of space that will not be used in practice (WWW),
                // but only for degenerate test cases (/test/{slow,quick}), by starting off
                // with small stack (default thread) then fall back to bigger ones (custom thread).
                // Since we're doubling the stack the time is: t + 2*t + 4*t + ...
                // where the total parse time with unbounded stack is T=k*t, which is
                // bounded by 2*T (much less in practice due to superlinear parsing time).
                let next_stack_size = if default_stack_size_sufficient {
                    13 * MIB // assume we need much more if default stack size isn't enough
                } else { // exponential backoff to limit parsing time to at most twice as long
                    2 * stack_size
                };
                // Note: detect almost full stack by setting wiggle room of 1/2 MiB for StackLimit
                // (if this isn't enough then the granularity of StackLimit checks is insufficient)
                let relative_stack_size = stack_size - MIB*1/2;
                stack_size = next_stack_size;

                let relative_path = block_field(&ocaml_source_text, 0);
                let file_path = str_field(&relative_path, 1);
                let content = str_field(&ocaml_source_text, 2);
                let env = env.clone();

                let try_parse = move || {
                    let stack_limit = std::rc::Rc::new(StackLimit::relative(relative_stack_size));
                    let source_text = SourceText::make(&file_path.as_str(), &content.data());
                    let mut parser = $parser::make(&source_text, env);
                    let root = parser.parse_script(Some(stack_limit.clone()));
                    let errors = parser.errors();
                    let state = parser.sc_state();
                    if (*stack_limit).exceeded() {
                        // Not always printing warning here because this would fail some HHVM tests
                        let istty = libc::isatty(libc::STDERR_FILENO as i32) != 0;
                        if istty || std::env::var_os("HH_TEST_MODE").is_some() {
                            eprintln!("[hrust] warning: parser exceeded stack of {} MiB on: {}",
                                      (*stack_limit).get() / MIB,
                                      file_path.as_str(),
                            );
                        }
                        None
                    } else {
                        ocamlpool_enter();

                        // traversing the parsed syntax tree uses about 1/3 of the stack
                        let context = SerializationContext::new(ocaml_source_text_value);
                        let ocaml_root = root.to_ocaml(&context);
                        let ocaml_errors = to_list(&errors, &context);
                        let ocaml_state = state.to_ocaml(&context);
                        let res = caml_tuple(&[
                            ocaml_state,
                            ocaml_root,
                            ocaml_errors
                        ]);
                        let l = ocaml::Value::new(res);
                        ocamlpool_leave();
                        Some(l)
                    }
                };

                let l_opt = if default_stack_size_sufficient {
                    try_parse()
                } else {
                    std::thread::Builder::new().stack_size(stack_size).spawn(try_parse)
                        .expect("ERROR: thread::spawn")
                        .join().expect("ERROR: failed to wait on new thread")
                };

                match l_opt {
                    Some(ocaml_result) => {
                        l = ocaml_result;
                        break;
                    },
                    _ => default_stack_size_sufficient = false,
                }
            }
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
        caml_raise!($name, |ocaml_source_text, offset|, <l>, {
            let relative_path = block_field(&ocaml_source_text, 0);
            let file_path = str_field(&relative_path, 1);
            let content = str_field(&ocaml_source_text, 2);
            let source_text = SourceText::make(&file_path.as_str(), &content.data());

            let offset = offset.usize_val();

            let is_experimental_mode = false;

            let mut lexer : Lexer<MinimalToken> = Lexer::make_at(
                &source_text,
                is_experimental_mode,
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
