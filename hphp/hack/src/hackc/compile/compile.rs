// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod dump_expr_tree;

use std::fmt;

use aast_parser::rust_aast_parser_types::Env as AastEnv;
use aast_parser::rust_aast_parser_types::ParserResult;
use aast_parser::AastParser;
use aast_parser::Error as AastError;
use anyhow::anyhow;
use anyhow::Result;
use bitflags::bitflags;
use bytecode_printer::Context;
use decl_provider::DeclProvider;
use emit_unit::emit_unit;
use env::emitter::Emitter;
use error::Error;
use error::ErrorKind;
use hhbc::hackc_unit::HackCUnit;
use hhbc::FatalOp;
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
pub struct NativeEnv<'a> {
    pub filepath: RelativePath,
    pub aliased_namespaces: &'a str,
    pub include_roots: &'a str,
    pub emit_class_pointers: i32,
    pub check_int_overflow: i32,
    pub hhbc_flags: HHBCFlags,
    pub parser_flags: ParserFlags,
    pub flags: EnvFlags,
}

impl Default for NativeEnv<'_> {
    fn default() -> Self {
        Self {
            filepath: RelativePath::make(Prefix::Dummy, Default::default()),
            aliased_namespaces: "",
            include_roots: "",
            emit_class_pointers: 0,
            check_int_overflow: 0,
            hhbc_flags: HHBCFlags::empty(),
            parser_flags: ParserFlags::empty(),
            flags: EnvFlags::empty(),
        }
    }
}

bitflags! {
    pub struct EnvFlags: u8 {
        const IS_SYSTEMLIB = 1 << 0;
        const IS_EVALED = 1 << 1;
        const FOR_DEBUGGER_EVAL = 1 << 2;
        const UNUSED_PLACEHOLDER = 1 << 3;
        const DISABLE_TOPLEVEL_ELABORATION = 1 << 4;
        const DUMP_IR = 1 << 5;
        const ENABLE_IR = 1 << 6;
    }
}

// Keep in sync with compiler_ffi.rs
bitflags! {
      pub struct HHBCFlags: u32 {
        const LTR_ASSIGN=1 << 0;
        const UVS=1 << 1;
        // No longer using bit 3.
        const AUTHORITATIVE=1 << 4;
        const JIT_ENABLE_RENAME_FUNCTION=1 << 5;
        const LOG_EXTERN_COMPILER_PERF=1 << 6;
        const ENABLE_INTRINSICS_EXTENSION=1 << 7;
        // No longer using bit 8.
        // No longer using bit 9.
        const EMIT_CLS_METH_POINTERS=1 << 10;
        const EMIT_METH_CALLER_FUNC_POINTERS=1 << 11;
        // No longer using bit 12.
        const ARRAY_PROVENANCE=1 << 13;
        // No longer using bit 14.
        const FOLD_LAZY_CLASS_KEYS=1 << 15;
        // No longer using bit 16.
    }
}

// Mapping must match getParserFlags() in runtime-option.cpp and compiler_ffi.rs
bitflags! {
    pub struct ParserFlags: u32 {
        const ABSTRACT_STATIC_PROPS=1 << 0;
        const ALLOW_NEW_ATTRIBUTE_SYNTAX=1 << 1;
        const ALLOW_UNSTABLE_FEATURES=1 << 2;
        const CONST_DEFAULT_FUNC_ARGS=1 << 3;
        const CONST_STATIC_PROPS=1 << 4;
        // No longer using bits 5-7
        const DISABLE_LVAL_AS_AN_EXPRESSION=1 << 8;
        // No longer using bit 9
        const DISALLOW_INST_METH=1 << 10;
        const DISABLE_XHP_ELEMENT_MANGLING=1 << 11;
        const DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS=1 << 12;
        const DISALLOW_FUNC_PTRS_IN_CONSTANTS=1 << 13;
        // No longer using bit 14.
        // No longer using bit 15.
        const ENABLE_ENUM_CLASSES=1 << 16;
        const ENABLE_XHP_CLASS_MODIFIER=1 << 17;
        // No longer using bits 18-19.
        const ENABLE_CLASS_LEVEL_WHERE_CLAUSES=1 << 20;
  }
}

