// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use parser::parser::Parser;
use parser::parser_env::ParserEnv;
use parser::positioned_smart_constructors::PositionedSmartConstructors;
use parser::smart_constructors::NoState;
use parser::smart_constructors::WithKind;
use parser::source_text::SourceText;
use parser_rust as parser;

use rust_editable_positioned_syntax::{EditablePositionedSyntax, EditablePositionedSyntaxTrait};

use ocaml::caml;
use ocamlpool_rust::utils::{block_field, str_field};
use oxidized::relative_path::RelativePath;
use rust_to_ocaml::rust_to_ocaml::{SerializationContext, ToOcaml};

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

    let editable_root = EditablePositionedSyntax::from_positioned_syntax(&root, &Rc::new(source_text));
    let rewritten_editable_root = rust_ppl_class_rewriter::rewrite_ppl_classes(editable_root);

    ocamlpool_enter();
    let ocaml_root = rewritten_editable_root.to_ocaml(&context);
    ocamlpool_leave();
    res = ocaml::Value::new(ocaml_root);
} -> res);
