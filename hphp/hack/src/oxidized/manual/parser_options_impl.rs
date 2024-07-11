// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::parser_options::ParserOptions;
use crate::i_set;

impl Default for ParserOptions {
    fn default() -> Self {
        Self {
            hhvm_compat_mode: false,
            hhi_mode: false,
            auto_namespace_map: vec![],
            codegen: false,
            deregister_php_stdlib: false,
            allow_unstable_features: false, // true in /etc/hh.conf
            disable_lval_as_an_expression: false, // true in ocaml
            enable_class_level_where_clauses: false,
            disable_legacy_soft_typehints: true,
            const_static_props: false,
            disable_legacy_attribute_syntax: false,
            const_default_func_args: false,
            const_default_lambda_args: false,
            abstract_static_props: false,
            disallow_func_ptrs_in_constants: false,
            enable_xhp_class_modifier: false,    // true in ocaml
            disable_xhp_element_mangling: false, // true in ocaml
            disable_xhp_children_declarations: false, // true in ocaml
            keep_user_attributes: false,
            is_systemlib: false,
            interpret_soft_types_as_like_types: false,
            everything_sdt: false,
            disallow_static_constants_in_default_func_args: false,
            disallow_direct_superglobals_refs: false,
            stack_size: 32 * 1024 * 1024, // 32 MiB is the largest stack size we can use without requiring sudo
            union_intersection_type_hints: false,
            unwrap_concurrent: false,
            disallow_silence: false,
            no_parser_readonly_check: false,
            parser_errors_only: false,
            allowed_decl_fixme_codes: i_set::ISet::new(),
            disable_hh_ignore_error: 0,
            use_legacy_experimental_feature_config: true,
            experimental_features: vec![],
            consider_unspecified_experimental_features_released: true,
        }
    }
}
