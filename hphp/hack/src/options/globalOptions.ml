(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  tco_experimental_features: SSet.t;
  tco_migration_flags: SSet.t;
  tco_dynamic_view: bool;
  tco_defer_class_declaration_threshold: int option;
  tco_max_times_to_defer_type_checking: int option;
  tco_prefetch_deferred_files: bool;
  tco_remote_type_check_threshold: int option;
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
  po_disallow_execution_operator: bool;
  po_disallow_toplevel_requires: bool;
  po_disable_nontoplevel_declarations: bool;
  po_disable_static_closures: bool;
  po_allow_goto: bool;
  tco_log_inference_constraints: bool;
  tco_disallow_array_typehint: bool;
  tco_disallow_array_literal: bool;
  tco_language_feature_logging: bool;
  tco_unsafe_rx: bool;
  tco_disallow_scrutinee_case_value_type_mismatch: bool;
  tco_timeout: int;
  tco_disallow_invalid_arraykey: bool;
  tco_disallow_byref_dynamic_calls: bool;
  tco_disallow_byref_calls: bool;
  ignored_fixme_codes: ISet.t;
  ignored_fixme_regex: string option;
  log_levels: int SMap.t;
  po_disable_lval_as_an_expression: bool;
  tco_shallow_class_decl: bool;
  po_rust_parser_errors: bool;
  po_rust_top_level_elaborator: bool;
  profile_type_check_duration_threshold: float;
  profile_type_check_twice: bool;
  profile_owner: string;
  profile_desc: string;
  tco_like_type_hints: bool;
  tco_union_intersection_type_hints: bool;
  tco_like_casts: bool;
  tco_simple_pessimize: float;
  tco_complex_coercion: bool;
  tco_disable_partially_abstract_typeconsts: bool;
  error_codes_treated_strictly: ISet.t;
  tco_check_xhp_attribute: bool;
  tco_check_redundant_generics: bool;
  tco_disallow_unresolved_type_variables: bool;
  tco_disallow_invalid_arraykey_constraint: bool;
  po_enable_class_level_where_clauses: bool;
  po_disable_legacy_soft_typehints: bool;
  po_disallowed_decl_fixmes: ISet.t;
  po_allow_new_attribute_syntax: bool;
  tco_global_inference: bool;
  tco_gi_reinfer_types: string list;
  tco_ordered_solving: bool;
  tco_const_static_props: bool;
  po_disable_legacy_attribute_syntax: bool;
  tco_const_attribute: bool;
  po_const_default_func_args: bool;
  po_disallow_silence: bool;
  po_abstract_static_props: bool;
  po_disable_unset_class_const: bool;
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  glean_service: string;
  glean_hostname: string;
  glean_port: int;
  glean_reponame: string;
  symbol_write_root_path: string;
  symbol_write_hhi_path: string;
  po_disallow_func_ptrs_in_constants: bool;
  tco_error_php_lambdas: bool;
  tco_disallow_discarded_nullable_awaitables: bool;
  po_enable_xhp_class_modifier: bool;
  po_disable_xhp_element_mangling: bool;
  po_disable_xhp_children_declarations: bool;
  po_enable_first_class_function_pointers: bool;
  po_disable_modes: bool;
  po_disable_array: bool;
}
[@@deriving show]

let tco_experimental_isarray = "is_array"

(* Whether Shapes::idx should return a non-nullable type when the input shape
    is known to contain the field. *)
let tco_experimental_stronger_shape_idx_ret = "stronger_shape_idx_return"

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

(*
 * Allows parsing of coroutine functions/suspend operator
 *)
let tco_experimental_coroutines = "coroutines"

(**
 * Enforce no duplication of traits in a class hierarchy. There is some clean-up
 * involved, so it's behind a flag.
 *)
let tco_experimental_no_trait_reuse = "no_trait_reuse"

(**
 * Prevent type param names from shadowing class names
 *)
let tco_experimental_type_param_shadowing = "type_param_shadowing"

(**
 * Enable trait method redeclarations, i.e. public function f(): void = T1::f;
 *)
let tco_experimental_trait_method_redeclarations = "trait_method_redeclarations"

(**
 * Enable abstract const type with default syntax, i.e.
 * abstract const type T as num = int;
 *)
let tco_experimental_abstract_type_const_with_default =
  "abstract_type_const_with_default"

