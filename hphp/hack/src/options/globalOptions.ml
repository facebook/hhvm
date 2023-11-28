(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

(* NOTE: this file is in the middle of a large refactoring.
   Please try to avoid changes other than adding a field to struct `t`,
   updating `default` value and `make` functions
   Encapsulation and helpers better fit in the respective modules: e.g.,
   TypecheckerOptions for tco_* fields
   ParserOptions for po_*fields
   etc.
*)

type saved_state_loading = {
  saved_state_manifold_api_key: string option;
  log_saved_state_age_and_distance: bool;
  use_manifold_cython_client: bool;
  zstd_decompress_by_file: bool;
  use_compressed_dep_graph: bool;
}
[@@deriving show, eq]

let default_saved_state_loading =
  {
    saved_state_manifold_api_key = None;
    log_saved_state_age_and_distance = false;
    use_manifold_cython_client = false;
    zstd_decompress_by_file = true;
    use_compressed_dep_graph = true;
  }

type saved_state = {
  loading: saved_state_loading;
  rollouts: Saved_state_rollouts.t;
  project_metadata_w_flags: bool;
}
[@@deriving show, eq]

let default_saved_state =
  {
    loading = default_saved_state_loading;
    rollouts = Saved_state_rollouts.default;
    project_metadata_w_flags = true;
  }

let with_saved_state_manifold_api_key saved_state_manifold_api_key ss =
  { ss with loading = { ss.loading with saved_state_manifold_api_key } }

let with_use_manifold_cython_client use_manifold_cython_client ss =
  { ss with loading = { ss.loading with use_manifold_cython_client } }

let with_log_saved_state_age_and_distance log_saved_state_age_and_distance ss =
  { ss with loading = { ss.loading with log_saved_state_age_and_distance } }

let with_zstd_decompress_by_file zstd_decompress_by_file ss =
  { ss with loading = { ss.loading with zstd_decompress_by_file } }

(** Naming conventions for fields in this struct:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option
*)
type t = {
  tco_saved_state: saved_state;
  tco_experimental_features: SSet.t;
  tco_migration_flags: SSet.t;
  tco_num_local_workers: int option;
  tco_max_typechecker_worker_memory_mb: int option;
  tco_defer_class_declaration_threshold: int option;
  tco_locl_cache_capacity: int;
  tco_locl_cache_node_threshold: int;
  so_naming_sqlite_path: string option;
  po_auto_namespace_map: (string * string) list;
  po_codegen: bool;
  po_deregister_php_stdlib: bool;
  po_disallow_toplevel_requires: bool;
  po_allow_unstable_features: bool;
  tco_log_large_fanouts_threshold: int option;
  tco_log_inference_constraints: bool;
  tco_language_feature_logging: bool;
  tco_timeout: int;
  tco_disallow_invalid_arraykey: bool;
  tco_disallow_byref_dynamic_calls: bool;
  tco_disallow_byref_calls: bool;
  code_agnostic_fixme: bool;
  allowed_fixme_codes_strict: ISet.t;
  log_levels: int SMap.t;
  po_disable_lval_as_an_expression: bool;
  tco_remote_old_decls_no_limit: bool;
  tco_use_old_decls_from_cas: bool;
  tco_fetch_remote_old_decls: bool;
  tco_populate_member_heaps: bool;
  tco_skip_hierarchy_checks: bool;
  tco_skip_tast_checks: bool;
  tco_like_type_hints: bool;
  tco_union_intersection_type_hints: bool;
  tco_coeffects: bool;
  tco_coeffects_local: bool;
  tco_strict_contexts: bool;
  tco_like_casts: bool;
  tco_check_xhp_attribute: bool;
  tco_check_redundant_generics: bool;
  tco_disallow_unresolved_type_variables: bool;
  tco_custom_error_config: Custom_error_config.t;
  po_enable_class_level_where_clauses: bool;
  po_disable_legacy_soft_typehints: bool;
  po_allowed_decl_fixme_codes: ISet.t;
  tco_const_static_props: bool;
  po_disable_legacy_attribute_syntax: bool;
  tco_const_attribute: bool;
  po_const_default_func_args: bool;
  po_const_default_lambda_args: bool;
  po_disallow_silence: bool;
  po_abstract_static_props: bool;
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  glean_reponame: string;
  symbol_write_index_inherited_members: bool;
  symbol_write_ownership: bool;
  symbol_write_root_path: string;
  symbol_write_hhi_path: string;
  symbol_write_ignore_paths: string list;
  symbol_write_index_paths: string list;
  symbol_write_index_paths_file: string option;
  symbol_write_index_paths_file_output: string option;
  symbol_write_include_hhi: bool;
  symbol_write_sym_hash_in: string option;
  symbol_write_exclude_out: string option;
  symbol_write_referenced_out: string option;
  symbol_write_reindexed_out: string option;
  symbol_write_sym_hash_out: bool;
  po_disallow_func_ptrs_in_constants: bool;
  tco_error_php_lambdas: bool;
  tco_disallow_discarded_nullable_awaitables: bool;
  po_enable_xhp_class_modifier: bool;
  po_disable_xhp_element_mangling: bool;
  po_disable_xhp_children_declarations: bool;
  po_disable_hh_ignore_error: int;
  po_keep_user_attributes: bool;
  tco_is_systemlib: bool;
  tco_higher_kinded_types: bool;
  tco_method_call_inference: bool;
  tco_report_pos_from_reason: bool;
  tco_typecheck_sample_rate: float;
  tco_enable_sound_dynamic: bool;
  tco_pessimise_builtins: bool;
  tco_enable_no_auto_dynamic: bool;
  tco_skip_check_under_dynamic: bool;
  tco_global_access_check_enabled: bool;
  po_interpret_soft_types_as_like_types: bool;
  tco_enable_strict_string_concat_interp: bool;
  tco_ignore_unsafe_cast: bool;
  tco_no_parser_readonly_check: bool;
  tco_enable_expression_trees: bool;
  tco_enable_modules: bool;
  tco_allowed_expression_tree_visitors: string list;
  tco_math_new_code: bool;
  tco_typeconst_concrete_concrete_error: bool;
  tco_enable_strict_const_semantics: int;
  tco_strict_wellformedness: int;
  tco_meth_caller_only_public_visibility: bool;
  tco_require_extends_implements_ancestors: bool;
  tco_strict_value_equality: bool;
  tco_enforce_sealed_subclasses: bool;
  tco_everything_sdt: bool;
  tco_explicit_consistent_constructors: int;
  tco_require_types_class_consts: int;
  tco_type_printer_fuel: int;
  tco_specify_manifold_api_key: bool;
  tco_profile_top_level_definitions: bool;
  tco_allow_all_files_for_module_declarations: bool;
  tco_allowed_files_for_module_declarations: string list;
  tco_record_fine_grained_dependencies: bool;
  tco_loop_iteration_upper_bound: int option;
  tco_expression_tree_virtualize_functions: bool;
  tco_use_type_alias_heap: bool;
  tco_populate_dead_unsafe_cast_heap: bool;
  po_disallow_static_constants_in_default_func_args: bool;
  tco_rust_elab: bool;
  dump_tast_hashes: bool;
  dump_tasts: string list;
  tco_autocomplete_mode: bool;
  tco_package_info: PackageInfo.t;
  po_unwrap_concurrent: bool;
  tco_log_exhaustivity_check: bool;
  po_disallow_direct_superglobals_refs: bool;
  tco_sticky_quarantine: bool;
  tco_autocomplete_skip_hierarchy_checks: bool;
}
[@@deriving eq, show]

let default =
  {
    tco_saved_state = default_saved_state;
    tco_experimental_features = SSet.empty;
    tco_migration_flags = SSet.empty;
    tco_num_local_workers = None;
    tco_max_typechecker_worker_memory_mb = None;
    tco_defer_class_declaration_threshold = None;
    tco_locl_cache_capacity = 30;
    tco_locl_cache_node_threshold = 10_000;
    so_naming_sqlite_path = None;
    po_auto_namespace_map = [];
    po_codegen = false;
    po_disallow_toplevel_requires = false;
    po_deregister_php_stdlib = false;
    po_allow_unstable_features = false;
    tco_log_large_fanouts_threshold = None;
    tco_log_inference_constraints = false;
    tco_language_feature_logging = false;
    tco_timeout = 0;
    tco_disallow_invalid_arraykey = true;
    tco_disallow_byref_dynamic_calls = false;
    tco_disallow_byref_calls = true;
    code_agnostic_fixme = false;
    allowed_fixme_codes_strict = ISet.empty;
    log_levels = SMap.empty;
    po_disable_lval_as_an_expression = true;
    tco_remote_old_decls_no_limit = false;
    tco_use_old_decls_from_cas = false;
    tco_fetch_remote_old_decls = true;
    tco_populate_member_heaps = true;
    tco_skip_hierarchy_checks = false;
    tco_skip_tast_checks = false;
    tco_like_type_hints = false;
    tco_union_intersection_type_hints = false;
    tco_coeffects = true;
    tco_coeffects_local = true;
    tco_strict_contexts = true;
    tco_like_casts = false;
    tco_check_xhp_attribute = false;
    tco_check_redundant_generics = false;
    tco_disallow_unresolved_type_variables = false;
    tco_custom_error_config = Custom_error_config.empty;
    po_enable_class_level_where_clauses = false;
    po_disable_legacy_soft_typehints = true;
    po_allowed_decl_fixme_codes = ISet.empty;
    tco_const_static_props = false;
    po_disable_legacy_attribute_syntax = false;
    tco_const_attribute = false;
    po_const_default_func_args = false;
    po_const_default_lambda_args = false;
    po_disallow_silence = false;
    po_abstract_static_props = false;
    po_parser_errors_only = false;
    tco_check_attribute_locations = true;
    glean_reponame = "www.hack.light";
    symbol_write_index_inherited_members = true;
    symbol_write_ownership = false;
    symbol_write_root_path = "www";
    symbol_write_hhi_path = "hhi";
    symbol_write_ignore_paths = [];
    symbol_write_index_paths = [];
    symbol_write_index_paths_file = None;
    symbol_write_index_paths_file_output = None;
    symbol_write_include_hhi = false;
    symbol_write_sym_hash_in = None;
    symbol_write_exclude_out = None;
    symbol_write_referenced_out = None;
    symbol_write_reindexed_out = None;
    symbol_write_sym_hash_out = false;
    po_disallow_func_ptrs_in_constants = false;
    tco_error_php_lambdas = false;
    tco_disallow_discarded_nullable_awaitables = false;
    po_enable_xhp_class_modifier = true;
    po_disable_xhp_element_mangling = true;
    po_disable_xhp_children_declarations = true;
    po_disable_hh_ignore_error = 0;
    po_keep_user_attributes = false;
    tco_is_systemlib = false;
    tco_higher_kinded_types = false;
    tco_method_call_inference = false;
    tco_report_pos_from_reason = false;
    tco_typecheck_sample_rate = 1.0;
    tco_enable_sound_dynamic = false;
    tco_pessimise_builtins = false;
    tco_enable_no_auto_dynamic = false;
    tco_skip_check_under_dynamic = false;
    tco_global_access_check_enabled = false;
    po_interpret_soft_types_as_like_types = false;
    tco_enable_strict_string_concat_interp = false;
    tco_ignore_unsafe_cast = false;
    tco_no_parser_readonly_check = false;
    tco_enable_expression_trees = false;
    tco_enable_modules = false;
    tco_allowed_expression_tree_visitors = [];
    tco_math_new_code = false;
    tco_typeconst_concrete_concrete_error = false;
    tco_enable_strict_const_semantics = 0;
    tco_strict_wellformedness = 0;
    tco_meth_caller_only_public_visibility = true;
    tco_require_extends_implements_ancestors = false;
    tco_strict_value_equality = false;
    tco_enforce_sealed_subclasses = false;
    tco_everything_sdt = false;
    tco_explicit_consistent_constructors = 0;
    tco_require_types_class_consts = 0;
    tco_type_printer_fuel = 100;
    tco_specify_manifold_api_key = false;
    tco_profile_top_level_definitions = false;
    tco_allow_all_files_for_module_declarations = true;
    tco_allowed_files_for_module_declarations = [];
    tco_record_fine_grained_dependencies = false;
    tco_loop_iteration_upper_bound = None;
    tco_expression_tree_virtualize_functions = false;
    tco_use_type_alias_heap = false;
    tco_populate_dead_unsafe_cast_heap = false;
    po_disallow_static_constants_in_default_func_args = false;
    tco_rust_elab = false;
    dump_tast_hashes = false;
    dump_tasts = [];
    tco_autocomplete_mode = false;
    tco_package_info = PackageInfo.empty;
    po_unwrap_concurrent = false;
    tco_log_exhaustivity_check = false;
    po_disallow_direct_superglobals_refs = false;
    tco_sticky_quarantine = false;
    tco_autocomplete_skip_hierarchy_checks = false;
  }

let set
    ?tco_saved_state
    ?po_deregister_php_stdlib
    ?po_disallow_toplevel_requires
    ?tco_log_large_fanouts_threshold
    ?tco_log_inference_constraints
    ?tco_experimental_features
    ?tco_migration_flags
    ?tco_num_local_workers
    ?tco_max_typechecker_worker_memory_mb
    ?tco_defer_class_declaration_threshold
    ?tco_locl_cache_capacity
    ?tco_locl_cache_node_threshold
    ?so_naming_sqlite_path
    ?po_auto_namespace_map
    ?po_codegen
    ?tco_language_feature_logging
    ?tco_timeout
    ?tco_disallow_invalid_arraykey
    ?tco_disallow_byref_dynamic_calls
    ?tco_disallow_byref_calls
    ?code_agnostic_fixme
    ?allowed_fixme_codes_strict
    ?log_levels
    ?po_disable_lval_as_an_expression
    ?tco_remote_old_decls_no_limit
    ?tco_use_old_decls_from_cas
    ?tco_fetch_remote_old_decls
    ?tco_populate_member_heaps
    ?tco_skip_hierarchy_checks
    ?tco_skip_tast_checks
    ?tco_like_type_hints
    ?tco_union_intersection_type_hints
    ?tco_coeffects
    ?tco_coeffects_local
    ?tco_strict_contexts
    ?tco_like_casts
    ?tco_check_xhp_attribute
    ?tco_check_redundant_generics
    ?tco_disallow_unresolved_type_variables
    ?tco_custom_error_config
    ?po_enable_class_level_where_clauses
    ?po_disable_legacy_soft_typehints
    ?po_allowed_decl_fixme_codes
    ?tco_const_static_props
    ?po_disable_legacy_attribute_syntax
    ?tco_const_attribute
    ?po_const_default_func_args
    ?po_const_default_lambda_args
    ?po_disallow_silence
    ?po_abstract_static_props
    ?po_parser_errors_only
    ?tco_check_attribute_locations
    ?glean_reponame
    ?symbol_write_index_inherited_members
    ?symbol_write_ownership
    ?symbol_write_root_path
    ?symbol_write_hhi_path
    ?symbol_write_ignore_paths
    ?symbol_write_index_paths
    ?symbol_write_index_paths_file
    ?symbol_write_index_paths_file_output
    ?symbol_write_include_hhi
    ?symbol_write_sym_hash_in
    ?symbol_write_exclude_out
    ?symbol_write_referenced_out
    ?symbol_write_reindexed_out
    ?symbol_write_sym_hash_out
    ?po_disallow_func_ptrs_in_constants
    ?tco_error_php_lambdas
    ?tco_disallow_discarded_nullable_awaitables
    ?po_enable_xhp_class_modifier
    ?po_disable_xhp_element_mangling
    ?po_disable_xhp_children_declarations
    ?po_disable_hh_ignore_error
    ?po_keep_user_attributes
    ?po_allow_unstable_features
    ?tco_is_systemlib
    ?tco_higher_kinded_types
    ?tco_method_call_inference
    ?tco_report_pos_from_reason
    ?tco_typecheck_sample_rate
    ?tco_enable_sound_dynamic
    ?tco_pessimise_builtins
    ?tco_enable_no_auto_dynamic
    ?tco_skip_check_under_dynamic
    ?tco_global_access_check_enabled
    ?po_interpret_soft_types_as_like_types
    ?tco_enable_strict_string_concat_interp
    ?tco_ignore_unsafe_cast
    ?tco_no_parser_readonly_check
    ?tco_enable_expression_trees
    ?tco_enable_modules
    ?tco_allowed_expression_tree_visitors
    ?tco_math_new_code
    ?tco_typeconst_concrete_concrete_error
    ?tco_enable_strict_const_semantics
    ?tco_strict_wellformedness
    ?tco_meth_caller_only_public_visibility
    ?tco_require_extends_implements_ancestors
    ?tco_strict_value_equality
    ?tco_enforce_sealed_subclasses
    ?tco_everything_sdt
    ?tco_explicit_consistent_constructors
    ?tco_require_types_class_consts
    ?tco_type_printer_fuel
    ?tco_specify_manifold_api_key
    ?tco_profile_top_level_definitions
    ?tco_allow_all_files_for_module_declarations
    ?tco_allowed_files_for_module_declarations
    ?tco_record_fine_grained_dependencies
    ?tco_loop_iteration_upper_bound
    ?tco_expression_tree_virtualize_functions
    ?tco_use_type_alias_heap
    ?tco_populate_dead_unsafe_cast_heap
    ?po_disallow_static_constants_in_default_func_args
    ?tco_rust_elab
    ?dump_tast_hashes
    ?dump_tasts
    ?tco_autocomplete_mode
    ?tco_package_info
    ?po_unwrap_concurrent
    ?tco_log_exhaustivity_check
    ?po_disallow_direct_superglobals_refs
    ?tco_sticky_quarantine
    ?tco_autocomplete_skip_hierarchy_checks
    options =
  let setting setting option =
    match setting with
    | None -> option
    | Some value -> value
  in
  let setting_opt setting option =
    match setting with
    | None -> option
    | Some _ -> setting
  in
  {
    tco_saved_state = setting tco_saved_state options.tco_saved_state;
    tco_experimental_features =
      setting tco_experimental_features options.tco_experimental_features;
    tco_migration_flags =
      setting tco_migration_flags options.tco_migration_flags;
    tco_num_local_workers =
      setting_opt tco_num_local_workers options.tco_num_local_workers;
    tco_max_typechecker_worker_memory_mb =
      setting_opt
        tco_max_typechecker_worker_memory_mb
        options.tco_max_typechecker_worker_memory_mb;
    tco_defer_class_declaration_threshold =
      setting_opt
        tco_defer_class_declaration_threshold
        options.tco_defer_class_declaration_threshold;
    tco_locl_cache_capacity =
      setting tco_locl_cache_capacity options.tco_locl_cache_capacity;
    tco_locl_cache_node_threshold =
      setting
        tco_locl_cache_node_threshold
        options.tco_locl_cache_node_threshold;
    so_naming_sqlite_path =
      setting_opt so_naming_sqlite_path options.so_naming_sqlite_path;
    po_auto_namespace_map =
      setting po_auto_namespace_map options.po_auto_namespace_map;
    po_codegen = setting po_codegen options.po_codegen;
    code_agnostic_fixme =
      setting code_agnostic_fixme options.code_agnostic_fixme;
    allowed_fixme_codes_strict =
      setting allowed_fixme_codes_strict options.allowed_fixme_codes_strict;
    po_deregister_php_stdlib =
      setting po_deregister_php_stdlib options.po_deregister_php_stdlib;
    po_disallow_toplevel_requires =
      setting
        po_disallow_toplevel_requires
        options.po_disallow_toplevel_requires;
    po_allow_unstable_features =
      setting po_allow_unstable_features options.po_allow_unstable_features;
    tco_log_large_fanouts_threshold =
      setting_opt
        tco_log_large_fanouts_threshold
        options.tco_log_large_fanouts_threshold;
    tco_log_inference_constraints =
      setting
        tco_log_inference_constraints
        options.tco_log_inference_constraints;
    tco_language_feature_logging =
      setting tco_language_feature_logging options.tco_language_feature_logging;
    tco_timeout = setting tco_timeout options.tco_timeout;
    tco_disallow_invalid_arraykey =
      setting
        tco_disallow_invalid_arraykey
        options.tco_disallow_invalid_arraykey;
    tco_disallow_byref_dynamic_calls =
      setting
        tco_disallow_byref_dynamic_calls
        options.tco_disallow_byref_dynamic_calls;
    tco_disallow_byref_calls =
      setting tco_disallow_byref_calls options.tco_disallow_byref_calls;
    log_levels = setting log_levels options.log_levels;
    po_disable_lval_as_an_expression =
      setting
        po_disable_lval_as_an_expression
        options.po_disable_lval_as_an_expression;
    tco_remote_old_decls_no_limit =
      setting
        tco_remote_old_decls_no_limit
        options.tco_remote_old_decls_no_limit;
    tco_use_old_decls_from_cas =
      setting tco_use_old_decls_from_cas options.tco_use_old_decls_from_cas;
    tco_fetch_remote_old_decls =
      setting tco_fetch_remote_old_decls options.tco_fetch_remote_old_decls;
    tco_populate_member_heaps =
      setting tco_populate_member_heaps options.tco_populate_member_heaps;
    tco_skip_hierarchy_checks =
      setting tco_skip_hierarchy_checks options.tco_skip_hierarchy_checks;
    tco_skip_tast_checks =
      setting tco_skip_tast_checks options.tco_skip_tast_checks;
    tco_like_type_hints =
      setting tco_like_type_hints options.tco_like_type_hints;
    tco_union_intersection_type_hints =
      setting
        tco_union_intersection_type_hints
        options.tco_union_intersection_type_hints;
    tco_coeffects = setting tco_coeffects options.tco_coeffects;
    tco_coeffects_local =
      setting tco_coeffects_local options.tco_coeffects_local;
    tco_strict_contexts =
      setting tco_strict_contexts options.tco_strict_contexts;
    tco_like_casts = setting tco_like_casts options.tco_like_casts;
    tco_check_xhp_attribute =
      setting tco_check_xhp_attribute options.tco_check_xhp_attribute;
    tco_check_redundant_generics =
      setting tco_check_redundant_generics options.tco_check_redundant_generics;
    tco_disallow_unresolved_type_variables =
      setting
        tco_disallow_unresolved_type_variables
        options.tco_disallow_unresolved_type_variables;
    tco_custom_error_config =
      setting tco_custom_error_config options.tco_custom_error_config;
    po_enable_class_level_where_clauses =
      setting
        po_enable_class_level_where_clauses
        options.po_enable_class_level_where_clauses;
    po_disable_legacy_soft_typehints =
      setting
        po_disable_legacy_soft_typehints
        options.po_disable_legacy_soft_typehints;
    po_allowed_decl_fixme_codes =
      setting po_allowed_decl_fixme_codes options.po_allowed_decl_fixme_codes;
    tco_const_static_props =
      setting tco_const_static_props options.tco_const_static_props;
    po_disable_legacy_attribute_syntax =
      setting
        po_disable_legacy_attribute_syntax
        options.po_disable_legacy_attribute_syntax;
    tco_const_attribute =
      setting tco_const_attribute options.tco_const_attribute;
    po_const_default_func_args =
      setting po_const_default_func_args options.po_const_default_func_args;
    po_const_default_lambda_args =
      setting po_const_default_lambda_args options.po_const_default_lambda_args;
    po_disallow_silence =
      setting po_disallow_silence options.po_disallow_silence;
    po_abstract_static_props =
      setting po_abstract_static_props options.po_abstract_static_props;
    po_parser_errors_only =
      setting po_parser_errors_only options.po_parser_errors_only;
    tco_check_attribute_locations =
      setting
        tco_check_attribute_locations
        options.tco_check_attribute_locations;
    glean_reponame = setting glean_reponame options.glean_reponame;
    symbol_write_index_inherited_members =
      setting
        symbol_write_index_inherited_members
        options.symbol_write_index_inherited_members;
    symbol_write_ownership =
      setting symbol_write_ownership options.symbol_write_ownership;
    symbol_write_root_path =
      setting symbol_write_root_path options.symbol_write_root_path;
    symbol_write_hhi_path =
      setting symbol_write_hhi_path options.symbol_write_hhi_path;
    symbol_write_ignore_paths =
      setting symbol_write_ignore_paths options.symbol_write_ignore_paths;
    symbol_write_index_paths =
      setting symbol_write_index_paths options.symbol_write_index_paths;
    symbol_write_index_paths_file =
      setting_opt
        symbol_write_index_paths_file
        options.symbol_write_index_paths_file;
    symbol_write_index_paths_file_output =
      setting_opt
        symbol_write_index_paths_file_output
        options.symbol_write_index_paths_file_output;
    symbol_write_include_hhi =
      setting symbol_write_include_hhi options.symbol_write_include_hhi;
    symbol_write_sym_hash_in =
      setting_opt symbol_write_sym_hash_in options.symbol_write_sym_hash_in;
    symbol_write_exclude_out =
      setting_opt symbol_write_exclude_out options.symbol_write_exclude_out;
    symbol_write_referenced_out =
      setting_opt
        symbol_write_referenced_out
        options.symbol_write_referenced_out;
    symbol_write_reindexed_out =
      setting_opt symbol_write_reindexed_out options.symbol_write_reindexed_out;
    symbol_write_sym_hash_out =
      setting symbol_write_sym_hash_out options.symbol_write_sym_hash_out;
    po_disallow_func_ptrs_in_constants =
      setting
        po_disallow_func_ptrs_in_constants
        options.po_disallow_func_ptrs_in_constants;
    tco_error_php_lambdas =
      setting tco_error_php_lambdas options.tco_error_php_lambdas;
    tco_disallow_discarded_nullable_awaitables =
      setting
        tco_disallow_discarded_nullable_awaitables
        options.tco_disallow_discarded_nullable_awaitables;
    po_enable_xhp_class_modifier =
      setting po_enable_xhp_class_modifier options.po_enable_xhp_class_modifier;
    po_disable_xhp_element_mangling =
      setting
        po_disable_xhp_element_mangling
        options.po_disable_xhp_element_mangling;
    po_disable_xhp_children_declarations =
      setting
        po_disable_xhp_children_declarations
        options.po_disable_xhp_children_declarations;
    po_disable_hh_ignore_error =
      setting po_disable_hh_ignore_error options.po_disable_hh_ignore_error;
    po_keep_user_attributes =
      setting po_keep_user_attributes options.po_keep_user_attributes;
    tco_is_systemlib = setting tco_is_systemlib options.tco_is_systemlib;
    tco_higher_kinded_types =
      setting tco_higher_kinded_types options.tco_higher_kinded_types;
    tco_method_call_inference =
      setting tco_method_call_inference options.tco_method_call_inference;
    tco_report_pos_from_reason =
      setting tco_report_pos_from_reason options.tco_report_pos_from_reason;
    tco_typecheck_sample_rate =
      setting tco_typecheck_sample_rate options.tco_typecheck_sample_rate;
    tco_enable_sound_dynamic =
      setting tco_enable_sound_dynamic options.tco_enable_sound_dynamic;
    tco_pessimise_builtins =
      setting tco_pessimise_builtins options.tco_pessimise_builtins;
    tco_enable_no_auto_dynamic =
      setting tco_enable_no_auto_dynamic options.tco_enable_no_auto_dynamic;
    tco_skip_check_under_dynamic =
      setting tco_skip_check_under_dynamic options.tco_skip_check_under_dynamic;
    tco_global_access_check_enabled =
      setting
        tco_global_access_check_enabled
        options.tco_global_access_check_enabled;
    po_interpret_soft_types_as_like_types =
      setting
        po_interpret_soft_types_as_like_types
        options.po_interpret_soft_types_as_like_types;
    tco_enable_strict_string_concat_interp =
      setting
        tco_enable_strict_string_concat_interp
        options.tco_enable_strict_string_concat_interp;
    tco_ignore_unsafe_cast =
      setting tco_ignore_unsafe_cast options.tco_ignore_unsafe_cast;
    tco_no_parser_readonly_check =
      setting tco_no_parser_readonly_check options.tco_no_parser_readonly_check;
    tco_enable_expression_trees =
      setting tco_enable_expression_trees options.tco_enable_expression_trees;
    tco_enable_modules = setting tco_enable_modules options.tco_enable_modules;
    tco_allowed_expression_tree_visitors =
      setting
        tco_allowed_expression_tree_visitors
        options.tco_allowed_expression_tree_visitors;
    tco_math_new_code = setting tco_math_new_code options.tco_math_new_code;
    tco_typeconst_concrete_concrete_error =
      setting
        tco_typeconst_concrete_concrete_error
        options.tco_typeconst_concrete_concrete_error;
    tco_enable_strict_const_semantics =
      setting
        tco_enable_strict_const_semantics
        options.tco_enable_strict_const_semantics;
    tco_strict_wellformedness =
      setting tco_strict_wellformedness options.tco_strict_wellformedness;
    tco_meth_caller_only_public_visibility =
      setting
        tco_meth_caller_only_public_visibility
        options.tco_meth_caller_only_public_visibility;
    tco_require_extends_implements_ancestors =
      setting
        tco_require_extends_implements_ancestors
        options.tco_require_extends_implements_ancestors;
    tco_strict_value_equality =
      setting tco_strict_value_equality options.tco_strict_value_equality;
    tco_enforce_sealed_subclasses =
      setting
        tco_enforce_sealed_subclasses
        options.tco_enforce_sealed_subclasses;
    tco_everything_sdt = setting tco_everything_sdt options.tco_everything_sdt;
    tco_explicit_consistent_constructors =
      setting
        tco_explicit_consistent_constructors
        options.tco_explicit_consistent_constructors;
    tco_require_types_class_consts =
      setting
        tco_require_types_class_consts
        options.tco_require_types_class_consts;
    tco_type_printer_fuel =
      setting tco_type_printer_fuel options.tco_type_printer_fuel;
    tco_specify_manifold_api_key =
      setting tco_specify_manifold_api_key options.tco_specify_manifold_api_key;
    tco_profile_top_level_definitions =
      setting
        tco_profile_top_level_definitions
        options.tco_profile_top_level_definitions;
    tco_allow_all_files_for_module_declarations =
      setting
        tco_allow_all_files_for_module_declarations
        options.tco_allow_all_files_for_module_declarations;
    tco_allowed_files_for_module_declarations =
      setting
        tco_allowed_files_for_module_declarations
        options.tco_allowed_files_for_module_declarations;
    tco_record_fine_grained_dependencies =
      setting
        tco_record_fine_grained_dependencies
        options.tco_record_fine_grained_dependencies;
    tco_loop_iteration_upper_bound =
      setting
        tco_loop_iteration_upper_bound
        options.tco_loop_iteration_upper_bound;
    tco_expression_tree_virtualize_functions =
      setting
        tco_expression_tree_virtualize_functions
        options.tco_expression_tree_virtualize_functions;
    tco_use_type_alias_heap =
      setting tco_use_type_alias_heap options.tco_use_type_alias_heap;
    tco_populate_dead_unsafe_cast_heap =
      setting
        tco_populate_dead_unsafe_cast_heap
        options.tco_populate_dead_unsafe_cast_heap;
    po_disallow_static_constants_in_default_func_args =
      setting
        po_disallow_static_constants_in_default_func_args
        options.po_disallow_static_constants_in_default_func_args;
    tco_rust_elab = setting tco_rust_elab options.tco_rust_elab;
    dump_tast_hashes = setting dump_tast_hashes options.dump_tast_hashes;
    dump_tasts = setting dump_tasts options.dump_tasts;
    tco_autocomplete_mode =
      setting tco_autocomplete_mode options.tco_autocomplete_mode;
    tco_package_info = setting tco_package_info options.tco_package_info;
    po_unwrap_concurrent =
      setting po_unwrap_concurrent options.po_unwrap_concurrent;
    tco_log_exhaustivity_check =
      setting tco_log_exhaustivity_check options.tco_log_exhaustivity_check;
    po_disallow_direct_superglobals_refs =
      setting
        po_disallow_direct_superglobals_refs
        options.po_disallow_direct_superglobals_refs;
    tco_sticky_quarantine =
      setting tco_sticky_quarantine options.tco_sticky_quarantine;
    tco_autocomplete_skip_hierarchy_checks =
      setting
        tco_autocomplete_skip_hierarchy_checks
        options.tco_autocomplete_skip_hierarchy_checks;
  }

let so_naming_sqlite_path t = t.so_naming_sqlite_path

let allowed_fixme_codes_strict t = t.allowed_fixme_codes_strict

let code_agnostic_fixme t = t.code_agnostic_fixme
