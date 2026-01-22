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

type 'a all_or_some =
  | All
  | ASome of 'a list
[@@deriving eq, show]

type 'a none_or_all_except =
  | NNone
  | All_except of 'a list
[@@deriving eq, show]

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

type extended_reasons_config =
  | Extended of int
  | Legacy
  | Debug
[@@deriving eq, show]

(** Naming conventions for fields in this struct:
  - tco_<feature/flag/setting> - type checker option
  - po_<feature/flag/setting> - parser option
  - so_<feature/flag/setting> - server option
*)
type t = {
  po: ParserOptions.t;
  tco_saved_state: saved_state;
  tco_experimental_features: SSet.t;
  tco_migration_flags: SSet.t;
  tco_num_local_workers: int option;
  tco_defer_class_declaration_threshold: int option;
  tco_locl_cache_capacity: int;
  tco_locl_cache_node_threshold: int;
  so_naming_sqlite_path: string option;
  po_disallow_toplevel_requires: bool;
  tco_log_large_fanouts_threshold: int option;
  tco_log_inference_constraints: bool;
  tco_language_feature_logging: bool;
  tco_timeout: int;
  tco_constraint_array_index_assign: bool;
  tco_constraint_method_call: bool;
  code_agnostic_fixme: bool;
  allowed_fixme_codes_strict: ISet.t;
  log_levels: int SMap.t;
  tco_remote_old_decls_no_limit: bool;
  tco_fetch_remote_old_decls: bool;
  tco_populate_member_heaps: bool;
  tco_skip_hierarchy_checks: bool;
  tco_skip_tast_checks: bool;
  tco_coeffects: bool;
  tco_coeffects_local: bool;
  tco_strict_contexts: bool;
  tco_like_casts: bool;
  tco_check_xhp_attribute: bool;
  tco_check_redundant_generics: bool;
  tco_disallow_unresolved_type_variables: bool;
  tco_custom_error_config: Custom_error_config.t;
  tco_const_attribute: bool;
  tco_check_attribute_locations: bool;
  tco_type_refinement_partition_shapes: bool;
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
  tco_error_php_lambdas: bool;
  tco_disallow_discarded_nullable_awaitables: bool;
  tco_typecheck_sample_rate: float;
  tco_pessimise_builtins: bool;
  tco_enable_no_auto_dynamic: bool;
  tco_skip_check_under_dynamic: bool;
  tco_global_access_check_enabled: bool;
  tco_ignore_unsafe_cast: bool;
  tco_enable_expression_trees: bool;
  tco_enable_function_references: bool;
  tco_allowed_expression_tree_visitors: string list;
  tco_typeconst_concrete_concrete_error: bool;
  tco_meth_caller_only_public_visibility: bool;
  tco_require_extends_implements_ancestors: bool;
  tco_strict_value_equality: bool;
  tco_enforce_sealed_subclasses: bool;
  tco_implicit_inherit_sdt: bool;
  tco_repo_stdlib_path: string option;
  tco_explicit_consistent_constructors: int;
  tco_require_types_class_consts: int;
  tco_check_bool_for_condition: int;
  tco_type_printer_fuel: int;
  tco_specify_manifold_api_key: bool;
  tco_profile_top_level_definitions: bool;
  tco_typecheck_if_name_matches_regexp: string option;
  tco_allow_all_files_for_module_declarations: bool;
  tco_allowed_files_for_module_declarations: string list;
  tco_record_fine_grained_dependencies: bool;
  tco_loop_iteration_upper_bound: int option;
  tco_populate_dead_unsafe_cast_heap: bool;
  dump_tast_hashes: bool;
  dump_tasts: string list;
  tco_autocomplete_mode: bool;
  tco_sticky_quarantine: bool;
  tco_lsp_invalidation: bool;
  tco_autocomplete_sort_text: bool;
  tco_extended_reasons: extended_reasons_config option;
  tco_disable_physical_equality: bool;
  hack_warnings: int none_or_all_except;
  warnings_default_all: bool;
  warnings_in_sandcastle: bool;
  warnings_generated_files: string list;
  tco_allowed_files_for_ignore_readonly: string list;
  tco_package_exclude_patterns: string list;
  tco_package_allow_typedef_violations: bool;
  tco_package_allow_classconst_violations: bool;
  tco_package_allow_reifiable_tconst_violations: bool;
  tco_package_allow_all_tconst_violations: bool;
  tco_package_allow_reified_generics_violations: bool;
  tco_package_allow_all_generics_violations: bool;
  tco_package_allow_function_pointers_violations: bool;
  re_no_cache: bool;
  hh_distc_should_disable_trace_store: bool;
  hh_distc_exponential_backoff_num_retries: int;
  tco_enable_abstract_method_optional_parameters: bool;
  recursive_case_types: bool;
  class_sub_classname: bool;
  class_class_type: bool;
  needs_concrete: bool;
  needs_concrete_override_check: bool;
  allow_class_string_cast: bool;
  class_pointer_ban_classname_new: int;
  class_pointer_ban_classname_type_structure: int;
  class_pointer_ban_classname_static_meth: int;
  class_pointer_ban_classname_class_const: int;
  class_pointer_ban_class_array_key: bool;
  tco_poly_function_pointers: bool;
  tco_check_packages: bool;
  fanout_strip_class_location: bool;
  tco_package_config_disable_transitivity_check: bool;
  tco_allow_require_package_on_interface_methods: bool;
}
[@@deriving eq, show]

