// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use direct_decl_smart_constructors::{self, DirectDeclSmartConstructors, NoSourceTextAllocator};
use oxidized_by_ref::{
    decl_parser_options::DeclParserOptions, direct_decl_parser::Decls, file_info,
};
use pair_smart_constructors::PairSmartConstructors;
use parser::{
    parser::Parser,
    syntax_by_ref::{self, positioned_syntax::PositionedSyntax},
    NoState,
};
use parser_core_types::{parser_env::ParserEnv, source_text::SourceText, syntax_tree::SyntaxTree};
use stack_limit::StackLimit;

pub type ConcreteSyntaxTree<'src, 'arena> = SyntaxTree<'src, PositionedSyntax<'arena>, NoState>;

type CstSmartConstructors<'a> = positioned_smart_constructors::PositionedSmartConstructors<
    PositionedSyntax<'a>,
    syntax_by_ref::positioned_token::TokenFactory<'a>,
    syntax_by_ref::arena_state::State<'a>,
>;

pub fn parse_script<'a>(
    opts: &'a DeclParserOptions<'a>,
    env: ParserEnv,
    source: &'a SourceText<'a>,
    mode: Option<file_info::Mode>,
    arena: &'a Bump,
    stack_limit: Option<&'a StackLimit>,
) -> (ConcreteSyntaxTree<'a, 'a>, Decls<'a>) {
    let sc0 = {
        let tf = syntax_by_ref::positioned_token::TokenFactory::new(arena);
        let state = syntax_by_ref::arena_state::State { arena };
        CstSmartConstructors::new(state, tf)
    };
    let sc1 = {
        let mode = mode.unwrap_or(file_info::Mode::Mstrict);
        DirectDeclSmartConstructors::new(
            opts,
            &source,
            mode,
            arena,
            NoSourceTextAllocator,
            true,  // omit_user_attributes_irrelevant_to_typechecking
            false, // simplify_naming_for_facts
        )
    };
    let sc = PairSmartConstructors::new(sc0, sc1);
    let mut parser = Parser::new(&source, env, sc);
    let root = parser.parse_script(stack_limit);
    let errors = parser.errors();
    let sc_state = parser.into_sc_state();
    let decls = sc_state.1.decls;
    let tree = ConcreteSyntaxTree::build(source, root.0, errors, mode, NoState, None);
    (tree, decls)
}
