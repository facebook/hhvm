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
}
[@@deriving show, eq]

let default_saved_state_loading =
  {
    saved_state_manifold_api_key = None;
    log_saved_state_age_and_distance = false;
    use_manifold_cython_client = false;
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
    project_metadata_w_flags = false;
  }

let with_saved_state_manifold_api_key saved_state_manifold_api_key ss =
  { ss with loading = { ss.loading with saved_state_manifold_api_key } }

let with_use_manifold_cython_client use_manifold_cython_client ss =
  { ss with loading = { ss.loading with use_manifold_cython_client } }

let with_log_saved_state_age_and_distance log_saved_state_age_and_distance ss =
  { ss with loading = { ss.loading with log_saved_state_age_and_distance } }

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
  tco_parallel_type_checking_threshold: int;
  tco_max_typechecker_worker_memory_mb: int option;
  tco_defer_class_declaration_threshold: int option;
  tco_prefetch_deferred_files: bool;
  tco_remote_type_check_threshold: int;
  tco_remote_type_check: bool;
  tco_remote_worker_key: string option;
  tco_remote_check_id: string option;
  tco_num_remote_workers: int;
  so_remote_version_specifier: string option;
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
  tco_simple_pessimize: float;
  tco_check_xhp_attribute: bool;
  tco_check_redundant_generics: bool;
  tco_disallow_unresolved_type_variables: bool;
  po_enable_class_level_where_clauses: bool;
  po_disable_legacy_soft_typehints: bool;
  po_allowed_decl_fixme_codes: ISet.t;
  po_allow_new_attribute_syntax: bool;
  tco_global_inference: bool;
  tco_gi_reinfer_types: string list;
  tco_ordered_solving: bool;
  tco_const_static_props: bool;
  po_disable_legacy_attribute_syntax: bool;
  tco_const_attribute: bool;
  po_const_default_func_args: bool;
  po_const_default_lambda_args: bool;
  po_disallow_silence: bool;
  po_abstract_static_props: bool;
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  glean_service: string;
  glean_hostname: string;
  glean_port: int;
  glean_reponame: string;
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
  symbol_write_sym_hash_out: bool;
  po_disallow_func_ptrs_in_constants: bool;
  tco_error_php_lambdas: bool;
  tco_disallow_discarded_nullable_awaitables: bool;
  po_enable_xhp_class_modifier: bool;
  po_disable_xhp_element_mangling: bool;
  po_disable_xhp_children_declarations: bool;
  po_enable_enum_classes: bool;
  po_disable_hh_ignore_error: int;
  tco_is_systemlib: bool;
  tco_higher_kinded_types: bool;
  tco_method_call_inference: bool;
  tco_report_pos_from_reason: bool;
  tco_typecheck_sample_rate: float;
  tco_enable_sound_dynamic: bool;
  tco_skip_check_under_dynamic: bool;
  tco_ifc_enabled: string list;
  tco_global_access_check_enabled: bool;
  po_enable_enum_supertyping: bool;
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
  tco_pessimise_builtins: bool;
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
  tco_substitution_mutation: bool;
  tco_use_type_alias_heap: bool;
  tco_populate_dead_unsafe_cast_heap: bool;
  po_disallow_static_constants_in_default_func_args: bool;
  tco_load_hack_64_distc_saved_state: bool;
  tco_ide_should_use_hack_64_distc: bool;
  tco_tast_under_dynamic: bool;
  tco_rust_elab: bool;
}
[@@deriving eq, show]

