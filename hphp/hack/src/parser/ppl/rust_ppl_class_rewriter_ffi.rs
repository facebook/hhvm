// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use ocaml::caml;
use ocamlpool_rust::utils::{block_field, str_field};
use oxidized::relative_path::RelativePath;
use parser_rust::{
    parser::Parser, parser_env::ParserEnv, smart_constructors::NoState,
    smart_constructors::WithKind, source_text::SourceText,
};
use positioned_parser::positioned_smart_constructors::PositionedSmartConstructors;
use rust_editable_positioned_syntax::{EditablePositionedSyntax, EditablePositionedSyntaxTrait};
use rust_ppl_class_rewriter::PplClassRewriter;
use rust_to_ocaml::{SerializationContext, ToOcaml};

type PositionedSyntaxParser<'a> = Parser<'a, WithKind<PositionedSmartConstructors>, NoState>;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml!(parse_and_rewrite_ppl_classes, |ocaml_source_text|, <res>, {

    let ocaml_source_text_value = ocaml_source_text.0;

    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make_with_raw(&relative_path, &content.data(), ocaml_source_text_value);

    let env = ParserEnv {
        is_experimental_mode : false,
        hhvm_compat_mode : false,
        php5_compat_mode : false,
        codegen : false,
        allow_new_attribute_syntax : false,
    };

    let mut parser = PositionedSyntaxParser::make(&source_text, env);
    let root = parser.parse_script(None);

    let context = SerializationContext::new(ocaml_source_text_value);

    let mut editable_root = EditablePositionedSyntax::from_positioned_syntax(&root, &Rc::new(source_text));
    PplClassRewriter::rewrite_ppl_classes(&mut editable_root);

    ocamlpool_enter();
    let ocaml_root = editable_root.to_ocaml(&context);
    ocamlpool_leave();
    res = ocaml::Value::new(ocaml_root);
} -> res);
