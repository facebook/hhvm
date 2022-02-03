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
use oxidized_by_ref::file_info;
use parser::parser::Parser;
use parser_core_types::{
    parser_env::ParserEnv, source_text::SourceText, syntax_error::SyntaxError,
};
use stack_limit::StackLimit;

pub use oxidized_by_ref::{
    decl_parser_options::DeclParserOptions,
    direct_decl_parser::{Decls, ParsedFile},
    typing_defs::UserAttribute,
};

/// Parse decls for typechecking.
/// - References the source text to avoid spending time or space copying
///   identifiers into the arena (when possible).
/// - Excludes user attributes which are irrelevant to typechecking.
pub fn parse_decls<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
    stack_limit: Option<&StackLimit>,
) -> ParsedFile<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, errors, state, mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        stack_limit,
        NoSourceTextAllocator,
        false, // retain_or_omit_user_attributes_for_facts
        false, // simplify_naming_for_facts
    );
    ParsedFile {
        mode,
        file_attributes: collect_file_attributes(
            arena,
            state.file_attributes.iter().copied(),
            state.file_attributes.len(),
        ),
        decls: state.decls,
        has_first_pass_parse_errors: !errors.is_empty(),
    }
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
    stack_limit: Option<&StackLimit>,
) -> ParsedFile<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, errors, state, mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        stack_limit,
        ArenaSourceTextAllocator(arena),
        true, // retain_or_omit_user_attributes_for_facts
        true, // simplify_naming_for_facts
    );
    ParsedFile {
        mode,
        file_attributes: collect_file_attributes(
            arena,
            state.file_attributes.iter().copied(),
            state.file_attributes.len(),
        ),
        decls: state.decls,
        has_first_pass_parse_errors: !errors.is_empty(),
    }
}

fn collect_file_attributes<'a>(
    arena: &'a Bump,
    file_attributes: impl Iterator<Item = &'a UserAttribute<'a>>,
    len: usize,
) -> &'a [&'a UserAttribute<'a>] {
    let mut attrs = bumpalo::collections::Vec::with_capacity_in(len, arena);
    attrs.extend(file_attributes);
    // Direct decl parser populates state.file_attributes in reverse of
    // syntactic order, so reverse it.
    attrs.reverse();
    attrs.into_bump_slice()
}

fn parse_script_with_text_allocator<'a, 'text, S: SourceTextAllocator<'text, 'a>>(
    opts: &'a DeclParserOptions<'a>,
    source: &SourceText<'text>,
    arena: &'a Bump,
    stack_limit: Option<&StackLimit>,
    source_text_allocator: S,
    retain_or_omit_user_attributes_for_facts: bool,
    simplify_naming_for_facts: bool,
) -> (
    Node<'a>,
    Vec<SyntaxError>,
    DirectDeclSmartConstructors<'a, 'text, S>,
    Option<file_info::Mode>,
) {
    let env = ParserEnv::from(opts);
    let (_, mode_opt) = parse_mode(source);
    let mode = mode_opt.unwrap_or(file_info::Mode::Mstrict);
    let sc = DirectDeclSmartConstructors::new(
        opts,
        source,
        mode,
        arena,
        source_text_allocator,
        retain_or_omit_user_attributes_for_facts,
        simplify_naming_for_facts,
    );
    let mut parser = Parser::new(source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state, mode_opt)
}
