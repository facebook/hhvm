// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::OcamlRep;
use parser_core_types::{positioned_syntax::PositionedSyntax, source_text::SourceText};
use rust_to_ocaml::{SerializationContext, ToOcaml};

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

/// `rewrite_coroutines` returns a pair of source text (old, new)
pub fn rewrite_coroutines<'a>(
    source: &'a SourceText<'a>,
    root: &PositionedSyntax,
) -> (SourceText<'a>, SourceText<'a>) {
    let rewrite_coroutines = ocaml::named_value("rewrite_coroutines_for_rust")
        .expect("rewrite_coroutines not registered");
    let ocaml_source_text = source.ocaml_source_text().unwrap().as_usize();
    let context = SerializationContext::new(ocaml_source_text);
    unsafe {
        ocamlpool_enter();
        let ocaml_root = root.to_ocaml(&context);
        ocamlpool_leave();
        let rewritten_ocaml_source = rewrite_coroutines
            .call_n(&[
                ocaml::Value::new(ocaml_source_text),
                ocaml::Value::new(ocaml_root),
            ])
            .unwrap();
        <(SourceText, SourceText)>::from_ocaml(rewritten_ocaml_source.0).unwrap()
    }
}
