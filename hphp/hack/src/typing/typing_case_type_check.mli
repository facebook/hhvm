(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
