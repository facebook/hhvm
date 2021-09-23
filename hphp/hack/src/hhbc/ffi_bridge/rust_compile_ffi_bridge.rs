// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use cxx::CxxString;
use decl_provider::NoDeclProvider;
use hhbc_by_ref_compile::EnvFlags;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;
use stack_limit::{StackLimit, GI, KI, MI};

#[cxx::bridge]
mod ffi {
    pub struct NativeEnv {
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
    let retryable = |stack_limit: &StackLimit, _nomain_stack_size: Option<usize>| {
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
        let mut w = String::new();
        let alloc = bumpalo::Bump::new();
        let compile_result = hhbc_by_ref_compile::from_text(
            &alloc,
            &compile_env,
            &stack_limit,
            &mut w,
            text,
            Some(&native_env),
            unified_decl_provider::DeclProvider::NoDeclProvider(NoDeclProvider),
        );
        match compile_result {
            Ok(_) => Ok(w),
            Err(e) => Err(e.to_string()),
        }
    };

    // Assume peak is 2.5x of stack. This is initial estimation, need
    // to be improved later.
    let stack_slack = |stack_size| stack_size * 6 / 10;
    let on_retry = &mut |stack_size_tried: usize| {
        if std::env::var_os("HH_TEST_MODE").is_some() {
            let env = ffi::NativeEnv::to_compile_env(&env).unwrap();
            eprintln!(
                "[hrust] warning: hackc_compile_from_text_ffi exceeded stack of {} KiB on: {}",
                (stack_size_tried - stack_slack(stack_size_tried)) / KI,
                env.filepath.path_str(),
            );
        }
    };
    let job = stack_limit::retry::Job {
        nonmain_stack_min: 13 * MI,
        // TODO(hrust) aast_parser_ffi only requies 1 * GI, it's like
        // rust compiler produce inconsistent binary.
        nonmain_stack_max: Some(7 * GI),
        ..Default::default()
    };

    job.with_elastic_stack(retryable, on_retry, stack_slack)
        .map_err(|e| format!("{}", e))
        .expect("compile_ffi: hackc_compile_from_text_cpp_ffi: retry failed")
        .map_err(|e| e.to_string())
}
