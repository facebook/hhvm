// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    decl_parser_options::DeclParserOptions, full_fidelity_parser_env::FullFidelityParserEnv,
};

#[derive(Clone, Debug, Default)]
pub struct ParserEnv {
    pub codegen: bool,
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
    pub allow_new_attribute_syntax: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub disable_xhp_children_declarations: bool,
    pub disallow_fun_and_cls_meth_pseudo_funcs: bool,
    pub interpret_soft_types_as_like_types: bool,
}

impl From<FullFidelityParserEnv> for ParserEnv {
    fn from(env: FullFidelityParserEnv) -> Self {
        Self {
            hhvm_compat_mode: env.hhvm_compat_mode,
            php5_compat_mode: env.php5_compat_mode,
            codegen: env.codegen,
            allow_new_attribute_syntax: env.allow_new_attribute_syntax,
            enable_xhp_class_modifier: env.enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.disable_xhp_element_mangling,
            disable_xhp_children_declarations: env.disable_xhp_children_declarations,
            disallow_fun_and_cls_meth_pseudo_funcs: env.disallow_fun_and_cls_meth_pseudo_funcs,
            interpret_soft_types_as_like_types: env.interpret_soft_types_as_like_types,
        }
    }
}

impl From<&DeclParserOptions> for ParserEnv {
    fn from(opts: &DeclParserOptions) -> Self {
        Self {
            hhvm_compat_mode: opts.hhvm_compat_mode,
            php5_compat_mode: opts.php5_compat_mode,
            disable_xhp_element_mangling: opts.disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: opts.interpret_soft_types_as_like_types,
            allow_new_attribute_syntax: opts.allow_new_attribute_syntax,
            enable_xhp_class_modifier: opts.enable_xhp_class_modifier,
            ..Default::default()
        }
    }
}
