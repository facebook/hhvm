// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused_variables)]

use aast_parser::{
    rust_aast_parser_types::{Env as AastEnv, Result as AastResult},
    AastParser, Error as AastError,
};
use anyhow;
use bitflags::bitflags;
use emit_program_rust::{emit_fatal_program, emit_program, FromAstFlags};
use hhas_program_rust::HhasProgram;
use hhbc_ast_rust::FatalOp;
use hhbc_hhas_rust::{print_program, Context, Write};
use instruction_sequence_rust::Error;
use itertools::{Either, Either::*};
use ocamlrep::rc::RcOc;
use options::{LangFlags, Options, PhpismFlags};
use oxidized::{
    ast as Tast, namespace_env::Env as NamespaceEnv, parser_options::ParserOptions, pos::Pos,
    relative_path::RelativePath,
};
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};
use stack_limit::StackLimit;

/// Common input needed for compilation.  Extra care is taken
/// so that everything is easily serializable at the FFI boundary
/// until the migration from OCaml is fully complete
pub struct Env {
    pub filepath: RelativePath,
    pub empty_namespace: ocamlrep::rc::RcOc<NamespaceEnv>,
    pub config_jsons: Vec<String>,
    pub config_list: Vec<String>,
    pub flags: EnvFlags,
}

bitflags! {
    // Note: these flags are intentionally packed into bits to overcome

    // the limitation of to-OCaml FFI functions having at most 5 parameters
    pub struct EnvFlags: u8 {
        const IS_SYSTEMLIB = 1 << 0;
        const IS_EVALED = 1 << 1;
        const FOR_DEBUGGER_EVAL = 1 << 2;
        const DUMP_SYMBOL_REFS = 1 << 3;
    }
}

/// Compilation profile. All times are in seconds,
/// except when they are ignored and should not be reported,
/// such as in the case hhvm.log_extern_compiler_perf is false
/// (this avoids the need to read Options from OCaml, as
/// they can be simply returned as NaNs to signal that
/// they should _not_ be passed back as JSON to HHVM process)
#[derive(Debug)]
pub struct Profile {
    pub parsing_t: f64,
    pub codegen_t: f64,
    pub printing_t: f64,
}

// TODO(hrust) switch over to Option<Duration> once FFI is no longer needed;
// however, for FFI it's easier to serialize to floating point in seconds
// and use NaN for ignored (i.e., when hhvm.log_extern_compiler_perf is false)
pub const IGNORED_DURATION: f64 = std::f64::NAN;
pub fn is_ignored_duration(dt: &f64) -> bool {
    dt.is_nan()
}

pub fn from_text<W>(
    env: Env,
    stack_limit: &StackLimit,
    writer: &mut W,
    text: &[u8],
) -> anyhow::Result<Profile>
where
    W: Write,
    W::Error: Send + Sync + 'static, // required by anyhow::Error
{
    let opts =
        Options::from_configs(&env.config_jsons, &env.config_list).map_err(anyhow::Error::msg)?;
    let log_extern_compiler_perf = opts.log_extern_compiler_perf();

    let mut ret = Profile {
        parsing_t: IGNORED_DURATION,
        codegen_t: IGNORED_DURATION,
        printing_t: IGNORED_DURATION,
    };

    let ast = profile(log_extern_compiler_perf, &mut ret.parsing_t, || {
        parse_file(&opts, stack_limit, &env.filepath, text)
    });

    let (program, codegen_t) = match ast {
        // TODO(shiqicao): change opts to Rc<Option> to avoid cloning
        Either::Right((ast, is_hh_file)) => emit(&env, opts.clone(), is_hh_file, &ast),
        Either::Left((pos, msg, is_runtime_error)) => {
            emit_fatal(&env, is_runtime_error, opts.clone(), pos, msg)
        }
    };
    let program = program?;
    ret.codegen_t = codegen_t;

    profile(log_extern_compiler_perf, &mut ret.printing_t, || {
        print_program(
            &mut Context::new(
                &opts,
                Some(&env.filepath),
                env.flags.contains(EnvFlags::DUMP_SYMBOL_REFS),
            ),
            writer,
            &program,
        )
    })?;
    Ok(ret)
}

