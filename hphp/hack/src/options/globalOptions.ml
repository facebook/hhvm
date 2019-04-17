(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  tco_safe_array : bool;
  tco_safe_vector_array : bool;
  tco_experimental_features : SSet.t;
  tco_migration_flags : SSet.t;
  tco_dynamic_view : bool;
  tco_disallow_array_as_tuple : bool;
  po_auto_namespace_map : (string * string) list;
  po_enable_hh_syntax_for_hhvm : bool;
  po_deregister_php_stdlib : bool;
  po_disallow_execution_operator : bool;
  po_disable_nontoplevel_declarations : bool;
  po_disable_static_closures : bool;
  po_allow_goto: bool;
  po_enable_concurrent : bool;
  po_enable_await_as_an_expression : bool;
  tco_log_inference_constraints : bool;
  tco_disallow_ambiguous_lambda : bool;
  tco_disallow_array_typehint: bool;
  tco_disallow_array_literal: bool;
  tco_language_feature_logging : bool;
  tco_unsafe_rx : bool;
  tco_disallow_implicit_returns_in_non_void_functions : bool;
  tco_disallow_unset_on_varray: bool;
  tco_disallow_scrutinee_case_value_type_mismatch : bool;
  tco_disallow_stringish_magic : bool;
  tco_disallow_anon_use_capture_by_ref : bool;
  tco_new_inference : float;
  tco_new_inference_lambda : bool;
  tco_timeout : int;
  tco_disallow_invalid_arraykey : bool;
  tco_disable_instanceof_refinement : bool;
  tco_disallow_ref_param_on_constructor : bool;
  tco_disallow_byref_dynamic_calls : bool;
  po_disable_instanceof : bool;
  ignored_fixme_codes : ISet.t;
  ignored_fixme_regex : string option;
  log_levels : int SMap.t;
  po_enable_stronger_await_binding : bool;
  po_disable_lval_as_an_expression : bool;
  po_disable_unsafe_expr : bool;
  po_disable_unsafe_block : bool;
  tco_typecheck_xhp_cvars : bool;
  tco_ignore_collection_expr_type_arguments : bool;
  tco_disallow_byref_prop_args : bool;
  tco_shallow_class_decl : bool;
} [@@deriving show]

let tco_experimental_instanceof = "instanceof"

let tco_experimental_isarray = "is_array"

let tco_experimental_goto = "goto"

(**
 * Prevents arraus from being promoted to shape-like or tuple-like arrays.
 *)
let tco_experimental_disable_shape_and_tuple_arrays =
  "disable_shape_and_tuple_arrays"

(* Whether Shapes::idx should return a non-nullable type when the input shape
    is known to contain the field. *)
let tco_experimental_stronger_shape_idx_ret =
    "stronger_shape_idx_return"

(* Whether subtype assertions of form TUnresolved[] <: t should be added to
 * todo list and checked later (usually generated from contravariant types) *)
let tco_experimental_unresolved_fix =
  "unresolved_fix"

(**
 * Allows parsing type hints for function calls, such as foo<int>(args);.
 *)
let tco_experimental_annotate_function_calls =
  "annotate_function_calls"

(**
 * Insist on instantiations for all generic types, even in non-strict files
 *)
let tco_experimental_generics_arity =
  "generics_arity"

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
let tco_experimental_coroutines =
  "coroutines"

(**
 * Disables optional and unknown shape fields syntax and typechecking.
 *
 * Please see the public documentation at
 * http://hhvm.com/blog/2017/09/26/hhvm-3-22.html#nullable-vs-optional-fields-in-shapes
 * to learn more about optional and unknown shape fields.
 *
 * When enabled, this flag results in the following behavior:
 *
 *   1. Shape fields with a nullable type are allowed to be omitted.
 *   2. All shapes are considered to support unknown fields, whether or not
 *      their last declared field is '...'.
 *
 * This temporary flag exists for backwards-compatiblity purposes, and will be
 * removed in a future release.
 *)
let tco_experimental_disable_optional_and_unknown_shape_fields =
  "disable_optional_and_unknown_shape_fields"

(**
 * Enforce no duplication of traits in a class hierarchy. There is some clean-up
 * involved, so it's behind a flag.
 *)
let tco_experimental_no_trait_reuse = "no_trait_reuse"

(**
 * Enable reified generics
 *)
let tco_experimental_reified_generics = "reified_generics"

(**
 * Prevent type param names from shadowing class names
 *)
