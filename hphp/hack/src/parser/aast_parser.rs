// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_check;
use crate::coeffects_check;
use crate::expression_tree_check;
use crate::readonly_check;
use bumpalo::Bump;
use lowerer::{lower, ScourComment};
use mode_parser::{parse_mode, Language};
use namespaces_rust as namespaces;
use ocamlrep::rc::RcOc;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{
    aast::Program, file_info::Mode, namespace_env::Env as NamespaceEnv, pos::Pos,
    scoured_comments::ScouredComments,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    parser_env::ParserEnv,
    source_text::SourceText,
    syntax_by_ref::{
        positioned_syntax::PositionedSyntax,
        positioned_token::{PositionedToken, TokenFactory},
        positioned_value::PositionedValue,
    },
    syntax_error::SyntaxError,
    syntax_tree::SyntaxTree,
};
pub use rust_aast_parser_types::{Env, ParserResult};
use rust_parser_errors::parse_errors_with_text;
use smart_constructors::NoState;
use stack_limit::StackLimit;
use std::borrow::Borrow;

type PositionedSyntaxTree<'src, 'arena> = SyntaxTree<'src, PositionedSyntax<'arena>, NoState>;

#[derive(Debug, FromOcamlRep, ToOcamlRep)]
pub enum Error {
    NotAHackFile(),
    ParserFatal(SyntaxError, Pos),
    Other(String),
}

impl<T: ToString> From<T> for Error {
    fn from(s: T) -> Self {
        Error::Other(s.to_string())
    }
}

pub type Result<T, E = Error> = std::result::Result<T, E>;

