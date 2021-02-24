// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// This module defines a C FFI entry point.

// The function defined here is
// `parser_positioned_full_triva_cpp_ffi`. It returns parse trees as
// JSON strings. It is implemented by calling
// `positioned_full_trivia_parser::parse_script_to_json`.

///`struct CParserEnv` is for passing options through the C FFI.
#[repr(C)]
struct CParserEnv {
    codegen: bool,
    hhvm_compat_mode: bool,
    php5_compat_mode: bool,
    allow_new_attribute_syntax: bool,
    enable_xhp_class_modifier: bool,
    disable_xhp_element_mangling: bool,
    disable_xhp_children_declarations: bool,
    disable_modes: bool,
    disallow_hash_comments: bool,
    disallow_fun_and_cls_meth_pseudo_funcs: bool,
    array_unification: bool,
    interpret_soft_types_as_like_types: bool,
}

impl CParserEnv {
    /// Returns `None` if `env` is null.
    ///
    /// # Safety
    /// * `env` must be a valid, aligned pointer to a `CParserEnv`
    unsafe fn to_parser_env(
        env: *const CParserEnv,
    ) -> Option<parser_core_types::parser_env::ParserEnv> {
        let env = env.as_ref()?;
        Some(parser_core_types::parser_env::ParserEnv {
            codegen: env.codegen,
            hhvm_compat_mode: env.hhvm_compat_mode,
            php5_compat_mode: env.php5_compat_mode,
            allow_new_attribute_syntax: env.allow_new_attribute_syntax,
            enable_xhp_class_modifier: env.enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.disable_xhp_element_mangling,
            disable_xhp_children_declarations: env.disable_xhp_children_declarations,
            disable_modes: env.disable_modes,
            disallow_hash_comments: env.disallow_hash_comments,
            disallow_fun_and_cls_meth_pseudo_funcs: env.disallow_fun_and_cls_meth_pseudo_funcs,
            array_unification: env.array_unification,
            interpret_soft_types_as_like_types: env.interpret_soft_types_as_like_types,
        })
    }
}

/// Return result of `parse_positioned_full_trivia_cpp_ffi` to Rust.
#[no_mangle]
unsafe extern "C" fn parse_positioned_full_trivia_free_string_cpp_ffi(s: *mut libc::c_char) {
    // Safety:
    //   - This should only ever be called on a pointer obtained by
    //     `CString::into_raw`.
    //   - `CString::from_raw` and `CString::to_raw` should not be
    //     used with C functions that can modify the string's length.
    let _ = std::ffi::CString::from_raw(s);
}

/// Calculate a parse tree from source text and render it as json.
#[cfg(unix)]
#[no_mangle]
unsafe extern "C" fn parse_positioned_full_trivia_cpp_ffi(
    filename: *const libc::c_char,
    source_text: *const libc::c_char,
    env: usize,
) -> *const libc::c_char {
    use std::os::unix::ffi::OsStrExt;
    // Safety: We rely on the C caller that `filename` be a properly
    // initialized null-terminated C string.
    let filepath = oxidized::relative_path::RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(cpp_helper::cstr::to_u8(
            filename,
        ))),
    );
    // Safety : We rely on the C caller that `text` be a properly
    // iniitalized null-terminated C string.
    let text: &[u8] = cpp_helper::cstr::to_u8(source_text);
    // Safety : We rely on the C caller that `env` can be legitmately
    // reinterpreted as a `*const CParserEnv` and that on doing so, it
    // points to a valid properly initialized value.
    let env: parser_core_types::parser_env::ParserEnv =
        CParserEnv::to_parser_env(env as *const CParserEnv).unwrap();
    let indexed_source = parser_core_types::indexed_source_text::IndexedSourceText::new(
        parser_core_types::source_text::SourceText::make(ocamlrep::rc::RcOc::new(filepath), text),
    );
    let alloc = bumpalo::Bump::new();
    let mut serializer = serde_json::Serializer::new(std::vec![]);
    let stack_limit: std::option::Option<&stack_limit::StackLimit> = None;
    match positioned_full_trivia_parser::parse_script_to_json(
        &alloc,
        &mut serializer,
        &indexed_source,
        env,
        stack_limit,
    ) {
        Ok(()) => {
            // Safety : No runtime assertion is made that `v` contains no
            // 0 bytes.
            cpp_helper::cstr::from_vec_u8(serializer.into_inner())
        }
        _ => std::ptr::null(),
    }
}
