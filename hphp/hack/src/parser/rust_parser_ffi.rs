// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use mode_parser::parse_mode;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::Allocator;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::to_ocaml;
use ocamlrep_ocamlpool::Pool;
use operator::Assoc;
use operator::Operator;
use oxidized::file_info;
use oxidized::full_fidelity_parser_env::FullFidelityParserEnv;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_by_ref::positioned_trivia::PositionedTrivia;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_tree::SyntaxTree;
use parser_core_types::token_kind::TokenKind;
use to_ocaml_impl::*;

pub fn parse<'a, ParseFn, Node, State>(
    ocaml_source_text_ptr: UnsafeOcamlPtr,
    env: FullFidelityParserEnv,
    parse_fn: ParseFn,
) -> UnsafeOcamlPtr
where
    ParseFn: Fn(&'a Bump, &SourceText<'a>, ParserEnv) -> (Node, Vec<SyntaxError>, State)
        + Clone
        + Send
        + Sync
        + std::panic::UnwindSafe
        + std::panic::RefUnwindSafe
        + 'static,
    Node: ToOcaml + 'a,
    State: ToOcamlRep + 'a,
{
    let ocaml_source_text = ocaml_source_text_ptr.as_usize();

    let leak_rust_tree = env.leak_rust_tree;
    let env = ParserEnv::from(env);

    // Safety: Requires no concurrent interaction with OCaml runtime
    // from other threads.
    let pool = unsafe { Pool::new() };
    let arena = Bump::new();

    // Safety: Similarly, the arena just needs to outlive the returned
    // Node and State (which may reference it). We ensure this by
    // not destroying the arena until after converting the node and
    // state to OCaml values.
    let arena_ref: &'a Bump = unsafe { (&arena as *const Bump).as_ref().unwrap() };

    // We only convert the source text from OCaml in this innermost
    // closure because it contains an Rc. If we converted it
    // earlier, we'd need to pass it across an unwind boundary or
    // send it between threads, but it has internal mutablility and
    // is not Send.
    let source_text = unsafe { SourceText::from_ocaml(ocaml_source_text).unwrap() };
    let (root, errors, state) = parse_fn(arena_ref, &source_text, env);
    // traversing the parsed syntax tree uses about 1/3 of the stack

    let ocaml_root = root.to_ocaml(&pool, ocaml_source_text_ptr).to_bits();
    let ocaml_errors = pool.add(&errors).to_bits();
    let ocaml_state = pool.add(&state).to_bits();
    let tree = if leak_rust_tree {
        let (_, mode) = parse_mode(&source_text);
        let tree = Box::new(SyntaxTree::build(&source_text, root, errors, mode, ()));
        // A rust pointer of (&SyntaxTree, &Arena) is passed to Ocaml,
        // Ocaml will pass it back to `rust_parser_errors::rust_parser_errors_positioned`
        // PLEASE ENSURE TYPE SAFETY MANUALLY!!!
        let tree = Box::leak(tree) as *const SyntaxTree<'_, _, ()> as usize;
        let arena = Box::leak(Box::new(arena)) as *const Bump as usize;
        Some(Box::leak(Box::new((tree, arena))) as *const (usize, usize) as usize)
    } else {
        None
    };
    let ocaml_tree = pool.add(&tree);

    let mut res = pool.block_with_size(4);
    // SAFETY: The to_bits/from_bits dance works around a lifetime issue:
    // we're not allowed to drop `root`, `errors`, or `state` while the
    // `Pool` is in scope (because otherwise, its memoization behavior would
    // work incorrectly in `ocamlrep::Allocator::add_root`). We are moving
    // those values, but since we're not using `add_root` here, it should be
    // okay.
    pool.set_field(&mut res, 0, unsafe {
        ocamlrep::OpaqueValue::from_bits(ocaml_state)
    });
    pool.set_field(&mut res, 1, unsafe {
        ocamlrep::OpaqueValue::from_bits(ocaml_root)
    });
    pool.set_field(&mut res, 2, unsafe {
        ocamlrep::OpaqueValue::from_bits(ocaml_errors)
    });
    pool.set_field(&mut res, 3, ocaml_tree);
    // Safety: The UnsafeOcamlPtr must point to the first field in
    // the block. It must be handed back to OCaml before the garbage
    // collector is given an opportunity to run.
    unsafe { UnsafeOcamlPtr::new(res.build().to_bits()) }
}

