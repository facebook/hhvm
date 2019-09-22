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
use syntax_tree::SyntaxTree;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

// "only_for_parser_errors" because it sets only a subset of options relevant to parser errors,
// leaving the rest default
unsafe fn parser_options_from_ocaml_only_for_parser_errors(
    ocaml_parser_options: &ocaml::Value,
) -> (ParserOptions, (bool, bool, bool)) {
    let hhvm_compat_mode = bool_field(ocaml_parser_options, 0);
    let hhi_mode = bool_field(ocaml_parser_options, 1);
    let codegen = bool_field(ocaml_parser_options, 2);

    let mut parser_options = ParserOptions::default();

    let po_disable_lval_as_an_expression = bool_field(ocaml_parser_options, 3);
    let po_enable_constant_visibility_modifiers = bool_field(ocaml_parser_options, 4);
    let po_disable_legacy_soft_typehints = bool_field(ocaml_parser_options, 5);
    let tco_const_static_props = bool_field(ocaml_parser_options, 6);
    let po_disable_legacy_attribute_syntax = bool_field(ocaml_parser_options, 7);
    let po_const_default_func_args = bool_field(ocaml_parser_options, 8);
    let po_abstract_static_props = bool_field(ocaml_parser_options, 9);
    let po_disallow_func_ptrs_in_constants = bool_field(ocaml_parser_options, 10);

    parser_options.po_disable_lval_as_an_expression = po_disable_lval_as_an_expression;
    parser_options.po_enable_constant_visibility_modifiers =
        po_enable_constant_visibility_modifiers;
    parser_options.po_disable_legacy_soft_typehints = po_disable_legacy_soft_typehints;
    parser_options.tco_const_static_props = tco_const_static_props;
    parser_options.po_disable_legacy_attribute_syntax = po_disable_legacy_attribute_syntax;
    parser_options.po_const_default_func_args = po_const_default_func_args;
    parser_options.po_abstract_static_props = po_abstract_static_props;
    parser_options.po_disallow_func_ptrs_in_constants = po_disallow_func_ptrs_in_constants;

    (parser_options, (hhvm_compat_mode, hhi_mode, codegen))
}

// See similar method in rust_parser_errors::parse_errors for explanation.
// TODO: factor this out to separate crate
fn drop_tree<T, S>(tree: Box<SyntaxTree<T, S>>)
where
    S: Clone,
{
    match tree.required_stack_size() {
        None => std::mem::drop(tree),
        Some(stack_size) => {
            let raw_pointer = Box::leak(tree) as *mut SyntaxTree<T, S> as usize;
            std::thread::Builder::new()
                .stack_size(stack_size)
                .spawn(move || {
                    let tree = unsafe { Box::from_raw(raw_pointer as *mut SyntaxTree<T, S>) };
                    std::mem::drop(tree)
                })
                .expect("ERROR: thread::spawn")
                .join()
                .expect("ERROR: failed to wait on new thread")
        }
    }
}

caml_raise!(rust_parser_errors_positioned, |ocaml_source_text, ocaml_tree, ocaml_parser_options|, <res>, {
    let (parser_options, (hhvm_compat_mode, hhi_mode, codegen)) =
        parser_options_from_ocaml_only_for_parser_errors(&ocaml_parser_options);
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

    let errors = rust_parser_errors::ParserErrors::parse_errors(&tree, parser_options, hhvm_compat_mode, hhi_mode, codegen);
    drop_tree(tree);
    ocamlpool_enter();
    let ocaml_errors = errors.ocamlvalue();
    ocamlpool_leave();
    res = ocaml::Value::new(ocaml_errors);
} -> res);
