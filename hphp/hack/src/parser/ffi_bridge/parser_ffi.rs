use std::path::PathBuf;

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
use ocamlrep::rc::RcOc;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use relative_path::Prefix;
use relative_path::RelativePath;

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
        interpret_soft_types_as_like_types: bool,
    }

    extern "Rust" {
        pub fn hackc_parse_positioned_full_trivia_cpp_ffi(
            source_text: &CxxString,
            env: &ParserEnv,
        ) -> Vec<u8>;
    }
}

pub fn hackc_parse_positioned_full_trivia_cpp_ffi(
    source_text: &CxxString,
    env: &ffi::ParserEnv,
) -> Vec<u8> {
    let filepath = RelativePath::make(Prefix::Dummy, PathBuf::new());
    let env: parser_core_types::parser_env::ParserEnv = ffi::ParserEnv::to_parser_env(env);
    let indexed_source = IndexedSourceText::new(SourceText::make(
        RcOc::new(filepath),
        source_text.as_bytes(),
    ));
    let alloc = bumpalo::Bump::new();
    let mut serializer = serde_json::Serializer::new(vec![]);
    match positioned_full_trivia_parser::parse_script_to_json(
        &alloc,
        &mut serializer,
        &indexed_source,
        env,
    ) {
        Ok(()) => serializer.into_inner(),
        Err(_) => {
            // Swallow errors.
            Default::default()
        }
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
            interpret_soft_types_as_like_types: env.interpret_soft_types_as_like_types,
        }
    }
}
