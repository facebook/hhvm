(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
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
  tco_remote_max_batch_size: int;
  tco_remote_min_batch_size: int;
  tco_num_remote_workers: int;
  so_remote_version_specifier: string option;
  so_remote_worker_vfs_checkout_threshold: int;
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
  allowed_fixme_codes_strict: ISet.t;
  allowed_fixme_codes_partial: ISet.t;
  codes_not_raised_partial: ISet.t;
  log_levels: int SMap.t;
  po_disable_lval_as_an_expression: bool;
  tco_shallow_class_decl: bool;
  tco_force_shallow_decl_fanout: bool;
  tco_remote_old_decls_no_limit: bool;
  tco_fetch_remote_old_decls: bool;
  tco_force_load_hot_shallow_decls: bool;
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
  tco_complex_coercion: bool;
  error_codes_treated_strictly: ISet.t;
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
  po_disallow_func_ptrs_in_constants: bool;
  tco_error_php_lambdas: bool;
  tco_disallow_discarded_nullable_awaitables: bool;
  po_enable_xhp_class_modifier: bool;
  po_disable_xhp_element_mangling: bool;
  po_disable_xhp_children_declarations: bool;
  po_enable_enum_classes: bool;
  po_disable_hh_ignore_error: bool;
  tco_enable_systemlib_annotations: bool;
  tco_higher_kinded_types: bool;
  tco_method_call_inference: bool;
  tco_report_pos_from_reason: bool;
  tco_typecheck_sample_rate: float;
  tco_enable_sound_dynamic: bool;
  po_disallow_fun_and_cls_meth_pseudo_funcs: bool;
  po_disallow_inst_meth: bool;
  tco_use_direct_decl_parser: bool;
  tco_ifc_enabled: string list;
  tco_global_write_check_enabled: string list;
  tco_global_write_check_functions_enabled: SSet.t;
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
  tco_enable_disk_heap: bool;
  tco_explicit_consistent_constructors: int;
  tco_require_types_class_consts: int;
  tco_type_printer_fuel: int;
  tco_log_saved_state_age_and_distance: bool;
  tco_specify_manifold_api_key: bool;
  tco_saved_state_manifold_api_key: string option;
  tco_profile_top_level_definitions: bool;
  tco_allow_all_files_for_module_declarations: bool;
  tco_allowed_files_for_module_declarations: string list;
  tco_use_manifold_cython_client: bool;
  tco_record_fine_grained_dependencies: bool;
  tco_loop_iteration_upper_bound: int option;
  tco_expression_tree_virtualize_functions: bool;
}
[@@deriving eq, show]

(**
 * Insist on instantiations for all generic types, even in non-strict files
 *)
let tco_experimental_generics_arity = "generics_arity"

(**
 * Forbid casting nullable values, since they have unexpected semantics. For
 * example, casting `null` to an int results in `0`, which may or may not be
 * what you were expecting.
 *)
let tco_experimental_forbid_nullable_cast = "forbid_nullable_cast"

(*
* Disallow static memoized functions in non-final classes
*)

let tco_experimental_disallow_static_memoized = "disallow_static_memoized"

(**
 * Enable abstract const type with default syntax, i.e.
 * abstract const type T as num = int;
 *)
let tco_experimental_abstract_type_const_with_default =
  "abstract_type_const_with_default"

(*
* Allow typechecker to do global inference and infer IFC flows
* with the <<InferFlows>> flag
*
*)
let tco_experimental_infer_flows = "ifc_infer_flows"

let tco_experimental_supportdynamic_type_hint = "supportdynamic_type_hint"

let tco_experimental_all =
  List.fold_right
    ~f:SSet.add
    ~init:SSet.empty
    [
      tco_experimental_generics_arity;
      tco_experimental_forbid_nullable_cast;
      tco_experimental_disallow_static_memoized;
      tco_experimental_abstract_type_const_with_default;
      tco_experimental_infer_flows;
      tco_experimental_supportdynamic_type_hint;
    ]

