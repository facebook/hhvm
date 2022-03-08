/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 */
use cxx::CxxString;
use oxidized::relative_path::RelativePath;
use std::str;

#[cxx::bridge]
mod ffi {
    struct ParserEnv {
        codegen: bool,
        hhvm_compat_mode: bool,
        php5_compat_mode: bool,
        allow_new_attribute_syntax: bool,
        enable_xhp_class_modifier: bool,
        disable_xhp_element_mangling: bool,
        disable_xhp_children_declarations: bool,
        disallow_fun_and_cls_meth_pseudo_funcs: bool,
        interpret_soft_types_as_like_types: bool,
    }

    extern "Rust" {
        pub fn hackc_parse_positioned_full_trivia_cpp_ffi(
            source_text: &CxxString,
            env: &ParserEnv,
        ) -> String;
    }
}

pub fn hackc_parse_positioned_full_trivia_cpp_ffi(
    source_text: &CxxString,
    env: &ffi::ParserEnv,
) -> String {
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::new(),
    );
    let env: parser_core_types::parser_env::ParserEnv = ffi::ParserEnv::to_parser_env(env);
    let indexed_source = parser_core_types::indexed_source_text::IndexedSourceText::new(
        parser_core_types::source_text::SourceText::make(
            ocamlrep::rc::RcOc::new(filepath),
            source_text.as_bytes(),
        ),
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
        Ok(()) => str::from_utf8(&serializer.into_inner())
            .unwrap()
            .to_string(),
        _ => String::new(),
    }
}

impl ffi::ParserEnv {
    fn to_parser_env(env: &ffi::ParserEnv) -> parser_core_types::parser_env::ParserEnv {
        parser_core_types::parser_env::ParserEnv {
            codegen: env.codegen,
            hhvm_compat_mode: env.hhvm_compat_mode,
            php5_compat_mode: env.php5_compat_mode,
            allow_new_attribute_syntax: env.allow_new_attribute_syntax,
            enable_xhp_class_modifier: env.enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.disable_xhp_element_mangling,
            disable_xhp_children_declarations: env.disable_xhp_children_declarations,
            disallow_fun_and_cls_meth_pseudo_funcs: env.disallow_fun_and_cls_meth_pseudo_funcs,
            interpret_soft_types_as_like_types: env.interpret_soft_types_as_like_types,
        }
    }
}