let default =
  {
    po = ParserOptions.default;
    tco_saved_state = default_saved_state;
    tco_experimental_features = SSet.empty;
    tco_migration_flags = SSet.empty;
    tco_num_local_workers = None;
    tco_defer_class_declaration_threshold = None;
    tco_locl_cache_capacity = 30;
    tco_locl_cache_node_threshold = 10_000;
    so_naming_sqlite_path = None;
    po_disallow_toplevel_requires = false;
    tco_log_large_fanouts_threshold = None;
    tco_log_inference_constraints = false;
    tco_language_feature_logging = false;
    tco_timeout = 0;
    tco_constraint_array_index_assign = false;
    tco_constraint_method_call = false;
    code_agnostic_fixme = false;
    allowed_fixme_codes_strict = ISet.empty;
    log_levels = SMap.empty;
    tco_remote_old_decls_no_limit = false;
    tco_fetch_remote_old_decls = true;
    tco_populate_member_heaps = true;
    tco_skip_hierarchy_checks = false;
    tco_skip_tast_checks = false;
    tco_coeffects = true;
    tco_coeffects_local = true;
    tco_strict_contexts = true;
    tco_like_casts = false;
    tco_check_xhp_attribute = false;
    tco_check_redundant_generics = false;
    tco_disallow_unresolved_type_variables = false;
    tco_custom_error_config = Custom_error_config.empty;
    tco_const_attribute = false;
    tco_check_attribute_locations = true;
    tco_type_refinement_partition_shapes = false;
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
    tco_error_php_lambdas = false;
    tco_disallow_discarded_nullable_awaitables = false;
    tco_typecheck_sample_rate = 1.0;
    tco_pessimise_builtins = false;
    tco_enable_no_auto_dynamic = false;
    tco_skip_check_under_dynamic = false;
    tco_global_access_check_enabled = false;
    tco_ignore_unsafe_cast = false;
    tco_enable_expression_trees = false;
    tco_enable_function_references = true;
    tco_allowed_expression_tree_visitors = [];
    tco_typeconst_concrete_concrete_error = false;
    tco_meth_caller_only_public_visibility = true;
    tco_require_extends_implements_ancestors = false;
    tco_strict_value_equality = false;
    tco_enforce_sealed_subclasses = false;
    tco_implicit_inherit_sdt = false;
    tco_repo_stdlib_path = None;
    tco_explicit_consistent_constructors = 0;
    tco_require_types_class_consts = 0;
    tco_check_bool_for_condition = 0;
    tco_type_printer_fuel = 100;
    tco_specify_manifold_api_key = false;
    tco_profile_top_level_definitions = false;
    tco_typecheck_if_name_matches_regexp = None;
    tco_allow_all_files_for_module_declarations = true;
    tco_allowed_files_for_module_declarations = [];
    tco_record_fine_grained_dependencies = false;
    tco_loop_iteration_upper_bound = None;
    tco_populate_dead_unsafe_cast_heap = false;
    dump_tast_hashes = false;
    dump_tasts = [];
    tco_autocomplete_mode = false;
    tco_sticky_quarantine = false;
    tco_lsp_invalidation = false;
    tco_autocomplete_sort_text = false;
    tco_extended_reasons = None;
    tco_disable_physical_equality = false;
    hack_warnings = All_except [];
    warnings_default_all = false;
    warnings_in_sandcastle = true;
    warnings_generated_files = [];
    tco_allowed_files_for_ignore_readonly = [];
    tco_package_exclude_patterns =
      [{|.*/__tests__/.*|}; {|.*/flib/intern/makehaste/.*|}];
    tco_package_allow_typedef_violations = true;
    tco_package_allow_classconst_violations = true;
    tco_package_allow_reifiable_tconst_violations = true;
    tco_package_allow_all_tconst_violations = true;
    tco_package_allow_reified_generics_violations = true;
    tco_package_allow_all_generics_violations = true;
    tco_package_allow_function_pointers_violations = true;
    re_no_cache = false;
    hh_distc_should_disable_trace_store = false;
    hh_distc_exponential_backoff_num_retries = 10;
    tco_enable_abstract_method_optional_parameters = false;
    recursive_case_types = false;
    class_sub_classname = true;
    class_class_type = true;
    needs_concrete = false;
    needs_concrete_override_check = false;
    allow_class_string_cast = true;
    class_pointer_ban_classname_new = 0;
    class_pointer_ban_classname_type_structure = 0;
    class_pointer_ban_classname_static_meth = 0;
    class_pointer_ban_classname_class_const = 0;
    class_pointer_ban_class_array_key = false;
    tco_poly_function_pointers = true;
    tco_check_packages = true;
    fanout_strip_class_location = false;
    tco_package_config_disable_transitivity_check = false;
    tco_allow_require_package_on_interface_methods = true;
  }

