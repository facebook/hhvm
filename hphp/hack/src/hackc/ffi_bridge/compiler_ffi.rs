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
use cxx::CxxString;
use decl_provider::DeclProvider;
use external_decl_provider::ExternalDeclProvider;
use facts_rust as facts;
use hhbc::hackc_unit;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use parser_core_types::source_text::SourceText;
use sha1::Digest;
use sha1::Sha1;

#[allow(clippy::derivable_impls)]
#[cxx::bridge]
pub mod compile_ffi {
    struct NativeEnv {
        /// Pointer to decl_provider opaque object, cast to usize. 0 means null.
        decl_provider: usize,

        filepath: String,
        aliased_namespaces: String,
        include_roots: String,
        emit_class_pointers: i32,
        check_int_overflow: i32,

        /// compiler::HHBCFlags
        hhbc_flags: u32,

        /// compiler::ParserFlags
        parser_flags: u32,

        /// compiler::EnvFlags
        flags: u8,
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
        type DeclParserOptions;
        type HackCUnitWrapper;

        fn make_env_flags(
            is_systemlib: bool,
            is_evaled: bool,
            for_debugger_eval: bool,
            disable_toplevel_elaboration: bool,
            enable_ir: bool,
        ) -> u8;

        /// Compile Hack source code to a HackCUnit or an error.
        unsafe fn hackc_compile_unit_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Box<HackCUnitWrapper>>;

        /// Compile Hack source code to either HHAS or an error.
        fn hackc_compile_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Vec<u8>>;

        fn hackc_create_direct_decl_parse_options(
            flags: i32,
            aliased_namespaces: &CxxString,
        ) -> Box<DeclParserOptions>;

        /// Invoke the hackc direct decl parser and return every shallow decl in the file.
        fn hackc_direct_decl_parse(
            options: &DeclParserOptions,
            filename: &CxxString,
            text: &CxxString,
        ) -> DeclResult;

        fn hash_unit(unit: &HackCUnitWrapper) -> [u8; 20];

        /// Return true if this type (class or alias) is in the given Decls.
        fn hackc_type_exists(decls: &DeclResult, symbol: &str) -> bool;

        /// For testing: return true if deserializing produces the expected Decls.
        fn hackc_verify_deserialization(decls: &DeclResult) -> bool;

        /// Serialize a FactsResult to JSON
        fn hackc_facts_to_json_cpp_ffi(facts: FactsResult, pretty: bool) -> String;

