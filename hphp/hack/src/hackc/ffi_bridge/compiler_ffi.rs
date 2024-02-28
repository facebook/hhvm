// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod compiler_ffi_impl;
mod ext_decl;
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
use serde::Deserialize;
use sha1::Digest;
use sha1::Sha1;

#[allow(clippy::derivable_impls)]
#[cxx::bridge(namespace = "HPHP::hackc")]
mod ffi {
    struct NativeEnv {
        /// Pointer to decl_provider opaque object, cast to usize. 0 means null.
        decl_provider: usize,

        filepath: String,
        aliased_namespaces: Vec<StringMapEntry>,
        include_roots: Vec<StringMapEntry>,

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
        optimize_reified_param_checks: bool,
        stress_shallow_decl_deps: bool,
        stress_folded_decl_deps: bool,
        enable_native_enum_class_labels: bool,
        optimize_param_lifetimes: bool,
        optimize_local_lifetimes: bool,
        optimize_local_iterators: bool,
        optimize_is_type_checks: bool,
    }

    struct ParserFlags {
        abstract_static_props: bool,
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
        strict_utf8: bool,
    }

    struct DeclParserConfig {
        aliased_namespaces: Vec<StringMapEntry>,
        disable_xhp_element_mangling: bool,
        interpret_soft_types_as_like_types: bool,
        enable_xhp_class_modifier: bool,
        php5_compat_mode: bool,
        hhvm_compat_mode: bool,
        include_assignment_values: bool,
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

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclAttribute {
        name: String,
        args: Vec<String>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclEnumType {
        base: String,
        constraint: String,
        includes: Vec<String>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclMethodParam {
        name: String,
        type_: String,
        accept_disposable: bool,
        is_inout: bool,
        has_default: bool,
        is_readonly: bool,
        def_value: String,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclSignature {
        tparams: Vec<ExtDeclTparam>,
        where_constraints: Vec<ExtDeclTypeConstraint>,
        return_type: String,
        params: Vec<ExtDeclMethodParam>,
        implicit_params: String,
        cross_package: String,
        return_disposable: bool,
        is_coroutine: bool,
        is_async: bool,
        is_generator: bool,
        instantiated_targs: bool,
        is_function_pointer: bool,
        returns_readonly: bool,
        readonly_this: bool,
        support_dynamic_type: bool,
        is_memoized: bool,
        variadic: bool,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclMethod {
        name: String,
        type_: String,

        attributes: Vec<ExtDeclAttribute>,
        signature: Vec<ExtDeclSignature>,

        // The source is Visibility in ast_defs.rs
        // Private / Public / Protected / Internal
        visibility: String,

        // The source is MethodFlags(u8 enum) in method_flags.rs
        is_abstract: bool,
        is_final: bool,
        is_dynamicallycallable: bool,
        is_override: bool,
        is_php_std_lib: bool,
        supports_dynamic_type: bool,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclFileFunc {
        name: String,
        type_: String,
        module: String,
        internal: bool,
        php_std_lib: bool,
        support_dynamic_type: bool,
        no_auto_dynamic: bool,
        no_auto_likes: bool,
        signature: Vec<ExtDeclSignature>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclModule {
        name: String,
        exports: Vec<String>,
        imports: Vec<String>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclTypeDef {
        name: String,
        module: String,
        visibility: String,
        tparams: Vec<ExtDeclTparam>,
        as_constraint: String,
        super_constraint: String,
        type_: String,
        is_ctx: bool,
        attributes: Vec<ExtDeclAttribute>,
        internal: bool,
        docs_url: String,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclTypeStructureSubType {
        name: String,
        optional: bool,
        type_: ExtDeclTypeStructure,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclTypeStructure {
        type_: String,
        nullable: bool,
        kind: String,
        subtypes: Vec<ExtDeclTypeStructureSubType>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclFileConst {
        name: String,
        type_: String,
        value: String,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclClassConst {
        name: String,
        type_: String,
        is_abstract: bool,
        value: String,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclClassConstVec {
        pub vec: Vec<ExtDeclClassConst>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclClassTypeConst {
        name: String,
        kind: String,
        is_ctx: bool,
        enforceable: bool,
        reifiable: bool,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclTypeConstraint {
        kind: String,
        type_: String,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclTparam {
        variance: String,
        name: String,
        tparams: Vec<ExtDeclTparam>,
        constraints: Vec<ExtDeclTypeConstraint>,
        reified: String,
        user_attributes: Vec<ExtDeclAttribute>,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclProp {
        name: String,
        type_: String,
        visibility: String,
        is_abstract: bool,
        is_const: bool,
        is_lateinit: bool,
        is_readonly: bool,
        needs_init: bool,
        is_php_std_lib: bool,
        is_lsb: bool,
        is_safe_global_variable: bool,
        no_auto_likes: bool,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclClass {
        kind: String, // ClassishKind (cls, iface, trait, enum)
        name: String,

        // Optional strings
        module: String,
        docs_url: String,

        // Flags
        final_: bool,
        abstract_: bool,
        is_xhp: bool,
        internal: bool,
        has_xhp_keyword: bool,
        xhp_marked_empty: bool,
        support_dynamic_type: bool,
        is_strict: bool,

        // Special Params
        tparams: Vec<ExtDeclTparam>,
        where_constraints: Vec<ExtDeclTypeConstraint>,
        xhp_attr_uses: Vec<String>,

        // Implementation
        extends: Vec<String>,
        uses: Vec<String>,
        implements: Vec<String>,
        require_extends: Vec<String>,
        require_implements: Vec<String>,
        require_class: Vec<String>,

        // Nested/Complex Types
        user_attributes: Vec<ExtDeclAttribute>,
        methods: Vec<ExtDeclMethod>,
        static_methods: Vec<ExtDeclMethod>,
        consts: Vec<ExtDeclClassConst>,
        typeconsts: Vec<ExtDeclClassTypeConst>,
        constructor: Vec<ExtDeclMethod>,
        enum_type: Vec<ExtDeclEnumType>,
        props: Vec<ExtDeclProp>,
        sprops: Vec<ExtDeclProp>,
        // Not supported yet
        //xhp_enum_values,
    }

    #[derive(Debug, PartialEq)]
    pub struct ExtDeclFile {
        typedefs: Vec<ExtDeclTypeDef>,
        functions: Vec<ExtDeclFileFunc>,
        constants: Vec<ExtDeclFileConst>,
        file_attributes: Vec<ExtDeclAttribute>,
        modules: Vec<ExtDeclModule>,
        classes: Vec<ExtDeclClass>,
        disable_xhp_element_mangling: bool,
        has_first_pass_parse_errors: bool,
        is_strict: bool,
    }

    #[derive(Default, Debug, PartialEq, Serialize, Deserialize, Clone)]
    pub struct FileFacts {
        pub types: Vec<TypeFacts>,
        pub functions: Vec<String>,
        pub constants: Vec<String>,
        pub modules: Vec<ModuleFacts>,
        pub file_attributes: Vec<AttrFacts>,
        pub sha1sum: String,
    }

    #[derive(Debug, PartialEq, Serialize, Deserialize, Default, Clone)]
    pub struct TypeFacts {
        pub name: String,
        pub kind: TypeKind,
        pub flags: u8,

        /// List of attributes and their arguments
        pub attributes: Vec<AttrFacts>,

        /// List of types which this `extends`, `implements`, or `use`s
        pub base_types: Vec<String>,

        /// List of classes which this `require class`
        pub require_class: Vec<String>,

        /// List of classes or interfaces which this `require extends`
        pub require_extends: Vec<String>,

        /// List of interfaces which this `require implements`
        pub require_implements: Vec<String>,

        pub methods: Vec<MethodFacts>,
    }

    #[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
    pub enum TypeKind {
        Unknown = 0,
        Class = 1,
        Interface = 2,
        Enum = 4,
        Trait = 8,
        TypeAlias = 16,
    }

    /// Represents `<<IAmAnAttribute(0, 'Hello', null)>>` as
    /// `{"IAmAnAttribute", vec[0, "Hello", null]}`
    #[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
    pub struct AttrFacts {
        pub name: String,
        pub args: Vec<String>, // Really Vec<hackc::AttrValue>, but all variants are String
    }

    #[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
    pub struct MethodFacts {
        pub name: String,
        pub attributes: Vec<AttrFacts>,
    }

    // Currently module facts are empty, but added for backward compatibility
    #[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
    pub struct ModuleFacts {
        pub name: String,
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

        /// Extract hackc::Facts encoded as a binary blob from Decls,
        /// including the source text SHA1 hash.
        fn decls_to_facts_binary(decls: &DeclsHolder, sha1sum: &CxxString) -> Result<Vec<u8>>;

        /// Decode a binary facts blob back to hackc::FileFacts
        fn binary_to_facts(json: &CxxString) -> Result<FileFacts>;

        /// Convert an DeclsHolder struct to binary
        fn decls_holder_to_binary(decls: &DeclsHolder) -> Result<Vec<u8>>;

        /// Decode a binary DeclsHolder blob back to DeclsHolder
        fn binary_to_decls_holder(json: &CxxString) -> Result<Box<DeclsHolder>>;

        /// Decode a binary DeclsHolder blob back to DeclsAndBlob
        fn binary_to_decls_and_blob(json: &CxxString) -> Result<DeclsAndBlob>;

        /// Format facts into a human readable string for debugging.
        fn facts_debug(facts: &FileFacts) -> String;

        /// Compute a SHA1 hash of a binary-serialized FileFacts.
        fn hash_facts(facts: &FileFacts) -> [u8; 20];

        /////////////////////// ext_decl.rs API
        ///
        /// Extract TypeDecls from DeclsHolder.
        fn get_file(decls: &DeclsHolder) -> ExtDeclFile;
        fn get_type_structure(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclTypeStructure>;

        fn get_classes(decls: &DeclsHolder) -> Vec<ExtDeclClass>;
        fn get_class(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclClass>;

        fn get_class_methods(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclMethod>;
        fn get_class_method(decls: &DeclsHolder, kls: &str, name: &str) -> Vec<ExtDeclMethod>;

        fn get_class_smethods(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclMethod>;
        fn get_class_smethod(decls: &DeclsHolder, kls: &str, name: &str) -> Vec<ExtDeclMethod>;

        fn get_class_consts(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclClassConst>;
        fn get_class_const(decls: &DeclsHolder, kls: &str, name: &str) -> Vec<ExtDeclClassConst>;

        fn get_class_typeconsts(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclClassTypeConst>;
        fn get_class_typeconst(
            decls: &DeclsHolder,
            kls: &str,
            name: &str,
        ) -> Vec<ExtDeclClassTypeConst>;

        fn get_class_props(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclProp>;
        fn get_class_prop(decls: &DeclsHolder, kls: &str, name: &str) -> Vec<ExtDeclProp>;

        fn get_class_sprops(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclProp>;
        fn get_class_sprop(decls: &DeclsHolder, kls: &str, name: &str) -> Vec<ExtDeclProp>;

        fn get_class_attributes(decls: &DeclsHolder, kls: &str) -> Vec<ExtDeclAttribute>;
        fn get_class_attribute(decls: &DeclsHolder, kls: &str, name: &str)
        -> Vec<ExtDeclAttribute>;

        fn get_file_attributes(decls: &DeclsHolder) -> Vec<ExtDeclAttribute>;
        fn get_file_attribute(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclAttribute>;

        fn get_file_consts(decls: &DeclsHolder) -> Vec<ExtDeclFileConst>;
        fn get_file_const(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclFileConst>;

        fn get_file_funcs(decls: &DeclsHolder) -> Vec<ExtDeclFileFunc>;
        fn get_file_func(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclFileFunc>;

        fn get_file_modules(decls: &DeclsHolder) -> Vec<ExtDeclModule>;
        fn get_file_module(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclModule>;

        fn get_file_typedefs(decls: &DeclsHolder) -> Vec<ExtDeclTypeDef>;
        fn get_file_typedef(decls: &DeclsHolder, name: &str) -> Vec<ExtDeclTypeDef>;

        /// Dereference a BytesId whose newtype has been laundered away by the bridge.
        /// SAFETY: i must have been a valid BytesId from a previous intern_bytes()
        /// call in this process.
        unsafe fn deref_bytes(i: u32) -> &'static [u8];
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
impl ffi::NativeEnv {
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
                parser_options: ParserOptions {
                    po_auto_namespace_map: (self.aliased_namespaces.iter())
                        .map(|e| (e.key.clone(), e.value.clone()))
                        .collect(),
                    po_abstract_static_props: self.parser_flags.abstract_static_props,
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
                    po_nameof_precedence: true,
                    po_strict_utf8: self.parser_flags.strict_utf8,
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
                optimize_reified_param_checks: self.hhbc_flags.optimize_reified_param_checks,
                stress_shallow_decl_deps: self.hhbc_flags.stress_shallow_decl_deps,
                stress_folded_decl_deps: self.hhbc_flags.stress_folded_decl_deps,
                enable_native_enum_class_labels: self.hhbc_flags.enable_native_enum_class_labels,
                optimize_param_lifetimes: self.hhbc_flags.optimize_param_lifetimes,
                optimize_local_lifetimes: self.hhbc_flags.optimize_local_lifetimes,
                optimize_local_iterators: self.hhbc_flags.optimize_local_iterators,
                optimize_is_type_checks: self.hhbc_flags.optimize_is_type_checks,
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
    use bincode::Options;
    let mut hasher = Sha1::new();
    let w = std::io::BufWriter::new(&mut hasher);
    bincode::options()
        .serialize_into(w, &intern::WithIntern(unit))
        .unwrap();
    hasher.finalize().into()
}

fn compile_from_text(env: &ffi::NativeEnv, source_text: &[u8]) -> Result<Vec<u8>, String> {
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
    let input_symbol_formatted = symbol.starts_with('\\');
    // TODO T123158488: fix case insensitive lookups
    holder.parsed_file.decls.types().any(|(mut sym, _)| {
        if !input_symbol_formatted {
            sym = &sym[1..];
        }
        sym == symbol
    })
}

pub fn direct_decl_parse_and_serialize(
    config: &ffi::DeclParserConfig,
    filename: &CxxString,
    text: &[u8],
) -> ffi::DeclsAndBlob {
    match parse_decls(config, filename, text) {
        Ok(decls) | Err(DeclsError(decls, _)) => ffi::DeclsAndBlob {
            serialized: decl_provider::serialize_decls(&decls.parsed_file.decls).unwrap(),
            has_errors: decls.parsed_file.has_first_pass_parse_errors,
            decls,
        },
    }
}

pub fn parse_decls(
    config: &ffi::DeclParserConfig,
    filename: &CxxString,
    text: &[u8],
) -> Result<Box<DeclsHolder>, DeclsError> {
    let decl_opts = DeclParserOptions {
        auto_namespace_map: (config.aliased_namespaces.iter())
            .map(|e| (e.key.clone(), e.value.clone()))
            .collect(),
        disable_xhp_element_mangling: config.disable_xhp_element_mangling,
        interpret_soft_types_as_like_types: config.interpret_soft_types_as_like_types,
        enable_xhp_class_modifier: config.enable_xhp_class_modifier,
        php5_compat_mode: config.php5_compat_mode,
        hhvm_compat_mode: config.hhvm_compat_mode,
        include_assignment_values: config.include_assignment_values,
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

fn verify_deserialization(result: &ffi::DeclsAndBlob) -> bool {
    let arena = bumpalo::Bump::new();
    let decls = decl_provider::deserialize_decls(&arena, &result.serialized).unwrap();
    decls == result.decls.parsed_file.decls
}

fn compile_unit_from_text(
    env: &ffi::NativeEnv,
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

fn decls_to_symbols(holder: &DeclsHolder) -> ffi::FileSymbols {
    facts::Facts::from_decls(&holder.parsed_file).into()
}

fn decls_to_facts_binary(decls: &DeclsHolder, sha1sum: &CxxString) -> Result<Vec<u8>> {
    use bincode::Options;
    let facts = facts::Facts::from_decls(&decls.parsed_file);
    let file_facts = ffi::FileFacts::from_facts(facts, sha1sum.to_string_lossy().into_owned());
    let mut buf = Vec::new();
    bincode::options().serialize_into(&mut buf, &file_facts)?;
    Ok(buf)
}

fn binary_to_facts(blob: &CxxString) -> bincode::Result<ffi::FileFacts> {
    use bincode::Options;
    bincode::options().deserialize_from(blob.as_bytes())
}

fn decls_holder_to_binary(decls: &DeclsHolder) -> bincode::Result<Vec<u8>> {
    use bincode::Options;
    let mut buf = Vec::new();
    bincode::options().serialize_into(&mut buf, &decls.parsed_file)?;
    Ok(buf)
}

fn binary_to_decls_holder(blob: &CxxString) -> bincode::Result<Box<DeclsHolder>> {
    use bincode::Options;
    let data = blob.as_bytes();
    let arena = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&arena) };
    let op = bincode::options().with_native_endian();
    let mut de = bincode::de::Deserializer::from_slice(data, op);
    let de = arena_deserializer::ArenaDeserializer::new(alloc, &mut de);
    let parsed_file = ParsedFile::deserialize(de)?;
    Ok(Box::new(DeclsHolder {
        parsed_file,
        _arena: arena,
    }))
}

fn binary_to_decls_and_blob(blob: &CxxString) -> bincode::Result<ffi::DeclsAndBlob> {
    let decls = binary_to_decls_holder(blob)?;
    Ok(ffi::DeclsAndBlob {
        serialized: decl_provider::serialize_decls(&decls.parsed_file.decls).unwrap(),
        has_errors: decls.parsed_file.has_first_pass_parse_errors,
        decls,
    })
}

fn facts_debug(facts: &ffi::FileFacts) -> String {
    format!("{facts:#?}")
}

fn hash_facts(facts: &ffi::FileFacts) -> [u8; 20] {
    let mut hasher = Sha1::new();
    let w = std::io::BufWriter::new(&mut hasher);
    bincode::serialize_into(w, facts).unwrap();
    hasher.finalize().into()
}

fn get_classes(holder: &DeclsHolder) -> Vec<ffi::ExtDeclClass> {
    ext_decl::get_classes(&holder.parsed_file)
}

fn get_class(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclClass> {
    match ext_decl::get_class(&holder.parsed_file, name) {
        Some(v) => vec![v],
        None => vec![],
    }
}

fn get_class_methods(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclMethod> {
    ext_decl::get_class_methods(&holder.parsed_file, kls, "")
}

fn get_class_method(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclMethod> {
    ext_decl::get_class_methods(&holder.parsed_file, kls, name)
}

fn get_class_smethods(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclMethod> {
    ext_decl::get_class_smethods(&holder.parsed_file, kls, "")
}

fn get_class_smethod(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclMethod> {
    ext_decl::get_class_smethods(&holder.parsed_file, kls, name)
}

fn get_class_consts(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclClassConst> {
    ext_decl::get_class_consts(&holder.parsed_file, kls, "")
}

fn get_class_const(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclClassConst> {
    ext_decl::get_class_consts(&holder.parsed_file, kls, name)
}

fn get_class_typeconsts(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclClassTypeConst> {
    ext_decl::get_class_typeconsts(&holder.parsed_file, kls, "")
}

fn get_class_typeconst(
    holder: &DeclsHolder,
    kls: &str,
    name: &str,
) -> Vec<ffi::ExtDeclClassTypeConst> {
    ext_decl::get_class_typeconsts(&holder.parsed_file, kls, name)
}

fn get_class_props(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclProp> {
    ext_decl::get_class_props(&holder.parsed_file, kls, "")
}

fn get_class_prop(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclProp> {
    ext_decl::get_class_props(&holder.parsed_file, kls, name)
}

fn get_class_sprops(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclProp> {
    ext_decl::get_class_sprops(&holder.parsed_file, kls, "")
}

fn get_class_sprop(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclProp> {
    ext_decl::get_class_sprops(&holder.parsed_file, kls, name)
}

fn get_class_attributes(holder: &DeclsHolder, kls: &str) -> Vec<ffi::ExtDeclAttribute> {
    ext_decl::get_class_attributes(&holder.parsed_file, kls, "")
}

fn get_class_attribute(holder: &DeclsHolder, kls: &str, name: &str) -> Vec<ffi::ExtDeclAttribute> {
    ext_decl::get_class_attributes(&holder.parsed_file, kls, name)
}

fn get_file_attributes(holder: &DeclsHolder) -> Vec<ffi::ExtDeclAttribute> {
    ext_decl::get_file_attributes(&holder.parsed_file, "")
}

fn get_file_attribute(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclAttribute> {
    ext_decl::get_file_attributes(&holder.parsed_file, name)
}

fn get_file_consts(holder: &DeclsHolder) -> Vec<ffi::ExtDeclFileConst> {
    ext_decl::get_file_consts(&holder.parsed_file, "")
}

fn get_file_const(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclFileConst> {
    ext_decl::get_file_consts(&holder.parsed_file, name)
}

fn get_file_funcs(holder: &DeclsHolder) -> Vec<ffi::ExtDeclFileFunc> {
    ext_decl::get_file_funcs(&holder.parsed_file, "")
}

fn get_file_func(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclFileFunc> {
    ext_decl::get_file_funcs(&holder.parsed_file, name)
}

fn get_file_modules(holder: &DeclsHolder) -> Vec<ffi::ExtDeclModule> {
    ext_decl::get_file_modules(&holder.parsed_file, "")
}

fn get_file_module(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclModule> {
    ext_decl::get_file_modules(&holder.parsed_file, name)
}

fn get_file_typedefs(holder: &DeclsHolder) -> Vec<ffi::ExtDeclTypeDef> {
    ext_decl::get_file_typedefs(&holder.parsed_file, "")
}

fn get_file_typedef(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclTypeDef> {
    ext_decl::get_file_typedefs(&holder.parsed_file, name)
}

fn get_file(holder: &DeclsHolder) -> ffi::ExtDeclFile {
    ext_decl::get_file(&holder.parsed_file)
}

fn get_type_structure(holder: &DeclsHolder, name: &str) -> Vec<ffi::ExtDeclTypeStructure> {
    ext_decl::get_type_structure(&holder.parsed_file, name)
}

// SAFETY: i must be a raw bitcast from a valid BytesId
unsafe fn deref_bytes(i: u32) -> &'static [u8] {
    use intern::InternId;
    let biased = i.try_into().expect("raw id should be nonzero");
    intern::string::BytesId::from_raw(biased).as_bytes()
}
