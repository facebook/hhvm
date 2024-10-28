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
use oxidized::experimental_features;
use oxidized::file_info::Mode;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::namespace_env::Mode as NamespaceMode;
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
        default_unstable_features: HashSet<experimental_features::FeatureName>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.auto_namespace_map.clone(),
            env.mode,
            env.parser_options.disable_xhp_element_mangling,
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
        default_unstable_features: HashSet<experimental_features::FeatureName>,
    ) -> Result<ParserResult> {
        if let Some(err) = Self::verify_utf8(indexed_source_text) {
            return Ok(err);
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
        default_experimental_features: HashSet<experimental_features::FeatureName>,
    ) -> Result<ParserResult> {
        let ns = NamespaceEnv::empty(
            env.parser_options.auto_namespace_map.clone(),
            env.mode,
            env.parser_options.disable_xhp_element_mangling,
        );
        Self::from_tree_with_namespace_env(
            env,
            Arc::new(ns),
            indexed_source_text,
            arena,
            language,
            mode,
            tree,
            default_experimental_features,
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
            scoured_comments: Default::default(),
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
        default_experimental_features: HashSet<experimental_features::FeatureName>,
    ) -> Result<ParserResult> {
        let lowering_t = Instant::now();
        match language {
            Language::Hack => {}
            _ => return Err(Error::NotAHackFile()),
        }
        let mode = mode.unwrap_or(Mode::Mstrict);
        let scoured_comments =
            Self::scour_comments_and_add_fixmes(env, indexed_source_text, tree.root())?;
        // Unless we are type checking in quick mode, we will want to run the
        // syntax error checks from rust_parser_errors.rs. If we are type checking
        // and not in quick mode, we also work out if we are in an hhi.
        let (should_check_for_extra_errors, hhi_mode, mut syntax_errors) = match env.mode {
            NamespaceMode::ForCodegen => (
                true,
                false,
                tree.errors().into_iter().cloned().collect::<Vec<_>>(),
            ),
            NamespaceMode::ForTypecheck => {
                let first_error = tree
                    .errors()
                    .into_iter()
                    .take(1)
                    .cloned()
                    .collect::<Vec<_>>();
                if env.quick_mode {
                    (false, false, first_error)
                } else {
                    let hhi_mode = indexed_source_text
                        .source_text()
                        .file_path()
                        .has_extension("hhi");
                    (true, hhi_mode, first_error)
                }
            }
        };
        let (active_experimental_features, uses_readonly) = if should_check_for_extra_errors {
            stack_limit::reset();
            let (parse_errors, uses_readonly, active_experimental_features) =
                parse_errors_with_text(
                    &tree,
                    indexed_source_text.clone(),
                    // TODO(hrust) change to parser_otions to ref in ParserErrors
                    env.parser_options.clone(),
                    true, /* hhvm_compat_mode */
                    hhi_mode,
                    env.mode,
                    env.is_systemlib,
                    default_experimental_features,
                );
            syntax_errors.extend(parse_errors);
            syntax_errors.sort_by(SyntaxError::compare_offset);
            (active_experimental_features, uses_readonly)
        } else {
            // If we aren't doin the extra checks, then the lowerer will be called in quick
            // mode and won't report syntax errors, and so we don't need the active
            // experimental features
            (default_experimental_features, false)
        };
        let mut lowerer_env = lowerer::Env::make(
            env.mode,
            env.quick_mode,
            env.show_all_errors,
            mode,
            active_experimental_features,
            indexed_source_text,
            &env.parser_options,
            Arc::clone(&ns),
            TokenFactory::new(arena),
            arena,
        );
        stack_limit::reset();
        let aast = lower(&mut lowerer_env, tree.root());
        let (lowering_t, elaboration_t) = (lowering_t.elapsed(), Instant::now());
        let lower_peak = stack_limit::peak() as u64;
        let mut aast = if env.elaborate_namespaces {
            namespaces::toplevel_elaborator::elaborate_toplevel_defs(ns, aast)
        } else {
            aast
        };
        if should_check_for_extra_errors {
            if uses_readonly && !env.parser_options.no_parser_readonly_check {
                syntax_errors.extend(readonly_check::check_program(
                    &mut aast,
                    matches!(env.mode, NamespaceMode::ForTypecheck),
                ));
            }
            syntax_errors.extend(aast_check::check_program(&aast, env.mode));
            syntax_errors.extend(modules_check::check_program(&aast));
            syntax_errors.extend(expression_tree_check::check_splices(&aast));
            syntax_errors.extend(coeffects_check::check_program(&aast, env.mode));
        }
        let (elaboration_t, error_t) = (elaboration_t.elapsed(), Instant::now());
        let error_peak = stack_limit::peak() as u64;
        let lowerer_parsing_errors = lowerer_env.parsing_errors().to_vec();
        let errors = lowerer_env.hh_errors().to_vec();
        let lint_errors = lowerer_env.lint_errors().to_vec();
        let error_t = error_t.elapsed();

        Ok(ParserResult {
            file_mode: mode,
            scoured_comments,
            aast,
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
            codegen: matches!(env.mode, NamespaceMode::ForCodegen),
            hhvm_compat_mode: matches!(env.mode, NamespaceMode::ForCodegen),
            php5_compat_mode: env.php5_compat_mode,
            enable_xhp_class_modifier: env.parser_options.enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.parser_options.disable_xhp_element_mangling,
            disable_xhp_children_declarations: env.parser_options.disable_xhp_children_declarations,
            interpret_soft_types_as_like_types: env
                .parser_options
                .interpret_soft_types_as_like_types,
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
            None | Some(Mode::Mhhi) => matches!(env.mode, NamespaceMode::ForTypecheck),
            _ => matches!(env.mode, NamespaceMode::ForTypecheck) && env.quick_mode,
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
                    disable_hh_ignore_error: env.parser_options.disable_hh_ignore_error,
                    allowed_decl_fixme_codes: &env.parser_options.allowed_decl_fixme_codes,
                };
            Ok(scourer.scour_comments(script))
        } else {
            Ok(Default::default())
        }
    }
}