let tco_migration_flags_all = List.fold_right ~init:SSet.empty ~f:SSet.add []

let default =
  {
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
    tco_remote_max_batch_size = 8_000;
    tco_remote_min_batch_size = 5_000;
    tco_num_remote_workers = 4;
    so_remote_version_specifier = None;
    so_remote_worker_vfs_checkout_threshold = 10000;
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
    allowed_fixme_codes_strict = ISet.empty;
    allowed_fixme_codes_partial = ISet.empty;
    codes_not_raised_partial = ISet.empty;
    log_levels = SMap.empty;
    po_disable_lval_as_an_expression = true;
    tco_shallow_class_decl = false;
    tco_force_shallow_decl_fanout = false;
    tco_remote_old_decls_no_limit = false;
    tco_fetch_remote_old_decls = false;
    tco_force_load_hot_shallow_decls = false;
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
    tco_complex_coercion = false;
    error_codes_treated_strictly = ISet.of_list [];
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
    po_disallow_func_ptrs_in_constants = false;
    tco_error_php_lambdas = false;
    tco_disallow_discarded_nullable_awaitables = false;
    po_enable_xhp_class_modifier = true;
    po_disable_xhp_element_mangling = true;
    po_disable_xhp_children_declarations = true;
    po_enable_enum_classes = true;
    po_disable_hh_ignore_error = false;
    tco_enable_systemlib_annotations = false;
    tco_higher_kinded_types = false;
    tco_method_call_inference = false;
    tco_report_pos_from_reason = false;
    tco_typecheck_sample_rate = 1.0;
    tco_enable_sound_dynamic = false;
    po_disallow_fun_and_cls_meth_pseudo_funcs = false;
    po_disallow_inst_meth = false;
    tco_use_direct_decl_parser = true;
    tco_ifc_enabled = [];
    tco_global_write_check_enabled = [];
    tco_global_write_check_functions_enabled = SSet.empty;
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
    tco_enable_disk_heap = true;
    tco_explicit_consistent_constructors = 0;
    tco_require_types_class_consts = 0;
    tco_type_printer_fuel = 100;
    tco_log_saved_state_age_and_distance = false;
    tco_specify_manifold_api_key = false;
    tco_saved_state_manifold_api_key = None;
    tco_profile_top_level_definitions = false;
    tco_allow_all_files_for_module_declarations = false;
    tco_allowed_files_for_module_declarations = [];
    tco_use_manifold_cython_client = false;
    tco_record_fine_grained_dependencies = false;
    tco_loop_iteration_upper_bound = None;
    tco_expression_tree_virtualize_functions = false;
  }