let tco_experimental_type_param_shadowing = "type_param_shadowing"

(**
 * Enable trait method redeclarations, i.e. public function f(): void = T1::f;
 *)
let tco_experimental_trait_method_redeclarations = "trait_method_redeclarations"

(**
 * Enable attributes on type constants
 *)
let tco_experimental_type_const_attributes = "type_const_attributes"

(**
 * Enable declaration linearization
 *)
let tco_experimental_decl_linearization = "decl_linearization"

(**
 * Enable keeping track of the current subtype proposition in the environment.
 *)
let tco_experimental_track_subtype_prop = "track_subtype_prop"

(**
 * Enable like-types
 *)
let tco_experimental_like_types = "like_types"

(**
 * Enable support for the Pocket Universes
 *)
let tco_experimental_pocket_universes = "pocket_universes"

(**
 * Enable abstract const type with default syntax, i.e.
 * abstract const type T as num = int;
 *)
let tco_experimental_abstract_type_const_with_default = "abstract_type_const_with_default"

let tco_experimental_all =
 SSet.empty |> List.fold_right SSet.add
   [
     tco_experimental_instanceof;
     tco_experimental_isarray;
     tco_experimental_goto;
     tco_experimental_disable_shape_and_tuple_arrays;
     tco_experimental_stronger_shape_idx_ret;
     tco_experimental_annotate_function_calls;
     tco_experimental_unresolved_fix;
     tco_experimental_generics_arity;
     tco_experimental_forbid_nullable_cast;
     tco_experimental_coroutines;
     tco_experimental_disallow_static_memoized;
     tco_experimental_disable_optional_and_unknown_shape_fields;
     tco_experimental_no_trait_reuse;
     tco_experimental_reified_generics;
     tco_experimental_type_param_shadowing;
     tco_experimental_trait_method_redeclarations;
     tco_experimental_type_const_attributes;
     tco_experimental_decl_linearization;
     tco_experimental_track_subtype_prop;
     tco_experimental_abstract_type_const_with_default;
     tco_experimental_like_types;
   ]

let tco_migration_flags_all =
  SSet.empty |> List.fold_right SSet.add
    [
      "array_cast"
    ]

