// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::{anyhow, Result};
use arena_deserializer::serde::Deserialize;
use bincode::Options;
use cxx::CxxString;
use external_decl_provider::ExternalDeclProvider;
use hhbc_by_ref_compile::EnvFlags;
use libc::c_char;
use no_pos_hash::position_insensitive_hash;
use oxidized::relative_path::{Prefix, RelativePath};
use oxidized_by_ref::{decl_parser_options, direct_decl_parser};
use parser_core_types::source_text::SourceText;

#[cxx::bridge]
mod compile_ffi {
    struct NativeEnv {
        decl_provider: u64,
        decl_getter: u64,
        filepath: String,
        aliased_namespaces: String,
        include_roots: String,
        emit_class_pointers: i32,
        check_int_overflow: i32,
        hhbc_flags: u32,
        parser_flags: u32,
        flags: u8,
    }
    struct DeclResult<'a> {
        hash: u64,
        serialized: Box<Bytes>,
        decls: Box<Decls<'a>>,
    }

    extern "Rust" {
        type Bump;
        type Bytes;
        type Decls<'a>;
        type DeclParserOptions<'a>;
        type HhasProgram<'a>;

        fn make_env_flags(
            is_systemlib: bool,
            is_evaled: bool,
            for_debugger_eval: bool,
            dump_symbol_refs: bool,
            disable_toplevel_elaboration: bool,
            enable_decl: bool,
        ) -> u8;

        unsafe fn hackc_compile_hhas_from_text_cpp_ffi<'a>(
            alloc: &'a Bump,
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<Box<HhasProgram<'a>>>;

        fn hackc_compile_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<String>;

        fn hackc_create_arena() -> Box<Bump>;
        fn hackc_create_direct_decl_parse_options(
            disable_xhp_element_mangling: bool,
            interpret_soft_types_as_like_types: bool,
        ) -> Box<DeclParserOptions<'static>>;

        unsafe fn hackc_direct_decl_parse<'a>(
            options: &'a DeclParserOptions<'a>,
            filename: &CxxString,
            text: &CxxString,
            bump: &'a Bump,
        ) -> DeclResult<'a>;

        fn hackc_print_decls(decls: &Decls<'_>);
        fn hackc_print_serialized_size(bytes: &Bytes);
        unsafe fn hackc_verify_deserialization(serialized: &Bytes, expected: &Decls<'_>) -> bool;
    }
}

struct Bump(bumpalo::Bump);
pub struct Bytes(ffi::Bytes);
pub struct Decls<'a>(direct_decl_parser::Decls<'a>);
struct DeclParserOptions<'a>(decl_parser_options::DeclParserOptions<'a>);
struct HhasProgram<'a>(hhbc_by_ref_hhas_program::HhasProgram<'a>);

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
    fn to_compile_env<'a>(
        env: &'a compile_ffi::NativeEnv,
    ) -> Option<hhbc_by_ref_compile::NativeEnv<&'a str>> {
        use std::os::unix::ffi::OsStrExt;
        Some(hhbc_by_ref_compile::NativeEnv {
            filepath: RelativePath::make(
                oxidized::relative_path::Prefix::Dummy,
                std::path::PathBuf::from(std::ffi::OsStr::from_bytes(env.filepath.as_bytes())),
            ),
            aliased_namespaces: &env.aliased_namespaces,
            include_roots: &env.include_roots,
            emit_class_pointers: env.emit_class_pointers,
            check_int_overflow: env.check_int_overflow,
            hhbc_flags: hhbc_by_ref_compile::HHBCFlags::from_bits(env.hhbc_flags)?,
            parser_flags: hhbc_by_ref_compile::ParserFlags::from_bits(env.parser_flags)?,
            flags: hhbc_by_ref_compile::EnvFlags::from_bits(env.flags)?,
        })
    }
}

