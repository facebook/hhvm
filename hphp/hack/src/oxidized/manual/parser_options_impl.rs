// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::parser_options::ParserOptions;
use crate::i_set;
use crate::package_info::PackageInfo;

impl Default for ParserOptions {
    fn default() -> Self {
        Self {
            disable_lval_as_an_expression: false, // true in ocaml
            const_static_props: false,
            const_default_func_args: false,
            abstract_static_props: false,
            disallow_func_ptrs_in_constants: false,
            enable_xhp_class_modifier: false,
            disable_xhp_element_mangling: false,
            allow_unstable_features: false, // true in /etc/hh.conf
            hhvm_compat_mode: false,
            hhi_mode: false,
            codegen: false,
            disable_legacy_soft_typehints: true,
            disable_legacy_attribute_syntax: false,
            disable_xhp_children_declarations: false,
            const_default_lambda_args: false,
            interpret_soft_types_as_like_types: false,
            is_systemlib: false,
            disallow_static_constants_in_default_func_args: false,
            auto_namespace_map: vec![],
            everything_sdt: false,
            deregister_php_stdlib: false,
            stack_size: 32 * 1024 * 1024, // 32 MiB is the largest stack size we can use without requiring sudo
            keep_user_attributes: false,
            union_intersection_type_hints: false,
            unwrap_concurrent: false,
            disallow_silence: false,
            no_parser_readonly_check: false,
            disable_hh_ignore_error: 0,
            allowed_decl_fixme_codes: i_set::ISet::new(),
            use_legacy_experimental_feature_config: true,
            experimental_features: std::collections::BTreeMap::default(),
            consider_unspecified_experimental_features_released: true,
            package_v2: false,
            package_info: PackageInfo::default(),
            package_v2_support_multifile_tests: false,
            enable_class_pointer_hint: true,
            disallow_non_annotated_memoize: false,
            treat_non_annotated_memoize_as_kbic: false,
        }
    }
}
