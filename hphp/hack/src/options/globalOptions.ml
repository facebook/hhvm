(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  tco_assume_php : bool;
  tco_safe_array : bool;
  tco_safe_vector_array : bool;
  tco_user_attrs : SSet.t option;
  tco_experimental_features : SSet.t;
  tco_migration_flags : SSet.t;
  po_auto_namespace_map : (string * string) list;
  po_deregister_php_stdlib : bool;
  ignored_fixme_codes : ISet.t;
}

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

(* Whether we use contextual type information to type lambdas etc *)
let tco_experimental_contextual_inference =
  "contextual_inference"

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

(**
 * Require call-time pass-by-ref annotations to match function definitions.
 *)
let tco_experimental_safe_pass_by_ref = "safe_pass_by_ref"

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
 * Support for in-out function parameters. This feature is incomplete and
 * under development, see T21979562 for details and progress.
 *)
let tco_experimental_inout_params = "inout_params"

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
     tco_experimental_contextual_inference;
     tco_experimental_generics_arity;
     tco_experimental_forbid_nullable_cast;
     tco_experimental_safe_pass_by_ref;
     tco_experimental_coroutines;
     tco_experimental_disallow_static_memoized;
     tco_experimental_inout_params;
     tco_experimental_disable_optional_and_unknown_shape_fields;
   ]

let tco_migration_flags_all =
  SSet.empty |> List.fold_right SSet.add
    [
      "array_cast"
    ]

let default = {
 tco_assume_php = true;
 tco_safe_array = false;
 tco_safe_vector_array = false;
 tco_user_attrs = None;
 (** Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
 tco_experimental_features = tco_experimental_all;
 tco_migration_flags = SSet.empty;
 po_auto_namespace_map = [];
 po_deregister_php_stdlib = false;
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
  }

let make ~tco_assume_php
         ~tco_safe_array
         ~tco_safe_vector_array
         ~po_deregister_php_stdlib
         ~tco_user_attrs
         ~tco_experimental_features
         ~tco_migration_flags
         ~po_auto_namespace_map
         ~ignored_fixme_codes = {
                   tco_assume_php;
                   tco_safe_array;
                   tco_safe_vector_array;
                   tco_user_attrs;
                   tco_experimental_features;
                   tco_migration_flags;
                   po_auto_namespace_map;
                   ignored_fixme_codes;
                   po_deregister_php_stdlib;
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
let po_auto_namespace_map t = t.po_auto_namespace_map
let po_deregister_php_stdlib t = t.po_deregister_php_stdlib
let ignored_fixme_codes t = t.ignored_fixme_codes
