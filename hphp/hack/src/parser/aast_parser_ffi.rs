// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast_parser::{
    rust_aast_parser_types::{Env, Result as ParserResult},
    AastParser, Error,
};
use ocamlrep_ocamlpool::ocaml_ffi;
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};

ocaml_ffi! {
    fn from_text(env: Env, source_text: SourceText) -> Result<ParserResult, Error> {
        let indexed_source_text = IndexedSourceText::new(source_text);
        AastParser::from_text(&env, &indexed_source_text)
    }
}
