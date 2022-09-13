// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use direct_decl_smart_constructors::ArenaSourceTextAllocator;
use direct_decl_smart_constructors::DirectDeclSmartConstructors;
use direct_decl_smart_constructors::NoSourceTextAllocator;
use direct_decl_smart_constructors::Node;
use direct_decl_smart_constructors::SourceTextAllocator;
use mode_parser::parse_mode;
use ocamlrep::rc::RcOc;
pub use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::relative_path::RelativePath;
pub use oxidized_by_ref::direct_decl_parser::Decls;
pub use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::file_info;
pub use oxidized_by_ref::typing_defs::UserAttribute;
use parser::parser::Parser;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::SyntaxError;

/// Parse decls for typechecking.
/// - References the source text to avoid spending time or space copying
///   identifiers into the arena (when possible).
/// - Excludes user attributes which are irrelevant to typechecking.
pub fn parse_decls<'a>(
    opts: &DeclParserOptions,
    filename: RelativePath,
    text: &'a [u8],
    arena: &'a Bump,
) -> ParsedFile<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, errors, state, mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        NoSourceTextAllocator,
        false, // retain_or_omit_user_attributes_for_facts
        false, // elaborate_xhp_namespaces_for_facts
    );
    ParsedFile {
        mode,
        file_attributes: collect_file_attributes(
            arena,
            state.file_attributes.iter().copied(),
            state.file_attributes.len(),
        ),
        decls: state.decls,
        disable_xhp_element_mangling: opts.disable_xhp_element_mangling,
        has_first_pass_parse_errors: !errors.is_empty(),
    }
}

/// Parse decls for bytecode compilation.
/// - Returns decls without reference to the source text to avoid the need to
///   keep the source text in memory when caching decls.
/// - Preserves user attributes in decls necessary for producing facts.
pub fn parse_decls_without_reference_text<'a, 'text>(
    opts: &DeclParserOptions,
    filename: RelativePath,
    text: &'text [u8],
    arena: &'a Bump,
) -> ParsedFile<'a> {
    let text = SourceText::make(RcOc::new(filename), text);
    let (_, errors, state, mode) = parse_script_with_text_allocator(
        opts,
        &text,
        arena,
        ArenaSourceTextAllocator(arena),
        true, // retain_or_omit_user_attributes_for_facts
        true, // elaborate_xhp_namespaces_for_facts
    );
    ParsedFile {
        mode,
        file_attributes: collect_file_attributes(
            arena,
            state.file_attributes.iter().copied(),
            state.file_attributes.len(),
        ),
        decls: state.decls,
        disable_xhp_element_mangling: opts.disable_xhp_element_mangling,
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

fn parse_script_with_text_allocator<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>>(
    opts: &'o DeclParserOptions,
    source: &SourceText<'t>,
    arena: &'a Bump,
    source_text_allocator: S,
    retain_or_omit_user_attributes_for_facts: bool,
    elaborate_xhp_namespaces_for_facts: bool,
) -> (
    Node<'a>,
    Vec<SyntaxError>,
    DirectDeclSmartConstructors<'a, 'o, 't, S>,
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
        elaborate_xhp_namespaces_for_facts,
    );
    let mut parser = Parser::new(source, env, sc);
    let root = parser.parse_script();
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    (root, errors, sc_state, mode_opt)
}
