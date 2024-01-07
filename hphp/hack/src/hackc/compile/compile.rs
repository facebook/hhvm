// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(clippy::todo)]

pub mod dump_expr_tree;

use std::collections::HashSet;
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
use options::HhbcFlags;
use options::Hhvm;
use options::Options;
use options::ParserOptions;
use oxidized::ast;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::namespace_env::Env as NamespaceEnv;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::ExperimentalFeature;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast_check_error::NastCheckError;
use oxidized::parsing_error::ParsingError;
use oxidized::pos::Pos;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::ErrorType;
use relative_path::Prefix;
use relative_path::RelativePath;
use serde::Deserialize;
use serde::Serialize;
use thiserror::Error;

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
        // Keep in sync with RepoOptionsFlags::initDeclConfig() in runtime-option.cpp
        let lang_flags = &self.hhvm.parser_options;
        DeclParserOptions {
            auto_namespace_map,
            disable_xhp_element_mangling: lang_flags.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: true,
            enable_xhp_class_modifier: lang_flags.po_enable_xhp_class_modifier,
            php5_compat_mode: true,
            hhvm_compat_mode: true,
            keep_user_attributes: true,
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
    let mut emitter = create_emitter(native_env, decl_provider, &alloc);
    let mut unit = emit_unit_from_text(
        &mut emitter,
        &native_env.flags,
        source_text,
        profile,
        &elab::CodegenOpts::default(),
    )?;

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
    namespace_env: Arc<NamespaceEnv>,
    ast: &'p mut ast::Program,
    profile: &'p mut Profile,
    invalid_utf8_offset: Option<usize>,
) -> Result<Unit<'arena>, Error> {
    // First rewrite and modify `ast` in place.
    stack_limit::reset();
    let result = rewrite_program::rewrite_program(emitter, ast, Arc::clone(&namespace_env));
    profile.rewrite_peak = stack_limit::peak() as u64;
    stack_limit::reset();
    let unit = match result {
        Ok(()) => {
            // Rewrite ok, now emit.
            emit_unit_from_ast(emitter, namespace_env, ast, invalid_utf8_offset)
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
    unit_from_text_with_opts(
        alloc,
        source_text,
        native_env,
        decl_provider,
        profile,
        &elab::CodegenOpts::default(),
    )
}

pub fn unit_from_text_with_opts<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    source_text: SourceText<'_>,
    native_env: &NativeEnv,
    decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
    profile: &mut Profile,
    opts: &elab::CodegenOpts,
) -> Result<Unit<'arena>> {
    let mut emitter = create_emitter(native_env, decl_provider, alloc);
    emit_unit_from_text(&mut emitter, &native_env.flags, source_text, profile, opts)
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
    namespace: Arc<NamespaceEnv>,
    ast: &mut ast::Program,
    invalid_utf8_offset: Option<usize>,
) -> Result<Unit<'arena>, Error> {
    emit_unit(emitter, namespace, ast, invalid_utf8_offset)
}

fn create_namespace_env(emitter: &Emitter<'_, '_>) -> NamespaceEnv {
    NamespaceEnv::empty(
        emitter.options().hhvm.aliased_namespaces_cloned().collect(),
        true, /* is_codegen */
        emitter
            .options()
            .hhvm
            .parser_options
            .po_disable_xhp_element_mangling,
    )
}

