// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use bumpalo::Bump;

use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{
    direct_decl_parser::{DeclLists, Decls},
    file_info,
};
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText};

pub fn parse_decls<'a>(
    filename: RelativePath,
    text: &'a [u8],
    auto_namespace_map: &'a BTreeMap<String, String>,
    arena: &'a Bump,
) -> Decls<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state, _mode) = direct_decl_parser::parse_script(
        &text,
        ParserEnv::default(),
        auto_namespace_map,
        arena,
        None,
    );
    state.decls
}

pub fn parse_decl_lists<'a>(
    filename: RelativePath,
    text: &'a [u8],
    auto_namespace_map: &'a BTreeMap<String, String>,
    arena: &'a Bump,
) -> (DeclLists<'a>, Option<file_info::Mode>) {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state, mode) = direct_decl_parser::parse_script(
        &text,
        ParserEnv::default(),
        auto_namespace_map,
        arena,
        None,
    );
    (DeclLists::from_decls(state.decls, arena), mode)
}
