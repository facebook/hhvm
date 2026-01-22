(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = GlobalOptions.t [@@deriving eq, show]

let num_local_workers t = t.GlobalOptions.tco_num_local_workers

let defer_class_declaration_threshold t =
  t.GlobalOptions.tco_defer_class_declaration_threshold

let language_feature_logging t = t.GlobalOptions.tco_language_feature_logging

let experimental_feature_enabled t feature =
  SSet.mem feature t.GlobalOptions.tco_experimental_features

let migration_flag_enabled t flag =
  SSet.mem flag t.GlobalOptions.tco_migration_flags

let log_inference_constraints t = t.GlobalOptions.tco_log_inference_constraints

let default = GlobalOptions.default

(* TYPECHECKER-SPECIFIC OPTIONS *)

(**
 * Insist on instantiations for all generic types, even in non-strict files
 *)
let experimental_generics_arity = "generics_arity"

(**
 * Forbid casting nullable values, since they have unexpected semantics. For
 * example, casting `null` to an int results in `0`, which may or may not be
 * what you were expecting.
 *)
let experimental_forbid_nullable_cast = "forbid_nullable_cast"

(*
* Disallow static memoized functions in non-final classes
*)
let experimental_disallow_static_memoized = "disallow_static_memoized"

(**
 * Enable abstract const type with default syntax, i.e.
 * abstract const type T as num = int;
 *)
let experimental_abstract_type_const_with_default =
  "abstract_type_const_with_default"

let experimental_supportdynamic_type_hint = "supportdynamic_type_hint"

let experimental_consider_type_const_enforceable =
  "experimental_consider_type_const_enforceable"

(** Resolve the this type inside enum classes using the proper context *)
let experimental_sound_enum_class_type_const = "sound_enum_class_this_type"

let experimental_try_constraint_method_inference =
  "experimental_try_constraint_method_inference"

let experimental_all =
  List.fold_right
    ~f:SSet.add
    ~init:SSet.empty
    [
      experimental_generics_arity;
      experimental_forbid_nullable_cast;
      experimental_disallow_static_memoized;
      experimental_abstract_type_const_with_default;
      experimental_supportdynamic_type_hint;
      experimental_consider_type_const_enforceable;
      experimental_sound_enum_class_type_const;
      experimental_try_constraint_method_inference;
    ]

let experimental_from_flags ~disallow_static_memoized =
  if disallow_static_memoized then
    SSet.singleton experimental_disallow_static_memoized
  else
    SSet.empty

let enable_experimental t feature =
  SSet.add feature t.GlobalOptions.tco_experimental_features

let migration_flags_all = List.fold_right ~init:SSet.empty ~f:SSet.add []

let timeout t = t.GlobalOptions.tco_timeout

let constraint_array_index_assign t =
  t.GlobalOptions.tco_constraint_array_index_assign

let constraint_method_call t = t.GlobalOptions.tco_constraint_method_call

let log_levels t = t.GlobalOptions.log_levels

let remote_old_decls_no_limit t = t.GlobalOptions.tco_remote_old_decls_no_limit

let fetch_remote_old_decls t = t.GlobalOptions.tco_fetch_remote_old_decls

let populate_member_heaps t = t.GlobalOptions.tco_populate_member_heaps

let skip_hierarchy_checks t = t.GlobalOptions.tco_skip_hierarchy_checks

let set_skip_hierarchy_checks t =
  GlobalOptions.{ t with tco_skip_hierarchy_checks = true }

let skip_tast_checks t = t.GlobalOptions.tco_skip_tast_checks

let call_coeffects t = t.GlobalOptions.tco_coeffects

let local_coeffects t = t.GlobalOptions.tco_coeffects_local

let any_coeffects t = call_coeffects t || local_coeffects t

let strict_contexts t = t.GlobalOptions.tco_strict_contexts

let enable_global_access_check t =
  GlobalOptions.{ t with tco_global_access_check_enabled = true }

let global_access_check_enabled t =
  t.GlobalOptions.tco_global_access_check_enabled

let like_casts t = t.GlobalOptions.tco_like_casts

let check_xhp_attribute t = t.GlobalOptions.tco_check_xhp_attribute

let check_redundant_generics t = t.GlobalOptions.tco_check_redundant_generics

let disallow_unresolved_type_variables t =
  t.GlobalOptions.tco_disallow_unresolved_type_variables

let custom_error_config t = t.GlobalOptions.tco_custom_error_config

let const_static_props t = t.GlobalOptions.po.ParserOptions.const_static_props

let const_attribute t = t.GlobalOptions.tco_const_attribute

let check_attribute_locations t = t.GlobalOptions.tco_check_attribute_locations

let type_refinement_partition_shapes t =
  t.GlobalOptions.tco_type_refinement_partition_shapes

let error_php_lambdas t = t.GlobalOptions.tco_error_php_lambdas

let disallow_discarded_nullable_awaitables t =
  t.GlobalOptions.tco_disallow_discarded_nullable_awaitables

let is_systemlib t = t.GlobalOptions.po.ParserOptions.is_systemlib

let enable_no_auto_dynamic t = t.GlobalOptions.tco_enable_no_auto_dynamic

let skip_check_under_dynamic t = t.GlobalOptions.tco_skip_check_under_dynamic

let set_skip_check_under_dynamic t =
  GlobalOptions.{ t with tco_skip_check_under_dynamic = true }

let interpret_soft_types_as_like_types t =
  t.GlobalOptions.po.ParserOptions.interpret_soft_types_as_like_types

let ignore_unsafe_cast t = t.GlobalOptions.tco_ignore_unsafe_cast

let set_tco_no_parser_readonly_check t b =
  let po =
    { t.GlobalOptions.po with ParserOptions.no_parser_readonly_check = b }
  in
  GlobalOptions.{ t with po }

let tco_no_parser_readonly_check t =
  t.GlobalOptions.po.ParserOptions.no_parser_readonly_check

let set_tco_enable_expression_trees t b =
  GlobalOptions.{ t with tco_enable_expression_trees = b }

let expression_trees_enabled t = t.GlobalOptions.tco_enable_expression_trees

let enable_function_references t =
  t.GlobalOptions.tco_enable_function_references

let set_function_references t b =
  GlobalOptions.{ t with tco_enable_function_references = b }

let allowed_expression_tree_visitors t =
  t.GlobalOptions.tco_allowed_expression_tree_visitors

let typeconst_concrete_concrete_error t =
  t.GlobalOptions.tco_typeconst_concrete_concrete_error

let meth_caller_only_public_visibility t =
  t.GlobalOptions.tco_meth_caller_only_public_visibility

let require_extends_implements_ancestors t =
  t.GlobalOptions.tco_require_extends_implements_ancestors

let strict_value_equality t = t.GlobalOptions.tco_strict_value_equality

let enforce_sealed_subclasses t = t.GlobalOptions.tco_enforce_sealed_subclasses

let repo_stdlib_path t = t.GlobalOptions.tco_repo_stdlib_path

let everything_sdt t = t.GlobalOptions.po.ParserOptions.everything_sdt

let implicit_inherit_sdt t = t.GlobalOptions.tco_implicit_inherit_sdt

let pessimise_builtins t =
  everything_sdt t || t.GlobalOptions.tco_pessimise_builtins

let explicit_consistent_constructors t =
  t.GlobalOptions.tco_explicit_consistent_constructors

let require_types_class_consts t =
  t.GlobalOptions.tco_require_types_class_consts

let check_bool_for_condition t = t.GlobalOptions.tco_check_bool_for_condition

let type_printer_fuel t = t.GlobalOptions.tco_type_printer_fuel

let log_saved_state_age_and_distance t =
  GlobalOptions.(t.tco_saved_state.loading.log_saved_state_age_and_distance)

let specify_manifold_api_key t = t.GlobalOptions.tco_specify_manifold_api_key

let saved_state t = t.GlobalOptions.tco_saved_state

let saved_state_loading t = GlobalOptions.(t.tco_saved_state.loading)

let saved_state_rollouts t = GlobalOptions.(t.tco_saved_state.rollouts)

let optimized_member_fanout t =
  GlobalOptions.(t.tco_saved_state.rollouts)
    .Saved_state_rollouts.optimized_member_fanout

let profile_top_level_definitions t =
  t.GlobalOptions.tco_profile_top_level_definitions

let typecheck_if_name_matches_regexp t =
  t.GlobalOptions.tco_typecheck_if_name_matches_regexp

let allow_all_files_for_module_declarations t =
  t.GlobalOptions.tco_allow_all_files_for_module_declarations

let allowed_files_for_module_declarations t =
  t.GlobalOptions.tco_allowed_files_for_module_declarations

let allowed_files_for_ignore_readonly t =
  t.GlobalOptions.tco_allowed_files_for_ignore_readonly

let record_fine_grained_dependencies t =
  t.GlobalOptions.tco_record_fine_grained_dependencies

let loop_iteration_upper_bound t =
  t.GlobalOptions.tco_loop_iteration_upper_bound

let typecheck_sample_rate t = t.GlobalOptions.tco_typecheck_sample_rate

let log_fanout t ~fanout_cardinal =
  match t.GlobalOptions.tco_log_large_fanouts_threshold with
  | None -> false
  | Some threshold -> Int.(fanout_cardinal >= threshold)

let populate_dead_unsafe_cast_heap t =
  t.GlobalOptions.tco_populate_dead_unsafe_cast_heap

let locl_cache_capacity t = t.GlobalOptions.tco_locl_cache_capacity

let locl_cache_node_threshold t = t.GlobalOptions.tco_locl_cache_node_threshold

let dump_tast_hashes t = t.GlobalOptions.dump_tast_hashes

let dump_tasts t = t.GlobalOptions.dump_tasts

let tco_autocomplete_mode t = t.GlobalOptions.tco_autocomplete_mode

let set_tco_autocomplete_mode t =
  { t with GlobalOptions.tco_autocomplete_mode = true }

let package_info t = t.GlobalOptions.po.ParserOptions.package_info

let tco_sticky_quarantine t = t.GlobalOptions.tco_sticky_quarantine

let tco_lsp_invalidation t = t.GlobalOptions.tco_lsp_invalidation

let tco_extended_reasons t = t.GlobalOptions.tco_extended_reasons

let enable_abstract_method_optional_parameters t =
  t.GlobalOptions.tco_enable_abstract_method_optional_parameters

let disable_physical_equality t = t.GlobalOptions.tco_disable_physical_equality

let hack_warnings t = t.GlobalOptions.hack_warnings

let warnings_generated_files t = t.GlobalOptions.warnings_generated_files

let check_packages t = t.GlobalOptions.tco_check_packages

let allow_require_package_on_interface_methods t =
  t.GlobalOptions.tco_allow_require_package_on_interface_methods

let set_package_info t package_info =
  let popt = { t.GlobalOptions.po with ParserOptions.package_info } in
  { t with GlobalOptions.po = popt }

let package_support_multifile_tests t =
  t.GlobalOptions.po.ParserOptions.package_support_multifile_tests

let package_allow_typedef_violations t =
  t.GlobalOptions.tco_package_allow_typedef_violations

let package_allow_classconst_violations t =
  t.GlobalOptions.tco_package_allow_classconst_violations

let package_allow_reifiable_tconst_violations t =
  t.GlobalOptions.tco_package_allow_reifiable_tconst_violations

let package_allow_all_tconst_violations t =
  t.GlobalOptions.tco_package_allow_all_tconst_violations

let package_allow_reified_generics_violations t =
  t.GlobalOptions.tco_package_allow_reified_generics_violations

let package_allow_all_generics_violations t =
  t.GlobalOptions.tco_package_allow_all_generics_violations

let package_allow_function_pointers_violations t =
  t.GlobalOptions.tco_package_allow_function_pointers_violations

let package_exclude_patterns t = t.GlobalOptions.tco_package_exclude_patterns

let class_sub_classname t = t.GlobalOptions.class_sub_classname

let enable_class_pointer_hint t =
  t.GlobalOptions.po.ParserOptions.enable_class_pointer_hint

let class_class_type t = t.GlobalOptions.class_class_type

let needs_concrete t = t.GlobalOptions.needs_concrete

let needs_concrete_override_check t =
  t.GlobalOptions.needs_concrete_override_check

let allow_class_string_cast t = t.GlobalOptions.allow_class_string_cast

let class_pointer_ban_classname_new t =
  t.GlobalOptions.class_pointer_ban_classname_new

let class_pointer_ban_classname_type_structure t =
  t.GlobalOptions.class_pointer_ban_classname_type_structure

let class_pointer_ban_classname_static_meth t =
  t.GlobalOptions.class_pointer_ban_classname_static_meth

let class_pointer_ban_classname_class_const t =
  t.GlobalOptions.class_pointer_ban_classname_class_const

let class_pointer_ban_class_array_key t =
  t.GlobalOptions.class_pointer_ban_class_array_key

let tco_poly_function_pointers t = t.GlobalOptions.tco_poly_function_pointers

let enable_recursive_case_types t =
  { t with GlobalOptions.recursive_case_types = true }
