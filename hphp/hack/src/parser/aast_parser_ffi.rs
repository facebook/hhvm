// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;

use aast_parser::rust_aast_parser_types::Env;
use aast_parser::AastParser;
use ocamlrep::FromOcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;

/// This is the entrypoint to the parser from ocaml.
#[no_mangle]
extern "C" fn from_text(env: usize, source_text: usize) -> usize {
    // XXX why this catch_unwind
    ocamlrep_ocamlpool::catch_unwind(|| {
        let env = unsafe { Env::from_ocaml(env).unwrap() };
        let source_text = match unsafe { SourceText::from_ocaml(source_text) } {
            Ok(source_text) => source_text,
            Err(e) => panic!("SourceText::from_ocaml failed: {:#?}", e),
        };
        let indexed_source_text = IndexedSourceText::new(source_text);
        let res = AastParser::from_text(&env, &indexed_source_text, HashSet::default());
        // Safety: Requires no concurrent interaction with OCaml
        // runtime from other threads.
        unsafe { to_ocaml(&res) }
    })
}
