// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod dump_expr_tree;

use std::fmt;
use std::time::Duration;
use std::time::Instant;

use aast_parser::rust_aast_parser_types::Env as AastEnv;
use aast_parser::rust_aast_parser_types::ParserProfile;
use aast_parser::rust_aast_parser_types::ParserResult;
use aast_parser::AastParser;
use aast_parser::Error as AastError;
use anyhow::anyhow;
use anyhow::Result;
use bytecode_printer::Context;
use decl_provider::DeclProvider;
use emit_unit::emit_unit;
use env::emitter::Emitter;
use error::Error;
use error::ErrorKind;
use hhbc::FatalOp;
use hhbc::Unit;
use hhvm_options::HhvmConfig;
use ocamlrep::rc::RcOc;
use options::Arg;
use options::HackLang;
use options::Hhvm;
use options::HhvmFlags;
use options::LangFlags;
use options::Options;
use options::Php7Flags;
use options::RepoFlags;
use oxidized::ast;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::parser_options::ParserOptions;
use oxidized::pos::Pos;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::ErrorType;
use thiserror::Error;
use types::readonly_check;
use types::readonly_nonlocal_infer;

/// Common input needed for compilation.
#[derive(Debug)]
pub struct NativeEnv {
    pub filepath: RelativePath,
    pub aliased_namespaces: String,
    pub include_roots: String,
    pub emit_class_pointers: i32,
    pub check_int_overflow: i32,
    pub hhbc_flags: HhbcFlags,
    pub parser_flags: ParserFlags,
    pub flags: EnvFlags,
}

impl Default for NativeEnv {
    fn default() -> Self {
        Self {
            filepath: RelativePath::make(Prefix::Dummy, Default::default()),
            aliased_namespaces: "".into(),
            include_roots: "".into(),
            emit_class_pointers: 0,
            check_int_overflow: 0,
            hhbc_flags: HhbcFlags::default(),
            parser_flags: ParserFlags::default(),
            flags: EnvFlags::default(),
        }
    }
}

#[derive(Debug, Default, Clone, clap::Parser)]
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

#[derive(Debug, Default, Clone, clap::Parser)]
pub struct HhbcFlags {
    /// PHP7 left-to-right assignment semantics
    #[clap(long)]
    pub ltr_assign: bool,

    /// PHP7 Uniform Variable Syntax
    #[clap(long)]
    pub uvs: bool,

    #[clap(long)]
    pub repo_authoritative: bool,
    #[clap(long)]
    pub jit_enable_rename_function: bool,
    #[clap(long)]
    pub log_extern_compiler_perf: bool,
    #[clap(long)]
    pub enable_intrinsics_extension: bool,
    #[clap(long)]
    pub emit_cls_meth_pointers: bool,
    #[clap(long)]
    pub emit_meth_caller_func_pointers: bool,
    #[clap(long)]
    pub array_provenance: bool,
    #[clap(long)]
    pub fold_lazy_class_keys: bool,
}

#[derive(Debug, Default, Clone, clap::Parser)]
pub struct ParserFlags {
    #[clap(long)]
    pub abstract_static_props: bool,
    #[clap(long)]
    pub allow_new_attribute_syntax: bool,
    #[clap(long)]
    pub allow_unstable_features: bool,
    #[clap(long)]
    pub const_default_func_args: bool,
    #[clap(long)]
    pub const_static_props: bool,
    #[clap(long)]
    pub disable_lval_as_an_expression: bool,
    #[clap(long)]
    pub disallow_inst_meth: bool,
    #[clap(long)]
    pub disable_xhp_element_mangling: bool,
    #[clap(long)]
    pub disallow_fun_and_cls_meth_pseudo_funcs: bool,
    #[clap(long)]
    pub disallow_func_ptrs_in_constants: bool,
    #[clap(long)]
    pub enable_enum_classes: bool,
    #[clap(long)]
    pub enable_xhp_class_modifier: bool,
    #[clap(long)]
    pub enable_class_level_where_clauses: bool,
}

