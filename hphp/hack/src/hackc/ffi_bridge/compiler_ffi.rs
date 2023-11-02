// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Module containing conversion methods between the Rust Facts and
// Rust/C++ shared Facts (in the compile_ffi module)
mod compiler_ffi_impl;
pub mod external_decl_provider;

use std::ffi::c_void;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::PathBuf;
use std::sync::Arc;

use anyhow::Result;
use compile::EnvFlags;
use cxx::CxxString;
use decl_provider::DeclProvider;
use decl_provider::SelfProvider;
use direct_decl_parser::DeclParserOptions;
use direct_decl_parser::ParsedFile;
use external_decl_provider::ExternalDeclProvider;
use hhbc::Unit;
use options::HhbcFlags;
use options::Hhvm;
use options::ParserOptions;
use parser_core_types::source_text::SourceText;
use relative_path::Prefix;
use relative_path::RelativePath;
use sha1::Digest;
use sha1::Sha1;

#[allow(clippy::derivable_impls)]
#[cxx::bridge(namespace = "HPHP::hackc")]
pub mod compile_ffi {
    enum JitEnableRenameFunction {
        Disable,
        Enable,
        RestrictedEnable,
    }
    struct NativeEnv {
        /// Pointer to decl_provider opaque object, cast to usize. 0 means null.
        decl_provider: usize,

        filepath: String,
        aliased_namespaces: Vec<StringMapEntry>,
        include_roots: Vec<StringMapEntry>,
        renamable_functions: Vec<String>,
        non_interceptable_functions: Vec<String>,
        jit_enable_rename_function: JitEnableRenameFunction,

        hhbc_flags: HhbcFlags,
        parser_flags: ParserFlags,
        flags: EnvFlags,
    }

    struct StringMapEntry {
        key: String,
        value: String,
    }

    /// compiler::EnvFlags exposed to C++
    struct EnvFlags {
        is_systemlib: bool,
        for_debugger_eval: bool,
        disable_toplevel_elaboration: bool,
        enable_ir: bool,
    }

    /// compiler::HhbcFlags exposed to C++
    struct HhbcFlags {
        ltr_assign: bool,
        uvs: bool,
        log_extern_compiler_perf: bool,
        enable_intrinsics_extension: bool,
        emit_cls_meth_pointers: bool,
        emit_meth_caller_func_pointers: bool,
        fold_lazy_class_keys: bool,
        readonly_nonlocal_infer: bool,
        optimize_reified_param_checks: bool,
        stress_shallow_decl_deps: bool,
        stress_folded_decl_deps: bool,
        enable_native_enum_class_labels: bool,
    }

    struct ParserFlags {
        abstract_static_props: bool,
        allow_new_attribute_syntax: bool,
        allow_unstable_features: bool,
        const_default_func_args: bool,
        const_static_props: bool,
        disable_lval_as_an_expression: bool,
        disallow_inst_meth: bool,
        disable_xhp_element_mangling: bool,
        disallow_func_ptrs_in_constants: bool,
        enable_xhp_class_modifier: bool,
        enable_class_level_where_clauses: bool,
        disallow_static_constants_in_default_func_args: bool,
        disallow_direct_superglobals_refs: bool,
    }

    struct DeclParserConfig {
        aliased_namespaces: Vec<StringMapEntry>,
        disable_xhp_element_mangling: bool,
        interpret_soft_types_as_like_types: bool,
        allow_new_attribute_syntax: bool,
        enable_xhp_class_modifier: bool,
        php5_compat_mode: bool,
        hhvm_compat_mode: bool,
    }

    pub struct DeclsAndBlob {
        serialized: Vec<u8>,
        decls: Box<DeclsHolder>,
        has_errors: bool,
    }

    /// Toplevel symbols from a single source file
    #[derive(Debug, Default, PartialEq)]
    pub struct FileSymbols {
        types: Vec<String>,
        functions: Vec<String>,
        constants: Vec<String>,
        modules: Vec<String>,
    }

