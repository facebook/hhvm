// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::custom_error_config::CustomErrorConfig;
use crate::gen::global_options::GlobalOptions;
use crate::gen::global_options::SavedState;
use crate::gen::global_options::SavedStateLoading;
use crate::gen::package_info::PackageInfo;
use crate::gen::saved_state_rollouts::SavedStateRollouts;
use crate::i_set;
use crate::s_map;
use crate::s_set;

impl Default for SavedStateLoading {
    fn default() -> Self {
        Self {
            saved_state_manifold_api_key: None,
            log_saved_state_age_and_distance: false,
            use_manifold_cython_client: false,
            zstd_decompress_by_file: true,
            use_compressed_dep_graph: true,
        }
    }
}

impl Default for SavedState {
    fn default() -> Self {
        Self {
            loading: SavedStateLoading::default(),
            rollouts: SavedStateRollouts::default(),
            project_metadata_w_flags: true,
        }
    }
}

impl Default for GlobalOptions {
    fn default() -> Self {
        Self {
            tco_saved_state: SavedState::default(),
            tco_experimental_features: s_set::SSet::new(),
            tco_migration_flags: s_set::SSet::new(),
            tco_num_local_workers: None,
            tco_defer_class_declaration_threshold: None,
            tco_locl_cache_capacity: 30,
            tco_locl_cache_node_threshold: 10_000,
            so_naming_sqlite_path: None,
            po_auto_namespace_map: vec![],
            po_codegen: false,
            po_deregister_php_stdlib: false,
            po_disallow_toplevel_requires: false,
            po_allow_unstable_features: false, // true in /etc/hh.conf
            tco_log_large_fanouts_threshold: None,
            tco_log_inference_constraints: false,
            tco_language_feature_logging: false,
            tco_timeout: 0,
            tco_disallow_invalid_arraykey: false, // true in ocaml, true in .hhconfig
            tco_disallow_byref_dynamic_calls: false,
            tco_disallow_byref_calls: true,
            code_agnostic_fixme: false,
            allowed_fixme_codes_strict: i_set::ISet::new(),
            log_levels: s_map::SMap::new(),
            po_disable_lval_as_an_expression: false, // true in ocaml
            tco_remote_old_decls_no_limit: false,
            tco_fetch_remote_old_decls: true,
            tco_populate_member_heaps: true,
            tco_skip_hierarchy_checks: false,
            tco_skip_tast_checks: false,
            tco_like_type_hints: false,
            tco_union_intersection_type_hints: false,
            tco_coeffects: true,
            tco_coeffects_local: true,
            tco_strict_contexts: true,
            tco_like_casts: false,
            tco_check_xhp_attribute: false,
            tco_check_redundant_generics: false,
            tco_disallow_unresolved_type_variables: false,
            tco_custom_error_config: CustomErrorConfig::default(),
            po_enable_class_level_where_clauses: false,
            po_disable_legacy_soft_typehints: true,
            po_allowed_decl_fixme_codes: i_set::ISet::new(),
            tco_const_static_props: false,
            po_disable_legacy_attribute_syntax: false,
            tco_const_attribute: false,
            po_const_default_func_args: false,
            po_const_default_lambda_args: false,
            po_disallow_silence: false,
            po_abstract_static_props: false,
            po_parser_errors_only: false,
            tco_check_attribute_locations: true,
            po_disallow_func_ptrs_in_constants: false,
            tco_error_php_lambdas: false,
            tco_disallow_discarded_nullable_awaitables: false,
            po_enable_xhp_class_modifier: false, // true in ocaml
            po_disable_xhp_element_mangling: false, // true in ocaml
            po_disable_xhp_children_declarations: false, // true in ocaml
            po_keep_user_attributes: false,
            glean_reponame: String::new(), // "www.autocomplete" in ocaml
            symbol_write_index_inherited_members: true,
            symbol_write_ownership: false,
            symbol_write_root_path: String::new(), // "www" in ocaml
            symbol_write_hhi_path: String::new(),  // "hhi" in ocaml
            symbol_write_ignore_paths: vec![],
            symbol_write_index_paths: vec![],
            symbol_write_index_paths_file: None,
            symbol_write_include_hhi: false,
            symbol_write_index_paths_file_output: None,
            symbol_write_sym_hash_in: None,
            symbol_write_exclude_out: None,
            symbol_write_referenced_out: None,
            symbol_write_reindexed_out: None,
            symbol_write_sym_hash_out: false,
            po_disable_hh_ignore_error: 0,
            tco_is_systemlib: false,
            tco_higher_kinded_types: false,
            tco_method_call_inference: false,
            tco_report_pos_from_reason: false,
            tco_typecheck_sample_rate: 1.0,
            tco_enable_sound_dynamic: false,
            tco_pessimise_builtins: false,
            tco_enable_no_auto_dynamic: false,
            tco_skip_check_under_dynamic: false,
            tco_global_access_check_enabled: false,
            po_interpret_soft_types_as_like_types: false,
            tco_enable_strict_string_concat_interp: false,
            tco_ignore_unsafe_cast: false,
            tco_no_parser_readonly_check: false,
            tco_enable_function_references: false,
            tco_enable_expression_trees: false,
            tco_allowed_expression_tree_visitors: vec![],
            tco_typeconst_concrete_concrete_error: false,
            tco_enable_strict_const_semantics: 0,
            tco_strict_wellformedness: 0,
            tco_meth_caller_only_public_visibility: true,
            tco_require_extends_implements_ancestors: false,
            tco_strict_value_equality: false,
            tco_enforce_sealed_subclasses: false,
            tco_everything_sdt: false,
            tco_implicit_inherit_sdt: false,
            tco_explicit_consistent_constructors: 0,
            tco_require_types_class_consts: 0,
            tco_type_printer_fuel: 100,
            tco_specify_manifold_api_key: false,
            tco_profile_top_level_definitions: false,
            tco_allow_all_files_for_module_declarations: false,
            tco_allowed_files_for_module_declarations: vec![],
            tco_record_fine_grained_dependencies: false,
            tco_loop_iteration_upper_bound: None,
            tco_use_type_alias_heap: false,
            tco_populate_dead_unsafe_cast_heap: false,
            po_disallow_static_constants_in_default_func_args: false,
            tco_rust_elab: false,
            dump_tast_hashes: false,
            dump_tasts: vec![],
            tco_autocomplete_mode: false,
            tco_package_info: PackageInfo::default(),
            po_unwrap_concurrent: false,
            tco_log_exhaustivity_check: false,
            po_disallow_direct_superglobals_refs: false,
            tco_use_old_decls_from_cas: false,
            tco_sticky_quarantine: false,
            tco_lsp_invalidation: false,
            tco_autocomplete_sort_text: false,
            po_nameof_precedence: false,
            po_strict_utf8: false,
        }
    }
}
