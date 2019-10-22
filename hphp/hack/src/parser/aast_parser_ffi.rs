// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use aast_parser::AastParser;
use ocamlpool_rust::{
    caml_raise,
    ocamlvalue::*,
    utils::{block_field, str_field},
};
use ocamlrep::OcamlRep;
use oxidized::{relative_path::RelativePath, rust_aast_parser_types::Env};
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

caml_raise!(from_text, |ocaml_env, ocaml_source_text|, <res>, {
    let relative_path_raw = block_field(&ocaml_source_text, 0);
    let relative_path = RelativePath::from_ocamlvalue(&relative_path_raw);
    let content = str_field(&ocaml_source_text, 2);
    let source_text = SourceText::make_with_raw(&relative_path, &content.data(), ocaml_source_text.0);
    let indexed_source_text = IndexedSourceText::new(source_text.clone());

    let env = Env::from_ocaml(ocaml_env.0).unwrap();
    ocamlpool_enter();
    let r = AastParser::from_text(&env, &indexed_source_text).unwrap().ocamlvalue();
    ocamlpool_leave();
    res = ocaml::Value::new(r);
} -> res );
