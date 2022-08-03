(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t [@@deriving eq, show]

let num_local_workers = GlobalOptions.tco_num_local_workers

let parallel_type_checking_threshold =
  GlobalOptions.tco_parallel_type_checking_threshold

let max_typechecker_worker_memory_mb =
  GlobalOptions.tco_max_typechecker_worker_memory_mb

let defer_class_declaration_threshold =
  GlobalOptions.tco_defer_class_declaration_threshold

let prefetch_deferred_files = GlobalOptions.tco_prefetch_deferred_files

let remote_type_check_threshold = GlobalOptions.tco_remote_type_check_threshold

let remote_type_check = GlobalOptions.tco_remote_type_check

let remote_worker_key = GlobalOptions.tco_remote_worker_key

let remote_check_id = GlobalOptions.tco_remote_check_id

let remote_max_batch_size = GlobalOptions.tco_remote_max_batch_size

let remote_min_batch_size = GlobalOptions.tco_remote_min_batch_size

let num_remote_workers = GlobalOptions.tco_num_remote_workers

let remote_version_specifier = GlobalOptions.so_remote_version_specifier

let language_feature_logging = GlobalOptions.tco_language_feature_logging

let experimental_feature_enabled =
  GlobalOptions.tco_experimental_feature_enabled

let migration_flag_enabled = GlobalOptions.tco_migration_flag_enabled

let log_inference_constraints = GlobalOptions.tco_log_inference_constraints

let default = GlobalOptions.default

let experimental_generics_arity = GlobalOptions.tco_experimental_generics_arity

let experimental_forbid_nullable_cast =
  GlobalOptions.tco_experimental_forbid_nullable_cast

let experimental_infer_flows = GlobalOptions.tco_experimental_infer_flows

let experimental_disallow_static_memoized =
  GlobalOptions.tco_experimental_disallow_static_memoized

let experimental_abstract_type_const_with_default =
  GlobalOptions.tco_experimental_abstract_type_const_with_default

let experimental_supportdynamic_type_hint =
  GlobalOptions.tco_experimental_supportdynamic_type_hint

let experimental_all = GlobalOptions.tco_experimental_all

let migration_flags_all = GlobalOptions.tco_migration_flags_all

let timeout = GlobalOptions.tco_timeout

let disallow_invalid_arraykey = GlobalOptions.tco_disallow_invalid_arraykey

let disallow_byref_dynamic_calls =
  GlobalOptions.tco_disallow_byref_dynamic_calls

let disallow_byref_calls = GlobalOptions.tco_disallow_byref_calls

let log_levels = GlobalOptions.log_levels

let shallow_class_decl = GlobalOptions.tco_shallow_class_decl

let force_shallow_decl_fanout = GlobalOptions.tco_force_shallow_decl_fanout

let remote_old_decls_no_limit = GlobalOptions.tco_remote_old_decls_no_limit

let fetch_remote_old_decls = GlobalOptions.tco_fetch_remote_old_decls

let force_load_hot_shallow_decls =
  GlobalOptions.tco_force_load_hot_shallow_decls

let populate_member_heaps = GlobalOptions.tco_populate_member_heaps

let skip_hierarchy_checks = GlobalOptions.tco_skip_hierarchy_checks

let skip_tast_checks = GlobalOptions.tco_skip_tast_checks

let call_coeffects = GlobalOptions.tco_call_coeffects

let local_coeffects = GlobalOptions.tco_local_coeffects

let any_coeffects t = call_coeffects t || local_coeffects t

let strict_contexts = GlobalOptions.tco_strict_contexts

let enable_ifc = GlobalOptions.enable_ifc

let ifc_enabled = GlobalOptions.ifc_enabled

let enable_global_write_check = GlobalOptions.enable_global_write_check

let global_write_check_enabled = GlobalOptions.global_write_check_enabled

let global_write_check_functions_enabled =
  GlobalOptions.global_write_check_functions_enabled

let like_type_hints = GlobalOptions.tco_like_type_hints

let like_casts = GlobalOptions.tco_like_casts

let simple_pessimize = GlobalOptions.tco_simple_pessimize

let complex_coercion = GlobalOptions.tco_complex_coercion

let check_xhp_attribute = GlobalOptions.tco_check_xhp_attribute

let check_redundant_generics = GlobalOptions.tco_check_redundant_generics

let disallow_unresolved_type_variables =
  GlobalOptions.tco_disallow_unresolved_type_variables

let const_static_props = GlobalOptions.tco_const_static_props

let global_inference = GlobalOptions.tco_global_inference

let const_attribute = GlobalOptions.tco_const_attribute

let abstract_static_props = GlobalOptions.po_abstract_static_props

let set_global_inference = GlobalOptions.set_global_inference

let check_attribute_locations = GlobalOptions.tco_check_attribute_locations

let error_php_lambdas = GlobalOptions.tco_error_php_lambdas

let disallow_discarded_nullable_awaitables =
  GlobalOptions.tco_disallow_discarded_nullable_awaitables

let enable_systemlib_annotations =
  GlobalOptions.tco_enable_systemlib_annotations

let higher_kinded_types = GlobalOptions.tco_higher_kinded_types

let method_call_inference = GlobalOptions.tco_method_call_inference

let report_pos_from_reason = GlobalOptions.tco_report_pos_from_reason

let enable_sound_dynamic = GlobalOptions.tco_enable_sound_dynamic

let use_direct_decl_parser = GlobalOptions.tco_use_direct_decl_parser

let interpret_soft_types_as_like_types =
  GlobalOptions.po_interpret_soft_types_as_like_types

let enable_strict_string_concat_interp =
  GlobalOptions.tco_enable_strict_string_concat_interp

let ignore_unsafe_cast = GlobalOptions.tco_ignore_unsafe_cast

let set_no_parser_readonly_check =
  GlobalOptions.set_tco_no_parser_readonly_check

let no_parser_readonly_check = GlobalOptions.tco_no_parser_readonly_check

let set_tco_enable_expression_trees =
  GlobalOptions.set_tco_enable_expression_trees

let expression_trees_enabled = GlobalOptions.expression_trees_enabled

let enable_modules = GlobalOptions.tco_enable_modules

let set_modules = GlobalOptions.set_tco_enable_modules

let allowed_expression_tree_visitors =
  GlobalOptions.allowed_expression_tree_visitors

let math_new_code = GlobalOptions.tco_math_new_code

let typeconst_concrete_concrete_error =
  GlobalOptions.tco_typeconst_concrete_concrete_error

let enable_strict_const_semantics =
  GlobalOptions.tco_enable_strict_const_semantics

let strict_wellformedness = GlobalOptions.tco_strict_wellformedness

let meth_caller_only_public_visibility =
  GlobalOptions.tco_meth_caller_only_public_visibility

let require_extends_implements_ancestors =
  GlobalOptions.tco_require_extends_implements_ancestors

let strict_value_equality = GlobalOptions.tco_strict_value_equality

let enforce_sealed_subclasses = GlobalOptions.tco_enforce_sealed_subclasses

let enable_enum_supertyping = GlobalOptions.po_enable_enum_supertyping

let everything_sdt = GlobalOptions.tco_everything_sdt

let pessimise_builtins = GlobalOptions.tco_pessimise_builtins

let enable_disk_heap = GlobalOptions.tco_enable_disk_heap

let explicit_consistent_constructors =
  GlobalOptions.tco_explicit_consistent_constructors

let require_types_class_consts = GlobalOptions.tco_require_types_class_consts

let type_printer_fuel = GlobalOptions.tco_type_printer_fuel

let log_saved_state_age_and_distance =
  GlobalOptions.tco_log_saved_state_age_and_distance

let specify_manifold_api_key = GlobalOptions.tco_specify_manifold_api_key

let saved_state_manifold_api_key =
  GlobalOptions.tco_saved_state_manifold_api_key

let profile_top_level_definitions =
  GlobalOptions.tco_profile_top_level_definitions

let allow_all_files_for_module_declarations =
  GlobalOptions.tco_allow_all_files_for_module_declarations

let allowed_files_for_module_declarations =
  GlobalOptions.tco_allowed_files_for_module_declarations

let use_manifold_cython_client = GlobalOptions.tco_use_manifold_cython_client

let record_fine_grained_dependencies =
  GlobalOptions.tco_record_fine_grained_dependencies

let loop_iteration_upper_bound = GlobalOptions.tco_loop_iteration_upper_bound
