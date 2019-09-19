// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlpool_rust::{caml_raise, ocamlvalue::Ocamlvalue, utils::*};
use parser_rust::{
    lexer::Lexer, minimal_token::MinimalToken, minimal_trivia::MinimalTrivia,
    source_text::SourceText,
};
use rust_to_ocaml::{to_list, SerializationContext};
use syntax_tree::mode_parser::parse_mode;

use oxidized::relative_path::RelativePath;

#[macro_export]
macro_rules! parse {
    ($name:ident, $parser:ident, $syntax:ty) => {
        use ocamlpool_rust::caml_raise;
        use $name::{ocamlpool_enter, ocamlpool_leave};
        mod $name {
            use super::*;
            extern "C" {
                pub fn ocamlpool_enter();
                pub fn ocamlpool_leave();
            }

            use ocaml::Value;
            use ocamlpool_rust::{caml_raise, ocamlvalue::Ocamlvalue, utils::*};
            use parser_rust::{
                self as parser,
                lexer::Lexer,
                parser_env::ParserEnv,
                source_text::SourceText,
                stack_limit::StackLimit,
            };
            use rust_to_ocaml::{to_list, SerializationContext, ToOcaml};
            use syntax_tree::{mode_parser::parse_mode, SyntaxTree};
            use oxidized::relative_path::RelativePath;

            pub unsafe fn parse(ocaml_source_text : Value, opts : Value, mut l : Value) -> Value {
                    let ocaml_source_text_value = ocaml_source_text.0;

                    let is_experimental_mode = bool_field(&opts, 0);
                    let hhvm_compat_mode = bool_field(&opts, 1);
                    let php5_compat_mode = bool_field(&opts, 2);
                    let codegen = bool_field(&opts, 3);
                    let allow_new_attribute_syntax = bool_field(&opts, 4);
                    let leak_rust_tree = bool_field(&opts, 5);
                    let env = ParserEnv {
                        is_experimental_mode,
                        hhvm_compat_mode,
                        php5_compat_mode,
                        codegen,
                        allow_new_attribute_syntax,
                    };

                    // Note: Determining the current thread size cannot be done portably,
                    // therefore assume the worst (running on non-main thread with min size, 2MiB)
                    const KI: usize = 1024;
                    const MI: usize = KI * KI;
                    const MAX_STACK_SIZE: usize = 1024 * MI;
                    let mut stack_size = 2 * MI;
                    let mut default_stack_size_sufficient = true;
                    parser::stack_limit::init();
                    loop {
                        if stack_size > MAX_STACK_SIZE {
                            panic!("Rust FFI exceeded maximum allowed stack of {} KiB", MAX_STACK_SIZE / KI);
                        }

                        // Avoid eagerly wasting of space that will not be used in practice (WWW),
                        // but only for degenerate test cases (/test/{slow,quick}), by starting off
                        // with small stack (default thread) then fall back to bigger ones (custom thread).
                        // Since we're doubling the stack the time is: t + 2*t + 4*t + ...
                        // where the total parse time with unbounded stack is T=k*t, which is
                        // bounded by 2*T (much less in practice due to superlinear parsing time).
                        let next_stack_size = if default_stack_size_sufficient {
                            13 * MI // assume we need much more if default stack size isn't enough
                        } else { // exponential backoff to limit parsing time to at most twice as long
                            2 * stack_size
                        };
                        // Note: detect almost full stack by setting "slack" of 60% for StackLimit because
                        // Syntax::to_ocaml is deeply & mutually recursive and uses nearly 2.5x of stack
                        // TODO: rewrite to_ocaml iteratively & reduce it to "stack_size - MB" as in HHVM
                        // (https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h)
                        let relative_stack_size = stack_size - stack_size*6/10;
                        let content = str_field(&ocaml_source_text, 2);
                        let relative_path_raw = block_field(&ocaml_source_text, 0);

                        let env = env.clone();
                        let try_parse = move || {
                            let stack_limit = StackLimit::relative(relative_stack_size);
                            stack_limit.reset();
                            let stack_limit_ref = &stack_limit;
                            let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
                            let file_path = relative_path.path_str().to_owned();
                            ocamlpool_enter();
                            let maybe_l = std::panic::catch_unwind(move || {
                                let source_text = SourceText::make_with_raw(
                                    &relative_path,
                                    &content.data(),
                                    ocaml_source_text_value,
                                );
                                let mut parser = $parser::make(&source_text, env);
                                let root = parser.parse_script(Some(&stack_limit_ref));
                                let errors = parser.errors();
                                let state = parser.sc_state();

                                // traversing the parsed syntax tree uses about 1/3 of the stack
                                let context = SerializationContext::new(ocaml_source_text_value);
                                let ocaml_root = root.to_ocaml(&context);
                                let ocaml_errors = errors.ocamlvalue();
                                let ocaml_state = state.to_ocaml(&context);
                                let tree = if leak_rust_tree {
                                    let required_stack_size = if default_stack_size_sufficient {
                                        None
                                    } else {
                                        Some(stack_size)
                                    };
                                    let mode = parse_mode(&source_text);
                                    let tree = Box::new(SyntaxTree::build(&source_text, root, errors, mode, (), required_stack_size));
                                    Some(Box::leak(tree) as *const SyntaxTree<$syntax, ()> as usize)
                                } else {
                                    None
                                };
                                let ocaml_tree = tree.ocamlvalue();

                                let res = caml_tuple(&[
                                    ocaml_state,
                                    ocaml_root,
                                    ocaml_errors,
                                    ocaml_tree,
                                ]);
                                let l = ocaml::Value::new(res);
                                l
                            });
                            ocamlpool_leave();  // note: must run even if a panic occurs
                            match maybe_l {
                                Ok(l) => Some(l),
                                Err(_) if stack_limit.exceeded() => {
                                    // Not always printing warning here because this would fail some HHVM tests
                                    let istty = libc::isatty(libc::STDERR_FILENO as i32) != 0;
                                    if istty || std::env::var_os("HH_TEST_MODE").is_some() {
                                        eprintln!("[hrust] warning: parser exceeded stack of {} KiB on: {}",
                                                  stack_limit.get() / KI,
                                                  file_path,
                                        );
                                    }
                                    None
                                }
                                Err(msg) => panic!(msg),
                            }
                        };
                        stack_size = next_stack_size;

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
                    l
            }
        }

        caml_raise!($name, |ocaml_source_text, opts|, <l>, {
            l = $name::parse(ocaml_source_text, opts, l)
        } -> l);
    }
}

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml_raise!(rust_parse_mode, |ocaml_source_text|, <l>, {
    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make(&relative_path, &content.data());

    let mode = parse_mode(&source_text);

    ocamlpool_enter();
    let ocaml_mode = mode.ocamlvalue();
    l = ocaml::Value::new(ocaml_mode);
    ocamlpool_leave();
} -> l);

macro_rules! scan_trivia {
    ($name:ident) => {
        caml_raise!($name, |ocaml_source_text, offset|, <l>, {
            let relative_path_raw = block_field(&ocaml_source_text, 0);
            let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
            let content = str_field(&ocaml_source_text, 2);
            let source_text = SourceText::make(&relative_path, &content.data());

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