impl HHBCFlags {
    pub fn from_hhvm_config(config: &HhvmConfig) -> Result<Self> {
        let mut hhbc_options = Self::empty();

        // Only ini version in use
        if let Some(true) = config.get_bool("php7.ltr_assign")? {
            hhbc_options |= Self::LTR_ASSIGN;
        }
        // Only ini version in use
        if let Some(true) = config.get_bool("php7.uvs")? {
            hhbc_options |= Self::UVS;
        }
        // Both variants in use
        if let Some(true) = config.get_bool("Repo.Authoritative")? {
            hhbc_options |= Self::AUTHORITATIVE;
        }
        // HDF uses both Eval.JitEnableRenameFunction and JitEnableRenameFunction
        // ini only uses the hhvm.jit_enable_rename_function
        if let Some(true) = config.get_bool("Eval.JitEnableRenameFunction")? {
            hhbc_options |= Self::JIT_ENABLE_RENAME_FUNCTION;
        }
        if let Some(true) = config.get_bool("JitEnableRenameFunction")? {
            hhbc_options |= Self::JIT_ENABLE_RENAME_FUNCTION;
        }
        // Only hdf version in use
        if let Some(true) = config.get_bool("Eval.LogExternCompilerPerf")? {
            hhbc_options |= Self::LOG_EXTERN_COMPILER_PERF;
        }
        // I think only the hdf is used correctly
        if let Some(true) = config.get_bool("Eval.EnableIntrinsicsExtension")? {
            hhbc_options |= Self::ENABLE_INTRINSICS_EXTENSION;
        }
        // Only the hdf versions used
        if let Some(true) = config.get_bool("Eval.EmitClsMethPointers")? {
            hhbc_options |= Self::EMIT_CLS_METH_POINTERS;
        }
        // Only the hdf versions used. Can kill variant in options_cli.rs
        if config
            .get_bool("Eval.EmitMethCallerFuncPointers")?
            .unwrap_or(true)
        {
            hhbc_options |= Self::EMIT_METH_CALLER_FUNC_POINTERS;
        }
        // ini might use hhvm.array_provenance
        // hdf might use Eval.ArrayProvenance
        // But super unclear here
        if let Some(true) = config.get_bool("Eval.ArrayProvenance")? {
            hhbc_options |= Self::ARRAY_PROVENANCE;
        }
        if let Some(true) = config.get_bool("array_provenance")? {
            hhbc_options |= Self::ARRAY_PROVENANCE;
        }
        // Only hdf version
        if config.get_bool("Eval.FoldLazyClassKeys")?.unwrap_or(true) {
            hhbc_options |= Self::FOLD_LAZY_CLASS_KEYS;
        }
        Ok(hhbc_options)
    }

    fn to_php7_flags(self) -> Php7Flags {
        let mut f = Php7Flags::empty();
        if self.contains(Self::UVS) {
            f |= Php7Flags::UVS;
        }
        if self.contains(Self::LTR_ASSIGN) {
            f |= Php7Flags::LTR_ASSIGN;
        }
        f
    }

    fn to_hhvm_flags(self) -> HhvmFlags {
        let mut f = HhvmFlags::empty();
        if self.contains(Self::ARRAY_PROVENANCE) {
            f |= HhvmFlags::ARRAY_PROVENANCE;
        }
        if self.contains(Self::EMIT_CLS_METH_POINTERS) {
            f |= HhvmFlags::EMIT_CLS_METH_POINTERS;
        }
        if self.contains(Self::EMIT_METH_CALLER_FUNC_POINTERS) {
            f |= HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS;
        }
        if self.contains(Self::ENABLE_INTRINSICS_EXTENSION) {
            f |= HhvmFlags::ENABLE_INTRINSICS_EXTENSION;
        }
        if self.contains(Self::FOLD_LAZY_CLASS_KEYS) {
            f |= HhvmFlags::FOLD_LAZY_CLASS_KEYS;
        }
        if self.contains(Self::JIT_ENABLE_RENAME_FUNCTION) {
            f |= HhvmFlags::JIT_ENABLE_RENAME_FUNCTION;
        }
        if self.contains(Self::LOG_EXTERN_COMPILER_PERF) {
            f |= HhvmFlags::LOG_EXTERN_COMPILER_PERF;
        }
        f
    }

    fn to_repo_flags(self) -> RepoFlags {
        let mut f = RepoFlags::empty();
        if self.contains(Self::AUTHORITATIVE) {
            f |= RepoFlags::AUTHORITATIVE;
        }
        f
    }
}

