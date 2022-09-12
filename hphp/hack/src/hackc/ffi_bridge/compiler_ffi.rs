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

use anyhow::Result;
use compile::EnvFlags;
use compile::HhbcFlags;
use compile::ParserFlags;
use cxx::CxxString;
use decl_provider::DeclProvider;
use direct_decl_parser::DeclParserOptions;
use external_decl_provider::ExternalDeclProvider;
use facts_rust as facts;
use hhbc::Unit;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use parser_core_types::source_text::SourceText;
use sha1::Digest;
use sha1::Sha1;

#[allow(clippy::derivable_impls)]
#[cxx::bridge(namespace = "HPHP::hackc")]
pub mod compile_ffi {
    struct NativeEnv {
        /// Pointer to decl_provider opaque object, cast to usize. 0 means null.
        decl_provider: usize,

        filepath: String,
        aliased_namespaces: String,
        include_roots: String,
        emit_class_pointers: i32,
        check_int_overflow: i32,

        hhbc_flags: HhbcFlags,
        parser_flags: ParserFlags,
        flags: EnvFlags,
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
        repo_authoritative: bool,
        jit_enable_rename_function: bool,
        log_extern_compiler_perf: bool,
        enable_intrinsics_extension: bool,
        emit_cls_meth_pointers: bool,
        emit_meth_caller_func_pointers: bool,
        fold_lazy_class_keys: bool,
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
        disallow_fun_and_cls_meth_pseudo_funcs: bool,
        disallow_func_ptrs_in_constants: bool,
        enable_enum_classes: bool,
        enable_xhp_class_modifier: bool,
        enable_class_level_where_clauses: bool,
    }

    struct DeclParserConfig {
        aliased_namespaces: String,
        disable_xhp_element_mangling: bool,
        interpret_soft_types_as_like_types: bool,
        allow_new_attribute_syntax: bool,
        enable_xhp_class_modifier: bool,
        php5_compat_mode: bool,
        hhvm_compat_mode: bool,
    }

    pub struct DeclResult {
        nopos_hash: u64,
        serialized: Vec<u8>,
        decls: Box<DeclsHolder>,
        has_errors: bool,
    }

    #[derive(Debug)]
    enum TypeKind {
        Class,
        Record,
        Interface,
        Enum,
        Trait,
        TypeAlias,
        Unknown,
        Mixed,
    }

    #[derive(Debug, PartialEq)]
    struct Attribute {
        name: String,
        args: Vec<String>,
    }

    #[derive(Debug, PartialEq)]
    struct MethodFacts {
        attributes: Vec<Attribute>,
    }

    #[derive(Debug, PartialEq)]
    struct Method {
        name: String,
        methfacts: MethodFacts,
    }

    #[derive(Debug, PartialEq)]
    pub struct TypeFacts {
        pub base_types: Vec<String>,
        pub kind: TypeKind,
        pub attributes: Vec<Attribute>,
        pub flags: isize,
        pub require_extends: Vec<String>,
        pub require_implements: Vec<String>,
        pub methods: Vec<Method>,
    }

    #[derive(Debug, PartialEq)]
    struct TypeFactsByName {
        name: String,
        typefacts: TypeFacts,
    }

    #[derive(Debug, PartialEq)]
    struct ModuleFactsByName {
        name: String,
        // Currently does not have modulefacts, since it would be an empty struct
        // modulefacts
    }

    #[derive(Debug, Default, PartialEq)]
    struct Facts {
        pub types: Vec<TypeFactsByName>,
        pub functions: Vec<String>,
        pub constants: Vec<String>,
        pub file_attributes: Vec<Attribute>,
        pub modules: Vec<ModuleFactsByName>,
    }

    #[derive(Debug, Default)]
    pub struct FactsResult {
        facts: Facts,
        sha1sum: String,
        has_errors: bool,
    }

