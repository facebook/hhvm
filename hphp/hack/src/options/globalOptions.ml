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
  tco_num_local_workers: int option;
  tco_parallel_type_checking_threshold: int;
  tco_max_typechecker_worker_memory_mb: int option;
  tco_defer_class_declaration_threshold: int option;
  tco_defer_class_memory_mb_threshold: int option;
  tco_max_times_to_defer_type_checking: int option;
  tco_prefetch_deferred_files: bool;
  tco_remote_type_check_threshold: int option;
  tco_remote_type_check: bool;
  tco_remote_worker_key: string option;
  tco_remote_check_id: string option;
  tco_remote_max_batch_size: int;
  tco_remote_min_batch_size: int;
  tco_num_remote_workers: int;
  tco_stream_errors: bool;
  so_remote_version_specifier: string option;
  so_remote_worker_vfs_checkout_threshold: int;
  so_naming_sqlite_path: string option;
  po_auto_namespace_map: (string * string) list;
  po_codegen: bool;
  po_deregister_php_stdlib: bool;
  po_disallow_toplevel_requires: bool;
  po_disable_nontoplevel_declarations: bool;
  po_allow_unstable_features: bool;
  tco_log_inference_constraints: bool;
  tco_disallow_array_typehint: bool;
  tco_disallow_array_literal: bool;
  tco_language_feature_logging: bool;
  tco_disallow_scrutinee_case_value_type_mismatch: bool;
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
  po_rust_parser_errors: bool;
  tco_like_type_hints: bool;
  tco_union_intersection_type_hints: bool;
  tco_coeffects: bool;
  tco_coeffects_local: bool;
  tco_strict_contexts: bool;
  tco_like_casts: bool;
  tco_simple_pessimize: float;
  tco_complex_coercion: bool;
  tco_disable_partially_abstract_typeconsts: bool;
  tco_disallow_partially_abstract_typeconst_definitions: bool;
  error_codes_treated_strictly: ISet.t;
  tco_check_xhp_attribute: bool;
  tco_check_redundant_generics: bool;
  tco_disallow_unresolved_type_variables: bool;
  tco_disallow_trait_reuse: bool;
  tco_disallow_invalid_arraykey_constraint: bool;
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
  po_disable_unset_class_const: bool;
  po_parser_errors_only: bool;
  tco_check_attribute_locations: bool;
  glean_service: string;
  glean_hostname: string;
  glean_port: int;
  glean_reponame: string;
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
  po_disable_modes: bool;
  po_disable_hh_ignore_error: bool;
  po_disable_array: bool;
  po_disable_array_typehint: bool;
  tco_enable_systemlib_annotations: bool;
  tco_higher_kinded_types: bool;
  tco_method_call_inference: bool;
  tco_report_pos_from_reason: bool;
  tco_typecheck_sample_rate: float;
  tco_enable_sound_dynamic: bool;
  po_disallow_hash_comments: bool;
  po_disallow_fun_and_cls_meth_pseudo_funcs: bool;
  po_disallow_inst_meth: bool;
  po_escape_brace: bool;
  tco_use_direct_decl_parser: bool;
  tco_ifc_enabled: string list;
  po_enable_enum_supertyping: bool;
  po_interpret_soft_types_as_like_types: bool;
  tco_enable_strict_string_concat_interp: bool;
  tco_ignore_unsafe_cast: bool;
  tco_readonly: bool;
  tco_enable_expression_trees: bool;
  tco_enable_modules: bool;
  tco_allowed_expression_tree_visitors: string list;
  tco_math_new_code: bool;
  tco_typeconst_concrete_concrete_error: bool;
  tco_meth_caller_only_public_visibility: bool;
  tco_require_extends_implements_ancestors: bool;
  tco_strict_value_equality: bool;
  tco_enforce_sealed_subclasses: bool;
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
 * Prevent type param names from shadowing class names
 *)
let tco_experimental_type_param_shadowing = "type_param_shadowing"

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

