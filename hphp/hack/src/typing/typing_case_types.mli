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
  A runtime data type describes the set of values that can carry a particular
  set of runtime tags. It is possible to map any type to a particular
  runtime data type, and based on this, reason about if two types can possibly
  intersect.
*)
type runtime_data_type

(**
  Maps a type hint to its associated runtime data type. This is intended to
  be used to only as part of checking if a case type is well-formed.

  When safe_for_are_disjoint is true, do not specially handle interfaces' sealed
  attribute or required ancestors.
  Temporary flag to avoid an issue with `are_disjoint` T215053987
*)
val data_type_from_hint :
  safe_for_are_disjoint:bool -> env -> Aast.hint -> env * runtime_data_type

(**
  Checks if two different runtime data types can possibly overlap. Two
  runtime data types are considered to overlap if there exist a value at
  runtime that can belong to both data types. If no such value can exist then
  [None] is returned. Otherwise an error is produced that explains why the
  two runtime data types are considered to be overlapping.
*)
val check_overlapping :
  env ->
  pos:Pos.t ->
  name:string ->
  runtime_data_type ->
  runtime_data_type ->
  Typing_error.Primary.CaseType.t option

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

(* returns true iff the two given types are known to be disjoint based on
   comparison of their datatypes *)
val are_locl_tys_disjoint : env -> locl_ty -> locl_ty -> bool

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

(** Computes runtime data types for types that cannot be decomposed into smaller types *)
module AtomicDataTypes : sig
  type t

  (** Types we consider as atomic *)
  type atomic_ty =
    | Primitive of Aast.tprim
    | Function
    | Nonnull
    | Tuple
    | Shape
    | Label
    | Class of string

  (** When safe_for_are_disjoint is true, do not specially handle interfaces'
      sealed attribute or required ancestors.
      Temporary flag to avoid an issue with `are_disjoint` T215053987 *)
  val of_ty : safe_for_are_disjoint:bool -> env -> atomic_ty -> env * t

  (** When safe_for_are_disjoint is true, do not specially handle interfaces'
      sealed attribute or required ancestors.
      Temporary flag to avoid an issue with `are_disjoint` T215053987 *)
  val of_tag :
    safe_for_are_disjoint:bool -> env -> Typing_defs_core.type_tag -> env * t

  (** Computes the complement for the set of values contained in [t] *)
  val complement : t -> t

  (** Returns true if the given data types are known to have no values in
      common, otherwise returns false *)
  val are_disjoint : env -> t -> t -> bool
end