let tco_experimental_all =
  SSet.empty
  |> List.fold_right
       SSet.add
       [
         tco_experimental_isarray;
         tco_experimental_stronger_shape_idx_ret;
         tco_experimental_generics_arity;
         tco_experimental_forbid_nullable_cast;
         tco_experimental_coroutines;
         tco_experimental_disallow_static_memoized;
         tco_experimental_no_trait_reuse;
         tco_experimental_trait_method_redeclarations;
         tco_experimental_abstract_type_const_with_default;
       ]

let tco_migration_flags_all =
  SSet.empty |> List.fold_right SSet.add ["array_cast"]

let default =
  {
    (* Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
    tco_experimental_features = tco_experimental_all;
    tco_migration_flags = SSet.empty;
    tco_dynamic_view = false;
    tco_defer_class_declaration_threshold = None;
    tco_max_times_to_defer_type_checking = None;
    tco_prefetch_deferred_files = false;
    tco_remote_type_check_threshold = None;
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
    po_disallow_execution_operator = false;
    po_disallow_toplevel_requires = false;
    po_deregister_php_stdlib = false;
    po_disable_nontoplevel_declarations = false;
    po_disable_static_closures = false;
    po_allow_goto = true;
    tco_log_inference_constraints = false;
    tco_disallow_array_typehint = false;
    tco_disallow_array_literal = false;
    tco_language_feature_logging = false;
    tco_unsafe_rx = true;
    tco_disallow_scrutinee_case_value_type_mismatch = false;
    tco_timeout = 0;
    tco_disallow_invalid_arraykey = false;
    tco_disallow_byref_dynamic_calls = false;
    tco_disallow_byref_calls = false;
    ignored_fixme_codes = Errors.default_ignored_fixme_codes;
    ignored_fixme_regex = None;
    log_levels = SMap.empty;
    po_disable_lval_as_an_expression = true;
    tco_shallow_class_decl = false;
    po_rust_parser_errors = false;
    po_rust_top_level_elaborator = true;
    profile_type_check_duration_threshold = 0.05;
    profile_type_check_twice = false;
    profile_owner = "";
    profile_desc = "";
    tco_like_type_hints = false;
    tco_union_intersection_type_hints = false;
    tco_like_casts = false;
    tco_simple_pessimize = 0.0;
    tco_complex_coercion = false;
    tco_disable_partially_abstract_typeconsts = false;
    error_codes_treated_strictly = ISet.of_list [];
    tco_check_xhp_attribute = false;
    tco_check_redundant_generics = false;
    tco_disallow_unresolved_type_variables = false;
    tco_disallow_invalid_arraykey_constraint = false;
    po_enable_class_level_where_clauses = false;
    po_disable_legacy_soft_typehints = false;
    po_disallowed_decl_fixmes = ISet.of_list [];
    po_allow_new_attribute_syntax = false;
    tco_global_inference = false;
    tco_gi_reinfer_types = [];
    tco_ordered_solving = false;
    tco_const_static_props = false;
    po_disable_legacy_attribute_syntax = false;
    tco_const_attribute = false;
    po_const_default_func_args = false;
    po_disallow_silence = false;
    po_abstract_static_props = false;
    po_disable_unset_class_const = false;
    po_parser_errors_only = false;
    tco_check_attribute_locations = false;
    glean_service = "";
    glean_hostname = "";
    glean_port = 0;
    glean_reponame = "www.autocomplete";
    symbol_write_root_path = "www";
    symbol_write_hhi_path = "hhi";
    po_disallow_func_ptrs_in_constants = false;
    tco_error_php_lambdas = false;
    tco_disallow_discarded_nullable_awaitables = false;
    po_enable_xhp_class_modifier = false;
    po_disable_xhp_element_mangling = false;
    po_disable_xhp_children_declarations = false;
    po_enable_first_class_function_pointers = false;
    po_disable_modes = false;
    po_disable_array = false;
  }

let make
    ?(po_deregister_php_stdlib = default.po_deregister_php_stdlib)
    ?(po_disallow_execution_operator = default.po_disallow_execution_operator)
    ?(po_disallow_toplevel_requires = default.po_disallow_toplevel_requires)
    ?(po_disable_nontoplevel_declarations =
      default.po_disable_nontoplevel_declarations)
    ?(po_disable_static_closures = default.po_disable_static_closures)
    ?(po_allow_goto = default.po_allow_goto)
    ?(tco_log_inference_constraints = default.tco_log_inference_constraints)
    ?(tco_experimental_features = default.tco_experimental_features)
    ?(tco_migration_flags = default.tco_migration_flags)
    ?(tco_dynamic_view = default.tco_dynamic_view)
    ?tco_defer_class_declaration_threshold
    ?tco_max_times_to_defer_type_checking
    ?(tco_prefetch_deferred_files = default.tco_prefetch_deferred_files)
    ?tco_remote_type_check_threshold
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
    ?(tco_disallow_array_typehint = default.tco_disallow_array_typehint)
    ?(tco_disallow_array_literal = default.tco_disallow_array_literal)
    ?(tco_language_feature_logging = default.tco_language_feature_logging)
    ?(tco_unsafe_rx = default.tco_unsafe_rx)
    ?(tco_disallow_scrutinee_case_value_type_mismatch =
      default.tco_disallow_scrutinee_case_value_type_mismatch)
    ?(tco_timeout = default.tco_timeout)
    ?(tco_disallow_invalid_arraykey = default.tco_disallow_invalid_arraykey)
    ?(tco_disallow_byref_dynamic_calls =
      default.tco_disallow_byref_dynamic_calls)
    ?(tco_disallow_byref_calls = default.tco_disallow_byref_calls)
    ?(ignored_fixme_codes = default.ignored_fixme_codes)
    ?ignored_fixme_regex
    ?(log_levels = default.log_levels)
    ?(po_disable_lval_as_an_expression =
      default.po_disable_lval_as_an_expression)
    ?(tco_shallow_class_decl = default.tco_shallow_class_decl)
    ?(po_rust_parser_errors = default.po_rust_parser_errors)
    ?(po_rust_top_level_elaborator = default.po_rust_top_level_elaborator)
    ?(profile_type_check_duration_threshold =
      default.profile_type_check_duration_threshold)
    ?(profile_type_check_twice = default.profile_type_check_twice)
    ?(profile_owner = default.profile_owner)
    ?(profile_desc = default.profile_desc)
    ?(tco_like_type_hints = default.tco_like_type_hints)
    ?(tco_union_intersection_type_hints =
      default.tco_union_intersection_type_hints)
    ?(tco_like_casts = default.tco_like_casts)
    ?(tco_simple_pessimize = default.tco_simple_pessimize)
    ?(tco_complex_coercion = default.tco_complex_coercion)
    ?(tco_disable_partially_abstract_typeconsts =
      default.tco_disable_partially_abstract_typeconsts)
    ?(error_codes_treated_strictly = default.error_codes_treated_strictly)
    ?(tco_check_xhp_attribute = default.tco_check_xhp_attribute)
    ?(tco_check_redundant_generics = default.tco_check_redundant_generics)
    ?(tco_disallow_unresolved_type_variables =
      default.tco_disallow_unresolved_type_variables)
    ?(tco_disallow_invalid_arraykey_constraint =
      default.tco_disallow_invalid_arraykey_constraint)
    ?(po_enable_class_level_where_clauses =
      default.po_enable_class_level_where_clauses)
    ?(po_disable_legacy_soft_typehints =
      default.po_disable_legacy_soft_typehints)
    ?(po_disallowed_decl_fixmes = default.po_disallowed_decl_fixmes)
    ?(po_allow_new_attribute_syntax = default.po_allow_new_attribute_syntax)
    ?(tco_global_inference = default.tco_global_inference)
    ?(tco_gi_reinfer_types = default.tco_gi_reinfer_types)
    ?(tco_ordered_solving = default.tco_ordered_solving)
    ?(tco_const_static_props = default.tco_const_static_props)
    ?(po_disable_legacy_attribute_syntax =
      default.po_disable_legacy_attribute_syntax)
    ?(tco_const_attribute = default.tco_const_attribute)
    ?(po_const_default_func_args = default.po_const_default_func_args)
    ?(po_disallow_silence = default.po_disallow_silence)
    ?(po_abstract_static_props = default.po_abstract_static_props)
    ?(po_disable_unset_class_const = default.po_disable_unset_class_const)
    ?(po_parser_errors_only = default.po_parser_errors_only)
    ?(tco_check_attribute_locations = default.tco_check_attribute_locations)
    ?(glean_service = default.glean_service)
    ?(glean_hostname = default.glean_hostname)
    ?(glean_port = default.glean_port)
    ?(glean_reponame = default.glean_reponame)
    ?(symbol_write_root_path = default.symbol_write_root_path)
    ?(symbol_write_hhi_path = default.symbol_write_hhi_path)
    ?(po_disallow_func_ptrs_in_constants =
      default.po_disallow_func_ptrs_in_constants)
    ?(tco_error_php_lambdas = default.tco_error_php_lambdas)
    ?(tco_disallow_discarded_nullable_awaitables =
      default.tco_disallow_discarded_nullable_awaitables)
    ?(po_enable_xhp_class_modifier = default.po_enable_xhp_class_modifier)
    ?(po_disable_xhp_element_mangling = default.po_disable_xhp_element_mangling)
    ?(po_disable_xhp_children_declarations =
      default.po_disable_xhp_children_declarations)
    ?(po_enable_first_class_function_pointers =
      default.po_enable_first_class_function_pointers)
    ?(po_disable_modes = default.po_disable_modes)
    ?(po_disable_array = default.po_disable_array)
    () =
  {
    tco_experimental_features;
    tco_migration_flags;
    tco_dynamic_view;
    tco_defer_class_declaration_threshold;
    tco_max_times_to_defer_type_checking;
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
    ignored_fixme_codes;
    ignored_fixme_regex;
    po_deregister_php_stdlib;
    po_disallow_execution_operator;
    po_disallow_toplevel_requires;
    po_disable_nontoplevel_declarations;
    po_disable_static_closures;
    po_allow_goto;
    tco_log_inference_constraints;
    tco_disallow_array_typehint;
    tco_disallow_array_literal;
    tco_language_feature_logging;
    tco_unsafe_rx;
    tco_disallow_scrutinee_case_value_type_mismatch;
    tco_timeout;
    tco_disallow_invalid_arraykey;
    tco_disallow_byref_dynamic_calls;
    tco_disallow_byref_calls;
    log_levels;
    po_disable_lval_as_an_expression;
    tco_shallow_class_decl;
    po_rust_parser_errors;
    po_rust_top_level_elaborator;
    profile_type_check_duration_threshold;
    profile_type_check_twice;
    profile_owner;
    profile_desc;
    tco_like_type_hints;
    tco_union_intersection_type_hints;
    tco_like_casts;
    tco_simple_pessimize;
    tco_complex_coercion;
    tco_disable_partially_abstract_typeconsts;
    error_codes_treated_strictly;
    tco_check_xhp_attribute;
    tco_check_redundant_generics;
    tco_disallow_unresolved_type_variables;
    tco_disallow_invalid_arraykey_constraint;
    po_enable_class_level_where_clauses;
    po_disable_legacy_soft_typehints;
    po_disallowed_decl_fixmes;
    po_allow_new_attribute_syntax;
    tco_global_inference;
    tco_gi_reinfer_types;
    tco_ordered_solving;
    tco_const_static_props;
    po_disable_legacy_attribute_syntax;
    tco_const_attribute;
    po_const_default_func_args;
    po_disallow_silence;
    po_abstract_static_props;
    po_disable_unset_class_const;
    po_parser_errors_only;
    tco_check_attribute_locations;
    glean_service;
    glean_hostname;
    glean_port;
    glean_reponame;
    symbol_write_root_path;
    symbol_write_hhi_path;
    po_disallow_func_ptrs_in_constants;
    tco_error_php_lambdas;
    tco_disallow_discarded_nullable_awaitables;
    po_enable_xhp_class_modifier;
    po_disable_xhp_element_mangling;
    po_disable_xhp_children_declarations;
    po_enable_first_class_function_pointers;
    po_disable_modes;
    po_disable_array;
  }

let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features

let tco_migration_flag_enabled t s = SSet.mem s t.tco_migration_flags

let tco_dynamic_view t = t.tco_dynamic_view

let tco_defer_class_declaration_threshold t =
  t.tco_defer_class_declaration_threshold

let tco_max_times_to_defer_type_checking t =
  t.tco_max_times_to_defer_type_checking

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

let po_disable_nontoplevel_declarations t =
  t.po_disable_nontoplevel_declarations

let po_disable_static_closures t = t.po_disable_static_closures

let po_allow_goto t = t.po_allow_goto

let tco_log_inference_constraints t = t.tco_log_inference_constraints

let po_codegen t = t.po_codegen

let po_disallow_execution_operator t = t.po_disallow_execution_operator

let po_disallow_toplevel_requires t = t.po_disallow_toplevel_requires

let tco_disallow_array_typehint t = t.tco_disallow_array_typehint

let tco_disallow_array_literal t = t.tco_disallow_array_literal

let tco_language_feature_logging t = t.tco_language_feature_logging

let tco_unsafe_rx t = t.tco_unsafe_rx

let tco_disallow_scrutinee_case_value_type_mismatch t =
  t.tco_disallow_scrutinee_case_value_type_mismatch

let tco_timeout t = t.tco_timeout

let tco_disallow_invalid_arraykey t = t.tco_disallow_invalid_arraykey

let tco_disallow_invalid_arraykey_constraint t =
  t.tco_disallow_invalid_arraykey_constraint

let tco_disallow_byref_dynamic_calls t = t.tco_disallow_byref_dynamic_calls

let tco_disallow_byref_calls t = t.tco_disallow_byref_calls

let ignored_fixme_codes t = t.ignored_fixme_codes

let ignored_fixme_regex t = t.ignored_fixme_regex

let log_levels t = t.log_levels

let po_disable_lval_as_an_expression t = t.po_disable_lval_as_an_expression

let tco_shallow_class_decl t = t.tco_shallow_class_decl

let po_rust_parser_errors t = t.po_rust_parser_errors

let po_rust_top_level_elaborator t = t.po_rust_top_level_elaborator

let profile_type_check_duration_threshold t =
  t.profile_type_check_duration_threshold

let profile_type_check_twice t = t.profile_type_check_twice

let profile_owner t = t.profile_owner

let profile_desc t = t.profile_desc

let tco_like_type_hints t = t.tco_like_type_hints

let tco_union_intersection_type_hints t = t.tco_union_intersection_type_hints

let tco_like_casts t = t.tco_like_casts

let tco_simple_pessimize t = t.tco_simple_pessimize

let tco_complex_coercion t = t.tco_complex_coercion

let tco_disable_partially_abstract_typeconsts t =
  t.tco_disable_partially_abstract_typeconsts

let error_codes_treated_strictly t = t.error_codes_treated_strictly

let tco_check_xhp_attribute t = t.tco_check_xhp_attribute

let tco_check_redundant_generics t = t.tco_check_redundant_generics

let tco_disallow_unresolved_type_variables t =
  t.tco_disallow_unresolved_type_variables

let po_enable_class_level_where_clauses t =
  t.po_enable_class_level_where_clauses

let po_disable_legacy_soft_typehints t = t.po_disable_legacy_soft_typehints

let po_disallowed_decl_fixmes t = t.po_disallowed_decl_fixmes

let po_allow_new_attribute_syntax t = t.po_allow_new_attribute_syntax

let tco_global_inference t = t.tco_global_inference

let tco_gi_reinfer_types t = t.tco_gi_reinfer_types

let tco_ordered_solving t = t.tco_ordered_solving

let tco_const_static_props t = t.tco_const_static_props

let po_disable_legacy_attribute_syntax t = t.po_disable_legacy_attribute_syntax

let tco_const_attribute t = t.tco_const_attribute

let po_const_default_func_args t = t.po_const_default_func_args

let po_disallow_silence t = t.po_disallow_silence

let po_abstract_static_props t = t.po_abstract_static_props

let po_disable_unset_class_const t = t.po_disable_unset_class_const

let tco_check_attribute_locations t = t.tco_check_attribute_locations

let glean_service t = t.glean_service

let glean_hostname t = t.glean_hostname

let glean_port t = t.glean_port

let glean_reponame t = t.glean_reponame

let symbol_write_root_path t = t.symbol_write_root_path

let symbol_write_hhi_path t = t.symbol_write_hhi_path

let set_global_inference t = { t with tco_global_inference = true }

let set_ordered_solving t b = { t with tco_ordered_solving = b }

let po_parser_errors_only t = t.po_parser_errors_only

let po_disallow_func_ptrs_in_constants t = t.po_disallow_func_ptrs_in_constants

let tco_error_php_lambdas t = t.tco_error_php_lambdas

let tco_disallow_discarded_nullable_awaitables t =
  t.tco_disallow_discarded_nullable_awaitables

let po_enable_xhp_class_modifier t = t.po_enable_xhp_class_modifier

let po_disable_xhp_element_mangling t = t.po_disable_xhp_element_mangling

let po_disable_xhp_children_declarations t =
  t.po_disable_xhp_children_declarations

let po_enable_first_class_function_pointers t =
  t.po_enable_first_class_function_pointers

let po_disable_modes t = t.po_disable_modes

let po_disable_array t = t.po_disable_array
