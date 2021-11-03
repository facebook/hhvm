// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use direct_decl_smart_constructors::{
    ArenaSourceTextAllocator, DirectDeclSmartConstructors, NoSourceTextAllocator, Node,
    SourceTextAllocator,
};
use mode_parser::parse_mode;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{
    decl_parser_options::DeclParserOptions, direct_decl_parser::Decls, file_info,
};
use parser::parser::Parser;
use parser_core_types::{
    parser_env::ParserEnv, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

/// Parse decls for typechecking.
/// - References the source text to avoid spending time or space copying
///   identifiers into the arena (when possible).
/// - Excludes user attributes which are irrelevant to typechecking.
pub fn parse_decls_and_mode<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (Decls<'a>, Option<file_info::Mode>) {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, _errors, state, mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        stack_limit,
        NoSourceTextAllocator,
        true, // omit_user_attributes_irrelevant_to_typechecking
    );
    (state.decls, mode)
}

/// Parse decls for typechecking.
/// - References the source text to avoid spending time or space copying
///   identifiers into the arena (when possible).
/// - Excludes user attributes which are irrelevant to typechecking.
pub fn parse_decls<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> Decls<'a> {
    parse_decls_and_mode(opts, filename, text, arena, stack_limit).0
}

/// Parse decls for decls in compilation.
/// - Returns decls without reference to the source text to avoid the need to
///   keep the source text in memory when caching decls.
/// - Preserves user attributes in decls necessary for producing facts.
pub fn parse_decls_without_reference_text<'a, 'text>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'text [u8],
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (Decls<'a>, bool) {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, errors, state, _mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        stack_limit,
        ArenaSourceTextAllocator(arena),
        false, // omit_user_attributes_irrelevant_to_typechecking
    );
    (state.decls, !errors.is_empty())
}

fn parse_script_with_text_allocator<'a, 'text, S: SourceTextAllocator<'text, 'a>>(
    opts: &'a DeclParserOptions<'a>,
    source: &SourceText<'text>,
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
    source_text_allocator: S,
    omit_user_attributes_irrelevant_to_typechecking: bool,
) -> (
    Node<'a>,
    Vec<SyntaxError>,
    DirectDeclSmartConstructors<'a, 'text, S>,
    Option<file_info::Mode>,
) {
    let env = ParserEnv::from(opts);
    let (_, mode_opt) = parse_mode(source);
    let mode = mode_opt.unwrap_or(file_info::Mode::Mpartial);
    let sc = DirectDeclSmartConstructors::new(
        opts,
        &source,
        mode,
        arena,
        source_text_allocator,
        omit_user_attributes_irrelevant_to_typechecking,
    );
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state, mode_opt)
}
