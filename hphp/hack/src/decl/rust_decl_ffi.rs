// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use bumpalo::Bump;

use decl_rust::direct_decl_parser::parse_decls_and_mode;
use ocamlrep::{bytes_from_ocamlrep, FromOcamlRep};
use oxidized::relative_path::RelativePath;
use stack_limit::{StackLimit, KI, MI, STACK_SLACK_1K};

#[no_mangle]
pub unsafe extern "C" fn hh_parse_decls_and_mode_ffi(
    filename_ptr: usize,
    text_ptr: usize,
    ns_map_ptr: usize,
) -> usize {
    fn inner(filename_ptr: usize, text_ptr: usize, ns_map_ptr: usize) -> usize {
        let make_retryable = move || {
            move |stack_limit: &StackLimit, _nonmain_stack_size: Option<usize>| {
                // SAFETY: the OCaml garbage collector must not run as long as text_ptr
                // and text_value exist. We don't call into OCaml here or anywhere in
                // the direct decl parser smart constructors, so it won't.
                let text_value = unsafe { ocamlrep::Value::from_bits(text_ptr) };
                let text = bytes_from_ocamlrep(text_value).expect("expected string");

                // SAFETY: We trust we've been handed a valid, immutable OCaml value
                let filename = unsafe { RelativePath::from_ocaml(filename_ptr).unwrap() };

                // SAFETY: We trust we've been handed a valid, immutable OCaml value
                let ns_map = unsafe { Vec::<(String, String)>::from_ocaml(ns_map_ptr).unwrap() };
                let ns_map = ns_map.into_iter().collect::<BTreeMap<_, _>>();

                let arena = Bump::new();

                let r = parse_decls_and_mode(filename, &text, &ns_map, &arena, Some(stack_limit));

                // SAFETY: We immediately hand this pointer to the OCaml runtime.
                // The use of UnsafeOcamlPtr is necessary here because we cannot return
                // `decls`, since it borrows `arena`, which is destroyed at the end of
                // this function scope. Instead, we convert the decls to OCaml
                // ourselves, and return the pointer (the converted OCaml value does not
                // borrow the arena).
                unsafe { ocamlrep_ocamlpool::to_ocaml(&r) }
            }
        };

        let on_retry = &mut |stack_size_tried: usize| {
            // Not always printing warning here because this would fail some HHVM tests
            if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
                eprintln!(
                    "[hrust] warning: hh_compile exceeded stack of {} KiB on: {:?}",
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
    ocamlrep_ocamlpool::catch_unwind(|| inner(filename_ptr, text_ptr, ns_map_ptr))
}
