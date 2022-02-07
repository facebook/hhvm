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
            tco_num_local_workers: None,
            tco_parallel_type_checking_threshold: 10,
            tco_max_typechecker_worker_memory_mb: None,
            tco_defer_class_declaration_threshold: None,
            tco_prefetch_deferred_files: false,
            tco_remote_type_check_threshold: None,
            tco_remote_type_check: true,
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
            po_disallow_toplevel_requires: false,
            po_allow_unstable_features: false,
            tco_log_inference_constraints: false,
            tco_language_feature_logging: false,
            tco_timeout: 0,
            tco_disallow_invalid_arraykey: false,
            tco_disallow_byref_dynamic_calls: false,
            tco_disallow_byref_calls: true,
            allowed_fixme_codes_strict: i_set::ISet::new(),
            allowed_fixme_codes_partial: i_set::ISet::new(),
            codes_not_raised_partial: i_set::ISet::new(),
            log_levels: s_map::SMap::new(),
            po_disable_lval_as_an_expression: false,
            tco_shallow_class_decl: false,
            tco_force_shallow_decl_fanout: false,
            tco_fetch_remote_old_decls: false,
            tco_force_load_hot_shallow_decls: false,
            tco_skip_hierarchy_checks: false,
            po_rust_parser_errors: false,
            tco_like_type_hints: false,
            tco_union_intersection_type_hints: false,
            tco_coeffects: true,
            tco_coeffects_local: true,
            tco_strict_contexts: true,
            tco_like_casts: false,
            tco_simple_pessimize: 0.0,
            tco_complex_coercion: false,
            error_codes_treated_strictly: i_set::ISet::new(),
            tco_check_xhp_attribute: false,
            tco_check_redundant_generics: false,
            tco_disallow_unresolved_type_variables: false,
            tco_disallow_trait_reuse: false,
            po_enable_class_level_where_clauses: false,
            po_disable_legacy_soft_typehints: true,
            po_allowed_decl_fixme_codes: i_set::ISet::new(),
            po_allow_new_attribute_syntax: false,
            tco_global_inference: false,
            tco_gi_reinfer_types: vec![],
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
            glean_service: String::new(),
            glean_hostname: String::new(),
            glean_port: 0,
            glean_reponame: String::new(),
            symbol_write_root_path: String::new(),
            symbol_write_hhi_path: String::new(),
            symbol_write_ignore_paths: vec![],
            symbol_write_index_paths: vec![],
            symbol_write_index_paths_file: None,
            symbol_write_include_hhi: true,
            symbol_write_index_paths_file_output: None,
            po_enable_enum_classes: true,
            po_disable_hh_ignore_error: false,
            po_disable_array: false,
            po_disable_array_typehint: false,
            tco_enable_systemlib_annotations: false,
            tco_higher_kinded_types: false,
            tco_method_call_inference: false,
            tco_report_pos_from_reason: false,
            tco_typecheck_sample_rate: 1.0,
            tco_enable_sound_dynamic: false,
            po_disallow_fun_and_cls_meth_pseudo_funcs: false,
            po_disallow_inst_meth: false,
            tco_use_direct_decl_parser: false,
            tco_ifc_enabled: vec![],
            tco_global_write_check_enabled: vec![],
            po_enable_enum_supertyping: false,
            po_interpret_soft_types_as_like_types: false,
            tco_enable_strict_string_concat_interp: false,
            tco_ignore_unsafe_cast: false,
            tco_readonly: false,
            tco_enable_modules: false,
            tco_enable_expression_trees: false,
            tco_allowed_expression_tree_visitors: vec![],
            tco_math_new_code: false,
            tco_typeconst_concrete_concrete_error: false,
            tco_enable_strict_const_semantics: false,
            tco_meth_caller_only_public_visibility: true,
            tco_require_extends_implements_ancestors: false,
            tco_strict_value_equality: false,
            tco_enforce_sealed_subclasses: false,
            tco_everything_sdt: false,
            tco_pessimise_builtins: false,
            tco_enable_disk_heap: true,
            tco_explicit_consistent_constructors: 0,
            tco_type_printer_fuel: 100,
            tco_log_saved_state_age_and_distance: false,
            tco_specify_manifold_api_key: false,
            tco_saved_state_manifold_api_key: None,
        }
    }
}

impl Eq for GlobalOptions {}

impl std::hash::Hash for GlobalOptions {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl no_pos_hash::NoPosHash for GlobalOptions {
    fn hash<H>(&self, _: &mut H) {
        unimplemented!()
    }
}

impl Ord for GlobalOptions {
    fn cmp(&self, _: &Self) -> std::cmp::Ordering {
        unimplemented!()
    }
}
