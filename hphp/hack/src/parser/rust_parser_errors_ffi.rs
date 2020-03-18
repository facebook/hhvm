// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{ptr::UnsafeOcamlPtr, OcamlRep};
use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::parser_options::ParserOptions;
use parser_core_types::{
    positioned_syntax::PositionedSyntax, source_text::SourceText, syntax_error::SyntaxError,
    syntax_tree::SyntaxTree,
};

// "only_for_parser_errors" because it sets only a subset of options relevant to parser errors,
// leaving the rest default
unsafe fn parser_options_from_ocaml_only_for_parser_errors(
    ocaml_opts: UnsafeOcamlPtr,
) -> (ParserOptions, (bool, bool, bool)) {
    let ocaml_opts = ocaml_opts.as_usize() as *const usize;

    let hhvm_compat_mode = bool::from_ocaml(*ocaml_opts.add(0)).unwrap();
    let hhi_mode = bool::from_ocaml(*ocaml_opts.add(1)).unwrap();
    let codegen = bool::from_ocaml(*ocaml_opts.add(2)).unwrap();

    let mut parser_options = ParserOptions::default();

    let po_disable_lval_as_an_expression = bool::from_ocaml(*ocaml_opts.add(3)).unwrap();
    let po_disable_legacy_soft_typehints = bool::from_ocaml(*ocaml_opts.add(4)).unwrap();
    let tco_const_static_props = bool::from_ocaml(*ocaml_opts.add(5)).unwrap();
    let po_disable_legacy_attribute_syntax = bool::from_ocaml(*ocaml_opts.add(6)).unwrap();
    let po_const_default_func_args = bool::from_ocaml(*ocaml_opts.add(7)).unwrap();
    let po_abstract_static_props = bool::from_ocaml(*ocaml_opts.add(8)).unwrap();
    let po_disallow_func_ptrs_in_constants = bool::from_ocaml(*ocaml_opts.add(9)).unwrap();
    let po_enable_xhp_class_modifier = bool::from_ocaml(*ocaml_opts.add(10)).unwrap();
    let po_disable_xhp_element_mangling = bool::from_ocaml(*ocaml_opts.add(11)).unwrap();
    let po_disable_xhp_children_declarations = bool::from_ocaml(*ocaml_opts.add(12)).unwrap();
    let po_enable_first_class_function_pointers = bool::from_ocaml(*ocaml_opts.add(13)).unwrap();
    let po_disable_modes = bool::from_ocaml(*ocaml_opts.add(14)).unwrap();
    let po_disable_array = bool::from_ocaml(*ocaml_opts.add(15)).unwrap();

    parser_options.po_disable_lval_as_an_expression = po_disable_lval_as_an_expression;
    parser_options.po_disable_legacy_soft_typehints = po_disable_legacy_soft_typehints;
    parser_options.tco_const_static_props = tco_const_static_props;
    parser_options.po_disable_legacy_attribute_syntax = po_disable_legacy_attribute_syntax;
    parser_options.po_const_default_func_args = po_const_default_func_args;
    parser_options.po_abstract_static_props = po_abstract_static_props;
    parser_options.po_disallow_func_ptrs_in_constants = po_disallow_func_ptrs_in_constants;
    parser_options.po_enable_xhp_class_modifier = po_enable_xhp_class_modifier;
    parser_options.po_disable_xhp_element_mangling = po_disable_xhp_element_mangling;
    parser_options.po_disable_xhp_children_declarations = po_disable_xhp_children_declarations;
    parser_options.po_enable_first_class_function_pointers =
        po_enable_first_class_function_pointers;
    parser_options.po_disable_modes = po_disable_modes;
    parser_options.po_disable_array = po_disable_array;

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

ocaml_ffi! {
    fn drop_tree_positioned(ocaml_tree: usize) {
        let tree_pointer = ocaml_tree as *mut SyntaxTree<PositionedSyntax, ()>;
        let tree = unsafe { Box::from_raw(tree_pointer) };
        drop_tree(tree);
    }

    fn rust_parser_errors_positioned(
        source_text: SourceText,
        ocaml_tree: usize,
        ocaml_parser_options: UnsafeOcamlPtr,
    ) -> Vec<SyntaxError> {
        let (parser_options, (hhvm_compat_mode, hhi_mode, codegen)) =
            unsafe { parser_options_from_ocaml_only_for_parser_errors(ocaml_parser_options) };
        let tree = unsafe {
            <SyntaxTree<PositionedSyntax, ()>>::ffi_pointer_into_boxed(ocaml_tree, &source_text)
        };

        let errors = rust_parser_errors::parse_errors(
            &tree,
            parser_options,
            hhvm_compat_mode,
            hhi_mode,
            codegen,
        );
        drop_tree(tree);
        errors
    }
}