impl HhbcFlags {
    pub fn from_hhvm_config(config: &HhvmConfig) -> Result<Self> {
        let mut flags = Self::default();

        // Use the config setting if provided; otherwise preserve default.
        let init = |flag: &mut bool, name: &str| -> Result<()> {
            match config.get_bool(name)? {
                Some(b) => Ok(*flag = b),
                None => Ok(()),
            }
        };

        init(&mut flags.ltr_assign, "php7.ltr_assign")?;
        init(&mut flags.uvs, "php7.uvs")?;
        init(&mut flags.repo_authoritative, "Repo.Authoritative")?;
        init(
            &mut flags.jit_enable_rename_function,
            "Eval.JitEnableRenameFunction",
        )?;
        init(
            &mut flags.jit_enable_rename_function,
            "JitEnableRenameFunction",
        )?;
        init(
            &mut flags.log_extern_compiler_perf,
            "Eval.LogExternCompilerPerf",
        )?;
        init(
            &mut flags.enable_intrinsics_extension,
            "Eval.EnableIntrinsicsExtension",
        )?;
        init(
            &mut flags.emit_cls_meth_pointers,
            "Eval.EmitClsMethPointers",
        )?;

        // Only the hdf versions used. Can kill variant in options_cli.rs
        flags.emit_meth_caller_func_pointers = config
            .get_bool("Eval.EmitMethCallerFuncPointers")?
            .unwrap_or(true);

        // ini might use hhvm.array_provenance
        // hdf might use Eval.ArrayProvenance
        // But super unclear here
        init(&mut flags.array_provenance, "Eval.ArrayProvenance")?;
        init(&mut flags.array_provenance, "array_provenance")?;

        // Only hdf version
        flags.fold_lazy_class_keys = config.get_bool("Eval.FoldLazyClassKeys")?.unwrap_or(true);
        Ok(flags)
    }

    fn to_php7_flags(&self) -> Php7Flags {
        let mut f = Php7Flags::empty();
        if self.uvs {
            f |= Php7Flags::UVS;
        }
        if self.ltr_assign {
            f |= Php7Flags::LTR_ASSIGN;
        }
        f
    }

    fn to_hhvm_flags(&self) -> HhvmFlags {
        let mut f = HhvmFlags::empty();
        if self.array_provenance {
            f |= HhvmFlags::ARRAY_PROVENANCE;
        }
        if self.emit_cls_meth_pointers {
            f |= HhvmFlags::EMIT_CLS_METH_POINTERS;
        }
        if self.emit_meth_caller_func_pointers {
            f |= HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS;
        }
        if self.enable_intrinsics_extension {
            f |= HhvmFlags::ENABLE_INTRINSICS_EXTENSION;
        }
        if self.fold_lazy_class_keys {
            f |= HhvmFlags::FOLD_LAZY_CLASS_KEYS;
        }
        if self.jit_enable_rename_function {
            f |= HhvmFlags::JIT_ENABLE_RENAME_FUNCTION;
        }
        if self.log_extern_compiler_perf {
            f |= HhvmFlags::LOG_EXTERN_COMPILER_PERF;
        }
        f
    }

    fn to_repo_flags(&self) -> RepoFlags {
        let mut f = RepoFlags::empty();
        if self.repo_authoritative {
            f |= RepoFlags::AUTHORITATIVE;
        }
        f
    }
}

impl ParserFlags {
    pub fn from_hhvm_config(config: &HhvmConfig) -> Result<Self> {
        let mut flags = Self::default();

        // Use the config setting if provided; otherwise preserve default.
        let init = |flag: &mut bool, name: &str| -> Result<()> {
            match config.get_bool(name)? {
                Some(b) => Ok(*flag = b),
                None => Ok(()),
            }
        };

        // Note: Could only find examples of Hack.Lang.AbstractStaticProps
        init(
            &mut flags.abstract_static_props,
            "Hack.Lang.AbstractStaticProps",
        )?;

        // TODO: I'm pretty sure allow_new_attribute_syntax is dead and we can kill this option
        init(
            &mut flags.allow_new_attribute_syntax,
            "hack.lang.allow_new_attribute_syntax",
        )?;

        // Both hdf and ini versions are being used
        init(
            &mut flags.allow_unstable_features,
            "Hack.Lang.AllowUnstableFeatures",
        )?;

        // TODO: could not find examples of const_default_func_args, kill it in options_cli.rs
        init(
            &mut flags.const_default_func_args,
            "Hack.Lang.ConstDefaultFuncArgs",
        )?;

        // Only hdf version found in use
        init(&mut flags.const_static_props, "Hack.Lang.ConstStaticProps")?;

        // TODO: Kill disable_lval_as_an_expression

        // Only hdf option in use
        init(&mut flags.disallow_inst_meth, "Hack.Lang.DisallowInstMeth")?;

        // Both ini and hdf variants in use
        init(
            &mut flags.disable_xhp_element_mangling,
            "Hack.Lang.DisableXHPElementMangling",
        )?;

        // Both ini and hdf variants in use
        init(
            &mut flags.disallow_fun_and_cls_meth_pseudo_funcs,
            "Hack.Lang.DisallowFunAndClsMethPseudoFuncs",
        )?;

        // Only hdf option in use
        init(
            &mut flags.disallow_func_ptrs_in_constants,
            "Hack.Lang.DisallowFuncPtrsInConstants",
        )?;

        // Only hdf option in use
        init(
            &mut flags.enable_enum_classes,
            "Hack.Lang.EnableEnumClasses",
        )?;

        // Both options in use
        init(
            &mut flags.enable_xhp_class_modifier,
            "Hack.Lang.EnableXHPClassModifier",
        )?;

        // Only hdf option in use. Kill variant in options_cli.rs
        init(
            &mut flags.enable_class_level_where_clauses,
            "Hack.Lang.EnableClassLevelWhereClauses",
        )?;
        Ok(flags)
    }

