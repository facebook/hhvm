// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{ptr::UnsafeOcamlPtr, Allocator, OcamlRep};
use ocamlrep_ocamlpool::{ocaml_ffi, Pool};
use oxidized::{file_info, full_fidelity_parser_env::FullFidelityParserEnv};
use parser_rust::{
    self,
    lexer::Lexer,
    minimal_token::MinimalToken,
    minimal_trivia::MinimalTrivia,
    operator::{Assoc, Operator},
    parser::Parser,
    parser_env::ParserEnv,
    smart_constructors::{NodeType, SmartConstructors},
    source_text::SourceText,
    stack_limit::StackLimit,
    token_kind::TokenKind,
};
use rust_to_ocaml::{SerializationContext, ToOcaml};
use syntax_tree::{mode_parser::parse_mode, SyntaxTree};

pub fn parse<'a, Sc, ScState>(
    ocaml_source_text: UnsafeOcamlPtr,
    env: FullFidelityParserEnv,
) -> UnsafeOcamlPtr
where
    Sc: SmartConstructors<'a, ScState>,
    Sc::R: NodeType,
    <Sc::R as NodeType>::R: ToOcaml,
    ScState: Clone + ToOcaml,
{
    let leak_rust_tree = env.leak_rust_tree;
    let env = ParserEnv::from(env);

    // Note: Determining the current thread size cannot be done portably,
    // therefore assume the worst (running on non-main thread with min size, 2MiB)
    const KI: usize = 1024;
    const MI: usize = KI * KI;
    const MAX_STACK_SIZE: usize = 1024 * MI;
    let mut stack_size = 2 * MI;
    let mut default_stack_size_sufficient = true;
    parser_rust::stack_limit::init();
    loop {
        if stack_size > MAX_STACK_SIZE {
            panic!(
                "Rust FFI exceeded maximum allowed stack of {} KiB",
                MAX_STACK_SIZE / KI
            );
        }

        // Avoid eagerly wasting of space that will not be used in practice (WWW),
        // but only for degenerate test cases (/test/{slow,quick}), by starting off
        // with small stack (default thread) then fall back to bigger ones (custom thread).
        // Since we're doubling the stack the time is: t + 2*t + 4*t + ...
        // where the total parse time with unbounded stack is T=k*t, which is
        // bounded by 2*T (much less in practice due to superlinear parsing time).
        let next_stack_size = if default_stack_size_sufficient {
            13 * MI // assume we need much more if default stack size isn't enough
        } else {
            // exponential backoff to limit parsing time to at most twice as long
            2 * stack_size
        };
        // Note: detect almost full stack by setting "slack" of 60% for StackLimit because
        // Syntax::to_ocaml is deeply & mutually recursive and uses nearly 2.5x of stack
        // TODO: rewrite to_ocaml iteratively & reduce it to "stack_size - MB" as in HHVM
        // (https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h)
        let relative_stack_size = stack_size - stack_size * 6 / 10;

        let env = env.clone();
        let try_parse = move || {
            let stack_limit = StackLimit::relative(relative_stack_size);
            stack_limit.reset();
            // Safety: the parser asks for a stack limit with the same lifetime
            // as the source text, but no syntax tree borrows the stack limit,
            // so we really only need it to live as long as the parser.
            // Transmute away its lifetime to satisfy the parser API.
            let stack_limit_ref: &'a StackLimit = unsafe { std::mem::transmute(&stack_limit) };
            let mut pool = Pool::new();
            let parse_result = std::panic::catch_unwind(move || {
                // We only convert the source text from OCaml in this innermost
                // closure because it contains an Rc. If we converted it
                // earlier, we'd need to pass it across an unwind boundary or
                // send it between threads, but it has internal mutablility and
                // is not Send.
                let source_text =
                    unsafe { SourceText::from_ocaml(ocaml_source_text.as_usize()).unwrap() };
                let mut parser = <Parser<'a, Sc, ScState>>::make(&source_text, env);
                let root = parser.parse_script(Some(&stack_limit_ref));
                let errors = parser.errors();
                let state = parser.sc_state();

                // traversing the parsed syntax tree uses about 1/3 of the stack
                let context = SerializationContext::new(ocaml_source_text.as_usize());
                let ocaml_root = unsafe { root.to_ocaml(&context) };
                let ocaml_errors = pool.add(&errors);
                let ocaml_state = unsafe { state.to_ocaml(&context) };
                let tree = if leak_rust_tree {
                    let required_stack_size = if default_stack_size_sufficient {
                        None
                    } else {
                        Some(stack_size)
                    };
                    let mode = parse_mode(&source_text);
                    let tree = Box::new(SyntaxTree::build(
                        &source_text,
                        root,
                        errors,
                        mode,
                        (),
                        required_stack_size,
                    ));
                    Some(Box::leak(tree) as *const SyntaxTree<_, ()> as usize)
                } else {
                    None
                };
                let ocaml_tree = pool.add(&tree);

                // Safety: We only invoke set_field with `res`, and only with
                // indices less than the size we gave. The UnsafeOcamlPtr must
                // point to the first field in the block. It must be handed back
                // to OCaml before the garbage collector is given an opportunity
                // to run.
                let res = pool.block_with_size(4);
                unsafe {
                    Pool::set_field(res, 0, ocamlrep::Value::from_bits(ocaml_state));
                    Pool::set_field(res, 1, ocamlrep::Value::from_bits(ocaml_root));
                    Pool::set_field(res, 2, ocaml_errors);
                    Pool::set_field(res, 3, ocaml_tree);
                    UnsafeOcamlPtr::new(res as usize)
                }
            });
            match parse_result {
                Ok(result) => Some(result),
                Err(_) if stack_limit.exceeded() => {
                    // Not always printing warning here because this would fail some HHVM tests
                    let istty = unsafe { libc::isatty(libc::STDERR_FILENO as i32) != 0 };
                    if istty || std::env::var_os("HH_TEST_MODE").is_some() {
                        let source_text = unsafe {
                            SourceText::from_ocaml(ocaml_source_text.as_usize()).unwrap()
                        };
                        let file_path = source_text.file_path().path_str();
                        eprintln!(
                            "[hrust] warning: parser exceeded stack of {} KiB on: {}",
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

        let result_opt = if default_stack_size_sufficient {
            try_parse()
        } else {
            std::thread::Builder::new()
                .stack_size(stack_size)
                .spawn(try_parse)
                .expect("ERROR: thread::spawn")
                .join()
                .expect("ERROR: failed to wait on new thread")
        };

        match result_opt {
            Some(ocaml_result) => return ocaml_result,
            _ => default_stack_size_sufficient = false,
        }
    }
}

#[macro_export]
macro_rules! parse {
    ($name:ident, $sc:ty, $scstate:ty $(,)?) => {
        // We don't use the ocaml_ffi! macro here because we want precise
        // control over the Pool--when a parse fails, we want to free the old
        // pool and create a new one.
        #[no_mangle]
        pub extern "C" fn $name(ocaml_source_text: usize, env: usize) -> usize {
            ocamlrep_ocamlpool::catch_unwind(|| {
                use ocamlrep::{ptr::UnsafeOcamlPtr, OcamlRep};
                use oxidized::full_fidelity_parser_env::FullFidelityParserEnv;
                let ocaml_source_text = unsafe { UnsafeOcamlPtr::new(ocaml_source_text) };
                let env = unsafe { FullFidelityParserEnv::from_ocaml(env).unwrap() };
                $crate::parse::<'_, $sc, $scstate>(ocaml_source_text, env).as_usize()
            })
        }
    };
}

fn trivia_lexer<'a>(source_text: SourceText<'a>, offset: usize) -> Lexer<'a, MinimalToken> {
    let is_experimental_mode = false;
    Lexer::make_at(&source_text, is_experimental_mode, offset)
}

ocaml_ffi! {
    fn rust_parse_mode(source_text: SourceText) -> Option<file_info::Mode> {
        parse_mode(&source_text)
    }

    fn scan_leading_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_leading_xhp_trivia()
    }
    fn scan_trailing_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_trailing_xhp_trivia()
    }
    fn scan_leading_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_leading_php_trivia()
    }
    fn scan_trailing_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        trivia_lexer(source_text, offset).scan_trailing_php_trivia()
    }

    fn trailing_from_token(token: TokenKind) -> Operator {
        Operator::trailing_from_token(token)
    }
    fn prefix_unary_from_token(token: TokenKind) -> Operator {
        Operator::prefix_unary_from_token(token)
    }

    fn is_trailing_operator_token(token: TokenKind) -> bool {
        Operator::is_trailing_operator_token(token)
    }
    fn is_binary_operator_token(token: TokenKind) -> bool {
        Operator::is_binary_operator_token(token)
    }

    fn is_comparison(op: Operator) -> bool {
        op.is_comparison()
    }
    fn is_assignment(op: Operator) -> bool {
        op.is_assignment()
    }

    fn rust_precedence_helper(op: Operator) -> usize {
        // NOTE: ParserEnv is not used in operator::precedence(), so we just create an empty ParserEnv
        // If operator::precedence() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser_rust::parser_env::ParserEnv;
        op.precedence(&ParserEnv::default())
    }

    fn rust_precedence_for_assignment_in_expressions_helper() -> usize {
        Operator::precedence_for_assignment_in_expressions()
    }

    fn rust_associativity_helper(op: Operator) -> Assoc {
        // NOTE: ParserEnv is not used in operator::associativity(), so we just create an empty ParserEnv
        // If operator::associativity() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser_rust::parser_env::ParserEnv;
        op.associativity(&ParserEnv::default())
    }
}