(*
* Allow typechecker to raise error when inheriting members
* that differ only by case
*)
let tco_experimental_case_sensitive_inheritance = "case_sensitive_inheritance"

let tco_experimental_all =
  SSet.empty
  |> List.fold_right
       SSet.add
       [
         tco_experimental_generics_arity;
         tco_experimental_forbid_nullable_cast;
         tco_experimental_disallow_static_memoized;
         tco_experimental_abstract_type_const_with_default;
         tco_experimental_infer_flows;
         tco_experimental_case_sensitive_inheritance;
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
    tco_num_local_workers = None;
    tco_parallel_type_checking_threshold = 10;
    tco_max_typechecker_worker_memory_mb = None;
    tco_defer_class_declaration_threshold = None;
    tco_defer_class_memory_mb_threshold = None;
    tco_max_times_to_defer_type_checking = None;
    tco_prefetch_deferred_files = false;
    tco_remote_type_check_threshold = None;
    tco_remote_type_check = true;
    tco_remote_worker_key = None;
    tco_remote_check_id = None;
    tco_remote_max_batch_size = 8_000;
    tco_remote_min_batch_size = 5_000;
    tco_num_remote_workers = 4;
    tco_stream_errors = false;
    so_remote_version_specifier = None;
    so_remote_worker_vfs_checkout_threshold = 10000;
    so_naming_sqlite_path = None;
    po_auto_namespace_map = [];
    po_codegen = false;
    po_disallow_toplevel_requires = false;
    po_deregister_php_stdlib = false;
    po_disable_nontoplevel_declarations = false;
    po_allow_unstable_features = false;
    tco_log_inference_constraints = false;
    tco_disallow_array_typehint = false;
    tco_disallow_array_literal = false;
    tco_language_feature_logging = false;
    tco_disallow_scrutinee_case_value_type_mismatch = false;
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
    po_rust_parser_errors = false;
    tco_like_type_hints = false;
    tco_union_intersection_type_hints = false;
    tco_coeffects = true;
    tco_coeffects_local = true;
    tco_strict_contexts = true;
    tco_like_casts = false;
    tco_simple_pessimize = 0.0;
    tco_complex_coercion = false;
    tco_disable_partially_abstract_typeconsts = false;
    tco_disallow_partially_abstract_typeconst_definitions = false;
    error_codes_treated_strictly = ISet.of_list [];
    tco_check_xhp_attribute = false;
    tco_check_redundant_generics = false;
    tco_disallow_unresolved_type_variables = false;
    tco_disallow_trait_reuse = false;
    tco_disallow_invalid_arraykey_constraint = false;
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
    po_disable_unset_class_const = false;
    po_parser_errors_only = false;
    tco_check_attribute_locations = true;
    glean_service = "";
    glean_hostname = "";
    glean_port = 0;
    glean_reponame = "www.autocomplete";
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
    po_disable_modes = false;
    po_disable_hh_ignore_error = false;
    po_disable_array = true;
    po_disable_array_typehint = true;
    tco_enable_systemlib_annotations = false;
    tco_higher_kinded_types = false;
    tco_method_call_inference = false;
    tco_report_pos_from_reason = false;
    tco_typecheck_sample_rate = 1.0;
    tco_enable_sound_dynamic = false;
    po_disallow_hash_comments = false;
    po_disallow_fun_and_cls_meth_pseudo_funcs = false;
    po_disallow_inst_meth = false;
    po_escape_brace = false;
    tco_use_direct_decl_parser = false;
    tco_ifc_enabled = [];
    po_enable_enum_supertyping = false;
    po_interpret_soft_types_as_like_types = false;
    tco_enable_strict_string_concat_interp = false;
    tco_ignore_unsafe_cast = false;
    tco_readonly = false;
    tco_enable_expression_trees = false;
    tco_enable_modules = false;
    tco_allowed_expression_tree_visitors = [];
    tco_math_new_code = false;
    tco_typeconst_concrete_concrete_error = false;
    tco_meth_caller_only_public_visibility = true;
    tco_require_extends_implements_ancestors = false;
    tco_strict_value_equality = false;
    tco_enforce_sealed_subclasses = false;
  }

