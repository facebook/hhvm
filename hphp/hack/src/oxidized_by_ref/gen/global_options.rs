// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2a77a2c2d322fbefbf2f3d04a3a7b4dd>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// Naming conventions for fields in this struct:
/// - tco_<feature/flag/setting> - type checker option
/// - po_<feature/flag/setting> - parser option
/// - so_<feature/flag/setting> - server option
#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct GlobalOptions<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_experimental_features: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_migration_flags: s_set::SSet<'a>,
    pub tco_num_local_workers: Option<isize>,
    pub tco_parallel_type_checking_threshold: isize,
    pub tco_max_typechecker_worker_memory_mb: Option<isize>,
    pub tco_defer_class_declaration_threshold: Option<isize>,
    pub tco_prefetch_deferred_files: bool,
    pub tco_remote_type_check_threshold: isize,
    pub tco_remote_type_check: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_remote_worker_key: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_remote_check_id: Option<&'a str>,
    pub tco_remote_max_batch_size: isize,
    pub tco_remote_min_batch_size: isize,
    pub tco_num_remote_workers: isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub so_remote_version_specifier: Option<&'a str>,
    pub so_remote_worker_vfs_checkout_threshold: isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub so_naming_sqlite_path: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub po_auto_namespace_map: &'a [(&'a str, &'a str)],
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
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub allowed_fixme_codes_strict: i_set::ISet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub log_levels: s_map::SMap<'a, isize>,
    pub po_disable_lval_as_an_expression: bool,
    pub tco_shallow_class_decl: bool,
    pub tco_force_shallow_decl_fanout: bool,
    pub tco_remote_old_decls_no_limit: bool,
    pub tco_fetch_remote_old_decls: bool,
    pub tco_force_load_hot_shallow_decls: bool,
    pub tco_populate_member_heaps: bool,
    pub tco_skip_hierarchy_checks: bool,
    pub tco_skip_tast_checks: bool,
    pub tco_like_type_hints: bool,
    pub tco_union_intersection_type_hints: bool,
    pub tco_coeffects: bool,
    pub tco_coeffects_local: bool,
    pub tco_strict_contexts: bool,
    pub tco_like_casts: bool,
    pub tco_simple_pessimize: f64,
    pub tco_complex_coercion: bool,
    pub tco_check_xhp_attribute: bool,
    pub tco_check_redundant_generics: bool,
    pub tco_disallow_unresolved_type_variables: bool,
    pub po_enable_class_level_where_clauses: bool,
    pub po_disable_legacy_soft_typehints: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub po_allowed_decl_fixme_codes: i_set::ISet<'a>,
    pub po_allow_new_attribute_syntax: bool,
    pub tco_global_inference: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_gi_reinfer_types: &'a [&'a str],
    pub tco_ordered_solving: bool,
    pub tco_const_static_props: bool,
    pub po_disable_legacy_attribute_syntax: bool,
    pub tco_const_attribute: bool,
    pub po_const_default_func_args: bool,
    pub po_const_default_lambda_args: bool,
    pub po_disallow_silence: bool,
    pub po_abstract_static_props: bool,
    pub po_parser_errors_only: bool,
    pub tco_check_attribute_locations: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub glean_service: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub glean_hostname: &'a str,
    pub glean_port: isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub glean_reponame: &'a str,
    pub symbol_write_ownership: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_root_path: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_hhi_path: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_ignore_paths: &'a [&'a str],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_index_paths: &'a [&'a str],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_index_paths_file: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_index_paths_file_output: Option<&'a str>,
    pub symbol_write_include_hhi: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_sym_hash_in: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub symbol_write_exclude_out: Option<&'a str>,
    pub symbol_write_sym_hash_out: bool,
    pub po_disallow_func_ptrs_in_constants: bool,
    pub tco_error_php_lambdas: bool,
    pub tco_disallow_discarded_nullable_awaitables: bool,
    pub po_enable_xhp_class_modifier: bool,
    pub po_disable_xhp_element_mangling: bool,
    pub po_disable_xhp_children_declarations: bool,
    pub po_enable_enum_classes: bool,
    pub po_disable_hh_ignore_error: isize,
    pub tco_is_systemlib: bool,
    pub tco_higher_kinded_types: bool,
    pub tco_method_call_inference: bool,
    pub tco_report_pos_from_reason: bool,
    pub tco_typecheck_sample_rate: f64,
    pub tco_enable_sound_dynamic: bool,
    pub po_disallow_fun_and_cls_meth_pseudo_funcs: bool,
    pub po_disallow_inst_meth: bool,
    pub tco_use_direct_decl_parser: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_ifc_enabled: &'a [&'a str],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_global_access_check_files_enabled: &'a [&'a str],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_global_access_check_functions_enabled: s_set::SSet<'a>,
    pub tco_global_access_check_on_write: bool,
    pub tco_global_access_check_on_read: bool,
    pub po_enable_enum_supertyping: bool,
    pub po_interpret_soft_types_as_like_types: bool,
    pub tco_enable_strict_string_concat_interp: bool,
    pub tco_ignore_unsafe_cast: bool,
    pub tco_no_parser_readonly_check: bool,
    pub tco_enable_expression_trees: bool,
    pub tco_enable_modules: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_allowed_expression_tree_visitors: &'a [&'a str],
    pub tco_math_new_code: bool,
    pub tco_typeconst_concrete_concrete_error: bool,
    pub tco_enable_strict_const_semantics: isize,
    pub tco_strict_wellformedness: isize,
    pub tco_meth_caller_only_public_visibility: bool,
    pub tco_require_extends_implements_ancestors: bool,
    pub tco_strict_value_equality: bool,
    pub tco_enforce_sealed_subclasses: bool,
    pub tco_everything_sdt: bool,
    pub tco_pessimise_builtins: bool,
    pub tco_explicit_consistent_constructors: isize,
    pub tco_require_types_class_consts: isize,
    pub tco_type_printer_fuel: isize,
    pub tco_log_saved_state_age_and_distance: bool,
    pub tco_specify_manifold_api_key: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_saved_state_manifold_api_key: Option<&'a str>,
    pub tco_profile_top_level_definitions: bool,
    pub tco_allow_all_files_for_module_declarations: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tco_allowed_files_for_module_declarations: &'a [&'a str],
    pub tco_use_manifold_cython_client: bool,
    pub tco_record_fine_grained_dependencies: bool,
    pub tco_loop_iteration_upper_bound: Option<isize>,
    pub tco_expression_tree_virtualize_functions: bool,
    pub tco_substitution_mutation: bool,
    pub tco_use_type_alias_heap: bool,
}
impl<'a> TrivialDrop for GlobalOptions<'a> {}
arena_deserializer::impl_deserialize_in_arena!(GlobalOptions<'arena>);