fn emit_unit_from_text<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    flags: &EnvFlags,
    source_text: SourceText<'_>,
    profile: &mut Profile,
    opts: &elab::CodegenOpts,
) -> Result<Unit<'arena>> {
    profile.log_enabled = emitter.options().log_extern_compiler_perf();
    let type_directed = emitter.decl_provider.is_some();

    let namespace_env = Arc::new(create_namespace_env(emitter));
    let path = source_text.file_path_rc();

    let invalid_utf8_offset = match std::str::from_utf8(source_text.text()) {
        Ok(_) => None,
        Err(e) => Some(e.valid_up_to()),
    };

    let parse_result = parse_file(
        emitter.options(),
        source_text,
        !flags.disable_toplevel_elaboration,
        Arc::clone(&namespace_env),
        flags.is_systemlib,
        emitter.for_debugger_eval,
        type_directed,
        profile,
    );

    let ((unit, profile), codegen_t) = match parse_result {
        Ok(mut ast) => {
            match elab::elaborate_program_for_codegen(
                Arc::clone(&namespace_env),
                &path,
                &mut ast,
                opts,
            ) {
                Ok(()) => profile_rust::time(move || {
                    (
                        rewrite_and_emit(
                            emitter,
                            namespace_env,
                            &mut ast,
                            profile,
                            invalid_utf8_offset,
                        ),
                        profile,
                    )
                }),
                Err(errs) => profile_rust::time(move || {
                    (
                        emit_fatal_naming_phase_error(emitter.alloc, &errs[0]),
                        profile,
                    )
                }),
            }
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

fn emit_fatal_naming_phase_error<'arena>(
    alloc: &'arena bumpalo::Bump,
    err: &NamingPhaseError,
) -> Result<Unit<'arena>, Error> {
    match err {
        NamingPhaseError::Naming(err) => emit_fatal_naming_error(alloc, err),
        NamingPhaseError::NastCheck(err) => emit_fatal_nast_check_error(alloc, err),
        NamingPhaseError::UnexpectedHint(_) => todo!(),
        NamingPhaseError::MalformedAccess(_) => todo!(),
        NamingPhaseError::ExperimentalFeature(err) => {
            emit_fatal_experimental_feature_error(alloc, err)
        }
        NamingPhaseError::Parsing(err) => emit_fatal_parsing_error(alloc, err),
    }
}

