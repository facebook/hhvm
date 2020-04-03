// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{gen::global_options::GlobalOptions, i_set, s_map, s_set};

impl Default for GlobalOptions {
    fn default() -> Self {
        Self {
            tco_experimental_features: s_set::SSet::new(),
            tco_migration_flags: s_set::SSet::new(),
            tco_dynamic_view: false,
            tco_defer_class_declaration_threshold: None,
            tco_max_times_to_defer_type_checking: None,
            tco_prefetch_deferred_files: false,
            tco_remote_type_check_threshold: None,
            tco_remote_type_check: false,
            tco_remote_worker_key: None,
            tco_remote_check_id: None,
            tco_remote_max_batch_size: 8000,
            tco_remote_min_batch_size: 5000,
            tco_num_remote_workers: 0,
            so_remote_version_specifier: None,
            so_remote_worker_vfs_checkout_threshold: 0,
            so_naming_sqlite_path: None,
            po_auto_namespace_map: vec![],
            po_codegen: false,
            po_deregister_php_stdlib: false,
            po_disallow_execution_operator: false,
            po_disallow_toplevel_requires: false,
            po_disable_nontoplevel_declarations: false,
            po_disable_static_closures: false,
            po_allow_goto: false,
            tco_log_inference_constraints: false,
            tco_disallow_array_typehint: false,
            tco_disallow_array_literal: false,
            tco_language_feature_logging: false,
            tco_unsafe_rx: false,
            tco_disallow_scrutinee_case_value_type_mismatch: false,
            tco_timeout: 0,
            tco_disallow_invalid_arraykey: false,
            tco_disallow_byref_dynamic_calls: false,
            tco_disallow_byref_calls: false,
            ignored_fixme_codes: i_set::ISet::new(),
            ignored_fixme_regex: None,
            log_levels: s_map::SMap::new(),
            po_disable_lval_as_an_expression: false,
            tco_shallow_class_decl: false,
            po_rust_parser_errors: false,
            po_rust_top_level_elaborator: true,
            profile_type_check_duration_threshold: 0.0,
            profile_type_check_twice: false,
            profile_owner: String::new(),
            profile_desc: String::new(),
            tco_like_type_hints: false,
            tco_union_intersection_type_hints: false,
            tco_like_casts: false,
            tco_simple_pessimize: 0.0,
            tco_complex_coercion: false,
            tco_disable_partially_abstract_typeconsts: false,
            error_codes_treated_strictly: i_set::ISet::new(),
            tco_check_xhp_attribute: false,
            tco_check_redundant_generics: false,
            tco_disallow_unresolved_type_variables: false,
            tco_disallow_invalid_arraykey_constraint: false,
            po_enable_class_level_where_clauses: false,
            po_disable_legacy_soft_typehints: false,
            po_disallowed_decl_fixmes: i_set::ISet::new(),
            po_allow_new_attribute_syntax: false,
            tco_global_inference: false,
            tco_gi_reinfer_types: vec![],
            tco_ordered_solving: false,
            tco_const_static_props: false,
            po_disable_legacy_attribute_syntax: false,
            tco_const_attribute: false,
            po_const_default_func_args: false,
            po_disallow_silence: false,
            po_abstract_static_props: false,
            po_disable_unset_class_const: false,
            po_parser_errors_only: false,
            tco_check_attribute_locations: false,
            po_disallow_func_ptrs_in_constants: false,
            tco_error_php_lambdas: false,
            tco_disallow_discarded_nullable_awaitables: false,
            po_enable_xhp_class_modifier: false,
            po_disable_xhp_element_mangling: false,
            po_disable_xhp_children_declarations: false,
            glean_service: String::new(),
            glean_hostname: String::new(),
            glean_port: 0,
            glean_reponame: String::new(),
            symbol_write_root_path: String::new(),
            symbol_write_hhi_path: String::new(),
            po_enable_first_class_function_pointers: false,
            po_disable_modes: false,
            po_disable_array: false,
        }
    }
}

impl Eq for GlobalOptions {}

impl std::hash::Hash for GlobalOptions {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl Ord for GlobalOptions {
    fn cmp(&self, _: &Self) -> std::cmp::Ordering {
        unimplemented!()
    }
}
