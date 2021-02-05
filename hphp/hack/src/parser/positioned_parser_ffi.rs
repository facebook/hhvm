// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

rust_parser_ffi::parse!(parse_positioned, positioned_parser::parse_script);

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
impl std::convert::From<&CParserEnv> for parser_core_types::parser_env::ParserEnv {
    fn from(env: &CParserEnv) -> parser_core_types::parser_env::ParserEnv {
        parser_core_types::parser_env::ParserEnv {
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
        }
    }
}

// Return result of `parse_positioned_cpp_ffi` to Rust.
#[no_mangle]
extern "C" fn parse_positioned_free_string_cpp_ffi(s: *mut libc::c_char) {
    let _ = unsafe { std::ffi::CString::from_raw(s) };
}

// Calculate a parse tree from source text and render it as json.
//
// (We don't have json support yet so for now we render it as a debug
// string.)
#[no_mangle]
extern "C" fn parse_positioned_cpp_ffi(
    filename: *const libc::c_char,
    source_text: *const libc::c_char,
    env: usize,
) -> *const libc::c_char {
    let filepath = oxidized::relative_path::RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(cpp_helper::cstr::to_str(filename)),
    );
    let text: &[u8] = cpp_helper::cstr::to_u8(source_text);
    let env: parser_core_types::parser_env::ParserEnv =
        unsafe { parser_core_types::parser_env::ParserEnv::from(&*(env as *const CParserEnv)) };
    let source: parser_core_types::source_text::SourceText =
        parser_core_types::source_text::SourceText::make(ocamlrep::rc::RcOc::new(filepath), text);
    let stack_limit: std::option::Option<&stack_limit::StackLimit> = None;
    // It seems to me that the behavior of `hh_single_compile` is to
    // ignore errors here.
    let (root, _errors, _): (
        parser_core_types::positioned_syntax::PositionedSyntax,
        std::vec::Vec<parser_core_types::syntax_error::SyntaxError>,
        smart_constructors::NoState,
    ) = positioned_parser::parse_script(&source, env, stack_limit);
    let parse_tree = format!("{:?}", root); // We don't have json yet.
    let cs = std::ffi::CString::new(parse_tree)
        .expect("positioned_parser_ffi: parse_positioned_cpp_ffi: String::new failed");
    cs.into_raw() as *const libc::c_char
}