fn emit_fatal_naming_error<'arena>(
    alloc: &'arena bumpalo::Bump,
    err: &NamingError,
) -> Result<Unit<'arena>, Error> {
    match err {
        NamingError::UnsupportedTraitUseAs(_) => todo!(),
        NamingError::UnsupportedInsteadOf(_) => todo!(),
        NamingError::UnexpectedArrow { .. } => todo!(),
        NamingError::MissingArrow { .. } => todo!(),
        NamingError::DisallowedXhpType { .. } => todo!(),
        NamingError::NameIsReserved { .. } => todo!(),
        NamingError::DollardollarUnused(_) => todo!(),
        NamingError::MethodNameAlreadyBound { .. } => todo!(),
        NamingError::ErrorNameAlreadyBound { .. } => todo!(),
        NamingError::UnboundName { .. } => todo!(),
        NamingError::InvalidFunPointer { .. } => todo!(),
        NamingError::Undefined { .. } => todo!(),
        NamingError::UndefinedInExprTree { .. } => todo!(),
        NamingError::ThisReserved(_) => todo!(),
        NamingError::StartWithT(_) => todo!(),
        NamingError::AlreadyBound { .. } => todo!(),
        NamingError::UnexpectedTypedef { .. } => todo!(),
        NamingError::FieldNameAlreadyBound(_) => todo!(),
        NamingError::PrimitiveTopLevel(_) => todo!(),
        NamingError::PrimitiveInvalidAlias { .. } => todo!(),
        NamingError::DynamicNewInStrictMode(_) => todo!(),
        NamingError::InvalidTypeAccessRoot { .. } => todo!(),
        NamingError::DuplicateUserAttribute { .. } => todo!(),
        NamingError::InvalidMemoizeLabel { .. } => todo!(),
        NamingError::UnboundAttributeName { .. } => todo!(),
        NamingError::ThisNoArgument(_) => todo!(),
        NamingError::ObjectCast(_) => todo!(),
        NamingError::ThisHintOutsideClass(_) => todo!(),
        NamingError::ParentOutsideClass(_) => todo!(),
        NamingError::SelfOutsideClass(_) => todo!(),
        NamingError::StaticOutsideClass(_) => todo!(),
        NamingError::ThisTypeForbidden { .. } => todo!(),
        NamingError::NonstaticPropertyWithLsb(_) => todo!(),
        NamingError::LowercaseThis { .. } => todo!(),
        NamingError::ClassnameParam(_) => todo!(),
        NamingError::TparamAppliedToType { .. } => todo!(),
        NamingError::TparamWithTparam { .. } => todo!(),
        NamingError::ShadowedTparam { .. } => todo!(),
        NamingError::MissingTypehint(_) => todo!(),
        NamingError::ExpectedVariable(_) => todo!(),
        NamingError::TooManyArguments(_) => todo!(),
        NamingError::TooFewArguments(_) => todo!(),
        NamingError::ExpectedCollection { .. } => todo!(),
        NamingError::IllegalCLASS(_) => todo!(),
        NamingError::IllegalTRAIT(_) => todo!(),
        NamingError::IllegalFun(_) => todo!(),
        NamingError::IllegalMemberVariableClass(_) => todo!(),
        NamingError::IllegalMethFun(_) => todo!(),
        NamingError::IllegalInstMeth(_) => todo!(),
        NamingError::IllegalMethCaller(_) => todo!(),
        NamingError::IllegalClassMeth(_) => todo!(),
        NamingError::LvarInObjGet { .. } => todo!(),
        NamingError::ClassMethNonFinalSelf { .. } => todo!(),
        NamingError::ClassMethNonFinalCLASS { .. } => todo!(),
        NamingError::ConstWithoutTypehint { .. } => todo!(),
        NamingError::PropWithoutTypehint { .. } => todo!(),
        NamingError::IllegalConstant(_) => todo!(),
        NamingError::InvalidRequireImplements(_) => todo!(),
        NamingError::InvalidRequireExtends(_) => todo!(),
        NamingError::InvalidRequireClass(_) => todo!(),
        NamingError::DidYouMean { .. } => todo!(),
        NamingError::UsingInternalClass { .. } => todo!(),
        NamingError::TooFewTypeArguments(_) => todo!(),
        NamingError::DynamicClassNameInStrictMode(_) => todo!(),
        NamingError::XhpOptionalRequiredAttr { .. } => todo!(),
        NamingError::XhpRequiredWithDefault { .. } => todo!(),
        NamingError::ArrayTypehintsDisallowed(_) => todo!(),
        NamingError::WildcardHintDisallowed(_) => todo!(),
        NamingError::WildcardTparamDisallowed(_) => todo!(),
        NamingError::IllegalUseOfDynamicallyCallable { .. } => todo!(),
        NamingError::ParentInFunctionPointer { .. } => todo!(),
        NamingError::SelfInNonFinalFunctionPointer { .. } => todo!(),
        NamingError::InvalidWildcardContext(_) => todo!(),
        NamingError::ReturnOnlyTypehint { .. } => todo!(),
        NamingError::UnexpectedTypeArguments(_) => todo!(),
        NamingError::TooManyTypeArguments(_) => todo!(),
        NamingError::ThisAsLexicalVariable(_) => todo!(),
        NamingError::HKTUnsupportedFeature { .. } => todo!(),
        NamingError::HKTPartialApplication { .. } => todo!(),
        NamingError::HKTWildcard(_) => todo!(),
        NamingError::HKTImplicitArgument { .. } => todo!(),
        NamingError::HKTClassWithConstraintsUsed { .. } => todo!(),
        NamingError::HKTAliasWithImplicitConstraints { .. } => todo!(),
        NamingError::ExplicitConsistentConstructor { .. } => todo!(),
        NamingError::ModuleDeclarationOutsideAllowedFiles(_) => todo!(),
        NamingError::InternalModuleLevelTrait(_) => todo!(),
        NamingError::DynamicMethodAccess(_) => todo!(),
        NamingError::DeprecatedUse { .. } => todo!(),
        NamingError::UnnecessaryAttribute { .. } => todo!(),
        NamingError::TparamNonShadowingReuse { .. } => todo!(),
        NamingError::DynamicHintDisallowed(_) => todo!(),
        NamingError::IllegalTypedLocal {
            join,
            id_pos,
            id_name,
            def_pos: _,
        } => {
            // For now, we can only generate this particular error. All of the
            // infrastructure for displaying naming errors is in OCaml, and until
            // the naming phase is completely ported, we can just special case the
            // ones that might come up.
            let msg = if *join {
                "It is assigned in another branch. Consider moving the definition to an enclosing block."
            } else {
                "It is already defined. Typed locals must have their type declared before they can be assigned."
            };
            emit_unit::emit_fatal_unit(
                alloc,
                FatalOp::Parse,
                id_pos.clone(),
                format!("Illegal definition of typed local variable {id_name}. {msg}"),
            )
        }
        NamingError::ToplevelStatement(_) => todo!(),
    }
}

