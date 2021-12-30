// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Module containing conversion methods between the Rust Facts and
// Rust/C++ shared Facts (in the compile_ffi module)
mod rust_compile_ffi_impl;

use anyhow::{anyhow, Result};
use arena_deserializer::serde::Deserialize;
use bincode::Options;
use compile::EnvFlags;
use cxx::CxxString;
use external_decl_provider::{ExternalDeclProvider, ExternalDeclProviderResult};
use facts_rust::{facts, facts_parser};
use libc::{c_char, c_int};
use no_pos_hash::position_insensitive_hash;
use oxidized::file_info::NameType;
use oxidized::relative_path::{Prefix, RelativePath};
use oxidized_by_ref::decl_parser_options;
use parser_core_types::source_text::SourceText;

#[cxx::bridge]
pub mod compile_ffi {
    struct NativeEnv {
        decl_provider: usize,
        decl_getter: usize,
        filepath: String,
        aliased_namespaces: String,
        include_roots: String,
        emit_class_pointers: i32,
        check_int_overflow: i32,
        hhbc_flags: u32,
        parser_flags: u32,
        flags: u8,
    }

    pub struct DeclResult {
        hash: u64,
        serialized: Box<Bytes>,
        decls: Box<Decls>,
        attributes: Box<FileAttributes>,
        bump: Box<Bump>,
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

    #[derive(Debug, Default, PartialEq)]
    struct Facts {
        pub types: Vec<TypeFactsByName>,
        pub functions: Vec<String>,
        pub constants: Vec<String>,
        pub type_aliases: Vec<String>,
        pub file_attributes: Vec<Attribute>,
    }

    #[derive(Debug, Default)]
    pub struct FactsResult {
        facts: Facts,
        md5sum: String,
        sha1sum: String,
        has_errors: bool,
    }