        /// Extract Facts from Decls, passing along the source text hash.
        fn hackc_decls_to_facts_cpp_ffi(
            decl_flags: i32,
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

pub struct DeclParserOptions(direct_decl_parser::DeclParserOptions);

pub struct HackCUnitWrapper(hackc_unit::HackCUnit<'static>, bumpalo::Bump);

///////////////////////////////////////////////////////////////////////////////////

fn make_env_flags(
    is_systemlib: bool,
    is_evaled: bool,
    for_debugger_eval: bool,
    disable_toplevel_elaboration: bool,
    enable_ir: bool,
) -> u8 {
    let mut flags = EnvFlags::empty();
    if is_systemlib {
        flags |= EnvFlags::IS_SYSTEMLIB;
    }
    if is_evaled {
        flags |= EnvFlags::IS_EVALED;
    }
    if for_debugger_eval {
        flags |= EnvFlags::FOR_DEBUGGER_EVAL;
    }
    if disable_toplevel_elaboration {
        flags |= EnvFlags::DISABLE_TOPLEVEL_ELABORATION;
    }
    if enable_ir {
        flags |= EnvFlags::ENABLE_IR;
    }
    flags.bits()
}

impl compile_ffi::NativeEnv {
    fn to_compile_env<'a>(env: &'a compile_ffi::NativeEnv) -> Option<compile::NativeEnv<'a>> {
        Some(compile::NativeEnv {
            filepath: RelativePath::make(
                Prefix::Dummy,
                PathBuf::from(OsStr::from_bytes(env.filepath.as_bytes())),
            ),
            aliased_namespaces: &env.aliased_namespaces,
            include_roots: &env.include_roots,
            emit_class_pointers: env.emit_class_pointers,
            check_int_overflow: env.check_int_overflow,
            hhbc_flags: compile::HHBCFlags::from_bits(env.hhbc_flags)?,
            parser_flags: compile::ParserFlags::from_bits(env.parser_flags)?,
            flags: compile::EnvFlags::from_bits(env.flags)?,
        })
    }
}

fn hash_unit(HackCUnitWrapper(unit, _): &HackCUnitWrapper) -> [u8; 20] {
    let mut hasher = Sha1::new();
    let w = std::io::BufWriter::new(&mut hasher);
    bincode::serialize_into(w, unit).unwrap();
    hasher.finalize().into()
}

fn hackc_compile_from_text_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Vec<u8>, String> {
    let native_env = compile_ffi::NativeEnv::to_compile_env(env).unwrap();
    let text = SourceText::make(
        ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
        source_text.as_bytes(),
    );
    let mut output = Vec::new();
    let alloc = bumpalo::Bump::new();
    let decl_allocator = bumpalo::Bump::new();

    let decl_provider = if env.decl_provider != 0 {
        let provider = env.decl_provider as *const c_void;
        Some(ExternalDeclProvider {
            provider,
            arena: &decl_allocator,
        })
    } else {
        None
    };

    compile::from_text(
        &alloc,
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

fn hackc_type_exists(result: &compile_ffi::DeclResult, symbol: &str) -> bool {
    // TODO T123158488: fix case insensitive lookups
    result.decls.decls.types().any(|(sym, _)| sym == symbol)
}

pub fn hackc_create_direct_decl_parse_options(
    flags: i32,
    aliased_namespaces: &CxxString,
) -> Box<DeclParserOptions> {
    let config_opts =
        options::Options::from_configs(&[aliased_namespaces.to_str().unwrap()]).unwrap();
    let auto_namespace_map = match config_opts.hhvm.aliased_namespaces.get().as_map() {
        Some(m) => Vec::from_iter(m.iter().map(|(k, v)| (k.to_owned(), v.to_owned()))),
        None => Vec::new(),
    };
    Box::new(DeclParserOptions(direct_decl_parser::DeclParserOptions {
        auto_namespace_map,
        disable_xhp_element_mangling: ((1 << 0) & flags) != 0,
        interpret_soft_types_as_like_types: ((1 << 1) & flags) != 0,
        allow_new_attribute_syntax: ((1 << 2) & flags) != 0,
        enable_xhp_class_modifier: ((1 << 3) & flags) != 0,
        php5_compat_mode: ((1 << 4) & flags) != 0,
        hhvm_compat_mode: ((1 << 5) & flags) != 0,
        ..Default::default()
    }))
}

pub fn hackc_direct_decl_parse(
    opts: &DeclParserOptions,
    filename: &CxxString,
    text: &CxxString,
) -> compile_ffi::DeclResult {
    let text = text.as_bytes();
    let path = PathBuf::from(OsStr::from_bytes(filename.as_bytes()));
    let filename = RelativePath::make(Prefix::Root, path);
    let arena = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&arena) };
    let parsed_file: ParsedFile<'static> =
        direct_decl_parser::parse_decls_without_reference_text(&opts.0, filename, text, alloc);

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

fn hackc_verify_deserialization(result: &compile_ffi::DeclResult) -> bool {
    let arena = bumpalo::Bump::new();
    let decls = decl_provider::deserialize_decls(&arena, &result.serialized).unwrap();
    decls == result.decls.decls
}

fn hackc_compile_unit_from_text_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Box<HackCUnitWrapper>, String> {
    let bump = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };
    let native_env = compile_ffi::NativeEnv::to_compile_env(env).unwrap();
    let text = SourceText::make(
        ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
        source_text.as_bytes(),
    );

    let decl_allocator = bumpalo::Bump::new();
    let decl_provider = if env.decl_provider != 0 {
        let provider = env.decl_provider as *const c_void;
        Some(ExternalDeclProvider {
            provider,
            arena: &decl_allocator,
        })
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
    .map(|unit| Box::new(HackCUnitWrapper(unit, bump)))
    .map_err(|e| e.to_string())
}

pub fn hackc_facts_to_json_cpp_ffi(facts_result: compile_ffi::FactsResult, pretty: bool) -> String {
    if facts_result.has_errors {
        String::new()
    } else {
        let facts = facts::Facts::from(facts_result.facts);
        facts.to_json(pretty, &facts_result.sha1sum)
    }
}

pub fn hackc_decls_to_facts_cpp_ffi(
    decl_flags: i32,
    decl_result: &compile_ffi::DeclResult,
    sha1sum: &CxxString,
) -> compile_ffi::FactsResult {
    if decl_result.has_errors {
        compile_ffi::FactsResult {
            has_errors: true,
            ..Default::default()
        }
    } else {
        let disable_xhp_element_mangling = ((1 << 0) & decl_flags) != 0;
        let facts = compile_ffi::Facts::from(facts::Facts::from_decls(
            &decl_result.decls.decls,
            decl_result.decls.attributes,
            disable_xhp_element_mangling,
        ));
        compile_ffi::FactsResult {
            facts,
            sha1sum: sha1sum.to_string_lossy().to_string(),
            has_errors: false,
        }
    }
}
