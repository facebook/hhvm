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
  tco_defer_class_declaration_threshold : int option;
  tco_disallow_array_as_tuple : bool;
  po_auto_namespace_map : (string * string) list;
  po_codegen : bool;
  po_deregister_php_stdlib : bool;
  po_disallow_execution_operator : bool;
  po_disable_nontoplevel_declarations : bool;
  po_disable_static_closures : bool;
  po_allow_goto: bool;
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
  tco_new_inference_lambda : bool;
  tco_timeout : int;
  tco_disallow_invalid_arraykey : bool;
  tco_disable_instanceof_refinement : bool;
  tco_disallow_byref_dynamic_calls : bool;
  po_disable_instanceof : bool;
  ignored_fixme_codes : ISet.t;
  ignored_fixme_regex : string option;
  log_levels : int SMap.t;
  po_disable_lval_as_an_expression : bool;
  tco_typecheck_xhp_cvars : bool;
  tco_ignore_collection_expr_type_arguments : bool;
  tco_shallow_class_decl : bool;
  po_rust : bool;
  tco_like_types : bool;
  tco_pessimize_types : bool;
  tco_coercion_from_dynamic : bool;
  tco_disable_partially_abstract_typeconsts: bool;
  error_codes_treated_strictly : ISet.t;
  tco_check_xhp_attribute : bool;
  tco_disallow_unresolved_type_variables : bool;
  tco_disallow_invalid_arraykey_constraint : bool;
  tico_invalidate_files : bool;
  tico_invalidate_smart : bool;
  po_enable_constant_visibility_modifiers : bool;
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

(* Whether subtype assertions of form Tunion[] <: t should be added to
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
     tco_experimental_no_trait_reuse;
     tco_experimental_reified_generics;
     tco_experimental_trait_method_redeclarations;
     tco_experimental_type_const_attributes;
     tco_experimental_decl_linearization;
     tco_experimental_track_subtype_prop;
     tco_experimental_abstract_type_const_with_default;
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
 tco_defer_class_declaration_threshold = None;
 tco_disallow_array_as_tuple = false;
 po_auto_namespace_map = [];
 po_codegen = false;
 po_disallow_execution_operator = false;
 po_deregister_php_stdlib = false;
 po_disable_nontoplevel_declarations = false;
 po_disable_static_closures = false;
 po_allow_goto = true;
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
 tco_new_inference_lambda = false;
 tco_timeout = 0;
 tco_disallow_invalid_arraykey = false;
 tco_disable_instanceof_refinement = false;
 tco_disallow_byref_dynamic_calls = false;
 po_disable_instanceof = false;
 ignored_fixme_codes = Errors.default_ignored_fixme_codes;
 ignored_fixme_regex = None;
 log_levels = SMap.empty;
 po_disable_lval_as_an_expression = false;
 tco_typecheck_xhp_cvars = false;
 tco_ignore_collection_expr_type_arguments = false;
 tco_shallow_class_decl = false;
 po_rust = false;
 tco_like_types = false;
 tco_pessimize_types = false;
 tco_coercion_from_dynamic = false;
 tco_disable_partially_abstract_typeconsts = false;
 error_codes_treated_strictly = ISet.of_list [];
 tco_check_xhp_attribute = false;
 tco_disallow_unresolved_type_variables = false;
 tco_disallow_invalid_arraykey_constraint = false;
 tico_invalidate_files = false;
 tico_invalidate_smart = false;
 po_enable_constant_visibility_modifiers = false;
}

let make
  ?(tco_safe_array = default.tco_safe_array)
  ?(tco_safe_vector_array = default.tco_safe_vector_array)
  ?(po_deregister_php_stdlib = default.po_deregister_php_stdlib)
  ?(po_disallow_execution_operator = default.po_disallow_execution_operator)
  ?(po_disable_nontoplevel_declarations = default.po_disable_nontoplevel_declarations)
  ?(po_disable_static_closures = default.po_disable_static_closures)
  ?(po_allow_goto = default.po_allow_goto)
  ?(tco_log_inference_constraints = default.tco_log_inference_constraints)
  ?(tco_experimental_features = default.tco_experimental_features)
  ?(tco_migration_flags = default.tco_migration_flags)
  ?(tco_dynamic_view = default.tco_dynamic_view)
  ?tco_defer_class_declaration_threshold
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
  ?(tco_new_inference_lambda = default.tco_new_inference_lambda)
  ?(tco_timeout = default.tco_timeout)
  ?(tco_disallow_invalid_arraykey = default.tco_disallow_invalid_arraykey)
  ?(tco_disable_instanceof_refinement = default.tco_disable_instanceof_refinement)
  ?(tco_disallow_byref_dynamic_calls = default.tco_disallow_byref_dynamic_calls)
  ?(po_disable_instanceof = default.po_disable_instanceof)
  ?(ignored_fixme_codes = default.ignored_fixme_codes)
  ?ignored_fixme_regex
  ?(log_levels = default.log_levels)
  ?(po_disable_lval_as_an_expression = default.po_disable_lval_as_an_expression)
  ?(tco_typecheck_xhp_cvars = default.tco_typecheck_xhp_cvars)
  ?(tco_ignore_collection_expr_type_arguments = default.tco_ignore_collection_expr_type_arguments)
  ?(tco_shallow_class_decl = default.tco_shallow_class_decl)
  ?(po_rust = default.po_rust)
  ?(tco_like_types = default.tco_like_types)
  ?(tco_pessimize_types = default.tco_pessimize_types)
  ?(tco_coercion_from_dynamic = default.tco_coercion_from_dynamic)
  ?(tco_disable_partially_abstract_typeconsts = default.tco_disable_partially_abstract_typeconsts)
  ?(error_codes_treated_strictly = default.error_codes_treated_strictly)
  ?(tco_check_xhp_attribute = default.tco_check_xhp_attribute)
  ?(tco_disallow_unresolved_type_variables = default.tco_disallow_unresolved_type_variables)
  ?(tco_disallow_invalid_arraykey_constraint = default.tco_disallow_invalid_arraykey_constraint)
  ?(tico_invalidate_files = default.tico_invalidate_files)
  ?(tico_invalidate_smart = default.tico_invalidate_smart)
  ?(po_enable_constant_visibility_modifiers = default.po_enable_constant_visibility_modifiers)
  ()