let make
    ?(po_deregister_php_stdlib = default.po_deregister_php_stdlib)
    ?(po_disallow_toplevel_requires = default.po_disallow_toplevel_requires)
    ?(po_disable_nontoplevel_declarations =
      default.po_disable_nontoplevel_declarations)
    ?(tco_log_inference_constraints = default.tco_log_inference_constraints)
    ?(tco_experimental_features = default.tco_experimental_features)
    ?(tco_migration_flags = default.tco_migration_flags)
    ?(tco_dynamic_view = default.tco_dynamic_view)
    ?tco_num_local_workers
    ?(tco_parallel_type_checking_threshold =
      default.tco_parallel_type_checking_threshold)
    ?tco_max_typechecker_worker_memory_mb
    ?tco_defer_class_declaration_threshold
    ?tco_defer_class_memory_mb_threshold
    ?tco_max_times_to_defer_type_checking
    ?(tco_prefetch_deferred_files = default.tco_prefetch_deferred_files)
    ?tco_remote_type_check_threshold
    ?(tco_remote_type_check = default.tco_remote_type_check)
    ?tco_remote_worker_key
    ?tco_remote_check_id
    ?(tco_remote_max_batch_size = default.tco_remote_max_batch_size)
    ?(tco_remote_min_batch_size = default.tco_remote_min_batch_size)
    ?(tco_num_remote_workers = default.tco_num_remote_workers)
    ?(tco_stream_errors = default.tco_stream_errors)
    ?so_remote_version_specifier
    ?(so_remote_worker_vfs_checkout_threshold =
      default.so_remote_worker_vfs_checkout_threshold)
    ?so_naming_sqlite_path
    ?(po_auto_namespace_map = default.po_auto_namespace_map)
    ?(tco_disallow_array_typehint = default.tco_disallow_array_typehint)
    ?(tco_disallow_array_literal = default.tco_disallow_array_literal)
    ?(tco_language_feature_logging = default.tco_language_feature_logging)
    ?(tco_disallow_scrutinee_case_value_type_mismatch =
      default.tco_disallow_scrutinee_case_value_type_mismatch)
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
    ?(po_rust_parser_errors = default.po_rust_parser_errors)
    ?(tco_like_type_hints = default.tco_like_type_hints)
    ?(tco_union_intersection_type_hints =
      default.tco_union_intersection_type_hints)
    ?(tco_coeffects = default.tco_coeffects)
    ?(tco_coeffects_local = default.tco_coeffects_local)
    ?(tco_strict_contexts = default.tco_strict_contexts)
    ?(tco_like_casts = default.tco_like_casts)
    ?(tco_simple_pessimize = default.tco_simple_pessimize)
    ?(tco_complex_coercion = default.tco_complex_coercion)
    ?(tco_disable_partially_abstract_typeconsts =
      default.tco_disable_partially_abstract_typeconsts)
    ?(tco_disallow_partially_abstract_typeconst_definitions =
      default.tco_disallow_partially_abstract_typeconst_definitions)
    ?(error_codes_treated_strictly = default.error_codes_treated_strictly)
    ?(tco_check_xhp_attribute = default.tco_check_xhp_attribute)
    ?(tco_check_redundant_generics = default.tco_check_redundant_generics)
    ?(tco_disallow_unresolved_type_variables =
      default.tco_disallow_unresolved_type_variables)
    ?(tco_disallow_trait_reuse = default.tco_disallow_trait_reuse)
    ?(tco_disallow_invalid_arraykey_constraint =
      default.tco_disallow_invalid_arraykey_constraint)
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
    ?(po_disable_unset_class_const = default.po_disable_unset_class_const)
    ?(po_parser_errors_only = default.po_parser_errors_only)
    ?(tco_check_attribute_locations = default.tco_check_attribute_locations)
    ?(glean_service = default.glean_service)
    ?(glean_hostname = default.glean_hostname)
    ?(glean_port = default.glean_port)
    ?(glean_reponame = default.glean_reponame)
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
    ?(po_disable_modes = default.po_disable_modes)
    ?(po_disable_hh_ignore_error = default.po_disable_hh_ignore_error)
    ?(po_disable_array = default.po_disable_array)
    ?(po_disable_array_typehint = default.po_disable_array_typehint)
    ?(po_allow_unstable_features = default.po_allow_unstable_features)
    ?(tco_enable_systemlib_annotations =
      default.tco_enable_systemlib_annotations)
    ?(tco_higher_kinded_types = default.tco_higher_kinded_types)
    ?(tco_method_call_inference = default.tco_method_call_inference)
    ?(tco_report_pos_from_reason = default.tco_report_pos_from_reason)
    ?(tco_typecheck_sample_rate = default.tco_typecheck_sample_rate)
    ?(tco_enable_sound_dynamic = default.tco_enable_sound_dynamic)
    ?(po_disallow_hash_comments = default.po_disallow_hash_comments)
    ?(po_disallow_fun_and_cls_meth_pseudo_funcs =
      default.po_disallow_fun_and_cls_meth_pseudo_funcs)
    ?(po_disallow_inst_meth = default.po_disallow_inst_meth)
    ?(po_escape_brace = default.po_escape_brace)
    ?(tco_use_direct_decl_parser = default.tco_use_direct_decl_parser)
    ?(tco_ifc_enabled = default.tco_ifc_enabled)
    ?(po_enable_enum_supertyping = default.po_enable_enum_supertyping)
    ?(po_interpret_soft_types_as_like_types =
      default.po_interpret_soft_types_as_like_types)
    ?(tco_enable_strict_string_concat_interp =
      default.tco_enable_strict_string_concat_interp)
    ?(tco_ignore_unsafe_cast = default.tco_ignore_unsafe_cast)
    ?(tco_readonly = default.tco_readonly)
    ?(tco_enable_expression_trees = default.tco_enable_expression_trees)
    ?(tco_enable_modules = default.tco_enable_modules)
    ?(tco_allowed_expression_tree_visitors =
      default.tco_allowed_expression_tree_visitors)
    ?(tco_math_new_code = default.tco_math_new_code)
    ?(tco_typeconst_concrete_concrete_error =
      default.tco_typeconst_concrete_concrete_error)
    ?(tco_meth_caller_only_public_visibility =
      default.tco_meth_caller_only_public_visibility)
    ?(tco_require_extends_implements_ancestors =
      default.tco_require_extends_implements_ancestors)
    ?(tco_strict_value_equality = default.tco_strict_value_equality)
    ?(tco_enforce_sealed_subclasses = default.tco_enforce_sealed_subclasses)
    () =
  {
    tco_experimental_features;
    tco_migration_flags;
    tco_dynamic_view;
    tco_num_local_workers;
    tco_parallel_type_checking_threshold;
    tco_max_typechecker_worker_memory_mb;
    tco_defer_class_declaration_threshold;
    tco_defer_class_memory_mb_threshold;
    tco_max_times_to_defer_type_checking;
    tco_prefetch_deferred_files;
    tco_remote_type_check_threshold;
    tco_remote_type_check;
    tco_remote_worker_key;
    tco_remote_check_id;
    tco_remote_max_batch_size;
    tco_remote_min_batch_size;
    tco_num_remote_workers;
    tco_stream_errors;
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
    po_disable_nontoplevel_declarations;
    po_allow_unstable_features;
    tco_log_inference_constraints;
    tco_disallow_array_typehint;
    tco_disallow_array_literal;
    tco_language_feature_logging;
    tco_disallow_scrutinee_case_value_type_mismatch;
    tco_timeout;
    tco_disallow_invalid_arraykey;
    tco_disallow_byref_dynamic_calls;
    tco_disallow_byref_calls;
    log_levels;
    po_disable_lval_as_an_expression;
    tco_shallow_class_decl;
    po_rust_parser_errors;
    tco_like_type_hints;
    tco_union_intersection_type_hints;
    tco_coeffects;
    tco_coeffects_local;
    tco_strict_contexts;
    tco_like_casts;
    tco_simple_pessimize;
    tco_complex_coercion;
    tco_disable_partially_abstract_typeconsts;
    tco_disallow_partially_abstract_typeconst_definitions;
    error_codes_treated_strictly;
    tco_check_xhp_attribute;
    tco_check_redundant_generics;
    tco_disallow_unresolved_type_variables;
    tco_disallow_trait_reuse;
    tco_disallow_invalid_arraykey_constraint;
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
    po_disable_unset_class_const;
    po_parser_errors_only;
    tco_check_attribute_locations;
    glean_service;
    glean_hostname;
    glean_port;
    glean_reponame;
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
    po_disable_modes;
    po_disable_hh_ignore_error;
    po_disable_array;
    po_disable_array_typehint;
    tco_enable_systemlib_annotations;
    tco_higher_kinded_types;
    tco_method_call_inference;
    tco_report_pos_from_reason;
    tco_typecheck_sample_rate;
    tco_enable_sound_dynamic;
    po_disallow_hash_comments;
    po_disallow_fun_and_cls_meth_pseudo_funcs;
    po_disallow_inst_meth;
    po_escape_brace;
    tco_use_direct_decl_parser;
    tco_ifc_enabled;
    po_enable_enum_supertyping;
    po_interpret_soft_types_as_like_types;
    tco_enable_strict_string_concat_interp;
    tco_ignore_unsafe_cast;
    tco_readonly;
    tco_enable_expression_trees;
    tco_enable_modules;
    tco_allowed_expression_tree_visitors;
    tco_math_new_code;
    tco_typeconst_concrete_concrete_error;
    tco_meth_caller_only_public_visibility;
    tco_require_extends_implements_ancestors;
    tco_strict_value_equality;
    tco_enforce_sealed_subclasses;
  }

