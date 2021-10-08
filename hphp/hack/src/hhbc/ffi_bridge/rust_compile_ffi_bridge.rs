// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use cxx::CxxString;
use external_decl_provider::ExternalDeclProvider;
use hhbc_by_ref_compile::EnvFlags;
use libc::c_char;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;

#[cxx::bridge]
mod ffi {
    pub struct NativeEnv {
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

    extern "Rust" {
        pub fn make_env_flags(
            is_systemlib: bool,
            is_evaled: bool,
            for_debugger_eval: bool,
            dump_symbol_refs: bool,
            disable_toplevel_elaboration: bool,
            enable_decl: bool,
        ) -> u8;

        pub fn hackc_compile_from_text_cpp_ffi(
            env: &NativeEnv,
            source_text: &CxxString,
        ) -> Result<String>;
    }
}

pub fn make_env_flags(
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

impl ffi::NativeEnv {
    pub fn to_compile_env<'a>(
        env: &'a ffi::NativeEnv,
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

pub fn hackc_compile_from_text_cpp_ffi<'a>(
    env: &ffi::NativeEnv,
    source_text: &CxxString,
) -> Result<String, String> {
    stack_limit::with_elastic_stack(|stack_limit| {
        let native_env: hhbc_by_ref_compile::NativeEnv<&str> =
            ffi::NativeEnv::to_compile_env(&env).unwrap();
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