    extern "Rust" {
        type Bump;
        type Bytes;
        type Decls;
        type DeclParserOptions;
        type HhasProgramWrapper;
        type FileAttributes;

        fn make_env_flags(
            is_systemlib: bool,
            is_evaled: bool,
            for_debugger_eval: bool,
            dump_symbol_refs: bool,
            disable_toplevel_elaboration: bool,
            enable_decl: bool,
        ) -> u8;

        unsafe fn hackc_compile_hhas_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Box<HhasProgramWrapper>>;

        fn hackc_compile_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Vec<u8>>;

        fn hackc_create_direct_decl_parse_options(
            flags: i32,
            aliased_namespaces: &CxxString,
        ) -> Box<DeclParserOptions>;

        fn hackc_direct_decl_parse(
            options: &DeclParserOptions,
            filename: &CxxString,
            text: &CxxString,
        ) -> DeclResult;

        fn hackc_decl_exists(
            decls: &Decls,
            kind: i32, /* HPHP::AutoloadMap::KindOf */
            symbol: &str,
        ) -> bool;
        fn hackc_print_decls(decls: &Decls);
        fn hackc_print_serialized_size(bytes: &Bytes);
        unsafe fn hackc_verify_deserialization(serialized: &Bytes, expected: &Decls) -> bool;
        fn hackc_hhas_to_string_cpp_ffi(
            env: &NativeEnv,
            prog: &HhasProgramWrapper,
        ) -> Result<Vec<u8>>;

        fn hackc_extract_facts_as_json_cpp_ffi(
            flags: i32,
            filename: &CxxString,
            source_text: &CxxString,
        ) -> String;

        fn hackc_extract_facts_cpp_ffi(
            flags: i32,
            filename: &CxxString,
            source_text: &CxxString,
        ) -> FactsResult;

        fn hackc_facts_to_json_cpp_ffi(facts: FactsResult, source_text: &CxxString) -> String;

        unsafe fn hackc_decls_to_facts_cpp_ffi(
            decl_flags: i32,
            decl_result: &DeclResult,
            source_text: &CxxString,
        ) -> FactsResult;
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Opaque to C++.

#[repr(C)]
pub struct Bump(bumpalo::Bump);
#[repr(C)]
pub struct Bytes(ffi::Bytes);
#[repr(C)]
pub struct Decls(direct_decl_parser::Decls<'static>);
#[repr(C)]
pub struct FileAttributes(&'static [&'static oxidized_by_ref::typing_defs::UserAttribute<'static>]);
#[repr(C)]
pub struct DeclParserOptions(
    decl_parser_options::DeclParserOptions<'static>,
    bumpalo::Bump,
);

#[repr(C)]
pub struct HhasProgramWrapper(hhas_program::HhasProgram<'static>, bumpalo::Bump);

///////////////////////////////////////////////////////////////////////////////////

fn make_env_flags(
    is_systemlib: bool,
    is_evaled: bool,
    for_debugger_eval: bool,
    dump_symbol_refs: bool,
    disable_toplevel_elaboration: bool,
    enable_decl: bool,
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
    if dump_symbol_refs {
        flags |= EnvFlags::DUMP_SYMBOL_REFS;
    }
    if disable_toplevel_elaboration {
        flags |= EnvFlags::DISABLE_TOPLEVEL_ELABORATION;
    }
    if enable_decl {
        flags |= EnvFlags::ENABLE_DECL;
    }
    flags.bits()
}

impl compile_ffi::NativeEnv {
    fn to_compile_env<'a>(env: &'a compile_ffi::NativeEnv) -> Option<compile::NativeEnv<&'a str>> {
        use std::os::unix::ffi::OsStrExt;
        Some(compile::NativeEnv {
            filepath: RelativePath::make(
                oxidized::relative_path::Prefix::Dummy,
                std::path::PathBuf::from(std::ffi::OsStr::from_bytes(env.filepath.as_bytes())),
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

fn hackc_compile_from_text_cpp_ffi<'a>(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Vec<u8>, String> {
    stack_limit::with_elastic_stack(|stack_limit| {
        let native_env: compile::NativeEnv<&str> =
            compile_ffi::NativeEnv::to_compile_env(&env).unwrap();
        let compile_env = compile::Env::<&str> {
            filepath: native_env.filepath.clone(),
            config_jsons: vec![],
            config_list: vec![],
            flags: native_env.flags,
        };

        let text = SourceText::make(
            ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
            source_text.as_bytes(),
        );
        let mut output = Vec::new();
        let alloc = bumpalo::Bump::new();
        let decl_getter_ptr = env.decl_getter as *const ();
        let hhvm_provider_ptr = env.decl_provider as *const ();
        let c_decl_getter_fn = unsafe {
            std::mem::transmute::<
                *const (),
                extern "C" fn(
                    *const std::ffi::c_void,
                    c_int,
                    *const c_char,
                ) -> ExternalDeclProviderResult<'static>,
            >(decl_getter_ptr)
        };
        let c_hhvm_provider_ptr =
            unsafe { std::mem::transmute::<*const (), *const std::ffi::c_void>(hhvm_provider_ptr) };

        let decl_allocator = bumpalo::Bump::new();
        compile::from_text(
            &alloc,
            &compile_env,
            &stack_limit,
            &mut output,
            text,
            Some(&native_env),
            unified_decl_provider::DeclProvider::ExternalDeclProvider(ExternalDeclProvider::new(
                c_decl_getter_fn,
                c_hhvm_provider_ptr,
                &decl_allocator,
            )),
        )?;
        Ok(output)
    })
    .map_err(|e| e.to_string())?
    .map_err(|e: anyhow::Error| format!("{}", e))
}

fn hackc_decl_exists(
    decls: &Decls,
    kind: i32, /* HPHP::AutoloadMap::KindOf */
    symbol: &str,
) -> bool {
    let kind = match kind {
        0 => NameType::Class,
        1 => NameType::Fun,
        2 => NameType::Const,
        3 => NameType::Typedef,
        _ => panic!("Requested kind of decl is not an available option"),
    };
    decls.0.get(kind, symbol) != None
}

fn hackc_print_decls<'a>(decls: &Decls) {
    println!("{:#?}", decls.0)
}

fn hackc_print_serialized_size(serialized: &Bytes) {
    println!("Decl-serialized size: {:#?}", serialized.0.len);
}

pub fn hackc_create_direct_decl_parse_options(
    flags: i32,
    aliased_namespaces: &CxxString,
) -> Box<DeclParserOptions> {
    let bump = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };
    let config_opts =
        options::Options::from_configs(&[aliased_namespaces.to_str().unwrap()], &[]).unwrap();
    let auto_namespace_map = match config_opts.hhvm.aliased_namespaces.get().as_map() {
        Some(m) => bumpalo::collections::Vec::from_iter_in(
            m.iter().map(|(k, v)| {
                (
                    alloc.alloc_str(k.as_str()) as &str,
                    alloc.alloc_str(v.as_str()) as &str,
                )
            }),
            alloc,
        ),
        None => {
            bumpalo::vec![in alloc;]
        }
    }
    .into_bump_slice();

    let opts = decl_parser_options::DeclParserOptions {
        auto_namespace_map,
        disable_xhp_element_mangling: ((1 << 0) & flags) != 0,
        interpret_soft_types_as_like_types: ((1 << 1) & flags) != 0,
        allow_new_attribute_syntax: ((1 << 2) & flags) != 0,
        enable_xhp_class_modifier: ((1 << 3) & flags) != 0,
        php5_compat_mode: ((1 << 4) & flags) != 0,
        hhvm_compat_mode: ((1 << 5) & flags) != 0,
        ..Default::default()
    };

    Box::new(DeclParserOptions(opts, bump))
}

impl compile_ffi::DeclResult {
    fn new(
        hash: u64,
        serialized: Bytes,
        decls: Decls,
        attributes: FileAttributes,
        bump: Bump,
        has_errors: bool,
    ) -> Self {
        Self {
            hash,
            serialized: Box::new(serialized),
            decls: Box::new(decls),
            attributes: Box::new(attributes),
            bump: Box::new(bump),
            has_errors,
        }
    }
}

pub fn hackc_direct_decl_parse(
    opts: &DeclParserOptions,
    filename: &CxxString,
    text: &CxxString,
) -> compile_ffi::DeclResult {
    use std::os::unix::ffi::OsStrExt;

    let bump = bumpalo::Bump::new();
    let alloc: &'static bumpalo::Bump =
        unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };

    let opts: &decl_parser_options::DeclParserOptions<'static> = &opts.0;
    let opts: &decl_parser_options::DeclParserOptions<'static> = unsafe {
        std::mem::transmute::<
            &'_ decl_parser_options::DeclParserOptions<'static>,
            &'static decl_parser_options::DeclParserOptions<'static>,
        >(opts)
    };

    let text = text.as_bytes();
    let path = std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes()));
    let filename = RelativePath::make(Prefix::Root, path);
    let result: direct_decl_parser::ParsedFile<'static> =
        direct_decl_parser::parse_decls_without_reference_text(opts, filename, text, alloc, None);

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let data = op
        .serialize(&result.decls)
        .map_err(|e| format!("failed to serialize, error: {}", e))
        .unwrap();

    compile_ffi::DeclResult::new(
        position_insensitive_hash(&result.decls),
        Bytes(ffi::Bytes::from(data)),
        Decls(result.decls),
        FileAttributes(result.file_attributes),
        Bump(bump),
        result.has_first_pass_parse_errors,
    )
}

unsafe fn hackc_verify_deserialization(serialized: &Bytes, expected: &Decls) -> bool {
    let arena = bumpalo::Bump::new();

    let data = std::slice::from_raw_parts(serialized.0.data, serialized.0.len);

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let mut de = bincode::de::Deserializer::from_slice(data, op);

    let de = arena_deserializer::ArenaDeserializer::new(&arena, &mut de);
    let decls = direct_decl_parser::Decls::deserialize(de)
        .map_err(|e| format!("failed to deserialize, error: {}", e))
        .unwrap();

    decls == expected.0
}

fn hackc_compile_hhas_from_text_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Box<HhasProgramWrapper>, String> {
    stack_limit::with_elastic_stack(|stack_limit| {
        let bump = bumpalo::Bump::new();
        let alloc: &'static bumpalo::Bump =
            unsafe { std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&bump) };
        let native_env: compile::NativeEnv<&str> =
            compile_ffi::NativeEnv::to_compile_env(&env).unwrap();
        let compile_env = compile::Env::<&str> {
            filepath: native_env.filepath.clone(),
            config_jsons: vec![],
            config_list: vec![],
            flags: native_env.flags,
        };
        let text = SourceText::make(
            ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
            source_text.as_bytes(),
        );
        let decl_getter_ptr = env.decl_getter as *const ();
        let hhvm_provider_ptr = env.decl_provider as *const ();
        let c_decl_getter_fn = unsafe {
            std::mem::transmute::<
                *const (),
                extern "C" fn(
                    *const std::ffi::c_void,
                    c_int,
                    *const c_char,
                ) -> ExternalDeclProviderResult<'static>,
            >(decl_getter_ptr)
        };
        let c_hhvm_provider_ptr =
            unsafe { std::mem::transmute::<*const (), *const std::ffi::c_void>(hhvm_provider_ptr) };

        let decl_allocator = bumpalo::Bump::new();
        let compile_result = compile::hhas_from_text(
            alloc,
            &compile_env,
            &stack_limit,
            text,
            Some(&native_env),
            unified_decl_provider::DeclProvider::ExternalDeclProvider(ExternalDeclProvider::new(
                c_decl_getter_fn,
                c_hhvm_provider_ptr,
                &decl_allocator,
            )),
        );

        match compile_result {
            Ok((hhas_prog, _)) => Ok(Box::new(HhasProgramWrapper(hhas_prog, bump))),
            Err(e) => Err(anyhow!("{}", e)),
        }
    })
    .map_err(|e| format!("{}", e))
    .expect("hackc_compile_hhas_from_text_cpp_ffi: retry failed")
    .map_err(|e| e.to_string())
}

#[no_mangle]
pub fn hackc_hhas_to_string_cpp_ffi(
    env: &compile_ffi::NativeEnv,
    prog: &HhasProgramWrapper,
) -> Result<Vec<u8>, String> {
    let native_env: compile::NativeEnv<&str> = compile_ffi::NativeEnv::to_compile_env(env).unwrap();
    let env = compile::Env::<&str> {
        filepath: native_env.filepath.clone(),
        config_jsons: vec![],
        config_list: vec![],
        flags: native_env.flags,
    };
    let mut output = Vec::new();
    compile::hhas_to_string(&env, Some(&native_env), &mut output, &prog.0)
        .map(|_| output)
        .map_err(|e| e.to_string())
}

pub fn hackc_extract_facts_as_json_cpp_ffi(
    flags: i32,
    filename: &CxxString,
    source_text: &CxxString,
) -> String {
    use std::os::unix::ffi::OsStrExt;
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes())),
    );
    let opts = facts_parser::FactsOpts {
        php5_compat_mode: ((1 << 0) & flags) != 0,
        hhvm_compat_mode: ((1 << 1) & flags) != 0,
        allow_new_attribute_syntax: ((1 << 2) & flags) != 0,
        enable_xhp_class_modifier: ((1 << 3) & flags) != 0,
        disable_xhp_element_mangling: ((1 << 4) & flags) != 0,
        filename: filepath,
    };
    match facts_parser::extract_as_json(source_text.as_bytes(), opts) {
        Some(s) => s,
        None => String::new(),
    }
}