impl ParserFlags {
    pub fn from_hhvm_config(config: &HhvmConfig) -> Result<Self> {
        let mut parser_options = ParserFlags::empty();
        // Note: Could only find examples of Hack.Lang.AbstractStaticProps
        if let Some(true) = config.get_bool("Hack.Lang.AbstractStaticProps")? {
            parser_options |= ParserFlags::ABSTRACT_STATIC_PROPS;
        }
        // TODO: I'm pretty sure allow_new_attribute_syntax is dead and we can kill this option
        if let Some(true) = config.get_bool("hack.lang.allow_new_attribute_syntax")? {
            parser_options |= ParserFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX;
        }
        // Both hdf and ini versions are being used
        if let Some(true) = config.get_bool("Hack.Lang.AllowUnstableFeatures")? {
            parser_options |= ParserFlags::ALLOW_UNSTABLE_FEATURES;
        }
        // TODO: could not find examples of const_default_func_args, kill it in options_cli.rs
        if let Some(true) = config.get_bool("Hack.Lang.ConstDefaultFuncArgs")? {
            parser_options |= ParserFlags::CONST_DEFAULT_FUNC_ARGS;
        }
        // Only hdf version found in use
        if let Some(true) = config.get_bool("Hack.Lang.ConstStaticProps")? {
            parser_options |= ParserFlags::CONST_STATIC_PROPS;
        }
        // TODO: Kill disable_lval_as_an_expression
        // Only hdf option in use
        if let Some(true) = config.get_bool("Hack.Lang.DisallowInstMeth")? {
            parser_options |= ParserFlags::DISALLOW_INST_METH;
        }
        // Both ini and hdf variants in use
        if let Some(true) = config.get_bool("Hack.Lang.DisableXHPElementMangling")? {
            parser_options |= ParserFlags::DISABLE_XHP_ELEMENT_MANGLING;
        }
        // Both ini and hdf variants in use
        if let Some(true) = config.get_bool("Hack.Lang.DisallowFunAndClsMethPseudoFuncs")? {
            parser_options |= ParserFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS;
        }
        // Only hdf option in use
        if let Some(true) = config.get_bool("Hack.Lang.DisallowFuncPtrsInConstants")? {
            parser_options |= ParserFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS;
        }
        // Only hdf option in use
        if let Some(true) = config.get_bool("Hack.Lang.EnableEnumClasses")? {
            parser_options |= ParserFlags::ENABLE_ENUM_CLASSES;
        }
        // Both options in use
        if let Some(true) = config.get_bool("Hack.Lang.EnableXHPClassModifier")? {
            parser_options |= ParserFlags::ENABLE_XHP_CLASS_MODIFIER;
        }
        // Only hdf option in use. Kill variant in options_cli.rs
        if let Some(true) = config.get_bool("Hack.Lang.EnableClassLevelWhereClauses")? {
            parser_options |= ParserFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES;
        }
        Ok(parser_options)
    }

    fn to_lang_flags(self) -> LangFlags {
        let mut f = LangFlags::empty();
        if self.contains(Self::ABSTRACT_STATIC_PROPS) {
            f |= LangFlags::ABSTRACT_STATIC_PROPS;
        }
        if self.contains(Self::ALLOW_NEW_ATTRIBUTE_SYNTAX) {
            f |= LangFlags::ALLOW_NEW_ATTRIBUTE_SYNTAX;
        }
        if self.contains(Self::ALLOW_UNSTABLE_FEATURES) {
            f |= LangFlags::ALLOW_UNSTABLE_FEATURES;
        }
        if self.contains(Self::CONST_DEFAULT_FUNC_ARGS) {
            f |= LangFlags::CONST_DEFAULT_FUNC_ARGS;
        }
        if self.contains(Self::CONST_STATIC_PROPS) {
            f |= LangFlags::CONST_STATIC_PROPS;
        }
        if self.contains(Self::DISABLE_LVAL_AS_AN_EXPRESSION) {
            f |= LangFlags::DISABLE_LVAL_AS_AN_EXPRESSION;
        }
        if self.contains(Self::DISALLOW_INST_METH) {
            f |= LangFlags::DISALLOW_INST_METH;
        }
        if self.contains(Self::DISABLE_XHP_ELEMENT_MANGLING) {
            f |= LangFlags::DISABLE_XHP_ELEMENT_MANGLING;
        }
        if self.contains(Self::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS) {
            f |= LangFlags::DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS;
        }
        if self.contains(Self::DISALLOW_FUNC_PTRS_IN_CONSTANTS) {
            f |= LangFlags::DISALLOW_FUNC_PTRS_IN_CONSTANTS;
        }
        if self.contains(Self::ENABLE_ENUM_CLASSES) {
            f |= LangFlags::ENABLE_ENUM_CLASSES;
        }
        if self.contains(Self::ENABLE_XHP_CLASS_MODIFIER) {
            f |= LangFlags::ENABLE_XHP_CLASS_MODIFIER;
        }
        if self.contains(Self::ENABLE_CLASS_LEVEL_WHERE_CLAUSES) {
            f |= LangFlags::ENABLE_CLASS_LEVEL_WHERE_CLAUSES;
        }
        f
    }
}