let default =
  {
    tco_saved_state = default_saved_state;
    tco_experimental_features = SSet.empty;
    tco_migration_flags = SSet.empty;
    tco_num_local_workers = None;
    tco_parallel_type_checking_threshold = 10;
    tco_max_typechecker_worker_memory_mb = None;
    tco_defer_class_declaration_threshold = None;
    tco_prefetch_deferred_files = false;
    tco_remote_type_check_threshold = 1_000_000;
    tco_remote_type_check = true;
    tco_remote_worker_key = None;
    tco_remote_check_id = None;
    tco_num_remote_workers = 4;
    so_remote_version_specifier = None;
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
    tco_fetch_remote_old_decls = false;
    tco_populate_member_heaps = true;
    tco_skip_hierarchy_checks = false;
    tco_skip_tast_checks = false;
    tco_like_type_hints = false;
    tco_union_intersection_type_hints = false;
    tco_coeffects = true;
    tco_coeffects_local = true;
    tco_strict_contexts = true;
    tco_like_casts = false;
    tco_simple_pessimize = 0.0;
    tco_check_xhp_attribute = false;
    tco_check_redundant_generics = false;
    tco_disallow_unresolved_type_variables = false;
    po_enable_class_level_where_clauses = false;
    po_disable_legacy_soft_typehints = true;
    po_allowed_decl_fixme_codes = ISet.of_list [];
    po_allow_new_attribute_syntax = false;
    tco_global_inference = false;
    tco_gi_reinfer_types = [];
    tco_ordered_solving = false;
    tco_const_static_props = false;
    po_disable_legacy_attribute_syntax = false;
    tco_const_attribute = false;
    po_const_default_func_args = false;
    po_const_default_lambda_args = false;
    po_disallow_silence = false;
    po_abstract_static_props = false;
    po_parser_errors_only = false;
    tco_check_attribute_locations = true;
    glean_service = "";
    glean_hostname = "";
    glean_port = 0;
    glean_reponame = "www.autocomplete";
    symbol_write_ownership = false;
    symbol_write_root_path = "www";
    symbol_write_hhi_path = "hhi";
    symbol_write_ignore_paths = [];
    symbol_write_index_paths = [];
    symbol_write_index_paths_file = None;
    symbol_write_index_paths_file_output = None;
    symbol_write_include_hhi = true;
    symbol_write_sym_hash_in = None;
    symbol_write_exclude_out = None;
    symbol_write_referenced_out = None;
    symbol_write_sym_hash_out = false;
    po_disallow_func_ptrs_in_constants = false;
    tco_error_php_lambdas = false;
    tco_disallow_discarded_nullable_awaitables = false;
    po_enable_xhp_class_modifier = true;
    po_disable_xhp_element_mangling = true;
    po_disable_xhp_children_declarations = true;
    po_enable_enum_classes = true;
    po_disable_hh_ignore_error = 0;
    tco_is_systemlib = false;
    tco_higher_kinded_types = false;
    tco_method_call_inference = false;
    tco_report_pos_from_reason = false;
    tco_typecheck_sample_rate = 1.0;
    tco_enable_sound_dynamic = false;
    tco_skip_check_under_dynamic = false;
    tco_ifc_enabled = [];
    tco_global_access_check_enabled = false;
    po_enable_enum_supertyping = true;
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
    tco_pessimise_builtins = false;
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
    tco_substitution_mutation = false;
    tco_use_type_alias_heap = false;
    tco_populate_dead_unsafe_cast_heap = false;
    po_disallow_static_constants_in_default_func_args = false;
    tco_load_hack_64_distc_saved_state = false;
    tco_ide_should_use_hack_64_distc = false;
    tco_tast_under_dynamic = false;
    tco_rust_elab = false;
  }

