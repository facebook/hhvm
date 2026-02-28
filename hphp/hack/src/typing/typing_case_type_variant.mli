(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

(**
  [filter_variants_using_datatype env reason variants intersecting_ty]:
  Given the [variants] of a case type and another locl_ty [intersecting_ty],
  produce a new locl_ty containing only the types in the variant that map to an intersecting
  data type. For example:
    Given
     [variants] = int | vec<int> | Vector<int>
     [intersecting_ty] = Container<string>

    This function will return the type `vec<int> | Vector<int>` because both `vec<int>` and
   `Vector<int>` overlap with the tag associated with `Container<string>`.

   Note that this function only considers the data type associated to each type and not
   the type itself. So even though `vec<int>` and `Container<string>` do not intersect at
   the type level, they do intersect when considering only the runtime data types.

  When safe_for_are_disjoint is true, do not specially handle interfaces' sealed
  attribute or required ancestors.
  Temporary flag to avoid an issue with `are_disjoint` T215053987
 *)
val filter_variants_using_datatype :
  safe_for_are_disjoint:bool ->
  env ->
  Typing_reason.t ->
  locl_ty list ->
  locl_ty ->
  env * locl_ty

(**
  Return whether or not any of the case type variants have a where clause
*)
val has_where_clauses : typedef_case_type_variant list -> bool

(**
   [get_variant_tys env name ty_args] looks up case type via [name].
  If the case type exists and all variants are unconditional, returns the list
  of variant types localized using [ty_args]
  If the case type doesn't exist or any variant has a where clause, returns
  [None].

  TODO T201569125 - Note: If we could use the ty_args to "evaluate" the where
  constraints, we could return the variant hints whose constraints are met, but
  I'm not sure if that's feasible.
*)
val get_variant_tys : env -> string -> locl_ty list -> env * locl_ty list option