let make
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
    ?(tco_remote_max_batch_size = default.tco_remote_max_batch_size)
    ?(tco_remote_min_batch_size = default.tco_remote_min_batch_size)
    ?(tco_num_remote_workers = default.tco_num_remote_workers)
    ?so_remote_version_specifier
    ?(so_remote_worker_vfs_checkout_threshold =
      default.so_remote_worker_vfs_checkout_threshold)
    ?so_naming_sqlite_path
    ?(po_auto_namespace_map = default.po_auto_namespace_map)
    ?(tco_language_feature_logging = default.tco_language_feature_logging)
    ?(tco_timeout = default.tco_timeout)
    ?(tco_disallow_invalid_arraykey = default.tco_disallow_invalid_arraykey)
    ?(tco_disallow_byref_dynamic_calls =
      default.tco_disallow_byref_dynamic_calls)
    ?(tco_disallow_byref_calls = default.tco_disallow_byref_calls)
    ?(allowed_fixme_codes_strict = default.allowed_fixme_codes_strict)
    ?(allowed_fixme_codes_partial = default.allowed_fixme_codes_partial)
    ?(codes_not_raised_partial = default.codes_not_raised_partial)
    ?(log_levels = default.log_levels)
    ?(po_disable_lval_as_an_expression =
      default.po_disable_lval_as_an_expression)
    ?(tco_shallow_class_decl = default.tco_shallow_class_decl)
    ?(tco_force_shallow_decl_fanout = default.tco_force_shallow_decl_fanout)
    ?(tco_remote_old_decls_no_limit = default.tco_remote_old_decls_no_limit)
    ?(tco_fetch_remote_old_decls = default.tco_fetch_remote_old_decls)
    ?(tco_force_load_hot_shallow_decls =
      default.tco_force_load_hot_shallow_decls)
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
    ?(tco_complex_coercion = default.tco_complex_coercion)
    ?(error_codes_treated_strictly = default.error_codes_treated_strictly)
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
    ?(tco_enable_systemlib_annotations =
      default.tco_enable_systemlib_annotations)
    ?(tco_higher_kinded_types = default.tco_higher_kinded_types)
    ?(tco_method_call_inference = default.tco_method_call_inference)
    ?(tco_report_pos_from_reason = default.tco_report_pos_from_reason)
    ?(tco_typecheck_sample_rate = default.tco_typecheck_sample_rate)
    ?(tco_enable_sound_dynamic = default.tco_enable_sound_dynamic)
    ?(po_disallow_fun_and_cls_meth_pseudo_funcs =
      default.po_disallow_fun_and_cls_meth_pseudo_funcs)
    ?(po_disallow_inst_meth = default.po_disallow_inst_meth)
    ?(tco_use_direct_decl_parser = default.tco_use_direct_decl_parser)
    ?(tco_ifc_enabled = default.tco_ifc_enabled)
    ?(tco_global_write_check_enabled = default.tco_global_write_check_enabled)
    ?(tco_global_write_check_functions_enabled =
      default.tco_global_write_check_functions_enabled)
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
    ?(tco_enable_disk_heap = default.tco_enable_disk_heap)
    ?(tco_explicit_consistent_constructors =
      default.tco_explicit_consistent_constructors)
    ?(tco_require_types_class_consts = default.tco_require_types_class_consts)
    ?(tco_type_printer_fuel = default.tco_type_printer_fuel)
    ?(tco_log_saved_state_age_and_distance =
      default.tco_log_saved_state_age_and_distance)
    ?(tco_specify_manifold_api_key = default.tco_specify_manifold_api_key)
    ?(tco_saved_state_manifold_api_key =
      default.tco_saved_state_manifold_api_key)
    ?(tco_profile_top_level_definitions =
      default.tco_profile_top_level_definitions)
    ?(tco_allow_all_files_for_module_declarations =
      default.tco_allow_all_files_for_module_declarations)
    ?(tco_allowed_files_for_module_declarations =
      default.tco_allowed_files_for_module_declarations)
    ?(tco_use_manifold_cython_client = default.tco_use_manifold_cython_client)
    ?(tco_record_fine_grained_dependencies =
      default.tco_record_fine_grained_dependencies)
    ?(tco_loop_iteration_upper_bound = default.tco_loop_iteration_upper_bound)
    ?(tco_expression_tree_virtualize_functions =
      default.tco_expression_tree_virtualize_functions)
    () =
  {
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
    tco_remote_max_batch_size;
    tco_remote_min_batch_size;
    tco_num_remote_workers;
    so_remote_version_specifier;
    so_remote_worker_vfs_checkout_threshold;
    so_naming_sqlite_path;
    po_auto_namespace_map;
    po_codegen = false;
    allowed_fixme_codes_strict;
    allowed_fixme_codes_partial;
    codes_not_raised_partial;
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
    tco_shallow_class_decl;
    tco_force_shallow_decl_fanout;
    tco_remote_old_decls_no_limit;
    tco_fetch_remote_old_decls;
    tco_force_load_hot_shallow_decls;
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
    tco_complex_coercion;
    error_codes_treated_strictly;
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
    po_disallow_func_ptrs_in_constants;
    tco_error_php_lambdas;
    tco_disallow_discarded_nullable_awaitables;
    po_enable_xhp_class_modifier;
    po_disable_xhp_element_mangling;
    po_disable_xhp_children_declarations;
    po_enable_enum_classes;
    po_disable_hh_ignore_error;
    tco_enable_systemlib_annotations;
    tco_higher_kinded_types;
    tco_method_call_inference;
    tco_report_pos_from_reason;
    tco_typecheck_sample_rate;
    tco_enable_sound_dynamic;
    po_disallow_fun_and_cls_meth_pseudo_funcs;
    po_disallow_inst_meth;
    tco_use_direct_decl_parser;
    tco_ifc_enabled;
    tco_global_write_check_enabled;
    tco_global_write_check_functions_enabled;
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
    tco_enable_disk_heap;
    tco_explicit_consistent_constructors;
    tco_require_types_class_consts;
    tco_type_printer_fuel;
    tco_log_saved_state_age_and_distance;
    tco_specify_manifold_api_key;
    tco_saved_state_manifold_api_key;
    tco_profile_top_level_definitions;
    tco_allow_all_files_for_module_declarations;
    tco_allowed_files_for_module_declarations;
    tco_use_manifold_cython_client;
    tco_record_fine_grained_dependencies;
    tco_loop_iteration_upper_bound;
    tco_expression_tree_virtualize_functions;
  }