impl<'a> NativeEnv<'a> {
    fn to_options(native_env: &NativeEnv<'a>) -> Options {
        let hhbc_flags = native_env.hhbc_flags;
        let config = [native_env.aliased_namespaces, native_env.include_roots];
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
#[derive(Debug, Default, Clone)]
pub struct Profile {
    /// Time in seconds spent in parsing and lowering.
    pub parsing_t: f64,

    /// Time in seconds spent in emitter.
    pub codegen_t: f64,

    /// Time in seconds spent in bytecode_printer.
    pub printing_t: f64,

    /// Parser arena allocation volume in bytes.
    pub parsing_bytes: i64,

    /// Emitter arena allocation volume in bytes.
    pub codegen_bytes: i64,

    /// Peak stack size during parsing, before lowering.
    pub parse_peak: i64,

    /// Peak stack size during parsing and lowering.
    pub lower_peak: i64,
    pub error_peak: i64,

    /// Peak stack size during codegen
    pub rewrite_peak: i64,
    pub emitter_peak: i64,

    /// Was the log_extern_compiler_perf flag set?
    pub log_enabled: bool,
}

impl std::ops::AddAssign for Profile {
    fn add_assign(&mut self, p: Self) {
        self.parsing_t += p.parsing_t;
        self.codegen_t += p.codegen_t;
        self.printing_t += p.printing_t;
        self.codegen_bytes += p.codegen_bytes;
        self.parse_peak += p.parse_peak;
        self.lower_peak += p.lower_peak;
        self.error_peak += p.error_peak;
        self.rewrite_peak += p.rewrite_peak;
        self.emitter_peak += p.emitter_peak;
    }
}

impl std::ops::Add for Profile {
    type Output = Self;
    fn add(mut self, p2: Self) -> Self {
        self += p2;
        self
    }
}

impl Profile {
    pub fn total_sec(&self) -> f64 {
        self.parsing_t + self.codegen_t + self.printing_t
    }
}

/// Compile Hack source code, write HHAS text to `writer`.
/// Update `profile` with stats from any passes that run,
/// even if the compiler ultimately returns Err.
pub fn from_text<'decl>(
    alloc: &bumpalo::Bump,
    writer: &mut dyn std::io::Write,
    source_text: SourceText<'_>,
    native_env: &NativeEnv<'_>,
    decl_provider: Option<&'decl dyn DeclProvider>,
    profile: &mut Profile,
) -> Result<()> {
    let mut emitter = create_emitter(native_env.flags, native_env, decl_provider, alloc);
    let mut unit = emit_unit_from_text(&mut emitter, native_env.flags, source_text, profile)?;

    if native_env.flags.contains(EnvFlags::ENABLE_IR) {
        let ir = bc_to_ir::bc_to_ir(&unit);
        unit = ir_to_bc::ir_to_bc(alloc, ir);
    }

    unit_to_string(native_env, writer, &unit, profile)?;
    profile.codegen_bytes = alloc.allocated_bytes() as i64;
    Ok(())
}

fn rewrite_and_emit<'p, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    namespace_env: RcOc<NamespaceEnv>,
    ast: &'p mut ast::Program,
    profile: &'p mut Profile,
) -> Result<HackCUnit<'arena>, Error> {
    // First rewrite and modify `ast` in place.
    stack_limit::reset();
    let result = rewrite_program::rewrite_program(emitter, ast, RcOc::clone(&namespace_env));
    profile.rewrite_peak = stack_limit::peak() as i64;
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
    profile.emitter_peak = stack_limit::peak() as i64;
    unit
}

pub fn unit_from_text<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    source_text: SourceText<'_>,
    native_env: &NativeEnv<'_>,
    decl_provider: Option<&'decl dyn DeclProvider>,
    profile: &mut Profile,
) -> Result<HackCUnit<'arena>> {
    let mut emitter = create_emitter(native_env.flags, native_env, decl_provider, alloc);
    emit_unit_from_text(&mut emitter, native_env.flags, source_text, profile)
}

