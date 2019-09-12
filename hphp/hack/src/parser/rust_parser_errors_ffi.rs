// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_rust as parser;

use ocamlpool_rust::{caml_raise, ocamlvalue::Ocamlvalue, utils::*};
use oxidized::{parser_options::ParserOptions, relative_path::RelativePath};
use parser::{positioned_syntax::PositionedSyntax, source_text::SourceText};
use parser_core_types::syntax_tree::SyntaxTree;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml_raise!(rust_parser_errors_positioned, |ocaml_source_text, ocaml_tree, _ocaml_parser_options|, <res>, {
    let parser_options = ParserOptions::default(); // TODO
    let ocaml_source_text_value = ocaml_source_text.0;

    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make_with_raw(&relative_path, &content.data(), ocaml_source_text_value);

    let tree_pointer = ocaml_tree.usize_val() as *mut SyntaxTree<PositionedSyntax, ()>;
    let mut tree = Box::from_raw(tree_pointer);
    // The tree already contains source text, but this source text contains a pointer into OCaml
    // heap, which might have been invalidated by GC in the meantime. Replacing the source text
    // with a current one prevents it. This will still end horribly if the tree starts storing some
    // other pointers into source text, but it's not the case at the moment.
    tree.replace_text_unsafe(&source_text);

    let errors = rust_parser_errors::ParserErrors::parse_errors(&tree, parser_options);
    ocamlpool_enter();
    let ocaml_errors = errors.ocamlvalue();
    ocamlpool_leave();
    res = ocaml::Value::new(ocaml_errors);
} -> res);