let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features

let tco_migration_flag_enabled t s = SSet.mem s t.tco_migration_flags

let tco_dynamic_view t = t.tco_dynamic_view

let tco_num_local_workers t = t.tco_num_local_workers

let tco_parallel_type_checking_threshold t =
  t.tco_parallel_type_checking_threshold

let tco_max_typechecker_worker_memory_mb t =
  t.tco_max_typechecker_worker_memory_mb

let tco_defer_class_declaration_threshold t =
  t.tco_defer_class_declaration_threshold

let tco_defer_class_memory_mb_threshold t =
  t.tco_defer_class_memory_mb_threshold

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

let tco_stream_errors t = t.tco_stream_errors

let so_remote_version_specifier t = t.so_remote_version_specifier

let so_remote_worker_vfs_checkout_threshold t =
  t.so_remote_worker_vfs_checkout_threshold

let so_naming_sqlite_path t = t.so_naming_sqlite_path

let po_auto_namespace_map t = t.po_auto_namespace_map

let po_deregister_php_stdlib t = t.po_deregister_php_stdlib

let po_disable_nontoplevel_declarations t =
  t.po_disable_nontoplevel_declarations

let tco_log_inference_constraints t = t.tco_log_inference_constraints

let po_codegen t = t.po_codegen

let po_disallow_toplevel_requires t = t.po_disallow_toplevel_requires

