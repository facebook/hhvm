// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_rust as parser;

use crate::direct_decl_smart_constructors::*;
use ocamlrep::rc::RcOc;
use oxidized::{direct_decl_parser::Decls, file_info::Mode, relative_path::RelativePath};
use parser::{
    parser::Parser, parser_env::ParserEnv, smart_constructors_wrappers::WithKind,
    source_text::SourceText,
};
use std::collections::BTreeMap;
use syntax_tree::mode_parser::parse_mode;

pub type DirectDeclParser<'a> = Parser<'a, WithKind<DirectDeclSmartConstructors<'a>>, State<'a>>;

pub fn parse_decls(filename: RelativePath, text: &str) -> Result<Decls, String> {
    let text = SourceText::make(RcOc::new(filename), text.as_bytes());
    let is_experimental = match parse_mode(&text) {
        Some(Mode::Mexperimental) => true,
        _ => false,
    };
    let env = ParserEnv {
        is_experimental_mode: is_experimental,
        ..ParserEnv::default()
    };
    let mut parser = DirectDeclParser::make(&text, env);
    let root = parser.parse_script(None);
    let decls = root.map(|_| {
        let decls = parser.into_sc_state().decls;
        Decls {
            classes: decls
                .classes
                .iter()
                .map(|(name, class)| (name.clone(), (**class).clone()))
                .collect::<BTreeMap<_, _>>(),
            funs: decls
                .funs
                .iter()
                .map(|(name, fun)| (name.clone(), (**fun).clone()))
                .collect::<BTreeMap<_, _>>(),
            typedefs: decls
                .typedefs
                .iter()
                .map(|(name, typedef)| (name.clone(), (**typedef).clone()))
                .collect::<BTreeMap<_, _>>(),
            consts: decls
                .consts
                .iter()
                .map(|(name, const_)| (name.clone(), (**const_).clone()))
                .collect::<BTreeMap<_, _>>(),
        }
    });
    decls
}
