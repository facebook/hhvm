// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_rust as parser;

use ocamlpool_rust::{
    caml_raise,
    ocamlvalue::*,
    utils::{block_field, bool_field, str_field},
};
use oxidized::{file_info, global_options::GlobalOptions, relative_path::RelativePath};
use parser::{
    indexed_source_text::IndexedSourceText,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    source_text::SourceText,
};
use syntax_tree::SyntaxTree;

use lowerer::{Env as LowererEnv, Lowerer};
use std::default::Default;

struct PositionedSyntaxLowerer {}
impl<'a> Lowerer<'a, PositionedToken, PositionedValue> for PositionedSyntaxLowerer {}

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml_raise!(lower, |ocaml_env, ocaml_source_text, ocaml_tree|, <res>, {
    let ocaml_source_text_value = ocaml_source_text.0;

    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make_with_raw(&relative_path, &content.data(), ocaml_source_text_value);
    let indexed_source_text = IndexedSourceText::new(&source_text);
    let tree = match <SyntaxTree<PositionedSyntax, ()>>::ffi_pointer_as_ref(ocaml_tree.usize_val(), &source_text) {
        Ok(t) => t,
        Err(msg) => panic!(msg),
    };
    let parser_options = GlobalOptions::default();
    let codegen = bool_field(&ocaml_env, 1);
    let elaborate_namespaces = bool_field(&ocaml_env, 3);
    let quick_mode = bool_field(&ocaml_env, 6);
    let mode = file_info::Mode::from_ocamlvalue(block_field(&ocaml_env, 11));

    let mut env = LowererEnv::make(
        codegen,
        elaborate_namespaces,
        quick_mode,
        mode,
        &indexed_source_text,
        &parser_options,
    );

    ocamlpool_enter();
    let r = PositionedSyntaxLowerer::lower(&mut env, tree.root()).ocamlvalue();
    ocamlpool_leave();
    res = ocaml::Value::new(r);
} -> res );