    extern "Rust" {
        type DeclsHolder;
        type UnitWrapper;

        /// Compile Hack source code to a Unit or an error.
        unsafe fn compile_unit_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Box<UnitWrapper>>;

        /// Compile Hack source code to either HHAS or an error.
        fn compile_from_text_cpp_ffi(env: &NativeEnv, source_text: &CxxString) -> Result<Vec<u8>>;

        /// Invoke the hackc direct decl parser and return every shallow decl in the file.
        fn direct_decl_parse(
            config: &DeclParserConfig,
            filename: &CxxString,
            text: &CxxString,
        ) -> DeclResult;

        fn hash_unit(unit: &UnitWrapper) -> [u8; 20];

        /// Return true if this type (class or alias) is in the given Decls.
        fn type_exists(decls: &DeclResult, symbol: &str) -> bool;

        /// For testing: return true if deserializing produces the expected Decls.
        fn verify_deserialization(decls: &DeclResult) -> bool;

        /// Serialize a FactsResult to JSON
        fn facts_to_json_cpp_ffi(facts: FactsResult, pretty: bool) -> String;

        /// Extract Facts from Decls, passing along the source text hash.
        fn decls_to_facts_cpp_ffi(
            decl_config: &DeclParserConfig,
            decls: &DeclResult,
            sha1sum: &CxxString,
        ) -> FactsResult;
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Opaque to C++, so we don't need repr(C).

pub struct DeclsHolder {
    _arena: bumpalo::Bump,
    decls: Decls<'static>,
    attributes: &'static [&'static oxidized_by_ref::typing_defs::UserAttribute<'static>],
}

pub struct UnitWrapper(Unit<'static>, bumpalo::Bump);

///////////////////////////////////////////////////////////////////////////////////

impl compile_ffi::NativeEnv {
    fn to_compile_env(&self) -> Option<compile::NativeEnv> {
        Some(compile::NativeEnv {
            filepath: RelativePath::make(
                Prefix::Dummy,
                PathBuf::from(OsStr::from_bytes(self.filepath.as_bytes())),
            ),
            aliased_namespaces: self.aliased_namespaces.clone(),
            include_roots: self.include_roots.clone(),
            emit_class_pointers: self.emit_class_pointers,
            check_int_overflow: self.check_int_overflow,
            hhbc_flags: HhbcFlags {
                ltr_assign: self.hhbc_flags.ltr_assign,
                uvs: self.hhbc_flags.uvs,
                repo_authoritative: self.hhbc_flags.repo_authoritative,
                jit_enable_rename_function: self.hhbc_flags.jit_enable_rename_function,
                log_extern_compiler_perf: self.hhbc_flags.log_extern_compiler_perf,
                enable_intrinsics_extension: self.hhbc_flags.enable_intrinsics_extension,
                emit_cls_meth_pointers: self.hhbc_flags.emit_cls_meth_pointers,
                emit_meth_caller_func_pointers: self.hhbc_flags.emit_meth_caller_func_pointers,
                fold_lazy_class_keys: self.hhbc_flags.fold_lazy_class_keys,
                ..Default::default()
            },
            parser_flags: ParserFlags {
                abstract_static_props: self.parser_flags.abstract_static_props,
                allow_new_attribute_syntax: self.parser_flags.allow_new_attribute_syntax,
                allow_unstable_features: self.parser_flags.allow_unstable_features,
                const_default_func_args: self.parser_flags.const_default_func_args,
                const_static_props: self.parser_flags.const_static_props,
                disable_lval_as_an_expression: self.parser_flags.disable_lval_as_an_expression,
                disallow_inst_meth: self.parser_flags.disallow_inst_meth,
                disable_xhp_element_mangling: self.parser_flags.disable_xhp_element_mangling,
                disallow_fun_and_cls_meth_pseudo_funcs: self
                    .parser_flags
                    .disallow_fun_and_cls_meth_pseudo_funcs,
                disallow_func_ptrs_in_constants: self.parser_flags.disallow_func_ptrs_in_constants,
                enable_enum_classes: self.parser_flags.enable_enum_classes,
                enable_xhp_class_modifier: self.parser_flags.enable_xhp_class_modifier,
                enable_class_level_where_clauses: self
                    .parser_flags
                    .enable_class_level_where_clauses,
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

fn compile_from_text_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Vec<u8>, String> {
    let native_env = env.to_compile_env().unwrap();
    let text = SourceText::make(
        ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
        source_text.as_bytes(),
    );
    let decl_allocator = bumpalo::Bump::new();

    let decl_provider = if env.decl_provider != 0 {
        Some(ExternalDeclProvider::new(
            env.decl_provider as *const c_void,
            &decl_allocator,
        ))
    } else {
        None
    };

    let mut output = Vec::new();
    compile::from_text(
        &mut output,
        text,
        &native_env,
        decl_provider
            .as_ref()
            .map(|provider| provider as &dyn DeclProvider),
        &mut Default::default(),
    )
    .map_err(|e| e.to_string())?;
    Ok(output)
}

fn type_exists(result: &compile_ffi::DeclResult, symbol: &str) -> bool {
    // TODO T123158488: fix case insensitive lookups
    result.decls.decls.types().any(|(sym, _)| sym == symbol)
}

pub fn direct_decl_parse(
    config: &compile_ffi::DeclParserConfig,
    filename: &CxxString,
    text: &CxxString,
) -> compile_ffi::DeclResult {
    let options = options::Options::from_configs(&[&config.aliased_namespaces]).unwrap();
    let auto_namespace_map = match options.hhvm.aliased_namespaces.get().as_map() {
        Some(m) => Vec::from_iter(m.iter().map(|(k, v)| (k.to_owned(), v.to_owned()))),
        None => Vec::new(),
    };
    let decl_opts = DeclParserOptions {
        auto_namespace_map,
        disable_xhp_element_mangling: config.disable_xhp_element_mangling,
        interpret_soft_types_as_like_types: config.interpret_soft_types_as_like_types,
        allow_new_attribute_syntax: config.allow_new_attribute_syntax,
        enable_xhp_class_modifier: config.enable_xhp_class_modifier,
        php5_compat_mode: config.php5_compat_mode,
        hhvm_compat_mode: config.hhvm_compat_mode,
        ..Default::default()
    };
    let text = text.as_bytes();
    let path = PathBuf::from(OsStr::from_bytes(filename.as_bytes()));
    let filename = RelativePath::make(Prefix::Root, path);
    let arena = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&arena) };
    let parsed_file: ParsedFile<'static> =
        direct_decl_parser::parse_decls_without_reference_text(&decl_opts, filename, text, alloc);

    compile_ffi::DeclResult {
        nopos_hash: no_pos_hash::position_insensitive_hash(&parsed_file.decls),
        serialized: decl_provider::serialize_decls(&parsed_file.decls).unwrap(),
        decls: Box::new(DeclsHolder {
            decls: parsed_file.decls,
            attributes: parsed_file.file_attributes,
            _arena: arena,
        }),
        has_errors: parsed_file.has_first_pass_parse_errors,
    }
}

fn verify_deserialization(result: &compile_ffi::DeclResult) -> bool {
    let arena = bumpalo::Bump::new();
    let decls = decl_provider::deserialize_decls(&arena, &result.serialized).unwrap();
    decls == result.decls.decls
}

fn compile_unit_from_text_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Box<UnitWrapper>, String> {
    let bump = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };
    let native_env = env.to_compile_env().unwrap();
    let text = SourceText::make(
        ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
        source_text.as_bytes(),
    );

    let decl_allocator = bumpalo::Bump::new();
    let decl_provider = if env.decl_provider != 0 {
        Some(ExternalDeclProvider::new(
            env.decl_provider as *const c_void,
            &decl_allocator,
        ))
    } else {
        None
    };

    compile::unit_from_text(
        alloc,
        text,
        &native_env,
        decl_provider
            .as_ref()
            .map(|provider| provider as &dyn DeclProvider),
        &mut Default::default(),
    )
    .map(|unit| Box::new(UnitWrapper(unit, bump)))
    .map_err(|e| e.to_string())
}

pub fn facts_to_json_cpp_ffi(facts_result: compile_ffi::FactsResult, pretty: bool) -> String {
    if facts_result.has_errors {
        String::new()
    } else {
        let facts = facts::Facts::from(facts_result.facts);
        facts.to_json(pretty, &facts_result.sha1sum)
    }
}

pub fn decls_to_facts_cpp_ffi(
    decl_config: &compile_ffi::DeclParserConfig,
    decl_result: &compile_ffi::DeclResult,
    sha1sum: &CxxString,
) -> compile_ffi::FactsResult {
    if decl_result.has_errors {
        compile_ffi::FactsResult {
            has_errors: true,
            ..Default::default()
        }
    } else {
        let facts = compile_ffi::Facts::from(facts::Facts::from_decls(
            &decl_result.decls.decls,
            decl_result.decls.attributes,
            decl_config.disable_xhp_element_mangling,
        ));
        compile_ffi::FactsResult {
            facts,
            sha1sum: sha1sum.to_string_lossy().to_string(),
            has_errors: false,
        }
    }
}
