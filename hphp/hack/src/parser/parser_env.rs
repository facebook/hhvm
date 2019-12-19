// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::From;

use oxidized::full_fidelity_parser_env::FullFidelityParserEnv;

#[derive(Clone, Debug, Default)]
pub struct ParserEnv {
    pub codegen: bool,
    pub is_experimental_mode: bool,
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
    pub allow_new_attribute_syntax: bool,
    pub enable_xhp_class_modifier: bool,
}

impl From<FullFidelityParserEnv> for ParserEnv {
    fn from(env: FullFidelityParserEnv) -> Self {
        Self {
            is_experimental_mode: env.is_experimental_mode(),
            hhvm_compat_mode: env.hhvm_compat_mode,
            php5_compat_mode: env.php5_compat_mode,
            codegen: env.codegen,
            allow_new_attribute_syntax: env.allow_new_attribute_syntax,
            enable_xhp_class_modifier: env.enable_xhp_class_modifier,
        }
    }
}
