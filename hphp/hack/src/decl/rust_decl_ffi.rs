// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use decl_rust::direct_decl_parser::parse_decls_and_mode;
use no_pos_hash::position_insensitive_hash;
use ocamlrep::{bytes_from_ocamlrep, ptr::UnsafeOcamlPtr};
use ocamlrep_caml_builtins::Int64;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, direct_decl_parser::Decls};
use stack_limit::{StackLimit, KI, MI, STACK_SLACK_1K};

ocaml_ffi_with_arena! {
    fn hh_parse_decls_and_mode_ffi<'a>(
        arena: &'a Bump,
        opts: &'a DeclParserOptions<'a>,
        filename: &'a oxidized_by_ref::relative_path::RelativePath<'a>,
        text: UnsafeOcamlPtr,
        include_file_decl_hash: bool,
        include_symbol_decl_hashes: bool,
    ) -> UnsafeOcamlPtr {
        // SAFETY: the OCaml garbage collector must not run as long as
        // text_value exists. We don't call into OCaml here or anywhere in
        // the direct decl parser smart constructors, so it won't.
        let text_value = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");

        let make_retryable = move || {
            move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
                let filename = RelativePath::make(filename.prefix(), filename.path().to_owned());

                let arena = &Bump::new();
                let (decls, mode) =
                    parse_decls_and_mode(opts, filename, text, arena, Some(stack_limit));

                let symbol_decl_hashes = if include_symbol_decl_hashes {
                    Some(
                        decls
                            .iter()
                            .map(|x| Int64(position_insensitive_hash(&x) as i64))
                            .collect::<Vec<Int64>>(),
                    )
                } else {
                    None
                };

                let file_decl_hash = if include_file_decl_hash {
                    Some(Int64(position_insensitive_hash(&decls) as i64))
                } else {
                    None
                };

                // SAFETY: We immediately hand this pointer to the OCaml runtime.
                // The use of to_ocaml is necessary here because we cannot return
                // `decls`, since it borrows `arena`, which is destroyed at the end of
                // this function scope. Instead, we convert the decls to OCaml
                // ourselves, and return the pointer (the converted OCaml value does not
                // borrow the arena).
                unsafe {
                    UnsafeOcamlPtr::new(ocamlrep_ocamlpool::to_ocaml(&(
                        decls,
                        mode,
                        file_decl_hash,
                        symbol_decl_hashes,
                    )))
                }
            }
        };

        let on_retry = &mut |stack_size_tried: usize| {
            // Not always printing warning here because this would fail some HHVM tests
            if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
                eprintln!(
                    "[hrust] warning: direct_decl_parser exceeded stack of {} KiB on: {:?}",
                    (stack_size_tried - STACK_SLACK_1K(stack_size_tried)) / KI,
                    filename,
                );
            }
        };

        const MAX_STACK_SIZE: usize = 1024 * MI;

        use stack_limit::retry;
        let job = retry::Job {
            nonmain_stack_min: 13 * MI, // assume we need much more if default stack size isn't enough
            nonmain_stack_max: Some(MAX_STACK_SIZE),
            ..Default::default()
        };

        match job.with_elastic_stack(make_retryable, on_retry, STACK_SLACK_1K) {
            Ok(ocaml_result) => ocaml_result,
            Err(failure) => {
                panic!(
                    "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                    failure.max_stack_size_tried / KI
                );
            }
        }
    }

    fn decls_hash<'a>(arena: &'a Bump, decls: Decls<'a>) -> Int64 {
        Int64(position_insensitive_hash(&decls) as i64)
    }
}