let make
    ~tco_saved_state
    ?(po_deregister_php_stdlib = default.po_deregister_php_stdlib)
    ?(po_disallow_toplevel_requires = default.po_disallow_toplevel_requires)
    ?tco_log_large_fanouts_threshold
    ?(tco_log_inference_constraints = default.tco_log_inference_constraints)
    ?(tco_experimental_features = default.tco_experimental_features)
    ?(tco_migration_flags = default.tco_migration_flags)
    ?tco_num_local_workers
    ?(tco_parallel_type_checking_threshold =
      default.tco_parallel_type_checking_threshold)
    ?tco_max_typechecker_worker_memory_mb
    ?tco_defer_class_declaration_threshold
    ?(tco_prefetch_deferred_files = default.tco_prefetch_deferred_files)
    ?(tco_remote_type_check_threshold = default.tco_remote_type_check_threshold)
    ?(tco_remote_type_check = default.tco_remote_type_check)
    ?tco_remote_worker_key
    ?tco_remote_check_id
    ?(tco_num_remote_workers = default.tco_num_remote_workers)
    ?so_remote_version_specifier
    ?so_naming_sqlite_path
    ?(po_auto_namespace_map = default.po_auto_namespace_map)
    ?(tco_language_feature_logging = default.tco_language_feature_logging)
    ?(tco_timeout = default.tco_timeout)
    ?(tco_disallow_invalid_arraykey = default.tco_disallow_invalid_arraykey)
    ?(tco_disallow_byref_dynamic_calls =
      default.tco_disallow_byref_dynamic_calls)
    ?(tco_disallow_byref_calls = default.tco_disallow_byref_calls)
    ?(code_agnostic_fixme = default.code_agnostic_fixme)
    ?(allowed_fixme_codes_strict = default.allowed_fixme_codes_strict)
    ?(log_levels = default.log_levels)
    ?(po_disable_lval_as_an_expression =
      default.po_disable_lval_as_an_expression)
    ?(tco_remote_old_decls_no_limit = default.tco_remote_old_decls_no_limit)
    ?(tco_fetch_remote_old_decls = default.tco_fetch_remote_old_decls)
    ?(tco_populate_member_heaps = default.tco_populate_member_heaps)
    ?(tco_skip_hierarchy_checks = default.tco_skip_hierarchy_checks)
    ?(tco_skip_tast_checks = default.tco_skip_tast_checks)
    ?(tco_like_type_hints = default.tco_like_type_hints)
    ?(tco_union_intersection_type_hints =
      default.tco_union_intersection_type_hints)
    ?(tco_coeffects = default.tco_coeffects)
    ?(tco_coeffects_local = default.tco_coeffects_local)
    ?(tco_strict_contexts = default.tco_strict_contexts)
    ?(tco_like_casts = default.tco_like_casts)
    ?(tco_simple_pessimize = default.tco_simple_pessimize)
    ?(tco_check_xhp_attribute = default.tco_check_xhp_attribute)
    ?(tco_check_redundant_generics = default.tco_check_redundant_generics)
    ?(tco_disallow_unresolved_type_variables =
      default.tco_disallow_unresolved_type_variables)
    ?(po_enable_class_level_where_clauses =
      default.po_enable_class_level_where_clauses)
    ?(po_disable_legacy_soft_typehints =
      default.po_disable_legacy_soft_typehints)
    ?(po_allowed_decl_fixme_codes = default.po_allowed_decl_fixme_codes)
    ?(po_allow_new_attribute_syntax = default.po_allow_new_attribute_syntax)
    ?(tco_global_inference = default.tco_global_inference)
    ?(tco_gi_reinfer_types = default.tco_gi_reinfer_types)
    ?(tco_ordered_solving = default.tco_ordered_solving)
    ?(tco_const_static_props = default.tco_const_static_props)
    ?(po_disable_legacy_attribute_syntax =
      default.po_disable_legacy_attribute_syntax)
    ?(tco_const_attribute = default.tco_const_attribute)
    ?(po_const_default_func_args = default.po_const_default_func_args)
    ?(po_const_default_lambda_args = default.po_const_default_lambda_args)
    ?(po_disallow_silence = default.po_disallow_silence)
    ?(po_abstract_static_props = default.po_abstract_static_props)
    ?(po_parser_errors_only = default.po_parser_errors_only)
    ?(tco_check_attribute_locations = default.tco_check_attribute_locations)
    ?(glean_service = default.glean_service)
    ?(glean_hostname = default.glean_hostname)
    ?(glean_port = default.glean_port)
    ?(glean_reponame = default.glean_reponame)
    ?(symbol_write_ownership = default.symbol_write_ownership)
    ?(symbol_write_root_path = default.symbol_write_root_path)
    ?(symbol_write_hhi_path = default.symbol_write_hhi_path)
    ?(symbol_write_ignore_paths = default.symbol_write_ignore_paths)
    ?(symbol_write_index_paths = default.symbol_write_index_paths)
    ?symbol_write_index_paths_file
    ?symbol_write_index_paths_file_output
    ?(symbol_write_include_hhi = default.symbol_write_include_hhi)
    ?symbol_write_sym_hash_in
    ?symbol_write_exclude_out
    ?symbol_write_referenced_out
    ?(symbol_write_sym_hash_out = default.symbol_write_sym_hash_out)
    ?(po_disallow_func_ptrs_in_constants =
      default.po_disallow_func_ptrs_in_constants)
    ?(tco_error_php_lambdas = default.tco_error_php_lambdas)
    ?(tco_disallow_discarded_nullable_awaitables =
      default.tco_disallow_discarded_nullable_awaitables)
    ?(po_enable_xhp_class_modifier = default.po_enable_xhp_class_modifier)
    ?(po_disable_xhp_element_mangling = default.po_disable_xhp_element_mangling)
    ?(po_disable_xhp_children_declarations =
      default.po_disable_xhp_children_declarations)
    ?(po_enable_enum_classes = default.po_enable_enum_classes)
    ?(po_disable_hh_ignore_error = default.po_disable_hh_ignore_error)
    ?(po_allow_unstable_features = default.po_allow_unstable_features)
    ?(tco_is_systemlib = default.tco_is_systemlib)
    ?(tco_higher_kinded_types = default.tco_higher_kinded_types)
    ?(tco_method_call_inference = default.tco_method_call_inference)
    ?(tco_report_pos_from_reason = default.tco_report_pos_from_reason)
    ?(tco_typecheck_sample_rate = default.tco_typecheck_sample_rate)
    ?(tco_enable_sound_dynamic = default.tco_enable_sound_dynamic)
    ?(tco_skip_check_under_dynamic = default.tco_skip_check_under_dynamic)
    ?(tco_ifc_enabled = default.tco_ifc_enabled)
    ?(tco_global_access_check_enabled = default.tco_global_access_check_enabled)
    ?(po_enable_enum_supertyping = default.po_enable_enum_supertyping)
    ?(po_interpret_soft_types_as_like_types =
      default.po_interpret_soft_types_as_like_types)
    ?(tco_enable_strict_string_concat_interp =
      default.tco_enable_strict_string_concat_interp)
    ?(tco_ignore_unsafe_cast = default.tco_ignore_unsafe_cast)
    ?(tco_no_parser_readonly_check = default.tco_no_parser_readonly_check)
    ?(tco_enable_expression_trees = default.tco_enable_expression_trees)
    ?(tco_enable_modules = default.tco_enable_modules)
    ?(tco_allowed_expression_tree_visitors =
      default.tco_allowed_expression_tree_visitors)
    ?(tco_math_new_code = default.tco_math_new_code)
    ?(tco_typeconst_concrete_concrete_error =
      default.tco_typeconst_concrete_concrete_error)
    ?(tco_enable_strict_const_semantics =
      default.tco_enable_strict_const_semantics)
    ?(tco_strict_wellformedness = default.tco_strict_wellformedness)
    ?(tco_meth_caller_only_public_visibility =
      default.tco_meth_caller_only_public_visibility)
    ?(tco_require_extends_implements_ancestors =
      default.tco_require_extends_implements_ancestors)
    ?(tco_strict_value_equality = default.tco_strict_value_equality)
    ?(tco_enforce_sealed_subclasses = default.tco_enforce_sealed_subclasses)
    ?(tco_everything_sdt = default.tco_everything_sdt)
    ?(tco_pessimise_builtins = default.tco_pessimise_builtins)
    ?(tco_explicit_consistent_constructors =
      default.tco_explicit_consistent_constructors)
    ?(tco_require_types_class_consts = default.tco_require_types_class_consts)
    ?(tco_type_printer_fuel = default.tco_type_printer_fuel)
    ?(tco_specify_manifold_api_key = default.tco_specify_manifold_api_key)
    ?(tco_profile_top_level_definitions =
      default.tco_profile_top_level_definitions)
    ?(tco_allow_all_files_for_module_declarations =
      default.tco_allow_all_files_for_module_declarations)
    ?(tco_allowed_files_for_module_declarations =
      default.tco_allowed_files_for_module_declarations)
    ?(tco_record_fine_grained_dependencies =
      default.tco_record_fine_grained_dependencies)
    ?(tco_loop_iteration_upper_bound = default.tco_loop_iteration_upper_bound)
    ?(tco_expression_tree_virtualize_functions =
      default.tco_expression_tree_virtualize_functions)
    ?(tco_substitution_mutation = default.tco_substitution_mutation)
    ?(tco_use_type_alias_heap = default.tco_use_type_alias_heap)
    ?(tco_populate_dead_unsafe_cast_heap =
      default.tco_populate_dead_unsafe_cast_heap)
    ?(po_disallow_static_constants_in_default_func_args =
      default.po_disallow_static_constants_in_default_func_args)
    ?(tco_load_hack_64_distc_saved_state =
      default.tco_load_hack_64_distc_saved_state)
    ?(tco_ide_should_use_hack_64_distc =
      default.tco_ide_should_use_hack_64_distc)
    ?(tco_tast_under_dynamic = default.tco_tast_under_dynamic)
    ?(tco_rust_elab = default.tco_rust_elab)
    () =
  {
    tco_saved_state;
    tco_experimental_features;
    tco_migration_flags;
    tco_num_local_workers;
    tco_parallel_type_checking_threshold;
    tco_max_typechecker_worker_memory_mb;
    tco_defer_class_declaration_threshold;
    tco_prefetch_deferred_files;
    tco_remote_type_check_threshold;
    tco_remote_type_check;
    tco_remote_worker_key;
    tco_remote_check_id;
    tco_num_remote_workers;
    so_remote_version_specifier;
    so_naming_sqlite_path;
    po_auto_namespace_map;
    po_codegen = false;
    code_agnostic_fixme;
    allowed_fixme_codes_strict;
    po_deregister_php_stdlib;
    po_disallow_toplevel_requires;
    po_allow_unstable_features;
    tco_log_large_fanouts_threshold;
    tco_log_inference_constraints;
    tco_language_feature_logging;
    tco_timeout;
    tco_disallow_invalid_arraykey;
    tco_disallow_byref_dynamic_calls;
    tco_disallow_byref_calls;
    log_levels;
    po_disable_lval_as_an_expression;
    tco_remote_old_decls_no_limit;
    tco_fetch_remote_old_decls;
    tco_populate_member_heaps;
    tco_skip_hierarchy_checks;
    tco_skip_tast_checks;
    tco_like_type_hints;
    tco_union_intersection_type_hints;
    tco_coeffects;
    tco_coeffects_local;
    tco_strict_contexts;
    tco_like_casts;
    tco_simple_pessimize;
    tco_check_xhp_attribute;
    tco_check_redundant_generics;
    tco_disallow_unresolved_type_variables;
    po_enable_class_level_where_clauses;
    po_disable_legacy_soft_typehints;
    po_allowed_decl_fixme_codes;
    po_allow_new_attribute_syntax;
    tco_global_inference;
    tco_gi_reinfer_types;
    tco_ordered_solving;
    tco_const_static_props;
    po_disable_legacy_attribute_syntax;
    tco_const_attribute;
    po_const_default_func_args;
    po_const_default_lambda_args;
    po_disallow_silence;
    po_abstract_static_props;
    po_parser_errors_only;
    tco_check_attribute_locations;
    glean_service;
    glean_hostname;
    glean_port;
    glean_reponame;
    symbol_write_ownership;
    symbol_write_root_path;
    symbol_write_hhi_path;
    symbol_write_ignore_paths;
    symbol_write_index_paths;
    symbol_write_index_paths_file;
    symbol_write_index_paths_file_output;
    symbol_write_include_hhi;
    symbol_write_sym_hash_in;
    symbol_write_exclude_out;
    symbol_write_referenced_out;
    symbol_write_sym_hash_out;
    po_disallow_func_ptrs_in_constants;
    tco_error_php_lambdas;
    tco_disallow_discarded_nullable_awaitables;
    po_enable_xhp_class_modifier;
    po_disable_xhp_element_mangling;
    po_disable_xhp_children_declarations;
    po_enable_enum_classes;
    po_disable_hh_ignore_error;
    tco_is_systemlib;
    tco_higher_kinded_types;
    tco_method_call_inference;
    tco_report_pos_from_reason;
    tco_typecheck_sample_rate;
    tco_enable_sound_dynamic;
    tco_skip_check_under_dynamic;
    tco_ifc_enabled;
    tco_global_access_check_enabled;
    po_enable_enum_supertyping;
    po_interpret_soft_types_as_like_types;
    tco_enable_strict_string_concat_interp;
    tco_ignore_unsafe_cast;
    tco_no_parser_readonly_check;
    tco_enable_expression_trees;
    tco_enable_modules;
    tco_allowed_expression_tree_visitors;
    tco_math_new_code;
    tco_typeconst_concrete_concrete_error;
    tco_enable_strict_const_semantics;
    tco_strict_wellformedness;
    tco_meth_caller_only_public_visibility;
    tco_require_extends_implements_ancestors;
    tco_strict_value_equality;
    tco_enforce_sealed_subclasses;
    tco_everything_sdt;
    tco_pessimise_builtins;
    tco_explicit_consistent_constructors;
    tco_require_types_class_consts;
    tco_type_printer_fuel;
    tco_specify_manifold_api_key;
    tco_profile_top_level_definitions;
    tco_allow_all_files_for_module_declarations;
    tco_allowed_files_for_module_declarations;
    tco_record_fine_grained_dependencies;
    tco_loop_iteration_upper_bound;
    tco_expression_tree_virtualize_functions;
    tco_substitution_mutation;
    tco_use_type_alias_heap;
    tco_populate_dead_unsafe_cast_heap;
    po_disallow_static_constants_in_default_func_args;
    tco_load_hack_64_distc_saved_state;
    tco_ide_should_use_hack_64_distc;
    tco_tast_under_dynamic;
    tco_rust_elab;
  }

let so_remote_version_specifier t = t.so_remote_version_specifier

let so_naming_sqlite_path t = t.so_naming_sqlite_path

let allowed_fixme_codes_strict t = t.allowed_fixme_codes_strict

let code_agnostic_fixme t = t.code_agnostic_fixme
