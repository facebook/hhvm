// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod dump_expr_tree;

use std::fmt;
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;

use aast_parser::rust_aast_parser_types::Env as AastEnv;
use aast_parser::rust_aast_parser_types::ParserProfile;
use aast_parser::rust_aast_parser_types::ParserResult;
use aast_parser::AastParser;
use aast_parser::Error as AastError;
use anyhow::anyhow;
use anyhow::Result;
use bstr::ByteSlice;
use bytecode_printer::Context;
use decl_provider::DeclProvider;
use emit_unit::emit_unit;
use env::emitter::Emitter;
use error::Error;
use error::ErrorKind;
use hhbc::FatalOp;
use hhbc::Unit;
use ocamlrep::rc::RcOc;
use options::HhbcFlags;
use options::Hhvm;
use options::Options;
use options::ParserOptions;
use oxidized::ast;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::pos::Pos;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::ErrorType;
use relative_path::Prefix;
use relative_path::RelativePath;
use serde::Deserialize;
use serde::Serialize;
use thiserror::Error;
use types::readonly_check;
use types::readonly_nonlocal_infer;

/// Common input needed for compilation.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NativeEnv {
    pub filepath: RelativePath,
    pub hhvm: Hhvm,
    pub hhbc_flags: HhbcFlags,
    pub flags: EnvFlags,
}

impl Default for NativeEnv {
    fn default() -> Self {
        Self {
            filepath: RelativePath::make(Prefix::Dummy, Default::default()),
            hhvm: Default::default(),
            hhbc_flags: HhbcFlags::default(),
            flags: EnvFlags::default(),
        }
    }
}

#[derive(Debug, Default, Clone, clap::Args, Serialize, Deserialize)]
pub struct EnvFlags {
    /// Enable features only allowed in systemlib
    #[clap(long)]
    pub is_systemlib: bool,

    /// Mutate the program as if we're in the debuger REPL
    #[clap(long)]
    pub for_debugger_eval: bool,

    /// Disable namespace elaboration for toplevel definitions
    #[clap(long)]
    pub disable_toplevel_elaboration: bool,

    /// Dump IR instead of HHAS
    #[clap(long)]
    pub dump_ir: bool,

    /// Compile files using the IR pass
    #[clap(long)]
    pub enable_ir: bool,
}

impl NativeEnv {
    fn to_options(&self) -> Options {
        Options {
            hhvm: Hhvm {
                parser_options: ParserOptions {
                    po_disable_legacy_soft_typehints: false,
                    ..self.hhvm.parser_options.clone()
                },
                ..self.hhvm.clone()
            },
            hhbc: self.hhbc_flags.clone(),
            ..Default::default()
        }
    }

    pub fn to_decl_parser_options(&self) -> DeclParserOptions {
        let auto_namespace_map = self.hhvm.aliased_namespaces_cloned().collect();
        // Keep in sync with getDeclFlags in runtime-option.cpp
        let lang_flags = &self.hhvm.parser_options;
        DeclParserOptions {
            auto_namespace_map,
            disable_xhp_element_mangling: lang_flags.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: true,
            allow_new_attribute_syntax: lang_flags.po_allow_new_attribute_syntax,
            enable_xhp_class_modifier: lang_flags.po_enable_xhp_class_modifier,
            php5_compat_mode: true,
            hhvm_compat_mode: true,
            ..Default::default()
        }
    }
}

/// Compilation profile. All times are in seconds,
/// except when they are ignored and should not be reported,
/// such as in the case hhvm.log_extern_compiler_perf is false.
#[derive(Debug, Default)]
pub struct Profile {
    pub parser_profile: ParserProfile,

    /// Time in seconds spent in emitter.
    pub codegen_t: Duration,

    /// Time in seconds spent in bytecode_printer.
    pub printing_t: Duration,

    /// Time taken by bc_to_ir
    pub bc_to_ir_t: Duration,

    /// Time taken by ir_to_bc
    pub ir_to_bc_t: Duration,

    /// Emitter arena allocation volume in bytes.
    pub codegen_bytes: u64,

    /// Peak stack size during codegen
    pub rewrite_peak: u64,
    pub emitter_peak: u64,

