// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1904eff6246ca2e8b4ce5346e9782836>>
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum AllOrSome<A> {
    All,
    ASome(Vec<A>),
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum NoneOrAllExcept<A> {
    NNone,
    #[rust_to_ocaml(name = "All_except")]
    AllExcept(Vec<A>),
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum ExtendedReasonsConfig {
    Extended(isize),
    Legacy,
    Debug,
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
    pub po: parser_options::ParserOptions,
    pub tco_saved_state: SavedState,
    pub tco_experimental_features: s_set::SSet,
    pub tco_migration_flags: s_set::SSet,
    pub tco_num_local_workers: Option<isize>,
    pub tco_defer_class_declaration_threshold: Option<isize>,
    pub tco_locl_cache_capacity: isize,
    pub tco_locl_cache_node_threshold: isize,
    pub so_naming_sqlite_path: Option<String>,
    pub po_disallow_toplevel_requires: bool,
    pub tco_log_large_fanouts_threshold: Option<isize>,
    pub tco_log_inference_constraints: bool,
    pub tco_language_feature_logging: bool,
    pub tco_timeout: isize,
    pub tco_disallow_invalid_arraykey: bool,
    pub code_agnostic_fixme: bool,
    pub allowed_fixme_codes_strict: i_set::ISet,
    pub log_levels: s_map::SMap<isize>,
    pub class_pointer_levels: s_map::SMap<isize>,
    pub tco_remote_old_decls_no_limit: bool,
    pub tco_fetch_remote_old_decls: bool,
    pub tco_populate_member_heaps: bool,
    pub tco_skip_hierarchy_checks: bool,
    pub tco_skip_tast_checks: bool,
    pub tco_coeffects: bool,
    pub tco_coeffects_local: bool,
    pub tco_strict_contexts: bool,
    pub tco_like_casts: bool,
    pub tco_check_xhp_attribute: bool,
    pub tco_check_redundant_generics: bool,
    pub tco_disallow_unresolved_type_variables: bool,
    pub tco_custom_error_config: custom_error_config::CustomErrorConfig,
    pub tco_const_attribute: bool,
    pub tco_check_attribute_locations: bool,
    pub tco_type_refinement_partition_shapes: bool,
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
    pub tco_error_php_lambdas: bool,
    pub tco_disallow_discarded_nullable_awaitables: bool,
    pub tco_higher_kinded_types: bool,
    pub tco_typecheck_sample_rate: f64,
    pub tco_enable_sound_dynamic: bool,
    pub tco_pessimise_builtins: bool,
    pub tco_enable_no_auto_dynamic: bool,
    pub tco_skip_check_under_dynamic: bool,
    pub tco_global_access_check_enabled: bool,
    pub tco_enable_strict_string_concat_interp: bool,
    pub tco_ignore_unsafe_cast: bool,
    pub tco_enable_expression_trees: bool,
    pub tco_enable_function_references: bool,
    pub tco_allowed_expression_tree_visitors: Vec<String>,
    pub tco_typeconst_concrete_concrete_error: bool,
    pub tco_meth_caller_only_public_visibility: bool,
    pub tco_require_extends_implements_ancestors: bool,
    pub tco_strict_value_equality: bool,
    pub tco_enforce_sealed_subclasses: bool,
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
    pub tco_populate_dead_unsafe_cast_heap: bool,
    pub dump_tast_hashes: bool,
    pub dump_tasts: Vec<String>,
    pub tco_autocomplete_mode: bool,
    pub tco_log_exhaustivity_check: bool,
    pub tco_sticky_quarantine: bool,
    pub tco_lsp_invalidation: bool,
    pub tco_autocomplete_sort_text: bool,
    pub tco_extended_reasons: Option<ExtendedReasonsConfig>,
    pub tco_disable_physical_equality: bool,
    pub hack_warnings: NoneOrAllExcept<isize>,
    pub warnings_default_all: bool,
    pub warnings_in_sandcastle: bool,
    pub tco_strict_switch: bool,
    pub tco_allowed_files_for_ignore_readonly: Vec<String>,
    pub tco_package_v2_exclude_patterns: Vec<String>,
    pub tco_package_v2_allow_classconst_violations: bool,
    pub tco_package_v2_allow_reifiable_tconst_violations: bool,
    pub tco_package_v2_allow_all_tconst_violations: bool,
    pub tco_package_v2_allow_reified_generics_violations: bool,
    pub tco_package_v2_allow_all_generics_violations: bool,
    pub re_no_cache: bool,
    pub hh_distc_should_disable_trace_store: bool,
    pub hh_distc_exponential_backoff_num_retries: isize,
    pub tco_enable_abstract_method_optional_parameters: bool,
    pub recursive_case_types: bool,
    pub class_sub_classname: bool,
    pub class_class_type: bool,
    pub safe_abstract: bool,
    pub needs_concrete: bool,
    pub allow_class_string_cast: bool,
}
