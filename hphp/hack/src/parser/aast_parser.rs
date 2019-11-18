// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_check;
use crate::rust_aast_parser_types::{Env, Result as ParserResult};
use coroutine_smart_constructors::{CoroutineSmartConstructors, State as CoroutineState};
use decl_mode_parser::DeclModeParser;
use lowerer::{Lowerer, ScourComment};
use ocamlrep_derive::OcamlRep;
use oxidized::{aast::Program, file_info::Mode, pos::Pos, scoured_comments::ScouredComments};
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    syntax_error,
    syntax_error::SyntaxError,
};
use parser_rust::{parser_env::ParserEnv, smart_constructors_wrappers::WithKind};
use regex::bytes::Regex;
use rust_coroutine::coroutine_lowerer;
use rust_parser_errors::ParserErrors;
use stack_limit::StackLimit;
use std::borrow::Borrow;
use syntax_tree::{make_syntax_tree, mode_parser::parse_mode, SyntaxTree};

type SmartConstructor<'a> = WithKind<
    CoroutineSmartConstructors<'a, PositionedSyntax, CoroutineState<'a, PositionedSyntax>>,
>;
type State<'a> = CoroutineState<'a, PositionedSyntax>;
type PositionedSyntaxTree<'a> = SyntaxTree<'a, PositionedSyntax, State<'a>>;

struct PositionedSyntaxLowerer;
impl<'a> lowerer::Lowerer<'a, PositionedToken, PositionedValue> for PositionedSyntaxLowerer {}

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

    // Full_fidelity_ast.lower_tree_ is inlined
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
        if lower_coroutines && rewritten_source.is_none() {
            let rewritten_source = coroutine_lowerer::rewrite_coroutines(
                indexed_source_text.source_text(),
                tree.root(),
            );
            Self::from_text_(
                env,
                &indexed_source_text,
                Some(&IndexedSourceText::new(rewritten_source)),
                stack_limit,
            )
        } else {
            // TODO(shiqicao): we need to use rewritten source/tree when lowering, but original one when checking errors
            let indexed_source_text = rewritten_source.unwrap_or(indexed_source_text);
            let tree = if let Some(rewritten_source) = rewritten_source {
                let (_, rewritten_tree) = Self::parse_text(env, rewritten_source, stack_limit)?;
                rewritten_tree
            } else {
                tree
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
            );
            let ret = PositionedSyntaxLowerer::lower(&mut lowerer_env, tree.root());
            let syntax_errors = match &ret {
                Ok(aast) => Self::check_synatx_error(&env, indexed_source_text, tree, Some(aast)),
                Err(_) => Self::check_synatx_error(env, indexed_source_text, tree, None),
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

    fn check_synatx_error(
        env: &Env,
        indexed_source_text: &'a IndexedSourceText<'a>,
        tree: PositionedSyntaxTree<'a>,
        aast: Option<&Program<Pos, (), (), ()>>,
    ) -> Vec<SyntaxError> {
        let find_errors = |hhi_mode: bool| -> Vec<SyntaxError> {
            let mut errors = tree.errors().into_iter().cloned().collect::<Vec<_>>();
            errors.extend(ParserErrors::parse_errors(
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
    ) -> Result<(Option<Mode>, PositionedSyntaxTree<'a>)> {
        let source_text = indexed_source_text.source_text();
        let mode = parse_mode(indexed_source_text.source_text());
        if mode == Some(Mode::Mexperimental) && env.codegen && !env.hacksperimental {
            let e = SyntaxError::make(
                0,
                0,
                syntax_error::experimental_in_codegen_without_hacksperimental,
            );
            let p = Self::pos_of_error(indexed_source_text, &e);
            return Err(Error::ParserFatal(e, p));
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
            let tree = decl_parser.parse_script(stack_limit);
            PositionedSyntaxTree::create(
                source_text,
                tree,
                decl_parser.errors(),
                mode,
                State::new(source_text, env.codegen),
                None,
            )
        } else {
            make_syntax_tree::<SmartConstructor<'a>, State<'a>>(
                source_text,
                parser_env,
                stack_limit,
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
            incldue_line_comments: env.include_line_comments,
            disallowed_decl_fixmes: &env.parser_options.po_disallowed_decl_fixmes,
            ignored_fixme: ignored_fixme_regex.as_ref(),
        };
        Ok(scourer.scour_comments(script))
    }

    fn pos_of_error<'b>(source_text: &'b IndexedSourceText<'b>, e: &'b SyntaxError) -> Pos {
        source_text.relative_pos(e.start_offset, e.end_offset)
    }
}
