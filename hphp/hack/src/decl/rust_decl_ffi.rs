// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use decl_rust::direct_decl_parser::parse_decls_and_mode;
use no_pos_hash::position_insensitive_hash;
use ocamlrep::{bytes_from_ocamlrep, FromOcamlRep, FromOcamlRepIn};
use ocamlrep_caml_builtins::Int64;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, direct_decl_parser::Decls};
use stack_limit::{StackLimit, KI, MI, STACK_SLACK_1K};

#[no_mangle]
pub unsafe extern "C" fn hh_parse_decls_and_mode_ffi(
    opts_ptr: usize,
    filename_ptr: usize,
    text_ptr: usize,
    include_hash: usize,
) -> usize {
    fn inner(opts_ptr: usize, filename_ptr: usize, text_ptr: usize, include_hash: usize) -> usize {
        // SAFETY: We trust we've been handed a valid, immutable OCaml value
        let include_hash = unsafe { bool::from_ocaml(include_hash).unwrap() };

        let make_retryable = move || {
            move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
                // SAFETY: the OCaml garbage collector must not run as long as text_ptr
                // and text_value exist. We don't call into OCaml here or anywhere in
                // the direct decl parser smart constructors, so it won't.
                let text_value = unsafe { ocamlrep::Value::from_bits(text_ptr) };
                let text = bytes_from_ocamlrep(text_value).expect("expected string");

                // SAFETY: We trust we've been handed a valid, immutable OCaml value,
                // and that the OCaml runtime (e.g., allocation, GC) has not been
                // called into since our FFI call began
                let filename = unsafe { RelativePath::from_ocaml(filename_ptr).unwrap() };

                let arena = Bump::new();

                // SAFETY: We trust we've been handed a valid, immutable OCaml value,
                // and that the OCaml runtime (e.g., allocation, GC) has not been
                // called into since our FFI call began
                let opts = unsafe {
                    DeclParserOptions::from_ocamlrep_in(
                        ocamlrep::Value::from_bits(opts_ptr),
                        &arena,
                    )
                    .unwrap()
                };

                let (decls, mode) =
                    parse_decls_and_mode(&opts, filename, &text, &arena, Some(stack_limit));

                let hash = if include_hash {
                    Some(Int64(position_insensitive_hash(&decls) as i64))
                } else {
                    None
                };

                // SAFETY: We immediately hand this pointer to the OCaml runtime.
                // The use of UnsafeOcamlPtr is necessary here because we cannot return
                // `decls`, since it borrows `arena`, which is destroyed at the end of
                // this function scope. Instead, we convert the decls to OCaml
                // ourselves, and return the pointer (the converted OCaml value does not
                // borrow the arena).
                unsafe { ocamlrep_ocamlpool::to_ocaml(&(decls, mode, hash)) }
            }
        };

        let on_retry = &mut |stack_size_tried: usize| {
            // Not always printing warning here because this would fail some HHVM tests
            if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
                eprintln!(
                    "[hrust] warning: direct_decl_parser exceeded stack of {} KiB on: {:?}",
                    (stack_size_tried - STACK_SLACK_1K(stack_size_tried)) / KI,
                    // SAFETY: We trust we've been handed a valid, immutable OCaml value
                    unsafe { RelativePath::from_ocaml(filename_ptr).unwrap() },
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
    ocamlrep_ocamlpool::catch_unwind(|| inner(opts_ptr, filename_ptr, text_ptr, include_hash))
}

ocaml_ffi_with_arena! {
    fn decls_hash<'a>(arena: &'a Bump, decls: Decls<'a>) -> Int64 {
        Int64(position_insensitive_hash(&decls) as i64)
    }
}
