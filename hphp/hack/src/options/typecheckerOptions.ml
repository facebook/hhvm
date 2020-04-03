(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t [@@deriving show]

let defer_class_declaration_threshold =
  GlobalOptions.tco_defer_class_declaration_threshold

let max_times_to_defer_type_checking =
  GlobalOptions.tco_max_times_to_defer_type_checking

let prefetch_deferred_files = GlobalOptions.tco_prefetch_deferred_files

let remote_type_check_threshold = GlobalOptions.tco_remote_type_check_threshold

let remote_type_check = GlobalOptions.tco_remote_type_check

let remote_worker_key = GlobalOptions.tco_remote_worker_key

let remote_check_id = GlobalOptions.tco_remote_check_id

let remote_max_batch_size = GlobalOptions.tco_remote_max_batch_size

let remote_min_batch_size = GlobalOptions.tco_remote_min_batch_size

let num_remote_workers = GlobalOptions.tco_num_remote_workers

let remote_version_specifier = GlobalOptions.so_remote_version_specifier

let disallow_array_typehint = GlobalOptions.tco_disallow_array_typehint

let disallow_array_literal = GlobalOptions.tco_disallow_array_literal

let language_feature_logging = GlobalOptions.tco_language_feature_logging

let unsafe_rx = GlobalOptions.tco_unsafe_rx

let experimental_feature_enabled =
  GlobalOptions.tco_experimental_feature_enabled

let migration_flag_enabled = GlobalOptions.tco_migration_flag_enabled

let log_inference_constraints = GlobalOptions.tco_log_inference_constraints

let default = GlobalOptions.default

let experimental_isarray = GlobalOptions.tco_experimental_isarray

let experimental_stronger_shape_idx_ret =
  GlobalOptions.tco_experimental_stronger_shape_idx_ret

let experimental_generics_arity = GlobalOptions.tco_experimental_generics_arity

let experimental_forbid_nullable_cast =
  GlobalOptions.tco_experimental_forbid_nullable_cast

let experimental_coroutines = GlobalOptions.tco_experimental_coroutines

let experimental_disallow_static_memoized =
  GlobalOptions.tco_experimental_disallow_static_memoized

let experimental_no_trait_reuse = GlobalOptions.tco_experimental_no_trait_reuse

let experimental_type_param_shadowing =
  GlobalOptions.tco_experimental_type_param_shadowing

let experimental_trait_method_redeclarations =
  GlobalOptions.tco_experimental_trait_method_redeclarations

let experimental_abstract_type_const_with_default =
  GlobalOptions.tco_experimental_abstract_type_const_with_default

let experimental_all = GlobalOptions.tco_experimental_all

let migration_flags_all = GlobalOptions.tco_migration_flags_all

let dynamic_view = GlobalOptions.tco_dynamic_view

let disallow_scrutinee_case_value_type_mismatch =
  GlobalOptions.tco_disallow_scrutinee_case_value_type_mismatch

let timeout = GlobalOptions.tco_timeout

let disallow_invalid_arraykey = GlobalOptions.tco_disallow_invalid_arraykey

let disallow_byref_dynamic_calls =
  GlobalOptions.tco_disallow_byref_dynamic_calls

let disallow_byref_calls = GlobalOptions.tco_disallow_byref_calls

let log_levels = GlobalOptions.log_levels

let shallow_class_decl = GlobalOptions.tco_shallow_class_decl

let like_type_hints = GlobalOptions.tco_like_type_hints

let union_intersection_type_hints =
  GlobalOptions.tco_union_intersection_type_hints

let like_casts = GlobalOptions.tco_like_casts

let simple_pessimize = GlobalOptions.tco_simple_pessimize

let complex_coercion = GlobalOptions.tco_complex_coercion

let disable_partially_abstract_typeconsts =
  GlobalOptions.tco_disable_partially_abstract_typeconsts

let check_xhp_attribute = GlobalOptions.tco_check_xhp_attribute

let check_redundant_generics = GlobalOptions.tco_check_redundant_generics

let disallow_unresolved_type_variables =
  GlobalOptions.tco_disallow_unresolved_type_variables

let disallow_invalid_arraykey_constraint =
  GlobalOptions.tco_disallow_invalid_arraykey_constraint

let const_static_props = GlobalOptions.tco_const_static_props

let global_inference = GlobalOptions.tco_global_inference

let const_attribute = GlobalOptions.tco_const_attribute

let abstract_static_props = GlobalOptions.po_abstract_static_props

let set_global_inference = GlobalOptions.set_global_inference

let check_attribute_locations = GlobalOptions.tco_check_attribute_locations

let error_php_lambdas = GlobalOptions.tco_error_php_lambdas

let disallow_discarded_nullable_awaitables =
  GlobalOptions.tco_disallow_discarded_nullable_awaitables
