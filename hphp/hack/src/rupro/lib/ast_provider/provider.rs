// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs;
use std::rc::Rc;

use owning_ref::RcRef;

use aast_parser::{AastParser, Error as ParserError};
use lint_rust::LintError;
use ocamlrep::rc::RcOc;
use parser_core_types::{
    indexed_source_text::IndexedSourceText, source_text::SourceText, syntax_error::SyntaxError,
};
use rust_aast_parser_types::Env as ParserEnv;

use crate::ast_provider::{AstCache, AstItem};
use crate::naming::Naming;
use crate::parsing_error::ParsingError;
use crate::pos::{RelativePath, RelativePathCtx};
use crate::sn_provider::SpecialNamesProvider;

struct MakeParserEnv {
    codegen: bool,
    php5_compat_mode: bool,
    elaborate_namespaces: bool,
    include_line_comments: bool,
    keep_errors: bool,
    quick_mode: bool,
    show_all_errors: bool,
    fail_open: bool,
    #[allow(unused)]
    disable_global_state_mutation: bool,
    is_systemlib: bool,
}

impl Default for MakeParserEnv {
    fn default() -> Self {
        Self {
            codegen: false,
            php5_compat_mode: false,
            elaborate_namespaces: true,
            include_line_comments: false,
            keep_errors: true,
            quick_mode: false,
            show_all_errors: false,
            fail_open: true,
            disable_global_state_mutation: false,
            is_systemlib: false,
        }
    }
}

#[derive(Debug)]
pub struct AstProvider {
    cache: Rc<dyn AstCache>,
    relative_path_ctx: Rc<RelativePathCtx>,
    special_names: Rc<SpecialNamesProvider>,
    parser_options: Rc<oxidized::parser_options::ParserOptions>,
}

impl AstProvider {
    pub fn new(
        cache: Rc<dyn AstCache>,
        relative_path_ctx: Rc<RelativePathCtx>,
        special_names: Rc<SpecialNamesProvider>,
        parser_options: Rc<oxidized::parser_options::ParserOptions>,
    ) -> Self {
        Self {
            cache,
            relative_path_ctx,
            special_names,
            parser_options,
        }
    }

    pub fn get_ast(&self, full: bool, name: &RelativePath) -> Result<Rc<AstItem>, ParsingError> {
        match self.cache.get_ast(name) {
            Some(ast) => Ok(Rc::clone(&ast)),
            None => {
                let ast = self.get_ast_no_cache(full, name)?;
                let ast = Rc::new(ast);
                self.cache.put_ast(name.clone(), Rc::clone(&ast));
                Ok(ast)
            }
        }
    }

    fn process_scour_comments(
        &self,
        _acc: &mut Vec<ParsingError>,
        sc: oxidized::scoured_comments::ScouredComments,
    ) {
        if !sc.error_pos.is_empty() {
            unimplemented!()
        }
    }

    fn process_syntax_errors(
        &self,
        acc: &mut Vec<ParsingError>,
        src: &IndexedSourceText<'_>,
        errs: Vec<SyntaxError>,
    ) {
        for e in errs {
            acc.push(ParsingError::ParsingError {
                pos: src.relative_pos(e.start_offset, e.end_offset),
                msg: e.message.into_owned(),
            });
        }
    }

    fn process_lowpri_errors(
        &self,
        acc: &mut Vec<ParsingError>,
        env: &ParserEnv,
        errs: Vec<(oxidized::pos::Pos, String)>,
    ) {
        if (!env.quick_mode || env.show_all_errors) && env.keep_errors {
            for (pos, msg) in errs {
                acc.push(ParsingError::ParsingError { pos, msg })
            }
        }
    }

    fn process_non_syntax_errors(
        &self,
        _acc: &mut Vec<ParsingError>,
        errs: Vec<oxidized::errors::Error>,
    ) {
        if !errs.is_empty() {
            unimplemented!()
        }
    }

    fn process_lint_errors(&self, errs: Vec<LintError>) {
        if !errs.is_empty() {
            unimplemented!()
        }
    }

