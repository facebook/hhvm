// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use aast_parser::{
    rust_aast_parser_types::{Env as AastEnv, Result as AastResult},
    AastParser, Error as AastError,
};
use anyhow::{anyhow, *};
use bitflags::bitflags;
use emit_program_rust::{self as emit_program, emit_program, FromAstFlags};
use emit_pu_rust;
use env::emitter::Emitter;
use hhas_program_rust::HhasProgram;
use hhbc_ast_rust::FatalOp;
use hhbc_hhas_rust::{context::Context, print_program, Write};
use instruction_sequence_rust::Error;
use itertools::{Either, Either::*};
use ocamlrep::{rc::RcOc, FromError, FromOcamlRep, Value};
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use options::{LangFlags, Options, Php7Flags, PhpismFlags};
use oxidized::{
    ast as Tast, namespace_env::Env as NamespaceEnv, parser_options::ParserOptions, pos::Pos,
    relative_path::RelativePath,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, source_text::SourceText, syntax_error::ErrorType,
};
use stack_limit::StackLimit;

/// Common input needed for compilation.  Extra care is taken
/// so that everything is easily serializable at the FFI boundary
/// until the migration from OCaml is fully complete
#[derive(Debug, FromOcamlRep)]
pub struct Env<STR: AsRef<str>> {
    pub filepath: RelativePath,
    pub config_jsons: Vec<STR>,
    pub config_list: Vec<STR>,
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
        const DISABLE_TOPLEVEL_ELABORATION = 1 << 4;
    }
}

impl FromOcamlRep for EnvFlags {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        Ok(EnvFlags::from_bits(value.as_int().unwrap() as u8).unwrap())
    }
}

/// Compilation profile. All times are in seconds,
/// except when they are ignored and should not be reported,
/// such as in the case hhvm.log_extern_compiler_perf is false
/// (this avoids the need to read Options from OCaml, as
/// they can be simply returned as NaNs to signal that
/// they should _not_ be passed back as JSON to HHVM process)
#[derive(Debug, ToOcamlRep)]
pub struct Profile {
    pub parsing_t: f64,
    pub codegen_t: f64,
    pub printing_t: f64,
}

pub fn emit_fatal_program<W, S: AsRef<str>>(
    env: &Env<S>,
    writer: &mut W,
    err_msg: &str,
) -> anyhow::Result<()>
where
    W: Write,
    W::Error: Send + Sync + 'static, // required by anyhow::Error
{
    let is_systemlib = env.flags.contains(EnvFlags::IS_SYSTEMLIB);
    let opts =
        Options::from_configs(&env.config_jsons, &env.config_list).map_err(anyhow::Error::msg)?;
    let mut emitter = Emitter::new(
        opts,
        is_systemlib,
        env.flags.contains(EnvFlags::FOR_DEBUGGER_EVAL),
    );
    let prog = emit_program::emit_fatal_program(FatalOp::Parse, &Pos::make_none(), err_msg);
    let prog = prog.map_err(|e| anyhow!("Unhandled Emitter error: {}", e))?;
    print_program(
        &mut Context::new(
            &mut emitter,
            Some(&env.filepath),
            env.flags.contains(EnvFlags::DUMP_SYMBOL_REFS),
            is_systemlib,
        ),
        writer,
        &prog,
    )?;
    Ok(())
}

pub fn from_text<W, S: AsRef<str>>(
    env: &Env<S>,
    stack_limit: &StackLimit,
    writer: &mut W,
    text: &[u8],
) -> anyhow::Result<Option<Profile>>
where
    W: Write,
    W::Error: Send + Sync + 'static, // required by anyhow::Error
{
    let source_text = SourceText::make(RcOc::new(env.filepath.clone()), text);
    from_text_(env, stack_limit, writer, source_text)
}