= {
  tco_safe_array;
  tco_safe_vector_array;
  tco_experimental_features;
  tco_migration_flags;
  tco_dynamic_view;
  tco_defer_class_declaration_threshold;
  tco_disallow_array_as_tuple;
  po_auto_namespace_map;
  po_codegen = false;
  ignored_fixme_codes;
  ignored_fixme_regex;
  po_deregister_php_stdlib;
  po_disallow_execution_operator;
  po_disable_nontoplevel_declarations;
  po_disable_static_closures;
  po_allow_goto;
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
  tco_new_inference_lambda;
  tco_timeout;
  tco_disallow_invalid_arraykey;
  tco_disable_instanceof_refinement;
  tco_disallow_byref_dynamic_calls;
  po_disable_instanceof;
  log_levels;
  po_disable_lval_as_an_expression;
  tco_typecheck_xhp_cvars;
  tco_ignore_collection_expr_type_arguments;
  tco_shallow_class_decl;
  po_rust;
  tco_like_types;
  tco_pessimize_types;
  tco_coercion_from_dynamic;
  tco_disable_partially_abstract_typeconsts;
  error_codes_treated_strictly;
  tco_check_xhp_attribute;
  tco_disallow_unresolved_type_variables;
  tco_disallow_invalid_arraykey_constraint;
  tico_invalidate_files;
  tico_invalidate_smart;
  po_enable_constant_visibility_modifiers;
}
let tco_safe_array t = t.tco_safe_array
let tco_safe_vector_array t = t.tco_safe_vector_array
let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features
let tco_migration_flag_enabled t s =
  SSet.mem s t.tco_migration_flags
let tco_dynamic_view t =
  t.tco_dynamic_view

let tco_defer_class_declaration_threshold t =
  t.tco_defer_class_declaration_threshold

let tco_disallow_array_as_tuple t =
  t.tco_disallow_array_as_tuple
let po_auto_namespace_map t = t.po_auto_namespace_map
let po_deregister_php_stdlib t = t.po_deregister_php_stdlib
let po_disable_nontoplevel_declarations t = t.po_disable_nontoplevel_declarations
let po_disable_static_closures t = t.po_disable_static_closures
let po_allow_goto t = t.po_allow_goto
let tco_log_inference_constraints t = t.tco_log_inference_constraints
let po_codegen t = t.po_codegen
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
let tco_new_inference_lambda t = t.tco_new_inference_lambda
let tco_timeout t = t.tco_timeout
let tco_disallow_invalid_arraykey t = t.tco_disallow_invalid_arraykey
let tco_disallow_invalid_arraykey_constraint t = t.tco_disallow_invalid_arraykey_constraint
let tco_disable_instanceof_refinement t = t.tco_disable_instanceof_refinement
let tco_disallow_byref_dynamic_calls t = t.tco_disallow_byref_dynamic_calls
let po_disable_instanceof t = t.po_disable_instanceof
let ignored_fixme_codes t = t.ignored_fixme_codes
let ignored_fixme_regex t = t.ignored_fixme_regex
let log_levels t = t.log_levels
let po_disable_lval_as_an_expression t = t.po_disable_lval_as_an_expression
let tco_typecheck_xhp_cvars t = t.tco_typecheck_xhp_cvars
let tco_shallow_class_decl t = t.tco_shallow_class_decl
let po_rust t = t.po_rust
let tco_like_types t = t.tco_like_types
let tco_pessimize_types t = t.tco_pessimize_types
let tco_coercion_from_dynamic t = t.tco_coercion_from_dynamic
let tco_disable_partially_abstract_typeconsts t = t.tco_disable_partially_abstract_typeconsts
let error_codes_treated_strictly t = t.error_codes_treated_strictly
let tico_invalidate_files t = t.tico_invalidate_files
let tico_invalidate_smart t = t.tico_invalidate_smart

let tco_ignore_collection_expr_type_arguments t = t.tco_ignore_collection_expr_type_arguments

let tco_check_xhp_attribute t = t.tco_check_xhp_attribute

let tco_disallow_unresolved_type_variables t = t.tco_disallow_unresolved_type_variables

let po_enable_constant_visibility_modifiers t = t.po_enable_constant_visibility_modifiers

let setup_pocket_universes env enabled =
  let exp_features = env.tco_experimental_features in
  let exp_features = if enabled then
      SSet.add tco_experimental_pocket_universes exp_features
    else
      SSet.remove tco_experimental_pocket_universes exp_features
  in { env with tco_experimental_features = exp_features }
