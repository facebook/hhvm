// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate ocaml;
use parser_rust as parser;

use ocamlpool_rust::caml_raise;
use parser::syntax_error::SyntaxError;

extern "C" {
    fn ocamlpool_leave();
}

caml_raise!(rust_parser_errors_positioned, |_ocaml_source_text, _ocaml_tree, _ocaml_parser_options|, <res>, {
    let res : Vec<SyntaxError> = vec![];
    res
} -> res);
