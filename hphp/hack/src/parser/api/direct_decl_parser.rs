// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use bumpalo::Bump;
use direct_decl_smart_constructors::ArenaSourceTextAllocator;
use direct_decl_smart_constructors::DirectDeclSmartConstructors;
use direct_decl_smart_constructors::NoSourceTextAllocator;
use direct_decl_smart_constructors::SourceTextAllocator;
use mode_parser::parse_mode;
pub use oxidized::decl_parser_options::DeclParserOptions;
pub use oxidized_by_ref::direct_decl_parser::Decls;
pub use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::file_info;
pub use oxidized_by_ref::typing_defs::UserAttribute;
use parser::parser::Parser;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use relative_path::RelativePath;

/// Parse decls for typechecking.
/// - References the source text to avoid spending time or space copying
///   identifiers into the arena (when possible).
///
/// WARNING
/// This function (1) doesn't respect po_deregister_php_stdlib which filters+adjusts certain
/// decls from hhi files, (2) produces decls in reverse order, (3) includes subsequent decls
/// in case of name-clash, rather than just the first. Unless you the caller have thought
/// through your desired semantics in these cases, you're probably buggy.
pub fn parse_decls_for_typechecking<'a>(
    opts: &DeclParserOptions,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
) -> ParsedFile<'a> {
    parse_script_with_text_allocator(
        opts,
        filename,
        text,
        arena,
        NoSourceTextAllocator,
        false, // elaborate_xhp_namespaces_for_facts
    )
}

/// The same as 'parse_decls_for_typechecking' except
/// - Returns decls without reference to the source text to avoid the need to
///   keep the source text in memory when caching decls.
///
/// WARNING
/// This function (1) doesn't respect po_deregister_php_stdlib which filters+adjusts certain
/// decls from hhi files, (2) produces decls in reverse order, (3) includes subsequent decls
/// in case of name-clash, rather than just the first. Unless you the caller have thought
/// through your desired semantics in these cases, you're probably buggy.
pub fn parse_decls_for_typechecking_without_reference_text<'a, 'text>(
    opts: &DeclParserOptions,
    filename: RelativePath,
    text: &'text [u8],
    arena: &'a Bump,
) -> ParsedFile<'a> {
    parse_script_with_text_allocator(
        opts,
        filename,
        text,
        arena,
        ArenaSourceTextAllocator(arena),
        false, // elaborate_xhp_namespaces_for_facts
    )
}

/// Parse decls for bytecode compilation.
/// - Returns decls without reference to the source text to avoid the need to
///   keep the source text in memory when caching decls.
/// - Expects the keep_user_attributes option to be set as it is necessary for
///   producing facts. (This means that you'll get decl_hash answers that differ
///   from parse_decls).
pub fn parse_decls_for_bytecode<'a, 'text>(
    opts: &DeclParserOptions,
    filename: RelativePath,
    text: &'text [u8],
    arena: &'a Bump,
) -> ParsedFile<'a> {
    parse_script_with_text_allocator(
        opts,
        filename,
        text,
        arena,
        ArenaSourceTextAllocator(arena),
        true, // elaborate_xhp_namespaces_for_facts
    )
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

fn parse_script_with_text_allocator<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>>(
    opts: &'o DeclParserOptions,
    filename: RelativePath,
    text: &'t [u8],
    arena: &'a Bump,
    source_text_allocator: S,
    elaborate_xhp_namespaces_for_facts: bool,
) -> ParsedFile<'a> {
    let source = SourceText::make(Arc::new(filename), text);
    let env = ParserEnv::from(opts);
    let (_, mode_opt) = parse_mode(&source);
    let mode_opt = mode_opt.map(file_info::Mode::from);
    let mode = mode_opt.unwrap_or(file_info::Mode::Mstrict);
    let sc = DirectDeclSmartConstructors::new(
        opts,
        &source,
        mode,
        arena,
        source_text_allocator,
        elaborate_xhp_namespaces_for_facts,
    );
    let mut parser = Parser::new(&source, env, sc);
    let _root = parser.parse_script(); // doing it for the side effect, not the return value
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();

    ParsedFile {
        mode: mode_opt,
        file_attributes: collect_file_attributes(
            arena,
            sc_state.file_attributes.iter().copied(),
            sc_state.file_attributes.len(),
        ),
        decls: sc_state.decls,
        disable_xhp_element_mangling: opts.disable_xhp_element_mangling,
        has_first_pass_parse_errors: !errors.is_empty(),
    }
}
