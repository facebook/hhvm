// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{ptr::UnsafeOcamlPtr, Allocator, OcamlRep};
use ocamlrep_ocamlpool::{ocaml_ffi, Pool};
use oxidized::{file_info, full_fidelity_parser_env::FullFidelityParserEnv};
use parser::{
    self,
    minimal_trivia::MinimalTrivia,
    operator::{Assoc, Operator},
    parser::Parser,
    parser_env::ParserEnv,
    smart_constructors::{NodeType, SmartConstructors},
    source_text::SourceText,
    token_kind::TokenKind,
};
use rust_to_ocaml::{SerializationContext, ToOcaml};
use stack_limit::{StackLimit, KI, MI};
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
    let ocaml_source_text = ocaml_source_text.as_usize();

    let leak_rust_tree = env.leak_rust_tree;
    let env = ParserEnv::from(env);

    let make_retryable = move || {
        let env = env.clone();
        Box::new(
            move |stack_limit: &StackLimit, nonmain_stack_size: Option<usize>| {
                // Safety: Requires no concurrent interaction with OCaml runtime
                // from other threads.
                let pool = unsafe { Pool::new() };

                // Safety: the parser asks for a stack limit with the same lifetime
                // as the source text, but no syntax tree borrows the stack limit,
                // so we really only need it to live as long as the parser.
                // Transmute away its lifetime to satisfy the parser API.
                let stack_limit_ref: &'a StackLimit = unsafe { std::mem::transmute(stack_limit) };

                // We only convert the source text from OCaml in this innermost
                // closure because it contains an Rc. If we converted it
                // earlier, we'd need to pass it across an unwind boundary or
                // send it between threads, but it has internal mutablility and
                // is not Send.
                let source_text = unsafe { SourceText::from_ocaml(ocaml_source_text).unwrap() };
                let mut parser = <Parser<'a, Sc, ScState>>::make(&source_text, env);
                let root = parser.parse_script(Some(&stack_limit_ref));
                let errors = parser.errors();
                let state = parser.sc_state();

                // traversing the parsed syntax tree uses about 1/3 of the stack
                let context = SerializationContext::new(ocaml_source_text);
                let ocaml_root = unsafe { root.to_ocaml(&context) };
                let ocaml_errors = pool.add(&errors);
                let ocaml_state = unsafe { state.to_ocaml(&context) };
                let tree = if leak_rust_tree {
                    let mode = parse_mode(&source_text);
                    let tree = Box::new(SyntaxTree::build(
                        &source_text,
                        root,
                        errors,
                        mode,
                        (),
                        nonmain_stack_size,
                    ));
                    Some(Box::leak(tree) as *const SyntaxTree<_, ()> as usize)
                } else {
                    None
                };
                let ocaml_tree = pool.add(&tree);

                let mut res = pool.block_with_size(4);
                Pool::set_field(&mut res, 0, unsafe {
                    ocamlrep::Value::from_bits(ocaml_state)
                });
                Pool::set_field(&mut res, 1, unsafe {
                    ocamlrep::Value::from_bits(ocaml_root)
                });
                Pool::set_field(&mut res, 2, ocaml_errors);
                Pool::set_field(&mut res, 3, ocaml_tree);
                // Safety: The UnsafeOcamlPtr must point to the first field in
                // the block. It must be handed back to OCaml before the garbage
                // collector is given an opportunity to run.
                unsafe { UnsafeOcamlPtr::new(res.build().to_bits()) }
            },
        )
    };

    fn stack_slack_for_traversal_and_parsing(stack_size: usize) -> usize {
        // Syntax::to_ocaml is deeply & mutually recursive and uses nearly 2.5x of stack
        // TODO: rewrite to_ocaml iteratively & reduce it to "stack_size - MB" as in HHVM
        // (https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h)
        stack_size * 6 / 10
    }

    const MAX_STACK_SIZE: usize = 1024 * MI;

    let on_retry = &mut |stack_size_tried: usize| {
        // Not always printing warning here because this would fail some HHVM tests
        let istty = unsafe { libc::isatty(libc::STDERR_FILENO as i32) != 0 };
        if istty || std::env::var_os("HH_TEST_MODE").is_some() {
            let source_text = unsafe { SourceText::from_ocaml(ocaml_source_text).unwrap() };
            let file_path = source_text.file_path().path_str();
            eprintln!(
                "[hrust] warning: parser exceeded stack of {} KiB on: {}",
                (stack_size_tried - stack_slack_for_traversal_and_parsing(stack_size_tried)) / KI,
                file_path,
            );
        }
    };

    use stack_limit::retry;
    let job = retry::Job {
        nonmain_stack_min: 13 * MI, // assume we need much more if default stack size isn't enough
        nonmain_stack_max: Some(MAX_STACK_SIZE),
        ..Default::default()
    };

    match job.with_elastic_stack(
        &make_retryable,
        on_retry,
        stack_slack_for_traversal_and_parsing,
    ) {
        Ok(ocaml_result) => ocaml_result,
        Err(failure) => {
            panic!(
                "Rust parser FFI exceeded maximum allowed stack of {} KiB",
                failure.max_stack_size_tried / KI
            );
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

ocaml_ffi! {
    fn rust_parse_mode(source_text: SourceText) -> Option<file_info::Mode> {
        parse_mode(&source_text)
    }

    fn scan_leading_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        minimal_parser::scan_leading_xhp_trivia(source_text, offset)
    }
    fn scan_trailing_xhp_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        minimal_parser::scan_trailing_xhp_trivia(source_text, offset)
    }
    fn scan_leading_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        minimal_parser::scan_leading_php_trivia(source_text, offset)
    }
    fn scan_trailing_php_trivia(source_text: SourceText, offset: usize) -> Vec<MinimalTrivia> {
        minimal_parser::scan_trailing_php_trivia(source_text, offset)
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
        use parser::parser_env::ParserEnv;
        op.precedence(&ParserEnv::default())
    }

    fn rust_precedence_for_assignment_in_expressions_helper() -> usize {
        Operator::precedence_for_assignment_in_expressions()
    }

    fn rust_associativity_helper(op: Operator) -> Assoc {
        // NOTE: ParserEnv is not used in operator::associativity(), so we just create an empty ParserEnv
        // If operator::associativity() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser::parser_env::ParserEnv;
        op.associativity(&ParserEnv::default())
    }
}