    /// Was the log_extern_compiler_perf flag set?
    pub log_enabled: bool,
}

impl Profile {
    pub fn fold(a: Self, b: Self) -> Profile {
        Profile {
            parser_profile: a.parser_profile.fold(b.parser_profile),

            codegen_t: a.codegen_t + b.codegen_t,
            printing_t: a.printing_t + b.printing_t,
            codegen_bytes: a.codegen_bytes + b.codegen_bytes,

            bc_to_ir_t: a.bc_to_ir_t + b.bc_to_ir_t,
            ir_to_bc_t: a.ir_to_bc_t + b.ir_to_bc_t,

            rewrite_peak: std::cmp::max(a.rewrite_peak, b.rewrite_peak),
            emitter_peak: std::cmp::max(a.emitter_peak, b.emitter_peak),

            log_enabled: a.log_enabled | b.log_enabled,
        }
    }

    pub fn total_t(&self) -> Duration {
        self.parser_profile.total_t
            + self.codegen_t
            + self.bc_to_ir_t
            + self.ir_to_bc_t
            + self.printing_t
    }
}

/// Compile Hack source code, write HHAS text to `writer`.
/// Update `profile` with stats from any passes that run,
/// even if the compiler ultimately returns Err.
pub fn from_text<'decl>(
    writer: &mut dyn std::io::Write,
    source_text: SourceText<'_>,
    native_env: &NativeEnv,
    decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
    profile: &mut Profile,
) -> Result<()> {
    let alloc = bumpalo::Bump::new();
    let path = source_text.file_path().path().to_path_buf();
    let mut emitter = create_emitter(&native_env.flags, native_env, decl_provider, &alloc);
    let mut unit = emit_unit_from_text(&mut emitter, &native_env.flags, source_text, profile)?;

    if native_env.flags.enable_ir {
        let bc_to_ir_t = Instant::now();
        let ir = bc_to_ir::bc_to_ir(&unit, &path);
        profile.bc_to_ir_t = bc_to_ir_t.elapsed();

        let ir_to_bc_t = Instant::now();
        unit = ir_to_bc::ir_to_bc(&alloc, ir);
        profile.ir_to_bc_t = ir_to_bc_t.elapsed();
    }

    unit_to_string(native_env, writer, &unit, profile)?;
    profile.codegen_bytes = alloc.allocated_bytes() as u64;
    Ok(())
}

fn rewrite_and_emit<'p, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    namespace_env: RcOc<NamespaceEnv>,
    ast: &'p mut ast::Program,
    profile: &'p mut Profile,
) -> Result<Unit<'arena>, Error> {
    // First rewrite and modify `ast` in place.
    stack_limit::reset();
    let result = rewrite_program::rewrite_program(emitter, ast, RcOc::clone(&namespace_env));
    profile.rewrite_peak = stack_limit::peak() as u64;
    stack_limit::reset();
    let unit = match result {
        Ok(()) => {
            // Rewrite ok, now emit.
            emit_unit_from_ast(emitter, namespace_env, ast)
        }
        Err(e) => match e.into_kind() {
            ErrorKind::IncludeTimeFatalException(fatal_op, pos, msg) => {
                emit_unit::emit_fatal_unit(emitter.alloc, fatal_op, pos, msg)
            }
            ErrorKind::Unrecoverable(x) => Err(Error::unrecoverable(x)),
        },
    };
    profile.emitter_peak = stack_limit::peak() as u64;
    unit
}

pub fn unit_from_text<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    source_text: SourceText<'_>,
    native_env: &NativeEnv,
    decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
    profile: &mut Profile,
) -> Result<Unit<'arena>> {
    let mut emitter = create_emitter(&native_env.flags, native_env, decl_provider, alloc);
    emit_unit_from_text(&mut emitter, &native_env.flags, source_text, profile)
}

