// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_check;
use coroutine_smart_constructors::State as CoroutineState;
use itertools::{
    Either,
    Either::{Left, Right},
};
use lowerer::{lower, ScourComment};
use mode_parser::parse_mode;
use namespaces_rust as namespaces;
use ocamlrep_derive::OcamlRep;
use oxidized::{
    aast::Program, file_info::Mode, pos::Pos, scoured_comments::ScouredComments, uast::AstAnnot,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    parser_env::ParserEnv,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    syntax_error::SyntaxError,
    syntax_tree::SyntaxTree,
};
use regex::bytes::Regex;
use rust_aast_parser_types::{Env, Result as ParserResult};
use rust_parser_errors::parse_errors;
use stack_limit::StackLimit;
use std::borrow::Borrow;

type State<'a> = CoroutineState<'a, PositionedSyntax>;
type PositionedSyntaxTreeWithCoroutineState<'a> = SyntaxTree<'a, PositionedSyntax, State<'a>>;
type PositionedSyntaxTree<'a> = SyntaxTree<'a, PositionedSyntax, ()>;

#[derive(Debug, OcamlRep)]
pub enum Error {
    ParserFatal(SyntaxError, Pos),
    Other(String),
}

impl<T: ToString> From<T> for Error {
    fn from(s: T) -> Self {
        Error::Other(s.to_string())
    }
}

type Result<T> = std::result::Result<T, Error>;