    fn to_lang_flags(&self) -> LangFlags {
        let mut f = LangFlags::empty();
        if self.abstract_static_props {
            f |= LangFlags::ABSTRACT_STATIC_PROPS;
        }
        if self.allow_new_attribute_syntax {
            f |= LangFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX;
        }
        if self.allow_unstable_features {
            f |= LangFlags::ALLOW_UNSTABLE_FEATURES;
        }
        if self.const_default_func_args {
            f |= LangFlags::CONST_DEFAULT_FUNC_ARGS;
        }
        if self.const_static_props {
            f |= LangFlags::CONST_STATIC_PROPS;
        }
        if self.disable_lval_as_an_expression {
            f |= LangFlags::DISABLE_LVAL_AS_AN_EXPRESSION;
        }
        if self.disallow_inst_meth {
            f |= LangFlags::DISALLOW_INST_METH;
        }
        if self.disable_xhp_element_mangling {
            f |= LangFlags::DISABLE_XHP_ELEMENT_MANGLING;
        }
        if self.disallow_fun_and_cls_meth_pseudo_funcs {
            f |= LangFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS;
        }
        if self.disallow_func_ptrs_in_constants {
            f |= LangFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS;
        }
        if self.enable_enum_classes {
            f |= LangFlags::ENABLE_ENUM_CLASSES;
        }
        if self.enable_xhp_class_modifier {
            f |= LangFlags::ENABLE_XHP_CLASS_MODIFIER;
        }
        if self.enable_class_level_where_clauses {
            f |= LangFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES;
        }
        f
    }
}