pub fn unit_to_string(
    native_env: &NativeEnv,
    writer: &mut dyn std::io::Write,
    program: &Unit<'_>,
    profile: &mut Profile,
) -> Result<()> {
    if native_env.flags.dump_ir {
        let ir = bc_to_ir::bc_to_ir(program, native_env.filepath.path());
        struct FmtFromIo<'a>(&'a mut dyn std::io::Write);
        impl fmt::Write for FmtFromIo<'_> {
            fn write_str(&mut self, s: &str) -> fmt::Result {
                self.0.write_all(s.as_bytes()).map_err(|_| fmt::Error)
            }
        }
        let print_result;
        (print_result, profile.printing_t) = profile_rust::time(|| {
            let verbose = false;
            ir::print_unit(&mut FmtFromIo(writer), &ir, verbose)
        });
        print_result?;
    } else {
        let print_result;
        (print_result, profile.printing_t) = profile_rust::time(|| {
            let opts = NativeEnv::to_options(native_env);
            bytecode_printer::print_unit(
                &Context::new(Some(&native_env.filepath), opts.array_provenance()),
                writer,
                program,
            )
        });
        print_result?;
    }
    Ok(())
}

fn emit_unit_from_ast<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    namespace: RcOc<NamespaceEnv>,
    ast: &mut ast::Program,
) -> Result<Unit<'arena>, Error> {
    emit_unit(emitter, namespace, ast)
}

fn check_readonly_and_emit<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    namespace_env: RcOc<NamespaceEnv>,
    ast: &mut ast::Program,
    profile: &mut Profile,
) -> Result<Unit<'arena>, Error> {
    match &emitter.decl_provider {
        // If a decl provider is available (DDB is enabled) *and*
        // `Hack.Lang.ReadonlyNonlocalInference` is set, then we can rewrite the
        // AST to automagically insert `readonly` annotations where needed.
        Some(decl_provider) if emitter.options().hhbc.readonly_nonlocal_infer => {
            let mut new_ast = readonly_nonlocal_infer::infer(ast, decl_provider.clone());
            let res = readonly_check::check_program(&mut new_ast, false);
            // Ignores all errors after the first...
            if let Some(readonly_check::ReadOnlyError(pos, msg)) = res.into_iter().next() {
                return emit_fatal(emitter.alloc, FatalOp::Parse, pos, msg);
            }
            *ast = new_ast;
        }
        None | Some(_) => (),
    }
    rewrite_and_emit(emitter, namespace_env, ast, profile)
}

fn emit_unit_from_text<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    flags: &EnvFlags,
    source_text: SourceText<'_>,
    profile: &mut Profile,
) -> Result<Unit<'arena>> {
    profile.log_enabled = emitter.options().log_extern_compiler_perf();
    let type_directed = emitter.decl_provider.is_some();

    let namespace_env = RcOc::new(NamespaceEnv::empty(
        emitter.options().hhvm.aliased_namespaces_cloned().collect(),
        true, /* is_codegen */
        emitter
            .options()
            .hhvm
            .parser_options
            .po_disable_xhp_element_mangling,
    ));
    let path = source_text.file_path_rc();

    let parse_result = parse_file(
        emitter.options(),
        source_text,
        !flags.disable_toplevel_elaboration,
        RcOc::clone(&namespace_env),
        flags.is_systemlib,
        type_directed,
        profile,
    );

    let ((unit, profile), codegen_t) = match parse_result {
        Ok(mut ast) => {
            elab::elaborate_program_for_codegen(RcOc::clone(&namespace_env), &path, &mut ast);
            profile_rust::time(move || {
                (
                    check_readonly_and_emit(emitter, namespace_env, &mut ast, profile),
                    profile,
                )
            })
        }
        Err(ParseError(pos, msg, fatal_op)) => {
            profile_rust::time(move || (emit_fatal(emitter.alloc, fatal_op, pos, msg), profile))
        }
    };
    profile.codegen_t = codegen_t;
    match unit {
        Ok(unit) => Ok(unit),
        Err(e) => Err(anyhow!("Unhandled Emitter error: {}", e)),
    }
}

fn emit_fatal<'arena>(
    alloc: &'arena bumpalo::Bump,
    fatal_op: FatalOp,
    pos: Pos,
    msg: impl AsRef<str> + 'arena,
) -> Result<Unit<'arena>, Error> {
    emit_unit::emit_fatal_unit(alloc, fatal_op, pos, msg)
}

