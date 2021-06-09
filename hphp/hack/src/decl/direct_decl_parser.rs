// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{
    decl_parser_options::DeclParserOptions, direct_decl_parser::Decls, file_info,
};
use parser_core_types::source_text::SourceText;
use stack_limit::StackLimit;

pub fn parse_decls_and_mode<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (Decls<'a>, Option<file_info::Mode>) {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state, mode) =
        direct_decl_parser::parse_script(opts, &text, arena, stack_limit);
    (state.decls, mode)
}

pub fn parse_decls<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> Decls<'a> {
    parse_decls_and_mode(opts, filename, text, arena, stack_limit).0
}

pub fn parse_decls_without_reference_text<'a, 'text>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'text [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> Decls<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state, _mode) =
        direct_decl_parser::parse_script_without_reference_text(opts, &text, arena, stack_limit);
    state.decls
}