pub fn from_text_<W, S: AsRef<str>>(
    env: &Env<S>,
    stack_limit: &StackLimit,
    writer: &mut W,
    source_text: SourceText,
) -> anyhow::Result<Option<Profile>>
where
    W: Write,
    W::Error: Send + Sync + 'static, // required by anyhow::Error
{
    let opts =
        Options::from_configs(&env.config_jsons, &env.config_list).map_err(anyhow::Error::msg)?;

    let (mut parse_result, parsing_t) = time(|| {
        parse_file(
            &opts,
            stack_limit,
            source_text,
            !env.flags.contains(EnvFlags::DISABLE_TOPLEVEL_ELABORATION),
        )
    });
    let log_extern_compiler_perf = opts.log_extern_compiler_perf();

    let mut emitter = Emitter::new(
        opts,
        env.flags.contains(EnvFlags::IS_SYSTEMLIB),
        env.flags.contains(EnvFlags::FOR_DEBUGGER_EVAL),
    );

    let (program, codegen_t) = match &mut parse_result {
        Either::Right(ast) => {
            let namespace = RcOc::new(NamespaceEnv::empty(
                emitter.options().hhvm.aliased_namespaces_cloned().collect(),
                true, /* is_codegen */
                emitter
                    .options()
                    .hhvm
                    .hack_lang
                    .flags
                    .contains(LangFlags::DISABLE_XHP_ELEMENT_MANGLING),
            ));
            // TODO(shiqicao): change opts to Rc<Option> to avoid cloning
            elaborate_namespaces_visitor::elaborate_program(RcOc::clone(&namespace), ast);
            emit_pu_rust::translate(ast);
            let e = &mut emitter;
            time(move || emit(e, &env, namespace, ast))
        }
        Either::Left((pos, msg, is_runtime_error)) => {
            time(|| emit_fatal(*is_runtime_error, pos, msg))
        }
    };
    let program = program.map_err(|e| anyhow!("Unhandled Emitter error: {}", e))?;

    let (print_result, printing_t) = time(|| {
        print_program(
            &mut Context::new(
                &mut emitter,
                Some(&env.filepath),
                env.flags.contains(EnvFlags::DUMP_SYMBOL_REFS),
                env.flags.contains(EnvFlags::IS_SYSTEMLIB),
            ),
            writer,
            &program,
        )
    });
    print_result?;

    if log_extern_compiler_perf {
        Ok(Some(Profile {
            parsing_t,
            codegen_t,
            printing_t,
        }))
    } else {
        Ok(None)
    }
}

fn emit<'p, S: AsRef<str>>(
    emitter: &mut Emitter,
    env: &Env<S>,
    namespace: RcOc<NamespaceEnv>,
    ast: &'p mut Tast::Program,
) -> Result<HhasProgram<'p>, Error> {
    let mut flags = FromAstFlags::empty();
    if env.flags.contains(EnvFlags::IS_EVALED) {
        flags |= FromAstFlags::IS_EVALED;
    }
    if env.flags.contains(EnvFlags::FOR_DEBUGGER_EVAL) {
        flags |= FromAstFlags::FOR_DEBUGGER_EVAL;
    }
    if env.flags.contains(EnvFlags::IS_SYSTEMLIB) {
        flags |= FromAstFlags::IS_SYSTEMLIB;
    }
    emit_program(emitter, flags, namespace, ast)
}

fn emit_fatal<'a>(
    is_runtime_error: bool,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> Result<HhasProgram<'a>, Error> {
    let op = if is_runtime_error {
        FatalOp::Runtime
    } else {
        FatalOp::Parse
    };
    emit_program::emit_fatal_program(op, pos, msg)
}

fn create_parser_options(opts: &Options) -> ParserOptions {
    let hack_lang_flags = |flag| opts.hhvm.hack_lang.flags.contains(flag);
    let phpism_flags = |flag| opts.phpism_flags.contains(flag);
    let mut popt = ParserOptions::default();
    popt.po_auto_namespace_map = opts.hhvm.aliased_namespaces_cloned().collect();
    popt.po_codegen = true;
    popt.po_disallow_silence = false;
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
    popt.po_const_default_lambda_args = hack_lang_flags(LangFlags::CONST_DEFAULT_LAMBDA_ARGS);
    popt.tco_const_static_props = hack_lang_flags(LangFlags::CONST_STATIC_PROPS);
    popt.po_abstract_static_props = hack_lang_flags(LangFlags::ABSTRACT_STATIC_PROPS);
    popt.po_disable_unset_class_const = hack_lang_flags(LangFlags::DISABLE_UNSET_CLASS_CONST);
    popt.po_disallow_func_ptrs_in_constants =
        hack_lang_flags(LangFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS);
    popt.po_enable_xhp_class_modifier = hack_lang_flags(LangFlags::ENABLE_XHP_CLASS_MODIFIER);
    popt.po_disable_xhp_element_mangling = hack_lang_flags(LangFlags::DISABLE_XHP_ELEMENT_MANGLING);
    popt.po_enable_first_class_function_pointers =
        hack_lang_flags(LangFlags::ENABLE_FIRST_CLASS_FUNCTION_POINTERS);
    popt.po_enable_enum_classes = hack_lang_flags(LangFlags::ENABLE_ENUM_CLASSES);
    popt.po_disable_array = hack_lang_flags(LangFlags::DISABLE_ARRAY);
    popt.po_disable_array_typehint = hack_lang_flags(LangFlags::DISABLE_ARRAY_TYPEHINT);
    popt.po_allow_unstable_features = hack_lang_flags(LangFlags::ALLOW_UNSTABLE_FEATURES);
    popt
}

