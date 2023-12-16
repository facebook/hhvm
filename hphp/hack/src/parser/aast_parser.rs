// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;
use std::time::Instant;

use bumpalo::Bump;
use hash::HashSet;
use lowerer::lower;
use lowerer::ScourComment;
use mode_parser::parse_mode;
use mode_parser::Language;
use namespaces_rust as namespaces;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use oxidized::aast::Program;
use oxidized::file_info::Mode;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::pos::Pos;
use oxidized::scoured_comments::ScouredComments;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_by_ref::positioned_syntax::PositionedSyntax;
use parser_core_types::syntax_by_ref::positioned_token::PositionedToken;
use parser_core_types::syntax_by_ref::positioned_token::TokenFactory;
use parser_core_types::syntax_by_ref::positioned_value::PositionedValue;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_tree::SyntaxTree;
pub use rust_aast_parser_types::Env;
pub use rust_aast_parser_types::ParserProfile;
pub use rust_aast_parser_types::ParserResult;
use rust_parser_errors::parse_errors_with_text;
use smart_constructors::NoState;

use crate::aast_check;
use crate::coeffects_check;
use crate::expression_tree_check;
use crate::modules_check;
use crate::readonly_check;

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
        default_unstable_features: HashSet<rust_parser_errors::UnstableFeatures>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.po_auto_namespace_map.clone(),
            env.codegen,
            env.parser_options.po_disable_xhp_element_mangling,
        );
        Self::from_text_with_namespace_env(
            env,
            Arc::new(ns),
            indexed_source_text,
            default_unstable_features,
        )
    }

    pub fn from_text_with_namespace_env(
        env: &Env,
        ns: Arc<NamespaceEnv>,
        indexed_source_text: &'src IndexedSourceText<'src>,
        default_unstable_features: HashSet<rust_parser_errors::UnstableFeatures>,
    ) -> Result<ParserResult> {
        // TODO(T120858428): remove this check (and always verify_utf8) once
        // we're strict everywhere!
        if env.parser_options.po_strict_utf8 {
            if let Some(err) = Self::verify_utf8(indexed_source_text) {
                return Ok(err);
            }
        }
        let start_t = Instant::now();
        let arena = Bump::new();
        stack_limit::reset();
        let (language, mode, tree) = Self::parse_text(&arena, env, indexed_source_text)?;
        let parsing_t = start_t.elapsed();
        let parse_peak = stack_limit::peak();
        let mut pr = Self::from_tree_with_namespace_env(
            env,
            ns,
            indexed_source_text,
            &arena,
            language,
            mode,
            tree,
            default_unstable_features,
        )?;

        pr.profile.parse_peak = parse_peak as u64;
        pr.profile.parsing_t = parsing_t;
        pr.profile.total_t = start_t.elapsed();
        Ok(pr)
    }

    pub fn from_tree<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        arena: &'arena Bump,
        language: Language,
        mode: Option<Mode>,
        tree: PositionedSyntaxTree<'src, 'arena>,
        default_unstable_features: HashSet<rust_parser_errors::UnstableFeatures>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.po_auto_namespace_map.clone(),
            env.codegen,
            env.parser_options.po_disable_xhp_element_mangling,
        );
        Self::from_tree_with_namespace_env(
            env,
            Arc::new(ns),
            indexed_source_text,
            arena,
            language,
            mode,
            tree,
            default_unstable_features,
        )
    }

    fn verify_utf8(text: &IndexedSourceText<'_>) -> Option<ParserResult> {
        let bytes = text.source_text().text();
        let Err(error) = std::str::from_utf8(bytes) else {
            return None;
        };

        let (_, _, offset) = text.offset_to_file_pos_triple(error.valid_up_to());

        let err = SyntaxError::make(offset, offset + 1, "Invalid utf8 sequence".into(), vec![]);
        Some(ParserResult {
            file_mode: Mode::Mstrict,
            scoured_comments: default_scoured_comments(),
            aast: Default::default(),
            lowerer_parsing_errors: Default::default(),
            syntax_errors: vec![err],
            errors: Default::default(),
            lint_errors: Default::default(),
            profile: Default::default(),
        })
    }

    fn from_tree_with_namespace_env<'arena>(
        env: &Env,
        ns: Arc<NamespaceEnv>,
        indexed_source_text: &'src IndexedSourceText<'src>,
        arena: &'arena Bump,
        language: Language,
        mode: Option<Mode>,
        tree: PositionedSyntaxTree<'src, 'arena>,
        default_unstable_features: HashSet<rust_parser_errors::UnstableFeatures>,
    ) -> Result<ParserResult> {
        let lowering_t = Instant::now();
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
            env.show_all_errors,
            mode,
            indexed_source_text,
            &env.parser_options,
            Arc::clone(&ns),
            TokenFactory::new(arena),
            arena,
        );
        stack_limit::reset();
        let ret = lower(&mut lowerer_env, tree.root());
        let (lowering_t, elaboration_t) = (lowering_t.elapsed(), Instant::now());
        let lower_peak = stack_limit::peak() as u64;
        let mut ret = if env.elaborate_namespaces {
            namespaces::toplevel_elaborator::elaborate_toplevel_defs(ns, ret)
        } else {
            ret
        };
        let (elaboration_t, error_t) = (elaboration_t.elapsed(), Instant::now());
        stack_limit::reset();
        let syntax_errors = Self::check_syntax_error(
            env,
            indexed_source_text,
            &tree,
            Some(&mut ret),
            default_unstable_features,
        );
        let error_peak = stack_limit::peak() as u64;
        let lowerer_parsing_errors = lowerer_env.parsing_errors().to_vec();
        let errors = lowerer_env.hh_errors().to_vec();
        let lint_errors = lowerer_env.lint_errors().to_vec();
        let error_t = error_t.elapsed();

        Ok(ParserResult {
            file_mode: mode,
            scoured_comments,
            aast: ret,
            lowerer_parsing_errors,
            syntax_errors,
            errors,
            lint_errors,
            profile: ParserProfile {
                lower_peak,
                lowering_t,
                elaboration_t,
                error_t,
                error_peak,
                arena_bytes: arena.allocated_bytes() as u64,
                ..Default::default()
            },
        })
    }

    fn check_syntax_error<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
        tree: &PositionedSyntaxTree<'src, 'arena>,
        aast: Option<&mut Program<(), ()>>,
        default_unstable_features: HashSet<rust_parser_errors::UnstableFeatures>,
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
                default_unstable_features,
            );
            errors.extend(parse_errors);
            errors.sort_by(SyntaxError::compare_offset);

            let mut empty_program = Program(vec![]);
            let aast = aast.unwrap_or(&mut empty_program);
            if uses_readonly && !env.parser_options.tco_no_parser_readonly_check {
                errors.extend(readonly_check::check_program(aast, !env.codegen));
            }
            errors.extend(aast_check::check_program(aast, !env.codegen));
            errors.extend(modules_check::check_program(aast));
            errors.extend(expression_tree_check::check_splices(aast));
            errors.extend(coeffects_check::check_program(aast, !env.codegen));
            errors
        };
        if env.codegen {
            find_errors(false /* hhi_mode */)
        } else {
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
        }
    }

    fn parse_text<'arena>(
        arena: &'arena Bump,
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'src>,
    ) -> Result<(Language, Option<Mode>, PositionedSyntaxTree<'src, 'arena>)> {
        let source_text = indexed_source_text.source_text();
        let (language, mode, parser_env) = Self::make_parser_env(env, source_text);
        let tree = Self::parse(arena, env, parser_env, source_text, mode)?;
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
            enable_xhp_class_modifier: env.parser_options.po_enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.parser_options.po_disable_xhp_element_mangling,
            disable_xhp_children_declarations: env
                .parser_options
                .po_disable_xhp_children_declarations,
            interpret_soft_types_as_like_types: env
                .parser_options
                .po_interpret_soft_types_as_like_types,
            nameof_precedence: env.parser_options.po_nameof_precedence,
            strict_utf8: env.parser_options.po_strict_utf8,
        };
        (language, mode.map(Into::into), parser_env)
    }

    fn parse<'arena>(
        arena: &'arena Bump,
        env: &Env,
        parser_env: ParserEnv,
        source_text: &'src SourceText<'src>,
        mode: Option<Mode>,
    ) -> Result<PositionedSyntaxTree<'src, 'arena>> {
        let quick_mode = match mode {
            None | Some(Mode::Mhhi) => !env.codegen,
            _ => !env.codegen && env.quick_mode,
        };
        let tree = if quick_mode {
            let (tree, errors, _state) =
                decl_mode_parser::parse_script(arena, source_text, parser_env);
            PositionedSyntaxTree::create(source_text, tree, errors, mode.map(Into::into), NoState)
        } else {
            let (tree, errors, _state) =
                positioned_by_ref_parser::parse_script(arena, source_text, parser_env);
            PositionedSyntaxTree::create(source_text, tree, errors, mode.map(Into::into), NoState)
        };
        Ok(tree)
    }

    fn scour_comments_and_add_fixmes<'arena>(
        env: &Env,
        indexed_source_text: &'src IndexedSourceText<'_>,
        script: &PositionedSyntax<'arena>,
    ) -> Result<ScouredComments> {
        if env.scour_comments {
            let scourer: ScourComment<'_, PositionedToken<'arena>, PositionedValue<'arena>> =
                ScourComment {
                    phantom: std::marker::PhantomData,
                    indexed_source_text,
                    include_line_comments: env.include_line_comments,
                    disable_hh_ignore_error: env.parser_options.po_disable_hh_ignore_error,
                    allowed_decl_fixme_codes: &env.parser_options.po_allowed_decl_fixme_codes,
                };
            Ok(scourer.scour_comments(script))
        } else {
            Ok(default_scoured_comments())
        }
    }
}

fn default_scoured_comments() -> ScouredComments {
    ScouredComments {
        comments: Default::default(),
        fixmes: Default::default(),
        misuses: Default::default(),
        error_pos: Default::default(),
        bad_ignore_pos: Default::default(),
    }
}
