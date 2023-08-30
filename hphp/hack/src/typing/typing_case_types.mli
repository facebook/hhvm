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
  runtime data type, and based on this reason about if two types can possibly
  intersect.
*)
type runtime_data_type

(**
  Maps a type hint to its associated runtime data type. This is intended to
  be used to only as part of checking if a case type is well-formed.
*)
val data_type_from_hint : env -> Aast.hint -> runtime_data_type

(**
  Checks if two different runtime data types can possibly overlap. Two
  runtime data types are considered to overlap if there exist a value at
  runtime that can belong to both data types. If no such value can exist then
  [None] is returned. Otherwise an message is produced that explains why the
  two runtime data types are considered to be overlapping.
*)
val check_overlapping :
  env ->
  runtime_data_type ->
  runtime_data_type ->
  (string * Pos_or_decl.t Message.t list Lazy.t) option

(**
  Given the variants of a case type (encoded as a locl_ty) and another locl_ty [intersecting_ty]
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
 *)
val filter_variants_using_datatype :
  env -> Typing_reason.t -> locl_ty list -> locl_ty -> env * locl_ty

(**
   Look up case type via [name]. If the case type exist returns the list of
   variant types. If the case type doesn't exist, returns [None].
*)
val get_variant_tys : env -> string -> locl_ty list -> env * locl_ty list option