let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features

let tco_migration_flag_enabled t s = SSet.mem s t.tco_migration_flags

let tco_num_local_workers t = t.tco_num_local_workers

let tco_parallel_type_checking_threshold t =
  t.tco_parallel_type_checking_threshold

let tco_max_typechecker_worker_memory_mb t =
  t.tco_max_typechecker_worker_memory_mb

let tco_defer_class_declaration_threshold t =
  t.tco_defer_class_declaration_threshold

let tco_prefetch_deferred_files t = t.tco_prefetch_deferred_files

let tco_remote_type_check_threshold t = t.tco_remote_type_check_threshold

let tco_remote_type_check t = t.tco_remote_type_check

let tco_remote_worker_key t = t.tco_remote_worker_key

let tco_remote_check_id t = t.tco_remote_check_id

let tco_remote_max_batch_size t = t.tco_remote_max_batch_size

let tco_remote_min_batch_size t = t.tco_remote_min_batch_size

let tco_num_remote_workers t = t.tco_num_remote_workers

let so_remote_version_specifier t = t.so_remote_version_specifier

let so_remote_worker_vfs_checkout_threshold t =
  t.so_remote_worker_vfs_checkout_threshold

let so_naming_sqlite_path t = t.so_naming_sqlite_path

let po_auto_namespace_map t = t.po_auto_namespace_map

let po_deregister_php_stdlib t = t.po_deregister_php_stdlib

let log_fanout t ~fanout_cardinal =
  match t.tco_log_large_fanouts_threshold with
  | None -> false
  | Some threshold -> Int.(fanout_cardinal >= threshold)

let tco_log_inference_constraints t = t.tco_log_inference_constraints

let po_codegen t = t.po_codegen

let po_disallow_toplevel_requires t = t.po_disallow_toplevel_requires

let tco_language_feature_logging t = t.tco_language_feature_logging

let tco_timeout t = t.tco_timeout

let tco_disallow_invalid_arraykey t = t.tco_disallow_invalid_arraykey

let tco_disallow_byref_dynamic_calls t = t.tco_disallow_byref_dynamic_calls

let tco_disallow_byref_calls t = t.tco_disallow_byref_calls

let allowed_fixme_codes_strict t = t.allowed_fixme_codes_strict

let allowed_fixme_codes_partial t = t.allowed_fixme_codes_partial

let codes_not_raised_partial t = t.codes_not_raised_partial

let log_levels t = t.log_levels

let po_disable_lval_as_an_expression t = t.po_disable_lval_as_an_expression

let tco_shallow_class_decl t = t.tco_shallow_class_decl

let tco_force_shallow_decl_fanout t = t.tco_force_shallow_decl_fanout

let tco_remote_old_decls_no_limit t = t.tco_remote_old_decls_no_limit

let tco_fetch_remote_old_decls t = t.tco_fetch_remote_old_decls