let set
    ?po
    ?tco_saved_state
    ?po_disallow_toplevel_requires
    ?tco_log_large_fanouts_threshold
    ?tco_log_inference_constraints
    ?tco_experimental_features
    ?tco_migration_flags
    ?tco_num_local_workers
    ?tco_defer_class_declaration_threshold
    ?tco_locl_cache_capacity
    ?tco_locl_cache_node_threshold
    ?so_naming_sqlite_path
    ?tco_language_feature_logging
    ?tco_timeout
    ?tco_constraint_array_index_assign
    ?tco_constraint_method_call
    ?code_agnostic_fixme
    ?allowed_fixme_codes_strict
    ?log_levels
    ?tco_remote_old_decls_no_limit
    ?tco_fetch_remote_old_decls
    ?tco_populate_member_heaps
    ?tco_skip_hierarchy_checks
    ?tco_skip_tast_checks
    ?tco_coeffects
    ?tco_coeffects_local
    ?tco_strict_contexts
    ?tco_like_casts
    ?tco_check_xhp_attribute
    ?tco_check_redundant_generics
    ?tco_disallow_unresolved_type_variables
    ?tco_custom_error_config
    ?tco_const_attribute
    ?tco_check_attribute_locations
    ?tco_type_refinement_partition_shapes
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
    ?tco_error_php_lambdas
    ?tco_disallow_discarded_nullable_awaitables
    ?tco_typecheck_sample_rate
    ?tco_pessimise_builtins
    ?tco_enable_no_auto_dynamic
    ?tco_skip_check_under_dynamic
    ?tco_global_access_check_enabled
    ?tco_ignore_unsafe_cast
    ?tco_enable_expression_trees
    ?tco_enable_function_references
    ?tco_allowed_expression_tree_visitors
    ?tco_typeconst_concrete_concrete_error
    ?tco_meth_caller_only_public_visibility
    ?tco_require_extends_implements_ancestors
    ?tco_strict_value_equality
    ?tco_enforce_sealed_subclasses
    ?tco_implicit_inherit_sdt
    ?tco_repo_stdlib_path
    ?tco_explicit_consistent_constructors
    ?tco_require_types_class_consts
    ?tco_check_bool_for_condition
    ?tco_type_printer_fuel
    ?tco_specify_manifold_api_key
    ?tco_profile_top_level_definitions
    ?tco_typecheck_if_name_matches_regexp
    ?tco_allow_all_files_for_module_declarations
    ?tco_allowed_files_for_module_declarations
    ?tco_record_fine_grained_dependencies
    ?tco_loop_iteration_upper_bound
    ?tco_populate_dead_unsafe_cast_heap
    ?dump_tast_hashes
    ?dump_tasts
    ?tco_autocomplete_mode
    ?tco_sticky_quarantine
    ?tco_lsp_invalidation
    ?tco_autocomplete_sort_text
    ?tco_extended_reasons
    ?tco_disable_physical_equality
    ?hack_warnings
    ?warnings_default_all
    ?warnings_in_sandcastle
    ?warnings_generated_files
    ?tco_allowed_files_for_ignore_readonly
    ?tco_package_exclude_patterns
    ?tco_package_allow_typedef_violations
    ?tco_package_allow_classconst_violations
    ?tco_package_allow_reifiable_tconst_violations
    ?tco_package_allow_all_tconst_violations
    ?tco_package_allow_reified_generics_violations
    ?tco_package_allow_all_generics_violations
    ?tco_package_allow_function_pointers_violations
    ?re_no_cache
    ?hh_distc_should_disable_trace_store
    ?hh_distc_exponential_backoff_num_retries
    ?tco_enable_abstract_method_optional_parameters
    ?recursive_case_types
    ?class_sub_classname
    ?class_class_type
    ?needs_concrete
    ?needs_concrete_override_check
    ?allow_class_string_cast
    ?class_pointer_ban_classname_new
    ?class_pointer_ban_classname_type_structure
    ?class_pointer_ban_classname_static_meth
    ?class_pointer_ban_classname_class_const
    ?class_pointer_ban_class_array_key
    ?tco_poly_function_pointers
    ?tco_check_packages
    ?fanout_strip_class_location
    ?tco_package_config_disable_transitivity_check
    ?tco_allow_require_package_on_interface_methods
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
    po = setting po options.po;
    tco_saved_state = setting tco_saved_state options.tco_saved_state;
    tco_experimental_features =
      setting tco_experimental_features options.tco_experimental_features;
    tco_migration_flags =
      setting tco_migration_flags options.tco_migration_flags;
    tco_num_local_workers =
      setting_opt tco_num_local_workers options.tco_num_local_workers;
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
    code_agnostic_fixme =
      setting code_agnostic_fixme options.code_agnostic_fixme;
    allowed_fixme_codes_strict =
      setting allowed_fixme_codes_strict options.allowed_fixme_codes_strict;
    po_disallow_toplevel_requires =
      setting
        po_disallow_toplevel_requires
        options.po_disallow_toplevel_requires;
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
    tco_constraint_array_index_assign =
      setting
        tco_constraint_array_index_assign
        options.tco_constraint_array_index_assign;
    tco_constraint_method_call =
      setting tco_constraint_method_call options.tco_constraint_method_call;
    log_levels = setting log_levels options.log_levels;
    tco_remote_old_decls_no_limit =
      setting
        tco_remote_old_decls_no_limit
        options.tco_remote_old_decls_no_limit;
    tco_fetch_remote_old_decls =
      setting tco_fetch_remote_old_decls options.tco_fetch_remote_old_decls;
    tco_populate_member_heaps =
      setting tco_populate_member_heaps options.tco_populate_member_heaps;
    tco_skip_hierarchy_checks =
      setting tco_skip_hierarchy_checks options.tco_skip_hierarchy_checks;
    tco_skip_tast_checks =
      setting tco_skip_tast_checks options.tco_skip_tast_checks;
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
    tco_const_attribute =
      setting tco_const_attribute options.tco_const_attribute;
    tco_check_attribute_locations =
      setting
        tco_check_attribute_locations
        options.tco_check_attribute_locations;
    tco_type_refinement_partition_shapes =
      setting
        tco_type_refinement_partition_shapes
        options.tco_type_refinement_partition_shapes;
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
    tco_error_php_lambdas =
      setting tco_error_php_lambdas options.tco_error_php_lambdas;
    tco_disallow_discarded_nullable_awaitables =
      setting
        tco_disallow_discarded_nullable_awaitables
        options.tco_disallow_discarded_nullable_awaitables;
    tco_typecheck_sample_rate =
      setting tco_typecheck_sample_rate options.tco_typecheck_sample_rate;
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
    tco_ignore_unsafe_cast =
      setting tco_ignore_unsafe_cast options.tco_ignore_unsafe_cast;
    tco_enable_expression_trees =
      setting tco_enable_expression_trees options.tco_enable_expression_trees;
    tco_enable_function_references =
      setting
        tco_enable_function_references
        options.tco_enable_function_references;
    tco_allowed_expression_tree_visitors =
      setting
        tco_allowed_expression_tree_visitors
        options.tco_allowed_expression_tree_visitors;
    tco_typeconst_concrete_concrete_error =
      setting
        tco_typeconst_concrete_concrete_error
        options.tco_typeconst_concrete_concrete_error;
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
    tco_implicit_inherit_sdt =
      setting tco_implicit_inherit_sdt options.tco_implicit_inherit_sdt;
    tco_repo_stdlib_path =
      setting_opt tco_repo_stdlib_path options.tco_repo_stdlib_path;
    tco_explicit_consistent_constructors =
      setting
        tco_explicit_consistent_constructors
        options.tco_explicit_consistent_constructors;
    tco_require_types_class_consts =
      setting
        tco_require_types_class_consts
        options.tco_require_types_class_consts;
    tco_check_bool_for_condition =
      setting tco_check_bool_for_condition options.tco_check_bool_for_condition;
    tco_type_printer_fuel =
      setting tco_type_printer_fuel options.tco_type_printer_fuel;
    tco_specify_manifold_api_key =
      setting tco_specify_manifold_api_key options.tco_specify_manifold_api_key;
    tco_profile_top_level_definitions =
      setting
        tco_profile_top_level_definitions
        options.tco_profile_top_level_definitions;
    tco_typecheck_if_name_matches_regexp =
      setting_opt
        tco_typecheck_if_name_matches_regexp
        options.tco_typecheck_if_name_matches_regexp;
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
    tco_populate_dead_unsafe_cast_heap =
      setting
        tco_populate_dead_unsafe_cast_heap
        options.tco_populate_dead_unsafe_cast_heap;
    dump_tast_hashes = setting dump_tast_hashes options.dump_tast_hashes;
    dump_tasts = setting dump_tasts options.dump_tasts;
    tco_autocomplete_mode =
      setting tco_autocomplete_mode options.tco_autocomplete_mode;
    tco_sticky_quarantine =
      setting tco_sticky_quarantine options.tco_sticky_quarantine;
    tco_lsp_invalidation =
      setting tco_lsp_invalidation options.tco_lsp_invalidation;
    tco_autocomplete_sort_text =
      setting tco_autocomplete_sort_text options.tco_autocomplete_sort_text;
    tco_extended_reasons =
      setting_opt tco_extended_reasons options.tco_extended_reasons;
    tco_disable_physical_equality =
      setting
        tco_disable_physical_equality
        options.tco_disable_physical_equality;
    hack_warnings = setting hack_warnings options.hack_warnings;
    warnings_default_all =
      setting warnings_default_all options.warnings_default_all;
    warnings_in_sandcastle =
      setting warnings_in_sandcastle options.warnings_in_sandcastle;
    warnings_generated_files =
      setting warnings_generated_files options.warnings_generated_files;
    tco_allowed_files_for_ignore_readonly =
      setting
        tco_allowed_files_for_ignore_readonly
        options.tco_allowed_files_for_ignore_readonly;
    tco_package_exclude_patterns =
      setting tco_package_exclude_patterns options.tco_package_exclude_patterns;
    tco_package_allow_typedef_violations =
      setting
        tco_package_allow_typedef_violations
        options.tco_package_allow_typedef_violations;
    tco_package_allow_classconst_violations =
      setting
        tco_package_allow_classconst_violations
        options.tco_package_allow_classconst_violations;
    tco_package_allow_reifiable_tconst_violations =
      setting
        tco_package_allow_reifiable_tconst_violations
        options.tco_package_allow_reifiable_tconst_violations;
    tco_package_allow_all_tconst_violations =
      setting
        tco_package_allow_all_tconst_violations
        options.tco_package_allow_all_tconst_violations;
    tco_package_allow_reified_generics_violations =
      setting
        tco_package_allow_reified_generics_violations
        options.tco_package_allow_reified_generics_violations;
    tco_package_allow_all_generics_violations =
      setting
        tco_package_allow_all_generics_violations
        options.tco_package_allow_all_generics_violations;
    tco_package_allow_function_pointers_violations =
      setting
        tco_package_allow_function_pointers_violations
        options.tco_package_allow_function_pointers_violations;
    re_no_cache = setting re_no_cache options.re_no_cache;
    hh_distc_should_disable_trace_store =
      setting
        hh_distc_should_disable_trace_store
        options.hh_distc_should_disable_trace_store;
    hh_distc_exponential_backoff_num_retries =
      setting
        hh_distc_exponential_backoff_num_retries
        options.hh_distc_exponential_backoff_num_retries;
    tco_enable_abstract_method_optional_parameters =
      setting
        tco_enable_abstract_method_optional_parameters
        options.tco_enable_abstract_method_optional_parameters;
    recursive_case_types =
      setting recursive_case_types options.recursive_case_types;
    class_sub_classname =
      setting class_sub_classname options.class_sub_classname;
    class_class_type = setting class_class_type options.class_class_type;
    needs_concrete = setting needs_concrete options.needs_concrete;
    needs_concrete_override_check =
      setting
        needs_concrete_override_check
        options.needs_concrete_override_check;
    allow_class_string_cast =
      setting allow_class_string_cast options.allow_class_string_cast;
    class_pointer_ban_classname_new =
      setting
        class_pointer_ban_classname_new
        options.class_pointer_ban_classname_new;
    class_pointer_ban_classname_type_structure =
      setting
        class_pointer_ban_classname_type_structure
        options.class_pointer_ban_classname_type_structure;
    class_pointer_ban_classname_static_meth =
      setting
        class_pointer_ban_classname_static_meth
        options.class_pointer_ban_classname_static_meth;
    class_pointer_ban_classname_class_const =
      setting
        class_pointer_ban_classname_class_const
        options.class_pointer_ban_classname_class_const;
    class_pointer_ban_class_array_key =
      setting
        class_pointer_ban_class_array_key
        options.class_pointer_ban_class_array_key;
    tco_poly_function_pointers =
      setting tco_poly_function_pointers options.tco_poly_function_pointers;
    tco_check_packages = setting tco_check_packages options.tco_check_packages;
    fanout_strip_class_location =
      setting fanout_strip_class_location options.fanout_strip_class_location;
    tco_package_config_disable_transitivity_check =
      setting
        tco_package_config_disable_transitivity_check
        options.tco_package_config_disable_transitivity_check;
    tco_allow_require_package_on_interface_methods =
      setting
        tco_allow_require_package_on_interface_methods
        options.tco_allow_require_package_on_interface_methods;
  }

let so_naming_sqlite_path t = t.so_naming_sqlite_path

let allowed_fixme_codes_strict t = t.allowed_fixme_codes_strict

let code_agnostic_fixme t = t.code_agnostic_fixme
