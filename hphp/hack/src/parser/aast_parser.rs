// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use coroutine_smart_constructors::{CoroutineSmartConstructors, State as CoroutineState};
use decl_mode_parser::DeclModeParser;
use lowerer::{Lowerer, ScourComment};
use oxidized::{
    file_info::Mode,
    rust_aast_parser_types::{Env, Result as ParserResult},
    scoured_comments::ScouredComments,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    source_text::SourceText,
};
use parser_rust::{parser_env::ParserEnv, smart_constructors_wrappers::WithKind};
use regex::bytes::Regex;
use syntax_tree::{make_syntax_tree, mode_parser::parse_mode, SyntaxTree};

type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;
type SmartConstructor<'a> = WithKind<
    CoroutineSmartConstructors<'a, PositionedSyntax, CoroutineState<'a, PositionedSyntax>>,
>;
type State<'a> = CoroutineState<'a, PositionedSyntax>;
type PositionedSyntaxTree<'a> = SyntaxTree<'a, PositionedSyntax, State<'a>>;

struct PositionedSyntaxLowerer;
impl<'a> lowerer::Lowerer<'a, PositionedToken, PositionedValue> for PositionedSyntaxLowerer {}

pub struct AastParser;
impl<'a> AastParser {
    // Full_fidelity_ast.lower_tree_ is inlined
    pub fn from_text(env: &Env, source: &'a IndexedSourceText<'a>) -> Result<ParserResult> {
        let (mode, tree) = Self::parse_text(env, source.source_text())?;
        let _scoured_comments = Self::scour_comments_and_add_fixmes(env, source, &tree.root())?;
        let lower_coroutines = env.lower_coroutines && env.codegen && tree.sc_state().seen_ppl();
        if lower_coroutines {
            // TODO:
            panic!("not impl");
        } else {
            let mut env = lowerer::Env::make(
                env.codegen,
                env.elaborate_namespaces,
                env.quick_mode,
                mode.unwrap_or(Mode::Mpartial),
                source,
                &env.parser_options,
            );
            let ret = PositionedSyntaxLowerer::lower(&mut env, tree.root())?;
            Ok(ParserResult { aast: ret.aast })
        }
    }

    fn _check_synatx_error() {
        // TODO:
    }

    fn parse_text(
        env: &Env,
        source_text: &'a SourceText,
    ) -> Result<(Option<Mode>, PositionedSyntaxTree<'a>)> {
        let mode = parse_mode(source_text);
        if mode == Some(Mode::Mexperimental) && env.codegen && !env.hacksperimental {
            return Err("TODO: SyntaxError".into());
        }
        let quick_mode = match mode {
            None | Some(Mode::Mdecl) | Some(Mode::Mphp) => !env.codegen,
            _ => !env.codegen && env.quick_mode,
        };
        let parser_env = ParserEnv {
            codegen: env.codegen,
            is_experimental_mode: mode == Some(Mode::Mexperimental),
            hhvm_compat_mode: env.codegen,
            php5_compat_mode: env.php5_compat_mode,
            allow_new_attribute_syntax: env.parser_options.po_allow_new_attribute_syntax,
        };

        let tree = if quick_mode {
            let mut decl_parser = DeclModeParser::make(source_text, parser_env);
            let tree = decl_parser.parse_script(None);
            PositionedSyntaxTree::create(
                source_text,
                tree,
                decl_parser.errors(),
                mode,
                State::new(source_text, env.codegen),
                None,
            )
        } else {
            make_syntax_tree::<SmartConstructor<'a>, State<'a>>(source_text, parser_env)
        };
        Ok((mode, tree))
    }

    fn scour_comments_and_add_fixmes(
        env: &Env,
        indexed_source_text: &'a IndexedSourceText,
        script: &PositionedSyntax,
    ) -> Result<ScouredComments> {
        let ignored_fixme_regex = env
            .parser_options
            .ignored_fixme_regex
            .as_ref()
            .map(|r| Regex::new(&r))
            .transpose()?;
        let scourer: ScourComment<PositionedToken, PositionedValue> = ScourComment {
            phantom: std::marker::PhantomData,
            indexed_source_text,
            collect_fixmes: env.keep_errors,
            incldue_line_comments: env.include_line_comments,
            disallowed_decl_fixmes: &env.parser_options.po_disallowed_decl_fixmes,
            ignored_fixme: ignored_fixme_regex.as_ref(),
        };
        Ok(scourer.scour_comments(script))
    }
}
