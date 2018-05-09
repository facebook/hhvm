(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  tco_assume_php : bool;
  tco_safe_array : bool;
  tco_safe_vector_array : bool;
  tco_user_attrs : SSet.t option;
  tco_experimental_features : SSet.t;
  tco_migration_flags : SSet.t;
  tco_dynamic_view : bool;
  tco_disallow_unsafe_comparisons : bool;
  po_auto_namespace_map : (string * string) list;
  po_enable_hh_syntax_for_hhvm : bool;
  po_deregister_php_stdlib : bool;
  po_use_full_fidelity : bool;
  tco_disallow_ambiguous_lambda : bool;
  tco_disallow_array_typehint: bool;
  tco_disallow_array_literal: bool;
  po_disallow_elvis_space : bool;
  ignored_fixme_codes : ISet.t;
} [@@deriving show]

let tco_experimental_instanceof = "instanceof"

let tco_experimental_isarray = "is_array"

(* Whether darray and varray are enabled. *)
let tco_experimental_darray_and_varray = "darray_and_varray"

let tco_experimental_goto = "goto"

(* Whether allow accessing tconsts on generics *)
let tco_experimental_tconst_on_generics = "tconst_on_generics"

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
 * Typechecker support for `is` expressions. This feature is incomplete and
 * under development, see T22779957 for details and progress.
 *)
let tco_experimental_is_expression = "is_expression"

(**
 * Typechecker support for `as` expressions. This feature is incomplete and
 * under development, see T26859386 for details and progress.
 *)
let tco_experimental_as_expression = "as_expression"

(**
 * Typechecker support for all features behind the hacksperimental flag.
 *)
let tco_hacksperimental = "hacksperimental"

(**
 * When overriding a class member: if the parent or child's member is typed,
 * require that the user specify a typehint for the other member.
 *)
let tco_decl_override_require_hint = "decl_override_require_hint"

(**
 * Classes may be extended by only a limited whitelist of child classes.
 *)
let tco_sealed_classes = "sealed_classes"

(**
 * Make void the type of null.
 *)
let tco_experimental_void_is_type_of_null = "void_is_type_of_null"


let tco_experimental_all =
 SSet.empty |> List.fold_right SSet.add
   [
     tco_experimental_instanceof;
     tco_experimental_isarray;
     tco_experimental_darray_and_varray;
     tco_experimental_goto;
     tco_experimental_tconst_on_generics;
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
     tco_experimental_is_expression;
     tco_experimental_as_expression;
     tco_hacksperimental;
     tco_decl_override_require_hint;
     tco_sealed_classes;
     tco_experimental_void_is_type_of_null;
   ]

let tco_migration_flags_all =
  SSet.empty |> List.fold_right SSet.add
    [
      "array_cast"
    ]

let default = {
 tco_assume_php = false;
 tco_safe_array = false;
 tco_safe_vector_array = false;
 tco_user_attrs = None;
 (** Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
 tco_experimental_features = tco_experimental_all;
 tco_migration_flags = SSet.empty;
 tco_dynamic_view = false;
 tco_disallow_unsafe_comparisons = false;
 po_auto_namespace_map = [];
 po_enable_hh_syntax_for_hhvm = false;
 po_deregister_php_stdlib = false;
 po_use_full_fidelity = false;
 tco_disallow_ambiguous_lambda = false;
 tco_disallow_array_typehint = false;
 tco_disallow_array_literal = false;
 po_disallow_elvis_space = false;
 ignored_fixme_codes = ISet.empty;
}

(* Use this instead of default when you don't have access to a project
* .hhconfig and it's acceptable to omit some errors. IDE integration is one
* example, assuming the IDE also talks to hh_client to get the full list with
* higher latency.
*
* Use 'default' if it's fine to be potentially stricter than the rest of the
* project requires, eg hh_single_typecheck *)
let make_permissive tcopt =
  { tcopt with
    tco_assume_php = true;
    tco_user_attrs = None;
    tco_experimental_features = tcopt.tco_experimental_features;
  }

let make ~tco_assume_php
         ~tco_safe_array
         ~tco_safe_vector_array
         ~po_deregister_php_stdlib
         ~po_use_full_fidelity
         ~tco_user_attrs
         ~tco_experimental_features
         ~tco_migration_flags
         ~tco_dynamic_view
         ~tco_disallow_unsafe_comparisons
         ~po_auto_namespace_map
         ~tco_disallow_ambiguous_lambda
         ~tco_disallow_array_typehint
         ~tco_disallow_array_literal
         ~po_disallow_elvis_space
         ~ignored_fixme_codes = {
                   tco_assume_php;
                   tco_safe_array;
                   tco_safe_vector_array;
                   tco_user_attrs;
                   tco_experimental_features;
                   tco_migration_flags;
                   tco_dynamic_view;
                   tco_disallow_unsafe_comparisons;
                   po_auto_namespace_map;
                   po_enable_hh_syntax_for_hhvm = false;
                   ignored_fixme_codes;
                   po_deregister_php_stdlib;
                   po_use_full_fidelity;
                   tco_disallow_ambiguous_lambda;
                   tco_disallow_array_typehint;
                   tco_disallow_array_literal;
                   po_disallow_elvis_space;
        }
let tco_assume_php t = t.tco_assume_php
let tco_safe_array t = t.tco_safe_array
let tco_safe_vector_array t = t.tco_safe_vector_array
let tco_user_attrs t = t.tco_user_attrs
let tco_allowed_attribute t name = match t.tco_user_attrs with
 | None -> true
 | Some attr_names -> SSet.mem name attr_names
let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features
let tco_migration_flag_enabled t s =
  SSet.mem s t.tco_migration_flags
let tco_dynamic_view t =
  t.tco_dynamic_view
let tco_disallow_unsafe_comparisons t =
  t.tco_disallow_unsafe_comparisons
let po_auto_namespace_map t = t.po_auto_namespace_map
let po_deregister_php_stdlib t = t.po_deregister_php_stdlib
let po_use_full_fidelity t = t.po_use_full_fidelity
let po_enable_hh_syntax_for_hhvm t = t.po_enable_hh_syntax_for_hhvm
let tco_disallow_ambiguous_lambda t = t.tco_disallow_ambiguous_lambda
let tco_disallow_array_typehint t = t.tco_disallow_array_typehint
let tco_disallow_array_literal t = t.tco_disallow_array_literal
let po_disallow_elvis_space t = t.po_disallow_elvis_space
let ignored_fixme_codes t = t.ignored_fixme_codes
