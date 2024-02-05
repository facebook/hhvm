// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e369cf9405c94b9fde084546fd7f32e6>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct SavedStateLoading {
    pub saved_state_manifold_api_key: Option<String>,
    pub log_saved_state_age_and_distance: bool,
    pub use_manifold_cython_client: bool,
    pub zstd_decompress_by_file: bool,
    pub use_compressed_dep_graph: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C)]
pub struct SavedState {
    pub loading: SavedStateLoading,
    pub rollouts: saved_state_rollouts::SavedStateRollouts,
    pub project_metadata_w_flags: bool,
}

/// Naming conventions for fields in this struct:
/// - tco_<feature/flag/setting> - type checker option
/// - po_<feature/flag/setting> - parser option
/// - so_<feature/flag/setting> - server option
#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct GlobalOptions {
    pub tco_saved_state: SavedState,
    pub tco_experimental_features: s_set::SSet,
    pub tco_migration_flags: s_set::SSet,
    pub tco_num_local_workers: Option<isize>,
    pub tco_defer_class_declaration_threshold: Option<isize>,
    pub tco_locl_cache_capacity: isize,
    pub tco_locl_cache_node_threshold: isize,
    pub so_naming_sqlite_path: Option<String>,
    pub po_auto_namespace_map: Vec<(String, String)>,
    pub po_codegen: bool,
    pub po_deregister_php_stdlib: bool,
    pub po_disallow_toplevel_requires: bool,
    pub po_allow_unstable_features: bool,
    pub tco_log_large_fanouts_threshold: Option<isize>,
    pub tco_log_inference_constraints: bool,
    pub tco_language_feature_logging: bool,
    pub tco_timeout: isize,
    pub tco_disallow_invalid_arraykey: bool,
    pub tco_disallow_byref_dynamic_calls: bool,
    pub tco_disallow_byref_calls: bool,
    pub code_agnostic_fixme: bool,
    pub allowed_fixme_codes_strict: i_set::ISet,
    pub log_levels: s_map::SMap<isize>,
    pub po_disable_lval_as_an_expression: bool,
    pub tco_remote_old_decls_no_limit: bool,
    pub tco_use_old_decls_from_cas: bool,
    pub tco_fetch_remote_old_decls: bool,
    pub tco_populate_member_heaps: bool,
    pub tco_skip_hierarchy_checks: bool,
    pub tco_skip_tast_checks: bool,
    pub tco_like_type_hints: bool,
    pub tco_union_intersection_type_hints: bool,
    pub tco_coeffects: bool,
    pub tco_coeffects_local: bool,
    pub tco_strict_contexts: bool,
    pub tco_like_casts: bool,
    pub tco_check_xhp_attribute: bool,
    pub tco_check_redundant_generics: bool,
    pub tco_disallow_unresolved_type_variables: bool,
    pub tco_custom_error_config: custom_error_config::CustomErrorConfig,
    pub po_enable_class_level_where_clauses: bool,
    pub po_disable_legacy_soft_typehints: bool,
    pub po_allowed_decl_fixme_codes: i_set::ISet,
    pub tco_const_static_props: bool,
    pub po_disable_legacy_attribute_syntax: bool,
    pub tco_const_attribute: bool,
    pub po_const_default_func_args: bool,
    pub po_const_default_lambda_args: bool,
    pub po_disallow_silence: bool,
    pub po_abstract_static_props: bool,
    pub po_parser_errors_only: bool,
    pub tco_check_attribute_locations: bool,
    pub glean_reponame: String,
    pub symbol_write_index_inherited_members: bool,
    pub symbol_write_ownership: bool,
    pub symbol_write_root_path: String,
    pub symbol_write_hhi_path: String,
    pub symbol_write_ignore_paths: Vec<String>,
    pub symbol_write_index_paths: Vec<String>,
    pub symbol_write_index_paths_file: Option<String>,
    pub symbol_write_index_paths_file_output: Option<String>,
    pub symbol_write_include_hhi: bool,
    pub symbol_write_sym_hash_in: Option<String>,
    pub symbol_write_exclude_out: Option<String>,
    pub symbol_write_referenced_out: Option<String>,
    pub symbol_write_reindexed_out: Option<String>,
    pub symbol_write_sym_hash_out: bool,
    pub po_disallow_func_ptrs_in_constants: bool,
    pub tco_error_php_lambdas: bool,
    pub tco_disallow_discarded_nullable_awaitables: bool,
    pub po_enable_xhp_class_modifier: bool,
    pub po_disable_xhp_element_mangling: bool,
    pub po_disable_xhp_children_declarations: bool,
    pub po_disable_hh_ignore_error: isize,
    pub po_keep_user_attributes: bool,
    pub tco_is_systemlib: bool,
    pub tco_higher_kinded_types: bool,
    pub tco_method_call_inference: bool,
    pub tco_report_pos_from_reason: bool,
    pub tco_typecheck_sample_rate: f64,
    pub tco_enable_sound_dynamic: bool,
    pub tco_pessimise_builtins: bool,
    pub tco_enable_no_auto_dynamic: bool,
    pub tco_skip_check_under_dynamic: bool,
    pub tco_global_access_check_enabled: bool,
    pub po_interpret_soft_types_as_like_types: bool,
    pub tco_enable_strict_string_concat_interp: bool,
    pub tco_ignore_unsafe_cast: bool,
    pub tco_no_parser_readonly_check: bool,
    pub tco_enable_expression_trees: bool,
    pub tco_enable_function_references: bool,
    pub tco_allowed_expression_tree_visitors: Vec<String>,
    pub tco_math_new_code: bool,
    pub tco_typeconst_concrete_concrete_error: bool,
    pub tco_enable_strict_const_semantics: isize,
    pub tco_strict_wellformedness: isize,
    pub tco_meth_caller_only_public_visibility: bool,
    pub tco_require_extends_implements_ancestors: bool,
    pub tco_strict_value_equality: bool,
    pub tco_enforce_sealed_subclasses: bool,
    pub tco_everything_sdt: bool,
    pub tco_implicit_inherit_sdt: bool,
    pub tco_explicit_consistent_constructors: isize,
    pub tco_require_types_class_consts: isize,
    pub tco_type_printer_fuel: isize,
    pub tco_specify_manifold_api_key: bool,
    pub tco_profile_top_level_definitions: bool,
    pub tco_allow_all_files_for_module_declarations: bool,
    pub tco_allowed_files_for_module_declarations: Vec<String>,
    pub tco_record_fine_grained_dependencies: bool,
    pub tco_loop_iteration_upper_bound: Option<isize>,
    pub tco_use_type_alias_heap: bool,
    pub tco_populate_dead_unsafe_cast_heap: bool,
    pub po_disallow_static_constants_in_default_func_args: bool,
    pub tco_rust_elab: bool,
    pub dump_tast_hashes: bool,
    pub dump_tasts: Vec<String>,
    pub tco_autocomplete_mode: bool,
    pub tco_package_info: package_info::PackageInfo,
    pub po_unwrap_concurrent: bool,
    pub tco_log_exhaustivity_check: bool,
    pub po_disallow_direct_superglobals_refs: bool,
    pub tco_sticky_quarantine: bool,
    pub tco_lsp_invalidation: bool,
    pub tco_autocomplete_sort_text: bool,
    pub po_nameof_precedence: bool,
    pub po_strict_utf8: bool,
}
