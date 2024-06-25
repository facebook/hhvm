// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use direct_decl_smart_constructors::DirectDeclSmartConstructors;
use direct_decl_smart_constructors::NoSourceTextAllocator;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::file_info;
use pair_smart_constructors::PairSmartConstructors;
use parser::parser::Parser;
use parser::syntax_by_ref;
use parser::syntax_by_ref::positioned_syntax::PositionedSyntax;
use parser::NoState;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_tree::SyntaxTree;

pub type ConcreteSyntaxTree<'src, 'arena> = SyntaxTree<'src, PositionedSyntax<'arena>, NoState>;

type CstSmartConstructors<'a> = positioned_smart_constructors::PositionedSmartConstructors<
    PositionedSyntax<'a>,
    syntax_by_ref::positioned_token::TokenFactory<'a>,
    syntax_by_ref::arena_state::State<'a>,
>;

pub fn parse_script<'a, 'o>(
    opts: &'o DeclParserOptions,
    env: ParserEnv,
    source: &'a SourceText<'a>,
    mode: Option<file_info::Mode>,
    arena: &'a Bump,
) -> (
    ConcreteSyntaxTree<'a, 'a>,
    direct_decl_parser::ParsedFile<'a>,
) {
    let sc0 = {
        let tf = syntax_by_ref::positioned_token::TokenFactory::new(arena);
        let state = syntax_by_ref::arena_state::State { arena };
        CstSmartConstructors::new(state, tf)
    };
    let sc1 = DirectDeclSmartConstructors::new(
        opts,
        source,
        mode.unwrap_or(file_info::Mode::Mstrict),
        arena,
        NoSourceTextAllocator,
        false, // elaborate_xhp_namespaces_for_facts
    );
    let sc = PairSmartConstructors::new(sc0, sc1);
    let mut parser = Parser::new(source, env, sc);
    let root = parser.parse_script();
    let errors = parser.errors();
    let has_first_pass_parse_errors = !errors.is_empty();
    let sc_state = parser.into_sc_state();
    let cst = ConcreteSyntaxTree::build(source, root.0, errors, mode.map(Into::into), NoState);
    let file_attributes = sc_state.1.file_attributes;
    let mut attrs = bumpalo::collections::Vec::with_capacity_in(file_attributes.len(), arena);
    attrs.extend(file_attributes.iter().copied());
    // Direct decl parser populates state.file_attributes in reverse of
    // syntactic order, so reverse it.
    attrs.reverse();
    let parsed_file = ParsedFile {
        mode,
        file_attributes: attrs.into_bump_slice(),
        decls: sc_state.1.decls,
        has_first_pass_parse_errors,
        disable_xhp_element_mangling: opts.disable_xhp_element_mangling,
    };
    (cst, parsed_file)
}