pub fn hackc_extract_facts_cpp_ffi(
    flags: i32,
    filename: &CxxString,
    source_text: &CxxString,
) -> compile_ffi::FactsResult {
    use std::os::unix::ffi::OsStrExt;
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes())),
    );
    let opts = facts_parser::FactsOpts {
        php5_compat_mode: ((1 << 0) & flags) != 0,
        hhvm_compat_mode: ((1 << 1) & flags) != 0,
        allow_new_attribute_syntax: ((1 << 2) & flags) != 0,
        enable_xhp_class_modifier: ((1 << 3) & flags) != 0,
        disable_xhp_element_mangling: ((1 << 4) & flags) != 0,
        filename: filepath,
    };
    let text = source_text.as_bytes();
    match facts_parser::from_text(text, opts) {
        (Some(facts), has_errors) => {
            let (md5sum, sha1sum) = facts::md5_and_sha1(text);
            compile_ffi::FactsResult {
                facts: facts.into(),
                md5sum,
                sha1sum,
                has_errors,
            }
        }
        (None, has_errors) => compile_ffi::FactsResult {
            has_errors,
            ..Default::default()
        },
    }
}

pub fn hackc_facts_to_json_cpp_ffi(
    facts: compile_ffi::FactsResult,
    source_text: &CxxString,
) -> String {
    if facts.has_errors {
        String::new()
    } else {
        let facts = facts::Facts::from(facts.facts);
        let text = source_text.as_bytes();
        facts.to_json(text)
    }
}

pub fn hackc_decls_to_facts_cpp_ffi(
    decl_flags: i32,
    decl_result: &compile_ffi::DeclResult,
    source_text: &CxxString,
) -> compile_ffi::FactsResult {
    let text = source_text.as_bytes();
    let (md5sum, sha1sum) = facts::md5_and_sha1(text);
    if decl_result.has_errors {
        compile_ffi::FactsResult {
            has_errors: true,
            ..Default::default()
        }
    } else {
        let disable_xhp_element_mangling = ((1 << 0) & decl_flags) != 0;
        let facts = compile_ffi::Facts::from(facts::Facts::facts_of_decls(
            &(*decl_result.decls).0,
            &(*decl_result.attributes).0,
            disable_xhp_element_mangling,
        ));
        compile_ffi::FactsResult {
            facts: facts.into(),
            md5sum,
            sha1sum,
            has_errors: false,
        }
    }
}