let tco_disallow_array_typehint t = t.tco_disallow_array_typehint

let tco_disallow_array_literal t = t.tco_disallow_array_literal

let tco_language_feature_logging t = t.tco_language_feature_logging

let tco_disallow_scrutinee_case_value_type_mismatch t =
  t.tco_disallow_scrutinee_case_value_type_mismatch

let tco_timeout t = t.tco_timeout

let tco_disallow_invalid_arraykey t = t.tco_disallow_invalid_arraykey

let tco_disallow_invalid_arraykey_constraint t =
  t.tco_disallow_invalid_arraykey_constraint

let tco_disallow_byref_dynamic_calls t = t.tco_disallow_byref_dynamic_calls

let tco_disallow_byref_calls t = t.tco_disallow_byref_calls

let allowed_fixme_codes_strict t = t.allowed_fixme_codes_strict

let allowed_fixme_codes_partial t = t.allowed_fixme_codes_partial

let codes_not_raised_partial t = t.codes_not_raised_partial

let log_levels t = t.log_levels

let po_disable_lval_as_an_expression t = t.po_disable_lval_as_an_expression

let tco_shallow_class_decl t = t.tco_shallow_class_decl

let po_rust_parser_errors t = t.po_rust_parser_errors

let tco_like_type_hints t = t.tco_like_type_hints