pub struct AastParser;
impl<'a> AastParser {
    pub fn from_text(
        env: &Env,
        source: &'a IndexedSourceText<'a>,
        stack_limit: Option<&'a StackLimit>,
    ) -> Result<ParserResult> {
        Self::from_text_(env, source, None, stack_limit)
    }

    /// `impl Trait` isn't allowed outside of fun signature, this function is to hint Rust type checker
    fn to_as_ref<'b>(
        tree: Either<PositionedSyntaxTree<'b>, &'b PositionedSyntaxTree<'b>>,
    ) -> impl AsRef<PositionedSyntaxTree<'b>> + 'b {
        tree
    }

    fn from_text_(
        env: &Env,
        indexed_source_text: &'a IndexedSourceText<'a>,
        rewritten_source: Option<&'a IndexedSourceText<'a>>,
        stack_limit: Option<&'a StackLimit>,
    ) -> Result<ParserResult> {
        let (mode, tree) = Self::parse_text(env, indexed_source_text, stack_limit)?;
        let mode = mode.unwrap_or(Mode::Mpartial);
        let scoured_comments =
            Self::scour_comments_and_add_fixmes(env, indexed_source_text, &tree.root())?;
        let lower_coroutines = env.lower_coroutines && env.codegen && tree.sc_state().seen_ppl();
        let tree = tree.drop_state();
        if lower_coroutines && rewritten_source.is_none() {
            let (source, rewritten_source) = coroutine_lowerer::rewrite_coroutines(
                indexed_source_text.source_text(),
                tree.root(),
            );
            Self::from_text_(
                env,
                &IndexedSourceText::new(source),
                Some(&IndexedSourceText::new(rewritten_source)),
                stack_limit,
            )
        } else {
            let indexed_source_text = rewritten_source.unwrap_or(indexed_source_text);
            let original_tree = &tree;
            let tree = if let Some(rewritten_source) = rewritten_source {
                let (_, rewritten_tree) = Self::parse_text(env, rewritten_source, stack_limit)?;
                Left(rewritten_tree.drop_state())
            } else {
                Right(&tree)
            };
            let mut lowerer_env = lowerer::Env::make(
                env.codegen,
                env.quick_mode,
                env.keep_errors,
                env.show_all_errors,
                env.fail_open,
                !scoured_comments.error_pos.is_empty(), /* disable_lowering_parsing_error */
                mode,
                indexed_source_text,
                &env.parser_options,
                env.elaborate_namespaces,
                stack_limit,
            );
            let ret = lower(&mut lowerer_env, Self::to_as_ref(tree).as_ref().root());
            let ret = if env.parser_options.po_rust_top_level_elaborator && env.elaborate_namespaces
            {
                ret.map(|ast| {
                    namespaces::toplevel_elaborator::elaborate_toplevel_defs::<AstAnnot>(
                        &env.parser_options,
                        ast,
                    )
                })
            } else {
                ret
            };
            let syntax_errors = match &ret {
                Ok(aast) => {
                    Self::check_syntax_error(&env, indexed_source_text, original_tree, Some(aast))
                }
                Err(_) => Self::check_syntax_error(env, indexed_source_text, original_tree, None),
            };
            let lowpri_errors = lowerer_env.lowpri_errors().borrow().to_vec();
            let errors = lowerer_env.hh_errors().borrow().to_vec();
            let lint_errors = lowerer_env.lint_errors().borrow().to_vec();

            Ok(ParserResult {
                file_mode: mode,
                scoured_comments,
                aast: ret,
                lowpri_errors,
                syntax_errors,
                errors,
                lint_errors,
            })
        }
    }

    fn check_syntax_error(
        env: &Env,
        indexed_source_text: &'a IndexedSourceText<'a>,
        tree: &'a PositionedSyntaxTree<'a>,
        aast: Option<&Program<Pos, (), (), ()>>,
    ) -> Vec<SyntaxError> {
        let find_errors = |hhi_mode: bool| -> Vec<SyntaxError> {
            let mut errors = tree.errors().into_iter().cloned().collect::<Vec<_>>();
            errors.extend(parse_errors(
                &tree,
                // TODO(hrust) change to parser_otions to ref in ParserErrors
                env.parser_options.clone(),
                true, /* hhvm_compat_mode */
                hhi_mode,
                env.codegen,
            ));
            errors.sort_by(SyntaxError::compare_offset);
            errors.extend(aast.map_or(vec![], aast_check::check_program));
            errors
        };
        if env.codegen {
            find_errors(false /* hhi_mode */)
        } else if env.keep_errors {
            let first_error = tree.errors().into_iter().next();
            match first_error {
                None if !env.quick_mode && !env.parser_options.po_parser_errors_only => {
                    let is_hhi = indexed_source_text
                        .source_text()
                        .file_path()
                        .has_extension("hhi");
                    find_errors(is_hhi)
                }
                None => vec![],
                Some(e) => vec![e.clone()],
            }
        } else {
            vec![]
        }
    }

    fn parse_text(
        env: &Env,
        indexed_source_text: &'a IndexedSourceText<'a>,
        stack_limit: Option<&'a StackLimit>,
    ) -> Result<(Option<Mode>, PositionedSyntaxTreeWithCoroutineState<'a>)> {
        let source_text = indexed_source_text.source_text();
        let mut mode = parse_mode(indexed_source_text.source_text());
        if mode == Some(Mode::Mpartial) && env.parser_options.po_disable_modes {
            mode = Some(Mode::Mstrict);
        }
        let quick_mode = match mode {
            None | Some(Mode::Mdecl) | Some(Mode::Mphp) => !env.codegen,
            _ => !env.codegen && env.quick_mode,
        };
        let parser_env = ParserEnv {
            codegen: env.codegen,
            hhvm_compat_mode: env.codegen,
            php5_compat_mode: env.php5_compat_mode,
            allow_new_attribute_syntax: env.parser_options.po_allow_new_attribute_syntax,
            enable_xhp_class_modifier: env.parser_options.po_enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.parser_options.po_disable_xhp_element_mangling,
            disable_xhp_children_declarations: env
                .parser_options
                .po_disable_xhp_children_declarations,
            disable_modes: env.parser_options.po_disable_modes,
        };

        let tree = if quick_mode {
            let (tree, errors, _state) =
                decl_mode_parser::parse_script(source_text, parser_env, stack_limit);
            PositionedSyntaxTreeWithCoroutineState::create(
                source_text,
                tree,
                errors,
                mode,
                State::new(source_text, env.codegen),
                None,
            )
        } else {
            let (tree, errors, state) =
                coroutine_parser_leak_tree::parse_script(source_text, parser_env, stack_limit);
            PositionedSyntaxTreeWithCoroutineState::create(
                source_text,
                tree,
                errors,
                mode,
                state,
                None,
            )
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
            include_line_comments: env.include_line_comments,
            disallowed_decl_fixmes: &env.parser_options.po_disallowed_decl_fixmes,
            ignored_fixme: ignored_fixme_regex.as_ref(),
        };
        Ok(scourer.scour_comments(script))
    }
}