fn hackc_compile_from_text_cpp_ffi<'a>(
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<String, String> {
    stack_limit::with_elastic_stack(|stack_limit| {
        let native_env: hhbc_by_ref_compile::NativeEnv<&str> =
            compile_ffi::NativeEnv::to_compile_env(&env).unwrap();
        let compile_env = hhbc_by_ref_compile::Env::<&str> {
            filepath: native_env.filepath.clone(),
            config_jsons: vec![],
            config_list: vec![],
            flags: native_env.flags,
        };

        let text = SourceText::make(
            ocamlrep::rc::RcOc::new(native_env.filepath.clone()),
            source_text.as_bytes(),
        );
        let mut output = String::new();
        let alloc = bumpalo::Bump::new();
        let decl_getter_ptr = env.decl_getter as *const ();
        let hhvm_provider_ptr = env.decl_provider as *const ();
        let c_decl_getter_fn = unsafe {
            std::mem::transmute::<
                *const (),
                extern "C" fn(*const std::ffi::c_void, *const c_char) -> *const std::ffi::c_void,
            >(decl_getter_ptr)
        };
        let c_hhvm_provider_ptr =
            unsafe { std::mem::transmute::<*const (), *const std::ffi::c_void>(hhvm_provider_ptr) };
        hhbc_by_ref_compile::from_text(
            &alloc,
            &compile_env,
            &stack_limit,
            &mut output,
            text,
            Some(&native_env),
            unified_decl_provider::DeclProvider::ExternalDeclProvider(ExternalDeclProvider::new(
                c_decl_getter_fn,
                c_hhvm_provider_ptr,
            )),
        )?;
        Ok(output)
    })
    .map_err(|e| e.to_string())?
    .map_err(|e: anyhow::Error| format!("{}", e))
}

fn hackc_create_arena() -> Box<Bump> {
    Box::new(Bump(bumpalo::Bump::new()))
}

fn hackc_print_decls<'a>(decls: &Decls<'a>) {
    println!("{:#?}", decls.0)
}

fn hackc_print_serialized_size(serialized: &Bytes) {
    println!("Decl-serialized size: {:#?}", serialized.0.len);
}

fn hackc_create_direct_decl_parse_options<'a>(
    disable_xhp_element_mangling: bool,
    interpret_soft_types_as_like_types: bool,
) -> Box<DeclParserOptions<'a>> {
    Box::new(DeclParserOptions(decl_parser_options::DeclParserOptions {
        auto_namespace_map: &[],
        disable_xhp_element_mangling,
        interpret_soft_types_as_like_types,
        everything_sdt: false,
    }))
}

impl<'a> compile_ffi::DeclResult<'a> {
    fn new(hash: u64, serialized: Bytes, decls: Decls<'a>) -> Self {
        Self {
            hash,
            serialized: Box::new(serialized),
            decls: Box::new(decls),
        }
    }
}

fn hackc_direct_decl_parse<'a>(
    opts: &'a DeclParserOptions<'a>,
    filename: &CxxString,
    text: &CxxString,
    bump: &'a Bump,
) -> compile_ffi::DeclResult<'a> {
    use std::os::unix::ffi::OsStrExt;

    let text = text.as_bytes();
    let path = std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes()));
    let filename = RelativePath::make(Prefix::Root, path);
    let decls = decl_rust::direct_decl_parser::parse_decls_without_reference_text(
        &opts.0, filename, text, &bump.0, None,
    );

    let op = bincode::config::Options::with_native_endian(bincode::options());
    let data = op
        .serialize(&decls)
        .map_err(|e| format!("failed to serialize, error: {}", e))
        .unwrap();

    compile_ffi::DeclResult::new(
        position_insensitive_hash(&decls),
        Bytes(ffi::Bytes::from(data)),
        Decls(decls),
    )
}

unsafe fn hackc_verify_deserialization<'a>(serialized: &Bytes, expected: &Decls<'a>) -> bool {
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

fn hackc_compile_hhas_from_text_cpp_ffi<'a>(
    alloc: &'a Bump,
    env: &compile_ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<Box<HhasProgram<'a>>, String> {
    stack_limit::with_elastic_stack(|stack_limit| {
        let native_env: hhbc_by_ref_compile::NativeEnv<&str> =
            compile_ffi::NativeEnv::to_compile_env(&env).unwrap();
        let compile_env = hhbc_by_ref_compile::Env::<&str> {
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
                extern "C" fn(*const std::ffi::c_void, *const c_char) -> *const std::ffi::c_void,
            >(decl_getter_ptr)
        };
        let c_hhvm_provider_ptr =
            unsafe { std::mem::transmute::<*const (), *const std::ffi::c_void>(hhvm_provider_ptr) };
        let compile_result = hhbc_by_ref_compile::hhas_from_text(
            &alloc.0,
            &compile_env,
            &stack_limit,
            text,
            Some(&native_env),
            unified_decl_provider::DeclProvider::ExternalDeclProvider(ExternalDeclProvider::new(
                c_decl_getter_fn,
                c_hhvm_provider_ptr,
            )),
        );
        match compile_result {
            Ok((hhas_prog, _)) => Ok(Box::new(HhasProgram(hhas_prog))),
            Err(e) => Err(anyhow!("{}", e)),
        }
    })
    .map_err(|e| format!("{}", e))
    .expect("hackc_compile_hhas_from_text_cpp_ffi: retry failed")
    .map_err(|e| e.to_string())
}