pub struct AastParser;
impl<'src> AastParser {
    pub fn from_text(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        stack_limit: Option<&StackLimit>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.po_auto_namespace_map.clone(),
            env.codegen,
            env.parser_options.po_disable_xhp_element_mangling,
        );
        Self::from_text_with_namespace_env(env, RcOc::new(ns), indexed_source_text, stack_limit)
    }

    pub fn from_text_with_namespace_env(
        env: &Env,
        ns: RcOc<NamespaceEnv>,
        indexed_source_text: &'src IndexedSourceText<'src>,
        stack_limit: Option<&StackLimit>,
    ) -> Result<ParserResult> {
        let arena = Bump::new();
        let (language, mode, tree) =
            Self::parse_text(&arena, env, indexed_source_text, stack_limit)?;
        let parse_peak = stack_limit.map_or(0, |sl| {
            let x = sl.peak();
            sl.reset();
            x
        });
        Self::from_tree_with_namespace_env(
            env,
            ns,
            indexed_source_text,
            stack_limit,
            &arena,
            language,
            mode,
            tree,
        )
        .map(|mut pr| {
            pr.parse_peak = parse_peak as i64;
            pr
        })
    }

    pub fn from_tree<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        stack_limit: Option<&StackLimit>,
        arena: &'arena Bump,
        language: Language,
        mode: Option<Mode>,
        tree: PositionedSyntaxTree<'src, 'arena>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.po_auto_namespace_map.clone(),
            env.codegen,
            env.parser_options.po_disable_xhp_element_mangling,
        );
        Self::from_tree_with_namespace_env(
            env,
            RcOc::new(ns),
            indexed_source_text,
            stack_limit,
            arena,
            language,
            mode,
            tree,
        )
    }

    pub fn from_tree_with_namespace_env<'arena>(
        env: &Env,
        ns: RcOc<NamespaceEnv>,
        indexed_source_text: &'src IndexedSourceText<'src>,
        stack_limit: Option<&StackLimit>,
        arena: &'arena Bump,
        language: Language,
        mode: Option<Mode>,
        tree: PositionedSyntaxTree<'src, 'arena>,
    ) -> Result<ParserResult> {
        match language {
            Language::Hack => {}
            _ => return Err(Error::NotAHackFile()),
        }
        let mode = mode.unwrap_or(Mode::Mstrict);
        let scoured_comments =
            Self::scour_comments_and_add_fixmes(env, indexed_source_text, tree.root())?;
        let mut lowerer_env = lowerer::Env::make(
            env.codegen,
            env.quick_mode,
            env.keep_errors,
            env.show_all_errors,
            env.fail_open,
            mode,
            indexed_source_text,
            &env.parser_options,
            RcOc::clone(&ns),
            stack_limit,
            TokenFactory::new(arena),
            arena,
        );
        let ret = lower(&mut lowerer_env, tree.root());
        let lower_peak = stack_limit.map_or(0, |sl| {
            let x = sl.peak();
            sl.reset();
            x
        });
        let mut ret = if env.elaborate_namespaces {
            ret.map(|ast| namespaces::toplevel_elaborator::elaborate_toplevel_defs(ns, ast))
        } else {
            ret
        };
        let syntax_errors = match &mut ret {
            Ok(aast) => {
                Self::check_syntax_error(env, indexed_source_text, &tree, Some(aast), stack_limit)
            }
            Err(_) => Self::check_syntax_error(env, indexed_source_text, &tree, None, stack_limit),
        };
        let error_peak = stack_limit.map_or(0, |sl| {
            let x = sl.peak();
            sl.reset();
            x
        });
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
            parse_peak: 0,
            lower_peak: lower_peak as i64,
            error_peak: error_peak as i64,
            arena_bytes: arena.allocated_bytes() as i64,
        })
    }

    fn check_syntax_error<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        tree: &PositionedSyntaxTree<'src, 'arena>,
        aast: Option<&mut Program<(), ()>>,
        stack_limit: Option<&StackLimit>,
    ) -> Vec<SyntaxError> {
        let find_errors = |hhi_mode: bool| -> Vec<SyntaxError> {
            let mut errors = tree.errors().into_iter().cloned().collect::<Vec<_>>();
            let (parse_errors, uses_readonly) = parse_errors_with_text(
                tree,
                indexed_source_text.clone(),
                // TODO(hrust) change to parser_otions to ref in ParserErrors
                env.parser_options.clone(),
                true, /* hhvm_compat_mode */
                hhi_mode,
                env.codegen,
                env.is_systemlib,
                stack_limit,
            );
            errors.extend(parse_errors);
            errors.sort_by(SyntaxError::compare_offset);

            let mut empty_program = Program(vec![]);
            let aast = aast.unwrap_or(&mut empty_program);
            if uses_readonly {
                errors.extend(readonly_check::check_program(aast, !env.codegen));
            }
            errors.extend(aast_check::check_program(aast, !env.codegen));
            errors.extend(expression_tree_check::check_splices(aast));
            errors.extend(coeffects_check::check_program(aast, !env.codegen));

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

    fn parse_text<'arena>(
        arena: &'arena Bump,
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        stack_limit: Option<&StackLimit>,
    ) -> Result<(Language, Option<Mode>, PositionedSyntaxTree<'src, 'arena>)> {
        let source_text = indexed_source_text.source_text();
        let (language, mode, parser_env) = Self::make_parser_env(env, source_text);
        let tree = Self::parse(arena, env, parser_env, source_text, mode, stack_limit)?;
        Ok((language, mode, tree))
    }

    pub fn make_parser_env(
        env: &Env,
        source_text: &'src SourceText<'src>,
    ) -> (Language, Option<Mode>, ParserEnv) {
        let (language, mode) = parse_mode(source_text);
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
            disallow_fun_and_cls_meth_pseudo_funcs: env
                .parser_options
                .po_disallow_fun_and_cls_meth_pseudo_funcs,
            interpret_soft_types_as_like_types: env
                .parser_options
                .po_interpret_soft_types_as_like_types,
        };
        (language, mode, parser_env)
    }

    fn parse<'arena>(
        arena: &'arena Bump,
        env: &Env,
        parser_env: ParserEnv,
        source_text: &'src SourceText<'src>,
        mode: Option<Mode>,
        stack_limit: Option<&StackLimit>,
    ) -> Result<PositionedSyntaxTree<'src, 'arena>> {
        let quick_mode = match mode {
            None | Some(Mode::Mhhi) => !env.codegen,
            _ => !env.codegen && env.quick_mode,
        };
        let tree = if quick_mode {
            let (tree, errors, _state) =
                decl_mode_parser::parse_script(arena, source_text, parser_env, stack_limit);
            PositionedSyntaxTree::create(source_text, tree, errors, mode, NoState, None)
        } else {
            let (tree, errors, _state) =
                positioned_by_ref_parser::parse_script(arena, source_text, parser_env, stack_limit);
            PositionedSyntaxTree::create(source_text, tree, errors, mode, NoState, None)
        };
        Ok(tree)
    }

    fn scour_comments_and_add_fixmes<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'_>,
        script: &PositionedSyntax<'arena>,
    ) -> Result<ScouredComments> {
        let scourer: ScourComment<'_, PositionedToken<'arena>, PositionedValue<'arena>> =
            ScourComment {
                phantom: std::marker::PhantomData,
                indexed_source_text,
                collect_fixmes: env.keep_errors,
                include_line_comments: env.include_line_comments,
                disable_hh_ignore_error: env.parser_options.po_disable_hh_ignore_error,
                allowed_decl_fixme_codes: &env.parser_options.po_allowed_decl_fixme_codes,
            };
        Ok(scourer.scour_comments(script))
    }
}