    extern "Rust" {
        type DeclsHolder;
        type UnitWrapper;

        /// Compile Hack source code to a Unit or an error.
        unsafe fn compile_unit_from_text(
            env: &NativeEnv,
            source_text: &[u8],
        ) -> Result<Box<UnitWrapper>>;

        /// Compile Hack source code to either HHAS or an error.
        fn compile_from_text(env: &NativeEnv, source_text: &[u8]) -> Result<Vec<u8>>;

        /// Invoke the hackc direct decl parser and return every shallow decl in the file,
        /// as well as a serialized blob holding the same content.
        fn direct_decl_parse_and_serialize(
            config: &DeclParserConfig,
            filename: &CxxString,
            text: &[u8],
        ) -> DeclsAndBlob;

        /// Invoke the hackc direct decl parser and return every shallow decl in the file.
        /// Return Err(_) if there were decl parsing errors, which will translate to
        /// throwing an exception to the C++ caller.
        fn parse_decls(
            config: &DeclParserConfig,
            filename: &CxxString,
            text: &[u8],
        ) -> Result<Box<DeclsHolder>>;

        fn hash_unit(unit: &UnitWrapper) -> [u8; 20];

        /// Return true if this type (class or alias) is in the given Decls.
        fn type_exists(decls: &DeclsHolder, symbol: &str) -> bool;

        /// For testing: return true if deserializing produces the expected Decls.
        fn verify_deserialization(decls: &DeclsAndBlob) -> bool;

        /// Extract toplevel symbols from Decls
        fn decls_to_symbols(decls: &DeclsHolder) -> FileSymbols;

        /// Extract hackc::Facts in condensed JSON format from Decls,
        /// including the source text SHA1 hash.
        fn decls_to_facts_json(decls: &DeclsHolder, sha1sum: &CxxString) -> String;
    }
}

// Opaque to C++, so we don't need repr(C).
#[derive(Debug)]
pub struct DeclsHolder {
    _arena: bumpalo::Bump,
    parsed_file: ParsedFile<'static>,
}

