// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast_parser::{rust_aast_parser_types::Env, AastParser, Error as AastParserError};
use ocamlrep::OcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};
use stack_limit::{StackLimit, KI, MI};

const MAX_STACK_SIZE: usize = 1024 * MI;

fn stack_slack_for_traversal_and_parsing(stack_size: usize) -> usize {
    // Syntax::to_ocaml is deeply & mutually recursive and uses nearly 2.5x of stack
    // TODO: rewrite to_ocaml iteratively & reduce it to "stack_size - MB" as in HHVM
    // (https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h)
    stack_size * 6 / 10
}

#[no_mangle]
extern "C" fn from_text(env: usize, source_text: usize) -> usize {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let make_retryable = move || {
            Box::new(
                move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
                    let env = unsafe { Env::from_ocaml(env).unwrap() };
                    let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                    let indexed_source_text = IndexedSourceText::new(source_text);
                    let res = AastParser::from_text(&env, &indexed_source_text, Some(stack_limit));
                    // Safety: Requires no concurrent interaction with OCaml
                    // runtime from other threads.
                    unsafe { to_ocaml(&res) }
                },
            )
        };

        let on_retry = &mut |stack_size_tried: usize| {
            // Not always printing warning here because this would fail some HHVM tests
            let istty = unsafe { libc::isatty(libc::STDERR_FILENO as i32) != 0 };
            if istty || std::env::var_os("HH_TEST_MODE").is_some() {
                let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                let file_path = source_text.file_path().path_str();
                eprintln!(
                    "[hrust] warning: aast_parser exceeded stack of {} KiB on: {}",
                    (stack_size_tried - stack_slack_for_traversal_and_parsing(stack_size_tried))
                        / KI,
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
            Ok(r) => r,
            Err(_) => {
                let r: &Result<(), AastParserError> =
                    &Err("Expression recursion limit reached".into());
                // Safety: Requires no concurrent interaction with OCaml runtime
                // from other threads.
                unsafe { to_ocaml(r) }
            }
        }
    })
}
