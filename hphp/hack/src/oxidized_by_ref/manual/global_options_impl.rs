// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{gen::global_options::GlobalOptions, i_set, s_map, s_set};

impl Default for GlobalOptions<'_> {
    fn default() -> Self {
        Self {
            tco_experimental_features: s_set::SSet::default(),
            tco_migration_flags: s_set::SSet::default(),
            tco_dynamic_view: false,
            tco_num_local_workers: None,
            tco_parallel_type_checking_threshold: 10,
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
            po_auto_namespace_map: &[],
            po_codegen: false,
            po_deregister_php_stdlib: false,
            po_disallow_toplevel_requires: false,
            po_disable_nontoplevel_declarations: false,
            po_disable_static_closures: true,
            po_allow_goto: false,
            po_allow_unstable_features: false,
            tco_log_inference_constraints: false,
            tco_disallow_array_typehint: false,
            tco_disallow_array_literal: false,
            tco_language_feature_logging: false,
            tco_unsafe_rx: false,
            tco_disallow_scrutinee_case_value_type_mismatch: false,
            tco_timeout: 0,
            tco_disallow_invalid_arraykey: false,
            tco_disallow_byref_dynamic_calls: false,
            tco_disallow_byref_calls: true,
            allowed_fixme_codes_strict: i_set::ISet::default(),
            allowed_fixme_codes_partial: i_set::ISet::default(),
            codes_not_raised_partial: i_set::ISet::default(),
            log_levels: s_map::SMap::default(),
            po_disable_lval_as_an_expression: false,
            tco_shallow_class_decl: false,
            po_rust_parser_errors: false,
            profile_type_check_duration_threshold: 0.0,
            profile_type_check_twice: false,
            profile_owner: None,
            profile_desc: "",
            tco_like_type_hints: false,
            tco_union_intersection_type_hints: false,
            tco_coeffects: false,
            tco_like_casts: false,
            tco_simple_pessimize: 0.0,
            tco_complex_coercion: false,
            tco_disable_partially_abstract_typeconsts: false,
            error_codes_treated_strictly: i_set::ISet::default(),
            tco_check_xhp_attribute: false,
            tco_check_redundant_generics: false,
            tco_disallow_unresolved_type_variables: false,
            tco_disallow_trait_reuse: false,
            tco_disallow_invalid_arraykey_constraint: false,
            po_enable_class_level_where_clauses: false,
            po_disable_legacy_soft_typehints: true,
            po_allowed_decl_fixme_codes: i_set::ISet::default(),
            po_allow_new_attribute_syntax: false,
            tco_global_inference: false,
            tco_gi_reinfer_types: &[],
            tco_ordered_solving: false,
            tco_const_static_props: false,
            po_disable_legacy_attribute_syntax: false,
            tco_const_attribute: false,
            po_const_default_func_args: false,
            po_const_default_lambda_args: false,
            po_disallow_silence: false,
            po_abstract_static_props: false,
            po_disable_unset_class_const: false,
            po_parser_errors_only: false,
            tco_check_attribute_locations: true,
            po_disallow_func_ptrs_in_constants: false,
            tco_error_php_lambdas: false,
            tco_disallow_discarded_nullable_awaitables: false,
            po_enable_xhp_class_modifier: false,
            po_disable_xhp_element_mangling: false,
            po_disable_xhp_children_declarations: false,
            glean_service: "",
            glean_hostname: "",
            glean_port: 0,
            glean_reponame: "",
            symbol_write_root_path: "",
            symbol_write_hhi_path: "",
            symbol_write_ignore_paths: &[],
            symbol_write_index_paths: &[],
            symbol_write_include_hhi: true,
            po_enable_first_class_function_pointers: false,
            po_disable_modes: false,
            po_disable_hh_ignore_error: false,
            po_disable_array: false,
            po_disable_array_typehint: false,
            tco_enable_systemlib_annotations: false,
            tco_higher_kinded_types: false,
            tco_method_call_inference: false,
            tco_report_pos_from_reason: false,
            tco_typecheck_sample_rate: 1.0,
        }
    }
}

impl Eq for GlobalOptions<'_> {}

impl std::hash::Hash for GlobalOptions<'_> {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl Ord for GlobalOptions<'_> {
    fn cmp(&self, _: &Self) -> std::cmp::Ordering {
        unimplemented!()
    }
}
