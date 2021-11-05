// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use direct_decl_parser::parse_decls_and_mode;
use hh_hash::{hash, position_insensitive_hash};
use ocamlrep::{bytes_from_ocamlrep, ptr::UnsafeOcamlPtr};
use ocamlrep_caml_builtins::Int64;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, direct_decl_parser::Decls};

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

        match stack_limit::with_elastic_stack(|stack_limit| {
            let filename = RelativePath::make(filename.prefix(), filename.path().to_owned());

            let arena = &Bump::new();
            let (decls, mode) =
                parse_decls_and_mode(opts, filename, text, arena, Some(stack_limit));

                let symbol_decl_hashes = if include_symbol_decl_hashes {
                    Some(
                        decls
                            .iter()
                            .map(|x| Int64(hash(&x) as i64))
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
        }) {
            Ok(ocaml_result) => ocaml_result,
            Err(failure) => {
                panic!(
                    "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                    failure.max_stack_size_tried / stack_limit::KI
                );
            }
        }
    }

    fn decls_hash<'a>(arena: &'a Bump, decls: Decls<'a>) -> Int64 {
        Int64(position_insensitive_hash(&decls) as i64)
    }
}