let tco_union_intersection_type_hints t = t.tco_union_intersection_type_hints

let tco_call_coeffects t = t.tco_coeffects

let tco_local_coeffects t = t.tco_coeffects_local

let tco_strict_contexts t = t.tco_strict_contexts

let ifc_enabled t = t.tco_ifc_enabled

(* Fully enable IFC on the tcopt *)
let enable_ifc t = { t with tco_ifc_enabled = ["/"] }

let tco_like_casts t = t.tco_like_casts

let tco_simple_pessimize t = t.tco_simple_pessimize

let tco_complex_coercion t = t.tco_complex_coercion

let tco_disable_partially_abstract_typeconsts t =
  t.tco_disable_partially_abstract_typeconsts

let tco_disallow_partially_abstract_typeconst_definitions t =
  t.tco_disallow_partially_abstract_typeconst_definitions

let error_codes_treated_strictly t = t.error_codes_treated_strictly

let tco_check_xhp_attribute t = t.tco_check_xhp_attribute

let tco_check_redundant_generics t = t.tco_check_redundant_generics

let tco_disallow_unresolved_type_variables t =
  t.tco_disallow_unresolved_type_variables

let tco_disallow_trait_reuse t = t.tco_disallow_trait_reuse

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

let po_disable_unset_class_const t = t.po_disable_unset_class_const

let tco_check_attribute_locations t = t.tco_check_attribute_locations

let glean_service t = t.glean_service

let glean_hostname t = t.glean_hostname

let glean_port t = t.glean_port

let glean_reponame t = t.glean_reponame

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

let set_tco_readonly t b = { t with tco_readonly = b }

let tco_readonly t = t.tco_readonly

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

let po_disable_modes t = t.po_disable_modes

let po_disable_hh_ignore_error t = t.po_disable_hh_ignore_error

let po_disable_array t = t.po_disable_array

let po_disable_array_typehint t = t.po_disable_array_typehint

let tco_enable_systemlib_annotations t = t.tco_enable_systemlib_annotations

let tco_higher_kinded_types t = t.tco_higher_kinded_types

let tco_method_call_inference t = t.tco_method_call_inference

let tco_report_pos_from_reason t = t.tco_report_pos_from_reason

let tco_typecheck_sample_rate t = t.tco_typecheck_sample_rate

let tco_enable_sound_dynamic t = t.tco_enable_sound_dynamic

let po_disallow_hash_comments t = t.po_disallow_hash_comments

let po_disallow_fun_and_cls_meth_pseudo_funcs t =
  t.po_disallow_fun_and_cls_meth_pseudo_funcs

let po_disallow_inst_meth t = t.po_disallow_inst_meth

let po_escape_brace t = t.po_escape_brace

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

let tco_meth_caller_only_public_visibility t =
  t.tco_meth_caller_only_public_visibility

let tco_require_extends_implements_ancestors t =
  t.tco_require_extends_implements_ancestors

let tco_strict_value_equality t = t.tco_strict_value_equality

let tco_enforce_sealed_subclasses t = t.tco_enforce_sealed_subclasses