// This is accessed in test_ffi.cpp; hence repr(C)
#[derive(Debug)]
#[repr(C)]
pub struct UnitWrapper(Unit<'static>, bumpalo::Bump);

///////////////////////////////////////////////////////////////////////////////////
impl From<compile_ffi::JitEnableRenameFunction> for options::JitEnableRenameFunction {
    fn from(other: compile_ffi::JitEnableRenameFunction) -> Self {
        match other {
            compile_ffi::JitEnableRenameFunction::Enable => Self::Enable,
            compile_ffi::JitEnableRenameFunction::Disable => Self::Disable,
            compile_ffi::JitEnableRenameFunction::RestrictedEnable => Self::RestrictedEnable,
            _ => panic!("Enum value does not match one of listed variants"),
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////
impl compile_ffi::NativeEnv {
    fn to_compile_env(&self) -> Option<compile::NativeEnv> {
        Some(compile::NativeEnv {
            filepath: RelativePath::make(
                Prefix::Dummy,
                PathBuf::from(OsStr::from_bytes(self.filepath.as_bytes())),
            ),
            hhvm: Hhvm {
                include_roots: (self.include_roots.iter())
                    .map(|e| (e.key.clone().into(), e.value.clone().into()))
                    .collect(),
                renamable_functions: self
                    .renamable_functions
                    .iter()
                    .map(|e| e.clone().into())
                    .collect(),
                jit_enable_rename_function: self.jit_enable_rename_function.into(),
                non_interceptable_functions: self
                    .non_interceptable_functions
                    .iter()
                    .map(|e| e.clone().into())
                    .collect(),
                parser_options: ParserOptions {
                    po_auto_namespace_map: (self.aliased_namespaces.iter())
                        .map(|e| (e.key.clone(), e.value.clone()))
                        .collect(),
                    po_abstract_static_props: self.parser_flags.abstract_static_props,
                    po_allow_new_attribute_syntax: self.parser_flags.allow_new_attribute_syntax,
                    po_allow_unstable_features: self.parser_flags.allow_unstable_features,
                    po_const_default_func_args: self.parser_flags.const_default_func_args,
                    tco_const_static_props: self.parser_flags.const_static_props,
                    po_disable_lval_as_an_expression: self
                        .parser_flags
                        .disable_lval_as_an_expression,
                    po_disable_xhp_element_mangling: self.parser_flags.disable_xhp_element_mangling,
                    po_disallow_func_ptrs_in_constants: self
                        .parser_flags
                        .disallow_func_ptrs_in_constants,
                    po_enable_xhp_class_modifier: self.parser_flags.enable_xhp_class_modifier,
                    po_enable_class_level_where_clauses: self
                        .parser_flags
                        .enable_class_level_where_clauses,
                    po_disallow_static_constants_in_default_func_args: self
                        .parser_flags
                        .disallow_static_constants_in_default_func_args,
                    po_disallow_direct_superglobals_refs: self
                        .parser_flags
                        .disallow_direct_superglobals_refs,
                    ..Default::default()
                },
            },
            hhbc_flags: HhbcFlags {
                ltr_assign: self.hhbc_flags.ltr_assign,
                uvs: self.hhbc_flags.uvs,
                log_extern_compiler_perf: self.hhbc_flags.log_extern_compiler_perf,
                enable_intrinsics_extension: self.hhbc_flags.enable_intrinsics_extension,
                emit_cls_meth_pointers: self.hhbc_flags.emit_cls_meth_pointers,
                emit_meth_caller_func_pointers: self.hhbc_flags.emit_meth_caller_func_pointers,
                fold_lazy_class_keys: self.hhbc_flags.fold_lazy_class_keys,
                readonly_nonlocal_infer: self.hhbc_flags.readonly_nonlocal_infer,
                optimize_reified_param_checks: self.hhbc_flags.optimize_reified_param_checks,
                stress_shallow_decl_deps: self.hhbc_flags.stress_shallow_decl_deps,
                stress_folded_decl_deps: self.hhbc_flags.stress_folded_decl_deps,
                enable_native_enum_class_labels: self.hhbc_flags.enable_native_enum_class_labels,
                ..Default::default()
            },
            flags: EnvFlags {
                is_systemlib: self.flags.is_systemlib,
                for_debugger_eval: self.flags.for_debugger_eval,
                disable_toplevel_elaboration: self.flags.disable_toplevel_elaboration,
                enable_ir: self.flags.enable_ir,
                ..Default::default()
            },
        })
    }
}

fn hash_unit(UnitWrapper(unit, _): &UnitWrapper) -> [u8; 20] {
    let mut hasher = Sha1::new();
    let w = std::io::BufWriter::new(&mut hasher);
    bincode::serialize_into(w, unit).unwrap();
    hasher.finalize().into()
}

fn compile_from_text(env: &compile_ffi::NativeEnv, source_text: &[u8]) -> Result<Vec<u8>, String> {
    let native_env = env.to_compile_env().unwrap();
    let text = SourceText::make(
        std::sync::Arc::new(native_env.filepath.clone()),
        source_text,
    );
    let decl_allocator = bumpalo::Bump::new();

    let external_decl_provider: Option<Arc<dyn DeclProvider<'_> + '_>> = if env.decl_provider != 0 {
        Some(Arc::new(ExternalDeclProvider::new(
            env.decl_provider as *const c_void,
            &decl_allocator,
        )))
    } else {
        None
    };

    let decl_provider = SelfProvider::wrap_existing_provider(
        external_decl_provider,
        native_env.to_decl_parser_options(),
        text.clone(),
        &decl_allocator,
    );

    let mut output = Vec::new();
    compile::from_text(
        &mut output,
        text,
        &native_env,
        decl_provider,
        &mut Default::default(),
    )
    .map_err(|e| e.to_string())?;
    Ok(output)
}

fn type_exists(holder: &DeclsHolder, symbol: &str) -> bool {
    // TODO T123158488: fix case insensitive lookups
    holder
        .parsed_file
        .decls
        .types()
        .any(|(sym, _)| *sym == symbol)
}

pub fn direct_decl_parse_and_serialize(
    config: &compile_ffi::DeclParserConfig,
    filename: &CxxString,
    text: &[u8],
) -> compile_ffi::DeclsAndBlob {
    match parse_decls(config, filename, text) {
        Ok(decls) | Err(DeclsError(decls, _)) => compile_ffi::DeclsAndBlob {
            serialized: decl_provider::serialize_decls(&decls.parsed_file.decls).unwrap(),
            has_errors: decls.parsed_file.has_first_pass_parse_errors,
            decls,
        },
    }
}

pub fn parse_decls(
    config: &compile_ffi::DeclParserConfig,
    filename: &CxxString,
    text: &[u8],
) -> Result<Box<DeclsHolder>, DeclsError> {
    let decl_opts = DeclParserOptions {
        auto_namespace_map: (config.aliased_namespaces.iter())
            .map(|e| (e.key.clone(), e.value.clone()))
            .collect(),
        disable_xhp_element_mangling: config.disable_xhp_element_mangling,
        interpret_soft_types_as_like_types: config.interpret_soft_types_as_like_types,
        allow_new_attribute_syntax: config.allow_new_attribute_syntax,
        enable_xhp_class_modifier: config.enable_xhp_class_modifier,
        php5_compat_mode: config.php5_compat_mode,
        hhvm_compat_mode: config.hhvm_compat_mode,
        keep_user_attributes: true,
        ..Default::default()
    };
    let path = PathBuf::from(OsStr::from_bytes(filename.as_bytes()));
    let relpath = RelativePath::make(Prefix::Root, path);
    let arena = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&arena) };
    let parsed_file: ParsedFile<'static> =
        direct_decl_parser::parse_decls_for_bytecode(&decl_opts, relpath, text, alloc);
    let holder = Box::new(DeclsHolder {
        parsed_file,
        _arena: arena,
    });
    match holder.parsed_file.has_first_pass_parse_errors {
        false => Ok(holder),
        true => Err(DeclsError(
            holder,
            PathBuf::from(OsStr::from_bytes(filename.as_bytes())),
        )),
    }
}

#[derive(thiserror::Error, Debug)]
#[error("{}: File contained first-pass parse errors", .1.display())]
pub struct DeclsError(Box<DeclsHolder>, PathBuf);

fn verify_deserialization(result: &compile_ffi::DeclsAndBlob) -> bool {
    let arena = bumpalo::Bump::new();
    let decls = decl_provider::deserialize_decls(&arena, &result.serialized).unwrap();
    decls == result.decls.parsed_file.decls
}

fn compile_unit_from_text(
    env: &compile_ffi::NativeEnv,
    source_text: &[u8],
) -> Result<Box<UnitWrapper>, String> {
    let bump = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };
    let native_env = env.to_compile_env().unwrap();
    let text = SourceText::make(
        std::sync::Arc::new(native_env.filepath.clone()),
        source_text,
    );

    let decl_allocator = bumpalo::Bump::new();
    let external_decl_provider: Option<Arc<dyn DeclProvider<'_> + '_>> = if env.decl_provider != 0 {
        Some(Arc::new(ExternalDeclProvider::new(
            env.decl_provider as *const c_void,
            &decl_allocator,
        )))
    } else {
        None
    };

    let decl_provider = SelfProvider::wrap_existing_provider(
        external_decl_provider,
        native_env.to_decl_parser_options(),
        text.clone(),
        &decl_allocator,
    );

    compile::unit_from_text(
        alloc,
        text,
        &native_env,
        decl_provider,
        &mut Default::default(),
    )
    .map(|unit| Box::new(UnitWrapper(unit, bump)))
    .map_err(|e| e.to_string())
}

fn decls_to_symbols(holder: &DeclsHolder) -> compile_ffi::FileSymbols {
    facts::Facts::from_decls(&holder.parsed_file).into()
}

fn decls_to_facts_json(decls: &DeclsHolder, sha1sum: &CxxString) -> String {
    let facts = facts::Facts::from_decls(&decls.parsed_file);
    facts.to_json(false, &sha1sum.to_string_lossy())
}