/// parse_file returns either error(Left) or ast(Right)
/// - Left((Position, message, is_runtime_error))
/// - Right(ast)
fn parse_file(
    opts: &Options,
    stack_limit: &StackLimit,
    source_text: SourceText,
    elaborate_namespaces: bool,
) -> Either<(Pos, String, bool), Tast::Program> {
    let mut aast_env = AastEnv::default();
    aast_env.codegen = true;
    aast_env.fail_open = false;
    // Ocaml's implementation
    // let enable_uniform_variable_syntax o = o.option_php7_uvs in
    // php5_compat_mode:
    //   (not (Hhbc_options.enable_uniform_variable_syntax hhbc_options))
    aast_env.php5_compat_mode = !opts.php7_flags.contains(Php7Flags::UVS);
    aast_env.keep_errors = false;
    aast_env.elaborate_namespaces = elaborate_namespaces;
    aast_env.parser_options = create_parser_options(opts);
    aast_env.lower_coroutines = opts
        .hhvm
        .hack_lang
        .flags
        .contains(LangFlags::ENABLE_COROUTINES);

    let indexed_source_text = IndexedSourceText::new(source_text);
    let ast_result = AastParser::from_text(&aast_env, &indexed_source_text, Some(stack_limit));
    match ast_result {
        Err(AastError::Other(msg)) => Left((Pos::make_none(), msg, false)),
        Err(AastError::NotAHackFile()) => {
            Left((Pos::make_none(), "Not a Hack file".to_string(), false))
        }
        Err(AastError::ParserFatal(syntax_error, pos)) => {
            Left((pos, syntax_error.message.to_string(), false))
        }
        Ok(ast) => match ast {
            AastResult { syntax_errors, .. } if !syntax_errors.is_empty() => {
                let error = syntax_errors
                    .iter()
                    .find(|e| e.error_type == ErrorType::RuntimeError)
                    .unwrap_or(&syntax_errors[0]);
                let pos = indexed_source_text.relative_pos(error.start_offset, error.end_offset);
                Left((
                    pos,
                    error.message.to_string(),
                    error.error_type == ErrorType::RuntimeError,
                ))
            }
            AastResult { lowpri_errors, .. } if !lowpri_errors.is_empty() => {
                let (pos, msg) = lowpri_errors.into_iter().next().unwrap();
                Left((pos, msg, false))
            }
            AastResult {
                errors,
                aast,
                scoured_comments,
                ..
            } => {
                let mut errors = errors.iter().filter(|e| {
                    scoured_comments.get_fixme(e.pos(), e.code()).is_none()
                        /* Ignore these errors to match legacy AST behavior */
                        && e.code() != 2086
                        /* Naming.MethodNeedsVisibility */
                        && e.code() != 2102
                        /* Naming.UnsupportedTraitUseAs */
                        && e.code() != 2103
                });
                if let Some(_) = errors.next() {
                    Left((Pos::make_none(), String::new(), false))
                } else {
                    match aast {
                        Ok(aast) => Right(aast),
                        Err(msg) => Left((Pos::make_none(), msg, false)),
                    }
                }
            }
        },
    }
}

fn time<T>(f: impl FnOnce() -> T) -> (T, f64) {
    let (r, t) = profile_rust::time(f);
    (r, t.as_secs_f64())
}