fn create_emitter<'arena, 'decl>(
    flags: &EnvFlags,
    native_env: &NativeEnv,
    decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
    alloc: &'arena bumpalo::Bump,
) -> Emitter<'arena, 'decl> {
    Emitter::new(
        NativeEnv::to_options(native_env),
        flags.is_systemlib,
        flags.for_debugger_eval,
        alloc,
        decl_provider,
        native_env.filepath.clone(),
    )
}

fn create_parser_options(opts: &Options, type_directed: bool) -> ParserOptions {
    ParserOptions {
        po_codegen: true,
        po_disallow_silence: false,
        tco_no_parser_readonly_check: type_directed,
        ..opts.hhvm.parser_options.clone()
    }
}

#[derive(Error, Debug)]
#[error("{0}: {1}")]
pub(crate) struct ParseError(Pos, String, FatalOp);

fn parse_file(
    opts: &Options,
    source_text: SourceText<'_>,
    elaborate_namespaces: bool,
    namespace_env: RcOc<NamespaceEnv>,
    is_systemlib: bool,
    type_directed: bool,
    profile: &mut Profile,
) -> Result<ast::Program, ParseError> {
    let aast_env = AastEnv {
        codegen: true,
        php5_compat_mode: !opts.hhbc.uvs,
        is_systemlib,
        elaborate_namespaces,
        parser_options: create_parser_options(opts, type_directed),
        ..AastEnv::default()
    };

    let indexed_source_text = IndexedSourceText::new(source_text);
    let ast_result =
        AastParser::from_text_with_namespace_env(&aast_env, namespace_env, &indexed_source_text);
    match ast_result {
        Err(AastError::Other(msg)) => Err(ParseError(Pos::NONE, msg, FatalOp::Parse)),
        Err(AastError::NotAHackFile()) => Err(ParseError(
            Pos::NONE,
            "Not a Hack file".to_string(),
            FatalOp::Parse,
        )),
        Err(AastError::ParserFatal(syntax_error, pos)) => Err(ParseError(
            pos,
            syntax_error.message.to_string(),
            FatalOp::Parse,
        )),
        Ok(ast) => match ast {
            ParserResult { syntax_errors, .. } if !syntax_errors.is_empty() => {
                let syntax_error = syntax_errors
                    .iter()
                    .find(|e| e.error_type == ErrorType::RuntimeError)
                    .unwrap_or(&syntax_errors[0]);
                let pos = indexed_source_text
                    .relative_pos(syntax_error.start_offset, syntax_error.end_offset);
                Err(ParseError(
                    pos.into(),
                    syntax_error.message.to_string(),
                    match syntax_error.error_type {
                        ErrorType::ParseError => FatalOp::Parse,
                        ErrorType::RuntimeError => FatalOp::Runtime,
                    },
                ))
            }
            ParserResult {
                lowerer_parsing_errors,
                ..
            } if !lowerer_parsing_errors.is_empty() => {
                let (pos, msg) = lowerer_parsing_errors.into_iter().next().unwrap();
                Err(ParseError(pos, msg, FatalOp::Parse))
            }
            ParserResult {
                errors,
                aast,
                profile: parser_profile,
                ..
            } => {
                profile.parser_profile = parser_profile;
                let mut errors = errors.iter().filter(|e| {
                    e.code() != 2086
                        /* Naming.MethodNeedsVisibility */
                        && e.code() != 2102
                        /* Naming.UnsupportedTraitUseAs */
                        && e.code() != 2103
                });
                match errors.next() {
                    Some(e) => Err(ParseError(
                        e.pos().clone(),
                        e.msg().to_str_lossy().into_owned(),
                        FatalOp::Parse,
                    )),
                    None => Ok(aast),
                }
            }
        },
    }
}

pub fn expr_to_string_lossy(flags: &EnvFlags, expr: &ast::Expr) -> String {
    use print_expr::Context;

    let opts = Options::default();
    let alloc = bumpalo::Bump::new();
    let emitter = Emitter::new(
        opts,
        flags.is_systemlib,
        flags.for_debugger_eval,
        &alloc,
        None,
        RelativePath::make(Prefix::Dummy, Default::default()),
    );
    let ctx = Context::new(&emitter);

    print_expr::expr_to_string_lossy(ctx, expr)
}