let tco_force_load_hot_shallow_decls t = t.tco_force_load_hot_shallow_decls

let tco_populate_member_heaps t = t.tco_populate_member_heaps

let tco_skip_hierarchy_checks t = t.tco_skip_hierarchy_checks

let tco_skip_tast_checks t = t.tco_skip_tast_checks

let tco_like_type_hints t = t.tco_like_type_hints

let tco_union_intersection_type_hints t = t.tco_union_intersection_type_hints

let tco_call_coeffects t = t.tco_coeffects

let tco_local_coeffects t = t.tco_coeffects_local

let tco_strict_contexts t = t.tco_strict_contexts

let ifc_enabled t = t.tco_ifc_enabled

(* Fully enable IFC on the tcopt *)
let enable_ifc t = { t with tco_ifc_enabled = ["/"] }

let global_write_check_enabled t = t.tco_global_write_check_enabled

let enable_global_write_check t =
  { t with tco_global_write_check_enabled = ["/"] }

let global_write_check_functions_enabled t =
  t.tco_global_write_check_functions_enabled

let tco_like_casts t = t.tco_like_casts

let tco_simple_pessimize t = t.tco_simple_pessimize

let tco_complex_coercion t = t.tco_complex_coercion

let error_codes_treated_strictly t = t.error_codes_treated_strictly

let tco_check_xhp_attribute t = t.tco_check_xhp_attribute

let tco_check_redundant_generics t = t.tco_check_redundant_generics

let tco_disallow_unresolved_type_variables t =
  t.tco_disallow_unresolved_type_variables

let po_enable_class_level_where_clauses t =
  t.po_enable_class_level_where_clauses

let po_disable_legacy_soft_typehints t = t.po_disable_legacy_soft_typehints

let po_allowed_decl_fixme_codes t = t.po_allowed_decl_fixme_codes

let po_allow_new_attribute_syntax t = t.po_allow_new_attribute_syntax

let po_allow_unstable_features t = t.po_allow_unstable_features

let tco_global_inference t = t.tco_global_inference

let tco_gi_reinfer_types t = t.tco_gi_reinfer_types

let tco_ordered_solving t = t.tco_ordered_solving

let tco_const_static_props t = t.tco_const_static_props

let po_disable_legacy_attribute_syntax t = t.po_disable_legacy_attribute_syntax

let tco_const_attribute t = t.tco_const_attribute

let po_const_default_func_args t = t.po_const_default_func_args

let po_const_default_lambda_args t = t.po_const_default_lambda_args

let po_disallow_silence t = t.po_disallow_silence

let po_abstract_static_props t = t.po_abstract_static_props

let tco_check_attribute_locations t = t.tco_check_attribute_locations

let glean_service t = t.glean_service

let glean_hostname t = t.glean_hostname

let glean_port t = t.glean_port

let glean_reponame t = t.glean_reponame

let symbol_write_ownership t = t.symbol_write_ownership

let symbol_write_root_path t = t.symbol_write_root_path

let symbol_write_hhi_path t = t.symbol_write_hhi_path

let symbol_write_ignore_paths t = t.symbol_write_ignore_paths

let symbol_write_index_paths t = t.symbol_write_index_paths

let symbol_write_index_paths_file t = t.symbol_write_index_paths_file

let symbol_write_index_paths_file_output t =
  t.symbol_write_index_paths_file_output

let symbol_write_include_hhi t = t.symbol_write_include_hhi

let set_global_inference t = { t with tco_global_inference = true }

let set_ordered_solving t b = { t with tco_ordered_solving = b }

let set_tco_no_parser_readonly_check t b =
  { t with tco_no_parser_readonly_check = b }

let tco_no_parser_readonly_check t = t.tco_no_parser_readonly_check

let po_parser_errors_only t = t.po_parser_errors_only

let po_disallow_func_ptrs_in_constants t = t.po_disallow_func_ptrs_in_constants

let tco_error_php_lambdas t = t.tco_error_php_lambdas

