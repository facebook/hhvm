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
use oxidized_by_ref::{decl_parser_options::DeclParserOptions, file_info};
use parser::parser::Parser;
use parser_core_types::{
    parser_env::ParserEnv, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

// Parse decls for type-checking
pub fn parse_script<'a>(
    opts: &'a DeclParserOptions<'a>,
    source: &SourceText<'a>,
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (
    Node<'a>,
    Vec<SyntaxError>,
    DirectDeclSmartConstructors<'a, 'a, NoSourceTextAllocator>,
    Option<file_info::Mode>,
) {
    parse_script_with_text_allocator(
        opts,
        source,
        arena,
        stack_limit,
        NoSourceTextAllocator,
        true, // omit_user_attributes_irrelevant_to_typechecking
    )
}

// Used for decls in compilation.
// - Returns decls without reference to the source text to avoid
//   keeping the source text in memory when caching decls.
// - Preserve attributes in decls necessary for producing facts.
pub fn parse_script_without_reference_text<'a, 'text>(
    opts: &'a DeclParserOptions<'a>,
    source: &SourceText<'text>,
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (
    Node<'a>,
    Vec<SyntaxError>,
    DirectDeclSmartConstructors<'a, 'text, ArenaSourceTextAllocator<'a>>,
    Option<file_info::Mode>,
) {
    parse_script_with_text_allocator(
        opts,
        source,
        arena,
        stack_limit,
        ArenaSourceTextAllocator(arena),
        false, // omit_user_attributes_irrelevant_to_typechecking
    )
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