let default = {
 tco_safe_array = true;
 tco_safe_vector_array = true;
 (** Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
 tco_experimental_features = tco_experimental_all;
 tco_migration_flags = SSet.empty;
 tco_dynamic_view = false;
 tco_disallow_array_as_tuple = false;
 po_auto_namespace_map = [];
 po_enable_hh_syntax_for_hhvm = false;
 po_disallow_execution_operator = false;
 po_deregister_php_stdlib = false;
 po_disable_nontoplevel_declarations = false;
 po_disable_static_closures = false;
 po_allow_goto = true;
 po_enable_concurrent = false;
 po_enable_await_as_an_expression = false;
 tco_log_inference_constraints = false;
 tco_disallow_ambiguous_lambda = false;
 tco_disallow_array_typehint = false;
 tco_disallow_array_literal = false;
 tco_language_feature_logging = false;
 tco_unsafe_rx = true;
 tco_disallow_implicit_returns_in_non_void_functions = true;
 tco_disallow_unset_on_varray = false;
 tco_disallow_scrutinee_case_value_type_mismatch = false;
 tco_disallow_stringish_magic = false;
 tco_disallow_anon_use_capture_by_ref = false;
 tco_new_inference = 0.0;
 tco_new_inference_lambda = false;
 tco_timeout = 0;
 tco_disallow_invalid_arraykey = false;
 tco_disable_instanceof_refinement = false;
 tco_disallow_ref_param_on_constructor = false;
 tco_disallow_byref_dynamic_calls = false;
 po_disable_instanceof = false;
 ignored_fixme_codes = Errors.default_ignored_fixme_codes;
 ignored_fixme_regex = None;
 log_levels = SMap.empty;
 po_enable_stronger_await_binding = false;
 po_disable_lval_as_an_expression = false;
 po_disable_unsafe_expr = false;
 po_disable_unsafe_block = false;
 tco_typecheck_xhp_cvars = false;
 tco_ignore_collection_expr_type_arguments = false;
 tco_disallow_byref_prop_args = false;
 tco_shallow_class_decl = false;
}

let make
  ?(tco_safe_array = default.tco_safe_array)
  ?(tco_safe_vector_array = default.tco_safe_vector_array)
  ?(po_deregister_php_stdlib = default.po_deregister_php_stdlib)
  ?(po_disallow_execution_operator = default.po_disallow_execution_operator)
  ?(po_disable_nontoplevel_declarations = default.po_disable_nontoplevel_declarations)
  ?(po_disable_static_closures = default.po_disable_static_closures)
  ?(po_allow_goto = default.po_allow_goto)
  ?(po_enable_concurrent = default.po_enable_concurrent)
  ?(po_enable_await_as_an_expression = default.po_enable_await_as_an_expression)
  ?(tco_log_inference_constraints = default.tco_log_inference_constraints)
  ?(tco_experimental_features = default.tco_experimental_features)
  ?(tco_migration_flags = default.tco_migration_flags)
  ?(tco_dynamic_view = default.tco_dynamic_view)
  ?(tco_disallow_array_as_tuple = default.tco_disallow_array_as_tuple)
  ?(po_auto_namespace_map = default.po_auto_namespace_map)
  ?(tco_disallow_ambiguous_lambda = default.tco_disallow_ambiguous_lambda)
  ?(tco_disallow_array_typehint = default.tco_disallow_array_typehint)
  ?(tco_disallow_array_literal = default.tco_disallow_array_literal)
  ?(tco_language_feature_logging = default.tco_language_feature_logging)
  ?(tco_unsafe_rx = default.tco_unsafe_rx)
  ?(tco_disallow_implicit_returns_in_non_void_functions = default.tco_disallow_implicit_returns_in_non_void_functions)
  ?(tco_disallow_unset_on_varray = default.tco_disallow_unset_on_varray)
  ?(tco_disallow_scrutinee_case_value_type_mismatch = default.tco_disallow_scrutinee_case_value_type_mismatch)
  ?(tco_disallow_stringish_magic = default.tco_disallow_stringish_magic)
  ?(tco_disallow_anon_use_capture_by_ref = default.tco_disallow_anon_use_capture_by_ref)
  ?(tco_new_inference = default.tco_new_inference)
  ?(tco_new_inference_lambda = default.tco_new_inference_lambda)
  ?(tco_timeout = default.tco_timeout)
  ?(tco_disallow_invalid_arraykey = default.tco_disallow_invalid_arraykey)
  ?(tco_disable_instanceof_refinement = default.tco_disable_instanceof_refinement)
  ?(tco_disallow_ref_param_on_constructor = default.tco_disallow_ref_param_on_constructor)
  ?(tco_disallow_byref_dynamic_calls = default.tco_disallow_byref_dynamic_calls)
  ?(po_disable_instanceof = default.po_disable_instanceof)
  ?(ignored_fixme_codes = default.ignored_fixme_codes)
  ?ignored_fixme_regex
  ?(log_levels = default.log_levels)
  ?(po_enable_stronger_await_binding = default.po_enable_stronger_await_binding)
  ?(po_disable_lval_as_an_expression = default.po_disable_lval_as_an_expression)
  ?(po_disable_unsafe_expr = default.po_disable_unsafe_expr)
  ?(po_disable_unsafe_block = default.po_disable_unsafe_block)
  ?(tco_typecheck_xhp_cvars = default.tco_typecheck_xhp_cvars)
  ?(tco_ignore_collection_expr_type_arguments = default.tco_ignore_collection_expr_type_arguments)
  ?(tco_disallow_byref_prop_args = default.tco_disallow_byref_prop_args)
  ?(tco_shallow_class_decl = default.tco_shallow_class_decl)
  ()
= {
  tco_safe_array;
  tco_safe_vector_array;
  tco_experimental_features;
  tco_migration_flags;
  tco_dynamic_view;
  tco_disallow_array_as_tuple;
  po_auto_namespace_map;
  po_enable_hh_syntax_for_hhvm = false;
  ignored_fixme_codes;
  ignored_fixme_regex;
  po_deregister_php_stdlib;
  po_disallow_execution_operator;
  po_disable_nontoplevel_declarations;
  po_disable_static_closures;
  po_allow_goto;
  po_enable_concurrent;
  po_enable_await_as_an_expression;
  tco_log_inference_constraints;
  tco_disallow_ambiguous_lambda;
  tco_disallow_array_typehint;
  tco_disallow_array_literal;
  tco_language_feature_logging;
  tco_unsafe_rx;
  tco_disallow_implicit_returns_in_non_void_functions;
  tco_disallow_unset_on_varray;
  tco_disallow_scrutinee_case_value_type_mismatch;
  tco_disallow_stringish_magic;
  tco_disallow_anon_use_capture_by_ref;
  tco_new_inference;
  tco_new_inference_lambda;
  tco_timeout;
  tco_disallow_invalid_arraykey;
  tco_disable_instanceof_refinement;
  tco_disallow_ref_param_on_constructor;
  tco_disallow_byref_dynamic_calls;
  po_disable_instanceof;
  log_levels;
  po_enable_stronger_await_binding;
  po_disable_lval_as_an_expression;
  po_disable_unsafe_expr;
  po_disable_unsafe_block;
  tco_typecheck_xhp_cvars;
  tco_ignore_collection_expr_type_arguments;
  tco_disallow_byref_prop_args;
  tco_shallow_class_decl;
}
let tco_safe_array t = t.tco_safe_array
let tco_safe_vector_array t = t.tco_safe_vector_array
let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features
let tco_migration_flag_enabled t s =
  SSet.mem s t.tco_migration_flags
let tco_dynamic_view t =
  t.tco_dynamic_view
let tco_disallow_array_as_tuple t =
  t.tco_disallow_array_as_tuple
let po_auto_namespace_map t = t.po_auto_namespace_map
let po_deregister_php_stdlib t = t.po_deregister_php_stdlib
let po_disable_nontoplevel_declarations t = t.po_disable_nontoplevel_declarations
let po_disable_static_closures t = t.po_disable_static_closures
let po_allow_goto t = t.po_allow_goto
let po_enable_concurrent t = t.po_enable_concurrent
let po_enable_await_as_an_expression t = t.po_enable_await_as_an_expression
let tco_log_inference_constraints t = t.tco_log_inference_constraints
let po_enable_hh_syntax_for_hhvm t = t.po_enable_hh_syntax_for_hhvm
let po_disallow_execution_operator t = t.po_disallow_execution_operator
let tco_disallow_ambiguous_lambda t = t.tco_disallow_ambiguous_lambda
let tco_disallow_array_typehint t = t.tco_disallow_array_typehint
let tco_disallow_array_literal t = t.tco_disallow_array_literal
let tco_language_feature_logging t = t.tco_language_feature_logging
let tco_unsafe_rx t = t.tco_unsafe_rx
let tco_disallow_implicit_returns_in_non_void_functions t =
  t.tco_disallow_implicit_returns_in_non_void_functions
let tco_disallow_unset_on_varray t = t.tco_disallow_unset_on_varray
let tco_disallow_scrutinee_case_value_type_mismatch t =
  t.tco_disallow_scrutinee_case_value_type_mismatch
let tco_disallow_stringish_magic t = t.tco_disallow_stringish_magic
let tco_disallow_anon_use_capture_by_ref t = t.tco_disallow_anon_use_capture_by_ref
let tco_new_inference t = t.tco_new_inference > 0.0
let tco_new_inference_lambda t = t.tco_new_inference_lambda
let tco_timeout t = t.tco_timeout
let tco_disallow_invalid_arraykey t = t.tco_disallow_invalid_arraykey
let tco_disable_instanceof_refinement t = t.tco_disable_instanceof_refinement
let tco_disallow_ref_param_on_constructor t = t.tco_disallow_ref_param_on_constructor
let tco_disallow_byref_dynamic_calls t = t.tco_disallow_byref_dynamic_calls
let po_disable_instanceof t = t.po_disable_instanceof
let ignored_fixme_codes t = t.ignored_fixme_codes
let ignored_fixme_regex t = t.ignored_fixme_regex
let log_levels t = t.log_levels
let po_enable_stronger_await_binding t = t.po_enable_stronger_await_binding
let po_disable_lval_as_an_expression t = t.po_disable_lval_as_an_expression
let po_disable_unsafe_expr t = t.po_disable_unsafe_expr
let po_disable_unsafe_block t = t.po_disable_unsafe_block
let tco_typecheck_xhp_cvars t = t.tco_typecheck_xhp_cvars
let tco_disallow_byref_prop_args t = t.tco_disallow_byref_prop_args
let tco_shallow_class_decl t = t.tco_shallow_class_decl

let tco_ignore_collection_expr_type_arguments t = t.tco_ignore_collection_expr_type_arguments
let setup_pocket_universes env enabled =
  let exp_features = env.tco_experimental_features in
  let exp_features = if enabled then
      SSet.add tco_experimental_pocket_universes exp_features
    else
      SSet.remove tco_experimental_pocket_universes exp_features
  in { env with tco_experimental_features = exp_features }
