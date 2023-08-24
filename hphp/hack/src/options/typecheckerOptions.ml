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

let max_typechecker_worker_memory_mb t =
  t.GlobalOptions.tco_max_typechecker_worker_memory_mb

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
* Allow typechecker to do global inference and infer IFC flows
* with the <<InferFlows>> flag
*)
let experimental_infer_flows = "ifc_infer_flows"

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

let experimental_always_pessimise_return = "always_pessimise_return"

let experimental_consider_type_const_enforceable =
  "experimental_consider_type_const_enforceable"

let experimental_all =
  List.fold_right
    ~f:SSet.add
    ~init:SSet.empty
    [
      experimental_generics_arity;
      experimental_forbid_nullable_cast;
      experimental_disallow_static_memoized;
      experimental_abstract_type_const_with_default;
      experimental_infer_flows;
      experimental_supportdynamic_type_hint;
      experimental_always_pessimise_return;
      experimental_consider_type_const_enforceable;
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

let disallow_invalid_arraykey t = t.GlobalOptions.tco_disallow_invalid_arraykey

let disallow_byref_dynamic_calls t =
  t.GlobalOptions.tco_disallow_byref_dynamic_calls

let disallow_byref_calls t = t.GlobalOptions.tco_disallow_byref_calls

let log_levels t = t.GlobalOptions.log_levels

let remote_old_decls_no_limit t = t.GlobalOptions.tco_remote_old_decls_no_limit

let fetch_remote_old_decls t = t.GlobalOptions.tco_fetch_remote_old_decls

let populate_member_heaps t = t.GlobalOptions.tco_populate_member_heaps

let skip_hierarchy_checks t = t.GlobalOptions.tco_skip_hierarchy_checks

let skip_tast_checks t = t.GlobalOptions.tco_skip_tast_checks

let call_coeffects t = t.GlobalOptions.tco_coeffects

let local_coeffects t = t.GlobalOptions.tco_coeffects_local

let any_coeffects t = call_coeffects t || local_coeffects t

let strict_contexts t = t.GlobalOptions.tco_strict_contexts

(* Fully enable IFC on the tcopt *)
let enable_ifc t = GlobalOptions.{ t with tco_ifc_enabled = ["/"] }

let ifc_enabled t = t.GlobalOptions.tco_ifc_enabled

let enable_global_access_check t =
  GlobalOptions.{ t with tco_global_access_check_enabled = true }

let global_access_check_enabled t =
  t.GlobalOptions.tco_global_access_check_enabled

let like_type_hints t = t.GlobalOptions.tco_like_type_hints

let like_casts t = t.GlobalOptions.tco_like_casts

let check_xhp_attribute t = t.GlobalOptions.tco_check_xhp_attribute

let check_redundant_generics t = t.GlobalOptions.tco_check_redundant_generics

let disallow_unresolved_type_variables t =
  t.GlobalOptions.tco_disallow_unresolved_type_variables

let custom_error_config t = t.GlobalOptions.tco_custom_error_config

let const_static_props t = t.GlobalOptions.tco_const_static_props

let const_attribute t = t.GlobalOptions.tco_const_attribute

let check_attribute_locations t = t.GlobalOptions.tco_check_attribute_locations

let error_php_lambdas t = t.GlobalOptions.tco_error_php_lambdas

let disallow_discarded_nullable_awaitables t =
  t.GlobalOptions.tco_disallow_discarded_nullable_awaitables

let is_systemlib t = t.GlobalOptions.tco_is_systemlib

let higher_kinded_types t = t.GlobalOptions.tco_higher_kinded_types

let method_call_inference t = t.GlobalOptions.tco_method_call_inference

let report_pos_from_reason t = t.GlobalOptions.tco_report_pos_from_reason

let enable_sound_dynamic t = t.GlobalOptions.tco_enable_sound_dynamic

let enable_no_auto_dynamic t = t.GlobalOptions.tco_enable_no_auto_dynamic

let skip_check_under_dynamic t = t.GlobalOptions.tco_skip_check_under_dynamic

let interpret_soft_types_as_like_types t =
  t.GlobalOptions.po_interpret_soft_types_as_like_types

let enable_strict_string_concat_interp t =
  t.GlobalOptions.tco_enable_strict_string_concat_interp

let ignore_unsafe_cast t = t.GlobalOptions.tco_ignore_unsafe_cast

let set_tco_no_parser_readonly_check t b =
  GlobalOptions.{ t with tco_no_parser_readonly_check = b }

let tco_no_parser_readonly_check t =
  t.GlobalOptions.tco_no_parser_readonly_check

let set_tco_enable_expression_trees t b =
  GlobalOptions.{ t with tco_enable_expression_trees = b }

let expression_trees_enabled t = t.GlobalOptions.tco_enable_expression_trees

let enable_modules t = t.GlobalOptions.tco_enable_modules

let set_modules t b = GlobalOptions.{ t with tco_enable_modules = b }

let allowed_expression_tree_visitors t =
  t.GlobalOptions.tco_allowed_expression_tree_visitors

let math_new_code t = t.GlobalOptions.tco_math_new_code

let typeconst_concrete_concrete_error t =
  t.GlobalOptions.tco_typeconst_concrete_concrete_error

let enable_strict_const_semantics t =
  t.GlobalOptions.tco_enable_strict_const_semantics

let strict_wellformedness t = t.GlobalOptions.tco_strict_wellformedness

let meth_caller_only_public_visibility t =
  t.GlobalOptions.tco_meth_caller_only_public_visibility

let require_extends_implements_ancestors t =
  t.GlobalOptions.tco_require_extends_implements_ancestors

let strict_value_equality t = t.GlobalOptions.tco_strict_value_equality

let enforce_sealed_subclasses t = t.GlobalOptions.tco_enforce_sealed_subclasses

let everything_sdt t = t.GlobalOptions.tco_everything_sdt

let pessimise_builtins t =
  enable_sound_dynamic t
  && (everything_sdt t || t.GlobalOptions.tco_pessimise_builtins)

let explicit_consistent_constructors t =
  t.GlobalOptions.tco_explicit_consistent_constructors

let require_types_class_consts t =
  t.GlobalOptions.tco_require_types_class_consts

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

let dummy_one t =
  GlobalOptions.(t.tco_saved_state.rollouts).Saved_state_rollouts.dummy_one

let profile_top_level_definitions t =
  t.GlobalOptions.tco_profile_top_level_definitions

let allow_all_files_for_module_declarations t =
  t.GlobalOptions.tco_allow_all_files_for_module_declarations

let allowed_files_for_module_declarations t =
  t.GlobalOptions.tco_allowed_files_for_module_declarations

let record_fine_grained_dependencies t =
  t.GlobalOptions.tco_record_fine_grained_dependencies

let loop_iteration_upper_bound t =
  t.GlobalOptions.tco_loop_iteration_upper_bound

let typecheck_sample_rate t = t.GlobalOptions.tco_typecheck_sample_rate

let log_fanout t ~fanout_cardinal =
  match t.GlobalOptions.tco_log_large_fanouts_threshold with
  | None -> false
  | Some threshold -> Int.(fanout_cardinal >= threshold)

let substitution_mutation t = t.GlobalOptions.tco_substitution_mutation

let use_type_alias_heap t = t.GlobalOptions.tco_use_type_alias_heap

let populate_dead_unsafe_cast_heap t =
  t.GlobalOptions.tco_populate_dead_unsafe_cast_heap

let load_hack_64_distc_saved_state t =
  t.GlobalOptions.tco_load_hack_64_distc_saved_state

let rust_elab t = t.GlobalOptions.tco_rust_elab

let locl_cache_capacity t = t.GlobalOptions.tco_locl_cache_capacity

let locl_cache_node_threshold t = t.GlobalOptions.tco_locl_cache_node_threshold

let dump_tast_hashes t = t.GlobalOptions.dump_tast_hashes

let tco_autocomplete_mode t = t.GlobalOptions.tco_autocomplete_mode

let set_tco_autocomplete_mode t =
  { t with GlobalOptions.tco_autocomplete_mode = true }

let package_info t = t.GlobalOptions.tco_package_info

let tco_log_exhaustivity_check t = t.GlobalOptions.tco_log_exhaustivity_check

let tco_enable_strict_switch t = t.GlobalOptions.tco_enable_strict_switch