    fn get_ast_no_cache(&self, full: bool, fln: &RelativePath) -> Result<AstItem, ParsingError> {
        let env = self.make_parser_env(MakeParserEnv {
            quick_mode: !full,
            keep_errors: true,
            ..Default::default()
        });
        let abs_fln = fln.to_absolute(&self.relative_path_ctx);
        let text = fs::read_to_string(&abs_fln).map_err(|io_err| ParsingError::IO {
            file: fln.clone(),
            err: format!("{:?}", io_err),
        })?;
        let rel_path = RcOc::new(oxidized::relative_path::RelativePath::make(
            oxidized::relative_path::Prefix::Dummy,
            abs_fln,
        ));
        let source_text = SourceText::make(rel_path, text.as_bytes());
        let indexed_source_text = IndexedSourceText::new(source_text);

        let parsed_file = stack_limit::with_elastic_stack(|stack_limit| {
            AastParser::from_text(&env, &indexed_source_text, Some(stack_limit))
        })
        .map_err(|failure| ParsingError::StackExceeded {
            file: fln.clone(),
            msg: format!(
                "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                failure.max_stack_size_tried / stack_limit::KI
            ),
        })?;

        let parsed_file = parsed_file.map_err(|failure| match failure {
            ParserError::NotAHackFile() => ParsingError::NotAHackFile { file: fln.clone() },
            ParserError::ParserFatal(e, p) => ParsingError::ParserFatal { pos: p, err: e },
            ParserError::Other(e) => ParsingError::Other {
                file: fln.clone(),
                msg: e,
            },
        })?;

        let mut errs = Vec::new();
        self.process_scour_comments(&mut errs, parsed_file.scoured_comments);
        self.process_syntax_errors(&mut errs, &indexed_source_text, parsed_file.syntax_errors);
        self.process_lowpri_errors(&mut errs, &env, parsed_file.lowpri_errors);
        self.process_non_syntax_errors(&mut errs, parsed_file.errors);
        self.process_lint_errors(parsed_file.lint_errors);

        let mut ast = parsed_file.aast.map_err(|msg| ParsingError::Other {
            file: fln.clone(),
            msg,
        })?;

        Naming::program(Rc::clone(&self.special_names), &mut ast);

        Ok((ast, errs))
    }

    fn make_parser_env(&self, flags: MakeParserEnv) -> ParserEnv {
        ParserEnv {
            codegen: flags.codegen,
            php5_compat_mode: flags.php5_compat_mode,
            elaborate_namespaces: flags.elaborate_namespaces,
            include_line_comments: flags.include_line_comments,
            keep_errors: flags.keep_errors,
            quick_mode: flags.quick_mode,
            show_all_errors: flags.show_all_errors,
            fail_open: flags.fail_open,
            is_systemlib: flags.is_systemlib,
            parser_options: (*self.parser_options).clone(),
        }
    }

    fn get_def_impl<'node, T: 'node>(
        &self,
        defs: &'node [oxidized::aast::Def<(), ()>],
        get_node: &dyn Fn(&'node oxidized::aast::Def<(), ()>) -> Option<&'node T>,
        get_name: &dyn Fn(&'node T) -> &'node str,
        name: &str,
    ) -> Result<&'node T, ()> {
        // TODO(hrust): I think the corresponding OCaml code will pick
        // the last occurrence, while this code will pick the first
        for def in defs {
            match def {
                oxidized::aast::Def::Namespace(ns) => {
                    match self.get_def_impl(&ns.1, get_node, get_name, name) {
                        Ok(def) => return Ok(def),
                        Err(()) => {}
                    }
                }
                def => match get_node(def) {
                    None => {}
                    Some(def) => {
                        if get_name(&def) == name {
                            return Ok(def);
                        }
                    }
                },
            }
        }
        Err(())
    }

    fn get_def<T>(
        &self,
        full: bool,
        fln: &RelativePath,
        get_node: &dyn Fn(&oxidized::aast::Def<(), ()>) -> Option<&T>,
        get_name: &dyn Fn(&T) -> &str,
        name: &str,
    ) -> Result<Option<RcRef<AstItem, T>>, ParsingError> {
        let defs = self.get_ast(full, fln)?;
        let defs = RcRef::new(defs);
        match defs.try_map(|&(ref defs, ref _parsing_errors)| {
            self.get_def_impl(&defs.0, &get_node, &get_name, name)
        }) {
            Ok(defs) => Ok(Some(defs)),
            Err(()) => Ok(None),
        }
    }

    pub fn find_fun_in_file(
        &self,
        full: bool,
        fln: &RelativePath,
        name: &str,
    ) -> Result<Option<RcRef<AstItem, oxidized::aast::FunDef<(), ()>>>, ParsingError> {
        self.get_def(
            full,
            fln,
            &|def| match def {
                oxidized::aast::Def::Fun(f) => Some(&**f),
                _ => None,
            },
            &|f| &f.fun.name.1,
            name,
        )
    }
}