fn emit_fatal_nast_check_error<'arena>(
    _alloc: &'arena bumpalo::Bump,
    err: &NastCheckError,
) -> Result<Unit<'arena>, Error> {
    match err {
        NastCheckError::RepeatedRecordFieldName { .. } => todo!(),
        NastCheckError::DynamicallyCallableReified(_) => todo!(),
        NastCheckError::NoConstructParent(_) => todo!(),
        NastCheckError::NonstaticMethodInAbstractFinalClass(_) => todo!(),
        NastCheckError::ConstructorRequired { .. } => todo!(),
        NastCheckError::NotInitialized { .. } => todo!(),
        NastCheckError::CallBeforeInit { .. } => todo!(),
        NastCheckError::AbstractWithBody(_) => todo!(),
        NastCheckError::NotAbstractWithoutTypeconst(_) => todo!(),
        NastCheckError::TypeconstDependsOnExternalTparam { .. } => todo!(),
        NastCheckError::InterfaceWithPartialTypeconst(_) => todo!(),
        NastCheckError::PartiallyAbstractTypeconstDefinition(_) => todo!(),
        NastCheckError::RefinementInTypestruct { .. } => todo!(),
        NastCheckError::MultipleXhpCategory(_) => todo!(),
        NastCheckError::ReturnInGen(_) => todo!(),
        NastCheckError::ReturnInFinally(_) => todo!(),
        NastCheckError::ToplevelBreak(_) => todo!(),
        NastCheckError::ToplevelContinue(_) => todo!(),
        NastCheckError::ContinueInSwitch(_) => todo!(),
        NastCheckError::AwaitInSyncFunction { .. } => todo!(),
        NastCheckError::InterfaceUsesTrait(_) => todo!(),
        NastCheckError::StaticMemoizedFunction(_) => todo!(),
        NastCheckError::Magic { .. } => todo!(),
        NastCheckError::NonInterface { .. } => todo!(),
        NastCheckError::ToStringReturnsString(_) => todo!(),
        NastCheckError::ToStringVisibility(_) => todo!(),
        NastCheckError::UsesNonTrait { .. } => todo!(),
        NastCheckError::RequiresNonClass { .. } => todo!(),
        NastCheckError::RequiresFinalClass { .. } => todo!(),
        NastCheckError::AbstractBody(_) => todo!(),
        NastCheckError::InterfaceWithMemberVariable(_) => todo!(),
        NastCheckError::InterfaceWithStaticMemberVariable(_) => todo!(),
        NastCheckError::IllegalFunctionName { .. } => todo!(),
        NastCheckError::EntrypointArguments(_) => todo!(),
        NastCheckError::EntrypointGenerics(_) => todo!(),
        NastCheckError::VariadicMemoize(_) => todo!(),
        NastCheckError::AbstractMethodMemoize(_) => todo!(),
        NastCheckError::InstancePropertyInAbstractFinalClass(_) => todo!(),
        NastCheckError::InoutParamsSpecial(_) => todo!(),
        NastCheckError::InoutParamsMemoize { .. } => todo!(),
        NastCheckError::InoutInTransformedPseudofunction { .. } => todo!(),
        NastCheckError::ReadingFromAppend(_) => todo!(),
        NastCheckError::ListRvalue(_) => todo!(),
        NastCheckError::IllegalDestructor(_) => todo!(),
        NastCheckError::IllegalContext { .. } => todo!(),
        NastCheckError::CaseFallthrough { .. } => todo!(),
        NastCheckError::DefaultFallthrough(_) => todo!(),
        NastCheckError::PhpLambdaDisallowed(_) => todo!(),
        NastCheckError::InternalMethodWithInvalidVisibility { .. } => todo!(),
        NastCheckError::PrivateAndFinal(_) => todo!(),
        NastCheckError::InternalMemberInsidePublicTrait { .. } => todo!(),
        NastCheckError::AttributeConflictingMemoize { .. } => todo!(),
        NastCheckError::SoftInternalWithoutInternal(_) => todo!(),
        NastCheckError::WrongExpressionKindBuiltinAttribute { .. } => todo!(),
        NastCheckError::AttributeTooManyArguments { .. } => todo!(),
        NastCheckError::AttributeTooFewArguments { .. } => todo!(),
        NastCheckError::AttributeNotExactNumberOfArgs { .. } => todo!(),
        NastCheckError::AttributeParamType { .. } => todo!(),
        NastCheckError::AttributeNoAutoDynamic(_) => todo!(),
        NastCheckError::GenericAtRuntime { .. } => todo!(),
        NastCheckError::GenericsNotAllowed(_) => todo!(),
        NastCheckError::LocalVariableModifiedAndUsed { .. } => todo!(),
        NastCheckError::LocalVariableModifiedTwice { .. } => todo!(),
        NastCheckError::AssignDuringCase(_) => todo!(),
        NastCheckError::ReadBeforeWrite { .. } => todo!(),
        NastCheckError::LateinitWithDefault(_) => todo!(),
        NastCheckError::MissingAssign(_) => todo!(),
    }
}