fn emit<'p>(
    env: &Env,
    opts: Options,
    is_hh: bool,
    ast: &Tast::Program,
) -> (Result<HhasProgram<'p>, Error>, f64) {
    let mut flags = FromAstFlags::empty();
    if is_hh {
        flags |= FromAstFlags::IS_HH_FILE;
    }
    if env.flags.contains(EnvFlags::IS_EVALED) {
        flags |= FromAstFlags::IS_EVALED;
    }
    if env.flags.contains(EnvFlags::FOR_DEBUGGER_EVAL) {
        flags |= FromAstFlags::FOR_DEBUGGER_EVAL;
    }
    if env.flags.contains(EnvFlags::IS_SYSTEMLIB) {
        flags |= FromAstFlags::IS_SYSTEMLIB;
    }
    let mut t = 0f64;
    let r = profile(opts.log_extern_compiler_perf(), &mut t, || {
        emit_program(opts, flags, &env.empty_namespace, ast)
    });
    (r, t)
}

fn emit_fatal(
    env: &Env,
    is_runtime_error: bool,
    opts: Options,
    pos: Pos,
    msg: String,
) -> (Result<HhasProgram, Error>, f64) {
    let op = if is_runtime_error {
        FatalOp::Runtime
    } else {
        FatalOp::Parse
    };
    let mut t = 0f64;
    let r = profile(opts.log_extern_compiler_perf(), &mut t, || {
        emit_fatal_program(opts, env.flags.contains(EnvFlags::IS_SYSTEMLIB), op, msg)
    });
    (r, t)
}

fn create_parser_options(opts: &Options) -> ParserOptions {
    let hack_lang_flags = |flag| opts.hhvm.hack_lang_flags.contains(flag);
    let phpism_flags = |flag| opts.phpism_flags.contains(flag);
    let mut popt = ParserOptions::default();
    popt.po_auto_namespace_map = opts
        .hhvm
        .aliased_namespaces
        .get()
        .iter()
        .map(|(x, y)| (x.to_owned(), y.to_owned()))
        .collect();
    popt.po_codegen = true;
    popt.po_disallow_silence = false;
    popt.po_disallow_execution_operator = phpism_flags(PhpismFlags::DISALLOW_EXECUTION_OPERATOR);
    popt.po_disable_nontoplevel_declarations =
        phpism_flags(PhpismFlags::DISABLE_NONTOPLEVEL_DECLARATIONS);
    popt.po_disable_static_closures = phpism_flags(PhpismFlags::DISABLE_STATIC_CLOSURES);
    popt.po_disable_lval_as_an_expression =
        hack_lang_flags(LangFlags::DISABLE_LVAL_AS_AN_EXPRESSION);
    popt.po_enable_class_level_where_clauses =
        hack_lang_flags(LangFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES);
    popt.po_disable_legacy_soft_typehints =
        hack_lang_flags(LangFlags::DISABLE_LEGACY_SOFT_TYPEHINTS);
    popt.po_allow_new_attribute_syntax = hack_lang_flags(LangFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX);
    popt.po_disable_legacy_attribute_syntax =
        hack_lang_flags(LangFlags::DISABLE_LEGACY_ATTRIBUTE_SYNTAX);
    popt.po_const_default_func_args = hack_lang_flags(LangFlags::CONST_DEFAULT_FUNC_ARGS);
    popt.tco_const_static_props = hack_lang_flags(LangFlags::CONST_STATIC_PROPS);
    popt.po_abstract_static_props = hack_lang_flags(LangFlags::ABSTRACT_STATIC_PROPS);
    popt.po_disable_unset_class_const = hack_lang_flags(LangFlags::DISABLE_UNSET_CLASS_CONST);
    popt.po_disallow_func_ptrs_in_constants =
        hack_lang_flags(LangFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS);
    popt.po_enable_xhp_class_modifier = hack_lang_flags(LangFlags::ENABLE_XHP_CLASS_MODIFIER);
    popt
}

/// parse_file returns either error(Left) or ast(Right)
/// - Left((Position, message, is_runtime_error))
/// - Right((ast, is_hh_file))
fn parse_file(
    opts: &Options,
    stack_limit: &StackLimit,
    filepath: &RelativePath,
    text: &[u8],
) -> Either<(Pos, String, bool), (Tast::Program, bool)> {
    let mut aast_env = AastEnv::default();
    aast_env.codegen = true;
    aast_env.keep_errors = true;
    aast_env.show_all_errors = true;
    aast_env.fail_open = true;
    aast_env.parser_options = create_parser_options(opts);
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
                scoured_comments,
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

fn profile<T, F>(log_extern_compiler_perf: bool, dt: &mut f64, f: F) -> T
where
    F: FnOnce() -> T,
{
    let t0 = std::time::Instant::now();
    let ret = f();
    *dt = if log_extern_compiler_perf {
        t0.elapsed().as_secs_f64()
    } else {
        IGNORED_DURATION
    };
    ret
}
