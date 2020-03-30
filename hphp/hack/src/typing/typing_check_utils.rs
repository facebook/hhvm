// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use aast_parser::{
    rust_aast_parser_types::{Env as AastEnv, Result as AastResult},
    AastParser, Error as AastError,
};
use anyhow;
use decl_provider_rust as decl_provider;
use itertools::{Either, Either::*};
use ocamlrep::rc::RcOc;
use oxidized::{ast, parser_options::ParserOptions, pos::Pos, relative_path::RelativePath};
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};
use stack_limit::StackLimit;
use typing_ast_print_rust as typing_ast_print;
use typing_defs_rust::typing_make_type::TypeBuilder;
use typing_rust as typing;

/// Compilation profile. All times are in seconds
#[derive(Debug)]
pub struct Profile {
    // Parse time
    pub parsing_t: f64,
    // Infer time
    pub infer_t: f64,
    // TAST check time
    pub check_t: f64,
}

pub fn from_text<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    stack_limit: &StackLimit,
    filepath: &RelativePath,
    text: &[u8],
) -> anyhow::Result<Profile> {
    let mut ret = Profile {
        parsing_t: 0.0,
        infer_t: 0.0,
        check_t: 0.0,
    };

    let mut parse_result = profile(&mut ret.parsing_t, || {
        parse_file(stack_limit, filepath, text)
    });

    let _program = match &mut parse_result {
        Either::Right((ast, _is_hh_file)) => {
            let tast = typing::program(builder, provider, ast);
            typing_ast_print::print(&tast);
            ()
        }
        Either::Left((_pos, _msg, _is_runtime_error)) => (),
    };

    profile(&mut ret.check_t, || {});
    Ok(ret)
}

/// parse_file returns either error(Left) or ast(Right)
/// - Left((Position, message, is_runtime_error))
/// - Right((ast, is_hh_file))
fn parse_file(
    //opts: &Options,
    stack_limit: &StackLimit,
    filepath: &RelativePath,
    text: &[u8],
) -> Either<(Pos, String, bool), (ast::Program, bool)> {
    let mut aast_env = AastEnv::default();
    aast_env.codegen = false;
    aast_env.keep_errors = true;
    aast_env.show_all_errors = true;
    aast_env.fail_open = true;
    aast_env.parser_options = ParserOptions::default();
    let source_text = SourceText::make(RcOc::new(filepath.clone()), text);
    let indexed_source_text = IndexedSourceText::new(source_text);
    let ast_result = AastParser::from_text(&aast_env, &indexed_source_text, Some(stack_limit));
    match ast_result {
        Err(AastError::Other(msg)) => Left((Pos::make_none(), msg, false)),
        Err(AastError::ParserFatal(syntax_error, pos)) => {
            Left((pos, syntax_error.message.to_string(), false))
        }
        Ok(ast) => match ast {
            AastResult { syntax_errors, .. } if !syntax_errors.is_empty() => unimplemented!(),
            AastResult {
                mut lowpri_errors, ..
            } if !lowpri_errors.is_empty() => {
                let (pos, msg) = lowpri_errors.pop().unwrap();
                Left((pos, msg, false))
            }
            AastResult {
                errors,
                aast,
                file_mode,
                ..
            } => {
                if !errors.is_empty() {
                    unimplemented!()
                } else {
                    match aast {
                        Ok(aast) => Right((aast, file_mode.is_hh_file())),
                        Err(msg) => Left((Pos::make_none(), msg, false)),
                    }
                }
            }
        },
    }
}

fn profile<T, F>(dt: &mut f64, f: F) -> T
where
    F: FnOnce() -> T,
{
    let t0 = std::time::Instant::now();
    let ret = f();
    *dt = t0.elapsed().as_secs_f64();
    ret
}