impl NativeEnv {
    fn to_options(native_env: &NativeEnv) -> Options {
        let hhbc_flags = &native_env.hhbc_flags;
        let config = [
            native_env.aliased_namespaces.as_str(),
            native_env.include_roots.as_str(),
        ];
        let opts = Options::from_configs(&config).unwrap();
        let hhvm = Hhvm {
            aliased_namespaces: opts.hhvm.aliased_namespaces,
            include_roots: opts.hhvm.include_roots,
            flags: hhbc_flags.to_hhvm_flags(),
            emit_class_pointers: Arg::new(native_env.emit_class_pointers.to_string()),
            hack_lang: HackLang {
                flags: native_env.parser_flags.to_lang_flags(),
                check_int_overflow: Arg::new(native_env.check_int_overflow.to_string()),
            },
        };
        Options {
            hhvm,
            php7_flags: hhbc_flags.to_php7_flags(),
            repo_flags: hhbc_flags.to_repo_flags(),
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
    decl_provider: Option<&'decl dyn DeclProvider>,
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
    decl_provider: Option<&'decl dyn DeclProvider>,
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
                &Context::new(&opts, Some(&native_env.filepath), opts.array_provenance()),
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
    // TODO: T128303794 Experimental. Add more gating before emitter.decl_provider available in prod.
    match &emitter.decl_provider {
        None => (),
        Some(decl_provider) => {
            let mut new_ast = readonly_nonlocal_infer::infer(ast, decl_provider);
            let res = readonly_check::check_program(&mut new_ast, false);
            // Ignores all errors after the first...
            if let Some(readonly_check::ReadOnlyError(pos, msg)) = res.into_iter().next() {
                return emit_fatal(emitter.alloc, FatalOp::Parse, pos, msg);
            }
            *ast = new_ast;
        }
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
            .hack_lang
            .flags
            .contains(LangFlags::DISABLE_XHP_ELEMENT_MANGLING),
    ));

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
            elaborate_namespaces_visitor::elaborate_program(RcOc::clone(&namespace_env), &mut ast);
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
    decl_provider: Option<&'decl dyn DeclProvider>,
    alloc: &'arena bumpalo::Bump,
) -> Emitter<'arena, 'decl> {
    Emitter::new(
        NativeEnv::to_options(native_env),
        flags.is_systemlib,
        flags.for_debugger_eval,
        alloc,
        decl_provider,
    )
}

fn create_parser_options(opts: &Options, type_directed: bool) -> ParserOptions {
    let hack_lang_flags = |flag| opts.hhvm.hack_lang.flags.contains(flag);
    ParserOptions {
        po_auto_namespace_map: opts.hhvm.aliased_namespaces_cloned().collect(),
        po_codegen: true,
        po_disallow_silence: false,
        po_disable_lval_as_an_expression: hack_lang_flags(LangFlags::DISABLE_LVAL_AS_AN_EXPRESSION),
        po_enable_class_level_where_clauses: hack_lang_flags(
            LangFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES,
        ),
        po_disable_legacy_soft_typehints: hack_lang_flags(LangFlags::DISABLE_LEGACY_SOFT_TYPEHINTS),
        po_allow_new_attribute_syntax: hack_lang_flags(LangFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX),
        po_disable_legacy_attribute_syntax: hack_lang_flags(
            LangFlags::DISABLE_LEGACY_ATTRIBUTE_SYNTAX,
        ),
        po_const_default_func_args: hack_lang_flags(LangFlags::CONST_DEFAULT_FUNC_ARGS),
        po_const_default_lambda_args: hack_lang_flags(LangFlags::CONST_DEFAULT_LAMBDA_ARGS),
        tco_const_static_props: hack_lang_flags(LangFlags::CONST_STATIC_PROPS),
        po_abstract_static_props: hack_lang_flags(LangFlags::ABSTRACT_STATIC_PROPS),
        po_disallow_func_ptrs_in_constants: hack_lang_flags(
            LangFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS,
        ),
        po_enable_xhp_class_modifier: hack_lang_flags(LangFlags::ENABLE_XHP_CLASS_MODIFIER),
        po_disable_xhp_element_mangling: hack_lang_flags(LangFlags::DISABLE_XHP_ELEMENT_MANGLING),
        po_enable_enum_classes: hack_lang_flags(LangFlags::ENABLE_ENUM_CLASSES),
        po_allow_unstable_features: hack_lang_flags(LangFlags::ALLOW_UNSTABLE_FEATURES),
        po_disallow_fun_and_cls_meth_pseudo_funcs: hack_lang_flags(
            LangFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS,
        ),
        po_disallow_inst_meth: hack_lang_flags(LangFlags::DISALLOW_INST_METH),
        tco_no_parser_readonly_check: type_directed,
        ..Default::default()
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
        php5_compat_mode: !opts.php7_flags.contains(Php7Flags::UVS),
        keep_errors: false,
        is_systemlib,
        elaborate_namespaces,
        parser_options: create_parser_options(opts, type_directed),
        ..AastEnv::default()
    };

    let indexed_source_text = IndexedSourceText::new(source_text);
    let ast_result =
        AastParser::from_text_with_namespace_env(&aast_env, namespace_env, &indexed_source_text);
    match ast_result {
        Err(AastError::Other(msg)) => Err(ParseError(Pos::make_none(), msg, FatalOp::Parse)),
        Err(AastError::NotAHackFile()) => Err(ParseError(
            Pos::make_none(),
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
                    pos,
                    syntax_error.message.to_string(),
                    match syntax_error.error_type {
                        ErrorType::ParseError => FatalOp::Parse,
                        ErrorType::RuntimeError => FatalOp::Runtime,
                    },
                ))
            }
            ParserResult { lowpri_errors, .. } if !lowpri_errors.is_empty() => {
                let (pos, msg) = lowpri_errors.into_iter().next().unwrap();
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
                        String::from(e.msg()),
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

    let opts = Options::from_configs(&[]).expect("Malformed options");

    let alloc = bumpalo::Bump::new();
    let emitter = Emitter::new(
        opts,
        flags.is_systemlib,
        flags.for_debugger_eval,
        &alloc,
        None,
    );
    let ctx = Context::new(&emitter);

    print_expr::expr_to_string_lossy(ctx, expr)
}