fn emit_fatal_experimental_feature_error<'arena>(
    _alloc: &'arena bumpalo::Bump,
    err: &ExperimentalFeature,
) -> Result<Unit<'arena>, Error> {
    match err {
        ExperimentalFeature::LikeType(_) => todo!(),
        ExperimentalFeature::Supportdyn(_) => todo!(),
        ExperimentalFeature::ConstAttr(_) => todo!(),
        ExperimentalFeature::ConstStaticProp(_) => todo!(),
    }
}

fn emit_fatal_parsing_error<'arena>(
    alloc: &'arena bumpalo::Bump,
    err: &ParsingError,
) -> Result<Unit<'arena>, Error> {
    match err {
        ParsingError::FixmeFormat(_) => todo!(),
        ParsingError::HhIgnoreComment(_) => todo!(),
        ParsingError::ParsingError {
            pos,
            msg,
            quickfixes: _,
        } => emit_unit::emit_fatal_unit(alloc, FatalOp::Parse, pos.clone(), msg.clone()),
        ParsingError::XhpParsingError { .. } => todo!(),
        ParsingError::PackageConfigError { .. } => todo!(),
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
    native_env: &NativeEnv,
    decl_provider: Option<Arc<dyn DeclProvider<'decl> + 'decl>>,
    alloc: &'arena bumpalo::Bump,
) -> Emitter<'arena, 'decl> {
    Emitter::new(
        NativeEnv::to_options(native_env),
        native_env.flags.is_systemlib,
        native_env.flags.for_debugger_eval,
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
    namespace_env: Arc<NamespaceEnv>,
    is_systemlib: bool,
    for_debugger_eval: bool,
    type_directed: bool,
    profile: &mut Profile,
) -> Result<ast::Program, ParseError> {
    let aast_env = AastEnv {
        codegen: true,
        php5_compat_mode: !opts.hhbc.uvs,
        is_systemlib,
        for_debugger_eval,
        elaborate_namespaces,
        parser_options: create_parser_options(opts, type_directed),
        ..AastEnv::default()
    };

    let indexed_source_text = IndexedSourceText::new(source_text);
    let ast_result = AastParser::from_text_with_namespace_env(
        &aast_env,
        namespace_env,
        &indexed_source_text,
        HashSet::default(),
    );
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