let tco_disallow_discarded_nullable_awaitables t =
  t.tco_disallow_discarded_nullable_awaitables

let po_enable_xhp_class_modifier t = t.po_enable_xhp_class_modifier

let po_disable_xhp_element_mangling t = t.po_disable_xhp_element_mangling

let po_disable_xhp_children_declarations t =
  t.po_disable_xhp_children_declarations

let po_enable_enum_classes t = t.po_enable_enum_classes

let po_disable_hh_ignore_error t = t.po_disable_hh_ignore_error

let tco_enable_systemlib_annotations t = t.tco_enable_systemlib_annotations

let tco_higher_kinded_types t = t.tco_higher_kinded_types

let tco_method_call_inference t = t.tco_method_call_inference

let tco_report_pos_from_reason t = t.tco_report_pos_from_reason

let tco_typecheck_sample_rate t = t.tco_typecheck_sample_rate

let tco_enable_sound_dynamic t = t.tco_enable_sound_dynamic

let po_disallow_fun_and_cls_meth_pseudo_funcs t =
  t.po_disallow_fun_and_cls_meth_pseudo_funcs

let po_disallow_inst_meth t = t.po_disallow_inst_meth

let tco_use_direct_decl_parser t = t.tco_use_direct_decl_parser

let po_enable_enum_supertyping t = t.po_enable_enum_supertyping

let po_interpret_soft_types_as_like_types t =
  t.po_interpret_soft_types_as_like_types

let tco_enable_strict_string_concat_interp t =
  t.tco_enable_strict_string_concat_interp

let tco_ignore_unsafe_cast t = t.tco_ignore_unsafe_cast

let set_tco_enable_expression_trees t b =
  { t with tco_enable_expression_trees = b }

let expression_trees_enabled t = t.tco_enable_expression_trees

let tco_enable_modules t = t.tco_enable_modules

let set_tco_enable_modules t b = { t with tco_enable_modules = b }

let allowed_expression_tree_visitors t = t.tco_allowed_expression_tree_visitors

let tco_math_new_code t = t.tco_math_new_code

let tco_typeconst_concrete_concrete_error t =
  t.tco_typeconst_concrete_concrete_error

let tco_enable_strict_const_semantics t = t.tco_enable_strict_const_semantics

let tco_strict_wellformedness t = t.tco_strict_wellformedness

let tco_meth_caller_only_public_visibility t =
  t.tco_meth_caller_only_public_visibility

let tco_require_extends_implements_ancestors t =
  t.tco_require_extends_implements_ancestors

let tco_strict_value_equality t = t.tco_strict_value_equality

let tco_enforce_sealed_subclasses t = t.tco_enforce_sealed_subclasses

let tco_everything_sdt t = t.tco_everything_sdt

let tco_pessimise_builtins t = t.tco_pessimise_builtins

let tco_enable_disk_heap t = t.tco_enable_disk_heap

let tco_explicit_consistent_constructors t =
  t.tco_explicit_consistent_constructors

let tco_require_types_class_consts t = t.tco_require_types_class_consts

let tco_type_printer_fuel t = t.tco_type_printer_fuel

let tco_log_saved_state_age_and_distance t =
  t.tco_log_saved_state_age_and_distance

let tco_specify_manifold_api_key t = t.tco_specify_manifold_api_key

let tco_saved_state_manifold_api_key t = t.tco_saved_state_manifold_api_key

let tco_profile_top_level_definitions t = t.tco_profile_top_level_definitions

let tco_allow_all_files_for_module_declarations t =
  t.tco_allow_all_files_for_module_declarations

let tco_allowed_files_for_module_declarations t =
  t.tco_allowed_files_for_module_declarations

let tco_use_manifold_cython_client t = t.tco_use_manifold_cython_client

let tco_record_fine_grained_dependencies t =
  t.tco_record_fine_grained_dependencies

let tco_loop_iteration_upper_bound t = t.tco_loop_iteration_upper_bound

let tco_expression_tree_virtualize_functions t =
  t.tco_expression_tree_virtualize_functions