#[macro_export]
macro_rules! parse_with_arena {
    ($name:ident, $parse_script:expr $(,)?) => {
        // We don't use the ocaml_ffi! macro here because we want precise
        // control over the Pool--when a parse fails, we want to free the old
        // pool and create a new one.
        #[no_mangle]
        pub extern "C" fn $name<'a>(ocaml_source_text: usize, env: usize) -> usize {
            ocamlrep_ocamlpool::catch_unwind(|| {
                use ocamlrep::ptr::UnsafeOcamlPtr;
                use ocamlrep::FromOcamlRep;
                use oxidized::full_fidelity_parser_env::FullFidelityParserEnv;

                let ocaml_source_text = unsafe { UnsafeOcamlPtr::new(ocaml_source_text) };
                let env = unsafe { FullFidelityParserEnv::from_ocaml(env).unwrap() };
                $crate::parse(ocaml_source_text, env, |a, s, e| $parse_script(a, s, e)).as_usize()
            })
        }
    };
}

pub fn scan_trivia<'a, F>(
    source_text: SourceText<'a>,
    offset: usize,
    width: usize,
    f: F,
) -> UnsafeOcamlPtr
where
    F: for<'b> Fn(&'b Bump, &'b SourceText<'b>, usize, usize) -> PositionedTrivia<'b>,
{
    let arena = Bump::new();
    let r = f(&arena, &source_text, offset, width);
    unsafe { UnsafeOcamlPtr::new(to_ocaml(r.as_slice())) }
}

ocaml_ffi! {
    fn rust_parse_mode(source_text: SourceText<'_>) -> Option<file_info::Mode> {
        let (_, mode) = parse_mode(&source_text);
        mode
    }

    fn scan_leading_xhp_trivia(
        source_text: SourceText<'_>,
        offset: usize,
        width: usize,
    ) -> UnsafeOcamlPtr {
        scan_trivia(
            source_text,
            offset,
            width,
            positioned_by_ref_parser::scan_leading_xhp_trivia,
        )
    }

    fn scan_trailing_xhp_trivia(
        source_text: SourceText<'_>,
        offset: usize,
        width: usize,
    ) -> UnsafeOcamlPtr {
        scan_trivia(
            source_text,
            offset,
            width,
            positioned_by_ref_parser::scan_trailing_xhp_trivia,
        )
    }

    fn scan_leading_php_trivia(
        source_text: SourceText<'_>,
        offset: usize,
        width: usize,
    ) -> UnsafeOcamlPtr {
        scan_trivia(
            source_text,
            offset,
            width,
            positioned_by_ref_parser::scan_leading_php_trivia,
        )
    }

    fn scan_trailing_php_trivia(
        source_text: SourceText<'_>,
        offset: usize,
        width: usize,
    ) -> UnsafeOcamlPtr {
        scan_trivia(
            source_text,
            offset,
            width,
            positioned_by_ref_parser::scan_trailing_php_trivia,
        )
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
        use parser_core_types::parser_env::ParserEnv;
        op.precedence(&ParserEnv::default())
    }

    fn rust_precedence_for_assignment_in_expressions_helper() -> usize {
        Operator::precedence_for_assignment_in_expressions()
    }

    fn rust_associativity_helper(op: Operator) -> Assoc {
        // NOTE: ParserEnv is not used in operator::associativity(), so we just create an empty ParserEnv
        // If operator::associativity() starts using ParserEnv, this function and the callsites in OCaml must be updated
        use parser_core_types::parser_env::ParserEnv;
        op.associativity(&ParserEnv::default())
    }
}
