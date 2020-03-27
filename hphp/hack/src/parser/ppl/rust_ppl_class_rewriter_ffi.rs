// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use ocamlrep::OcamlRep;
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};
use positioned_parser;
use rust_editable_positioned_syntax::{EditablePositionedSyntax, EditablePositionedSyntaxTrait};
use rust_ppl_class_rewriter::PplClassRewriter;
use rust_to_ocaml::{SerializationContext, ToOcaml};

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

#[no_mangle]
pub extern "C" fn parse_and_rewrite_ppl_classes(ocaml_source_text: usize) -> usize {
    let source_text = unsafe { SourceText::from_ocaml(ocaml_source_text).unwrap() };

    let env = ParserEnv {
        hhvm_compat_mode: false,
        php5_compat_mode: false,
        codegen: false,
        allow_new_attribute_syntax: false,
        enable_xhp_class_modifier: false,
        disable_xhp_element_mangling: false,
        disable_xhp_children_declarations: false,
        disable_modes: false,
    };

    let (root, _, _) = positioned_parser::parse_script(&source_text, env, None);

    let context = SerializationContext::new(ocaml_source_text);

    let mut editable_root =
        EditablePositionedSyntax::from_positioned_syntax(&root, &Rc::new(source_text));
    PplClassRewriter::rewrite_ppl_classes(&mut editable_root);

    unsafe {
        ocamlpool_enter();
        let ocaml_root = editable_root.to_ocaml(&context);
        ocamlpool_leave();
        ocaml_root
    }
}
