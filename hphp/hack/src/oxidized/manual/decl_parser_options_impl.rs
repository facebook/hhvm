// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::parser_env::ParserEnv;

use crate::decl_parser_options::DeclParserOptions;
use crate::parser_options::ParserOptions;

impl Default for DeclParserOptions {
    fn default() -> Self {
        Self::from_parser_options(&ParserOptions::default())
    }
}

impl DeclParserOptions {
    pub fn from_parser_options(opts: &ParserOptions) -> Self {
        Self {
            auto_namespace_map: opts.po_auto_namespace_map.clone(),
            gi_reinfer_types: opts.tco_gi_reinfer_types.clone(),
            disable_xhp_element_mangling: opts.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: opts.po_interpret_soft_types_as_like_types,
            allow_new_attribute_syntax: opts.po_allow_new_attribute_syntax,
            enable_xhp_class_modifier: opts.po_enable_xhp_class_modifier,
            everything_sdt: opts.tco_everything_sdt,
            global_inference: opts.tco_global_inference,
            php5_compat_mode: false,
            hhvm_compat_mode: false,
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
