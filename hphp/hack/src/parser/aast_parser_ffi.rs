// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast_parser::{rust_aast_parser_types::Env, AastParser, Error as AastParserError};
use ocamlrep::FromOcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};

#[no_mangle]
extern "C" fn from_text(env: usize, source_text: usize) -> usize {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let env = unsafe { Env::from_ocaml(env).unwrap() };
        let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
        let indexed_source_text = IndexedSourceText::new(source_text);

        match stack_limit::with_elastic_stack(|stack_limit| {
            let res = AastParser::from_text(&env, &indexed_source_text, Some(stack_limit));
            // Safety: Requires no concurrent interaction with OCaml
            // runtime from other threads.
            unsafe { to_ocaml(&res) }
        }) {
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