pub fn unit_to_string(
    native_env: &NativeEnv<'_>,
    writer: &mut dyn std::io::Write,
    program: &HackCUnit<'_>,
    profile: &mut Profile,
) -> Result<()> {
    if native_env.flags.contains(EnvFlags::DUMP_IR) {
        let ir = bc_to_ir::bc_to_ir(program);
        struct FmtFromIo<'a>(&'a mut dyn std::io::Write);
        impl fmt::Write for FmtFromIo<'_> {
            fn write_str(&mut self, s: &str) -> fmt::Result {
                self.0.write_all(s.as_bytes()).map_err(|_| fmt::Error)
            }
        }
        let print_result;
        (print_result, profile.printing_t) = time(|| {
            let verbose = false;
            ir::print_unit(&mut FmtFromIo(writer), &ir, verbose)
        });
        print_result?;
    } else {
        let print_result;
        (print_result, profile.printing_t) = time(|| {
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
) -> Result<HackCUnit<'arena>, Error> {
    emit_unit(emitter, namespace, ast)
}

fn check_readonly_and_emit<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    namespace_env: RcOc<NamespaceEnv>,
    ast: &mut ast::Program,
    profile: &mut Profile,
) -> Result<HackCUnit<'arena>, Error> {
    match &emitter.decl_provider {
        // T128303794
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
    flags: EnvFlags,
    source_text: SourceText<'_>,
    profile: &mut Profile,
) -> Result<HackCUnit<'arena>> {
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

    let (parse_result, parsing_t) = time(|| {
        parse_file(
            emitter.options(),
            source_text,
            !flags.contains(EnvFlags::DISABLE_TOPLEVEL_ELABORATION),
            RcOc::clone(&namespace_env),
            flags.contains(EnvFlags::IS_SYSTEMLIB),
            type_directed,
            profile,
        )
    });
    profile.parsing_t = parsing_t;

    let ((unit, profile), codegen_t) = match parse_result {
        Ok(mut ast) => {
            elaborate_namespaces_visitor::elaborate_program(RcOc::clone(&namespace_env), &mut ast);
            time(move || {
                (
                    check_readonly_and_emit(emitter, namespace_env, &mut ast, profile),
                    profile,
                )
            })
        }
        Err(ParseError(pos, msg, fatal_op)) => {
            time(move || (emit_fatal(emitter.alloc, fatal_op, pos, msg), profile))
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
) -> Result<HackCUnit<'arena>, Error> {
    emit_unit::emit_fatal_unit(alloc, fatal_op, pos, msg)
}

fn create_emitter<'arena, 'decl>(
    flags: EnvFlags,
    native_env: &NativeEnv<'_>,
    decl_provider: Option<&'decl dyn DeclProvider>,
    alloc: &'arena bumpalo::Bump,
) -> Emitter<'arena, 'decl> {
    Emitter::new(
        NativeEnv::to_options(native_env),
        flags.contains(EnvFlags::IS_SYSTEMLIB),
        flags.contains(EnvFlags::FOR_DEBUGGER_EVAL),
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
        fail_open: false,
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
                parse_peak,
                lower_peak,
                error_peak,
                arena_bytes,
                ..
            } => {
                profile.parse_peak = parse_peak;
                profile.lower_peak = lower_peak;
                profile.error_peak = error_peak;
                profile.parsing_bytes = arena_bytes;
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
                    None => match aast {
                        Ok(aast) => Ok(aast),
                        Err(msg) => Err(ParseError(Pos::make_none(), msg, FatalOp::Parse)),
                    },
                }
            }
        },
    }
}

fn time<T>(f: impl FnOnce() -> T) -> (T, f64) {
    let (r, t) = profile_rust::time(f);
    (r, t.as_secs_f64())
}

pub fn expr_to_string_lossy(flags: EnvFlags, expr: &ast::Expr) -> String {
    use print_expr::Context;

    let opts = Options::from_configs(&[]).expect("Malformed options");

    let alloc = bumpalo::Bump::new();
    let emitter = Emitter::new(
        opts,
        flags.contains(EnvFlags::IS_SYSTEMLIB),
        flags.contains(EnvFlags::FOR_DEBUGGER_EVAL),
        &alloc,
        None,
    );
    let ctx = Context::new(&emitter);

    print_expr::expr_to_string_lossy(ctx, expr)
}
