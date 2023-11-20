// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::parser_env::ParserEnv;

use crate::full_fidelity_parser_env::FullFidelityParserEnv;

impl From<FullFidelityParserEnv> for ParserEnv {
    fn from(env: FullFidelityParserEnv) -> Self {
        Self {
            hhvm_compat_mode: env.hhvm_compat_mode,
            php5_compat_mode: env.php5_compat_mode,
            codegen: env.codegen,
            enable_xhp_class_modifier: env.enable_xhp_class_modifier,
            disable_xhp_element_mangling: env.disable_xhp_element_mangling,
            disable_xhp_children_declarations: env.disable_xhp_children_declarations,
            interpret_soft_types_as_like_types: env.interpret_soft_types_as_like_types,
        }
    }
}
